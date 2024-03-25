// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file DDSFilterFactory.cpp
 */

#include "DDSFilterFactory.hpp"

#include <cstring>
#include <string>
#include <vector>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include "DDSFilterGrammar.hpp"
#include "DDSFilterExpressionParser.hpp"
#include "DDSFilterParseNode.hpp"

#include "DDSFilterExpression.hpp"
#include "DDSFilterCompoundCondition.hpp"
#include "DDSFilterCondition.hpp"
#include "DDSFilterConditionState.hpp"
#include "DDSFilterEmptyExpression.hpp"
#include "DDSFilterExpression.hpp"
#include "DDSFilterField.hpp"
#include "DDSFilterParameter.hpp"
#include "DDSFilterPredicate.hpp"
#include "DDSFilterValue.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

static ReturnCode_t transform_enum(
        std::shared_ptr<DDSFilterValue>& value,
        const std::shared_ptr<xtypes::TypeIdentifier>& type,
        const eprosima::fastcdr::string_255& string_value)
{
    const char* str_value = string_value.c_str();
    std::shared_ptr<xtypes::TypeObject> type_obj = std::make_shared<xtypes::TypeObject>();
    if (RETCODE_OK == DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(*type, *type_obj)
            && type_obj->_d() == xtypes::EK_COMPLETE && type_obj->complete()._d() == xtypes::TK_ENUM)
    {
        for (const auto& enum_value : type_obj->complete().enumerated_type().literal_seq())
        {
            if (enum_value.detail().name() == str_value)
            {
                value->kind = DDSFilterValue::ValueKind::SIGNED_INTEGER;
                value->signed_integer_value = enum_value.common().value();
                return RETCODE_OK;
            }
        }
    }

    return RETCODE_BAD_PARAMETER;
}

static ReturnCode_t transform_enums(
        std::shared_ptr<DDSFilterValue>& left_value,
        const std::shared_ptr<xtypes::TypeIdentifier>& left_type,
        std::shared_ptr<DDSFilterValue>& right_value,
        const std::shared_ptr<xtypes::TypeIdentifier>& right_type)
{
    if ((DDSFilterValue::ValueKind::ENUM == left_value->kind) &&
            (DDSFilterValue::ValueKind::STRING == right_value->kind))
    {
        return transform_enum(right_value, left_type, right_value->string_value);
    }

    if ((DDSFilterValue::ValueKind::ENUM == right_value->kind) &&
            (DDSFilterValue::ValueKind::STRING == left_value->kind))
    {
        return transform_enum(left_value, right_type, left_value->string_value);
    }

    return RETCODE_OK;
}

static bool check_value_compatibility(
        DDSFilterValue::ValueKind left,
        DDSFilterValue::ValueKind right,
        bool ignore_enum)
{
    if (!ignore_enum && DDSFilterValue::ValueKind::ENUM == right)
    {
        return DDSFilterValue::ValueKind::ENUM == left ||
               DDSFilterValue::ValueKind::SIGNED_INTEGER == left ||
               DDSFilterValue::ValueKind::UNSIGNED_INTEGER == left ||
               DDSFilterValue::ValueKind::STRING == left;
    }

    switch (left)
    {
        case DDSFilterValue::ValueKind::BOOLEAN:
            return DDSFilterValue::ValueKind::BOOLEAN == right ||
                   DDSFilterValue::ValueKind::SIGNED_INTEGER == right ||
                   DDSFilterValue::ValueKind::UNSIGNED_INTEGER == right;

        case DDSFilterValue::ValueKind::SIGNED_INTEGER:
        case DDSFilterValue::ValueKind::UNSIGNED_INTEGER:
            return DDSFilterValue::ValueKind::SIGNED_INTEGER == right ||
                   DDSFilterValue::ValueKind::UNSIGNED_INTEGER == right ||
                   DDSFilterValue::ValueKind::BOOLEAN == right ||
                   DDSFilterValue::ValueKind::FLOAT_CONST == right ||
                   DDSFilterValue::ValueKind::FLOAT_FIELD == right ||
                   DDSFilterValue::ValueKind::DOUBLE_FIELD == right ||
                   DDSFilterValue::ValueKind::LONG_DOUBLE_FIELD == right;

        case DDSFilterValue::ValueKind::CHAR:
        case DDSFilterValue::ValueKind::STRING:
            return DDSFilterValue::ValueKind::CHAR == right ||
                   DDSFilterValue::ValueKind::STRING == right;

        case DDSFilterValue::ValueKind::FLOAT_CONST:
        case DDSFilterValue::ValueKind::FLOAT_FIELD:
        case DDSFilterValue::ValueKind::DOUBLE_FIELD:
        case DDSFilterValue::ValueKind::LONG_DOUBLE_FIELD:
            return DDSFilterValue::ValueKind::FLOAT_CONST == right ||
                   DDSFilterValue::ValueKind::FLOAT_FIELD == right ||
                   DDSFilterValue::ValueKind::DOUBLE_FIELD == right ||
                   DDSFilterValue::ValueKind::LONG_DOUBLE_FIELD == right ||
                   DDSFilterValue::ValueKind::SIGNED_INTEGER == right ||
                   DDSFilterValue::ValueKind::UNSIGNED_INTEGER == right;

        case DDSFilterValue::ValueKind::ENUM:
            if (!ignore_enum)
            {
                return DDSFilterValue::ValueKind::ENUM == right ||
                       DDSFilterValue::ValueKind::SIGNED_INTEGER == right ||
                       DDSFilterValue::ValueKind::UNSIGNED_INTEGER == right ||
                       DDSFilterValue::ValueKind::STRING == right;
            }
    }

    return false;
}

static DDSFilterPredicate::OperationKind get_predicate_op(
        const parser::ParseNode& node)
{
    DDSFilterPredicate::OperationKind ret_val = DDSFilterPredicate::OperationKind::EQUAL;
    if (node.is<eq_op>())
    {
        ret_val = DDSFilterPredicate::OperationKind::EQUAL;
    }
    else if (node.is<ne_op>())
    {
        ret_val = DDSFilterPredicate::OperationKind::NOT_EQUAL;
    }
    else if (node.is<lt_op>())
    {
        ret_val = DDSFilterPredicate::OperationKind::LESS_THAN;
    }
    else if (node.is<le_op>())
    {
        ret_val = DDSFilterPredicate::OperationKind::LESS_EQUAL;
    }
    else if (node.is<gt_op>())
    {
        ret_val = DDSFilterPredicate::OperationKind::GREATER_THAN;
    }
    else if (node.is<ge_op>())
    {
        ret_val = DDSFilterPredicate::OperationKind::GREATER_EQUAL;
    }
    else if (node.is<like_op>())
    {
        ret_val = DDSFilterPredicate::OperationKind::LIKE;
    }
    else if (node.is<match_op>())
    {
        ret_val = DDSFilterPredicate::OperationKind::MATCH;
    }
    else
    {
        assert(false);
    }

    return ret_val;
}

struct ExpressionParsingState
{
    const std::shared_ptr<xtypes::TypeObject> type_object;
    const IContentFilterFactory::ParameterSeq& filter_parameters;
    DDSFilterExpression* filter;
};

template<>
ReturnCode_t DDSFilterFactory::convert_tree<DDSFilterCondition>(
        ExpressionParsingState& state,
        std::unique_ptr<DDSFilterCondition>& condition,
        const parser::ParseNode& node);


template<>
ReturnCode_t DDSFilterFactory::convert_tree<DDSFilterValue>(
        ExpressionParsingState& state,
        std::shared_ptr<DDSFilterValue>& value,
        const parser::ParseNode& node)
{
    if (node.value)
    {
        value = std::make_shared<DDSFilterValue>();
        value->copy_from(*node.value.get(), true);
    }
    else if (nullptr != node.type_id)
    {
        std::string field_name = node.content();
        auto it = state.filter->fields.find(field_name);
        if (it == state.filter->fields.end())
        {
            value = state.filter->fields[field_name] =
                    std::make_shared<DDSFilterField>(node.type_id, node.field_access_path, node.field_kind);
        }
        else
        {
            value = it->second;
        }
    }
    else
    {
        // Check parameter index
        if (node.parameter_index >= state.filter_parameters.length())
        {
            return RETCODE_BAD_PARAMETER;
        }

        if (state.filter->parameters[node.parameter_index])
        {
            value = state.filter->parameters[node.parameter_index];
        }
        else
        {
            auto param_value = std::make_shared<DDSFilterParameter>();
            if (!param_value->set_value(state.filter_parameters[node.parameter_index]))
            {
                return RETCODE_BAD_PARAMETER;
            }
            value = state.filter->parameters[node.parameter_index] = param_value;
        }
    }

    return RETCODE_OK;
}

template<>
ReturnCode_t DDSFilterFactory::convert_tree<DDSFilterPredicate>(
        ExpressionParsingState& state,
        std::unique_ptr<DDSFilterCondition>& condition,
        const parser::ParseNode& node)
{
    std::shared_ptr<DDSFilterValue> left;
    std::shared_ptr<DDSFilterValue> right;
    ReturnCode_t ret = convert_tree<DDSFilterValue>(state, left, node.left());
    if (RETCODE_OK == ret)
    {
        ret = convert_tree<DDSFilterValue>(state, right, node.right());
        if (RETCODE_OK == ret)
        {
            bool ignore_enum = false;
            if (node.is<like_op>() || node.is<match_op>())
            {
                // At least one fieldname should be a string
                if ( !((node.left().is<fieldname>() && (DDSFilterValue::ValueKind::STRING == left->kind)) ||
                        (node.right().is<fieldname>() && (DDSFilterValue::ValueKind::STRING == right->kind))))
                {
                    return RETCODE_BAD_PARAMETER;
                }

                ignore_enum = true;
            }

            if ((DDSFilterValue::ValueKind::ENUM == left->kind) && (DDSFilterValue::ValueKind::ENUM == right->kind))
            {
                if (*node.left().type_id != *node.right().type_id)
                {
                    return RETCODE_BAD_PARAMETER;
                }
            }
            else if (!check_value_compatibility(left->kind, right->kind, ignore_enum))
            {
                return RETCODE_BAD_PARAMETER;
            }

            ret = transform_enums(left, node.left().type_id, right, node.right().type_id);
            if (RETCODE_OK == ret)
            {
                condition.reset(new DDSFilterPredicate(get_predicate_op(node), left, right));
            }
        }
    }

    return ret;
}

template<>
ReturnCode_t DDSFilterFactory::convert_tree<between_op>(
        ExpressionParsingState& state,
        std::unique_ptr<DDSFilterCondition>& condition,
        const parser::ParseNode& node)
{
    /* The nodes here will be in the following situation:
     *
     *          between_op
     *          /         \
     * fieldname           and_op
     *                    /      \
     *                 op1        op2
     */

    std::shared_ptr<DDSFilterValue> field;
    ReturnCode_t ret = convert_tree<DDSFilterValue>(state, field, node.left());
    if (RETCODE_OK == ret)
    {
        const parser::ParseNode& and_node = node.right();
        assert(and_node.is<and_op>());

        std::shared_ptr<DDSFilterValue> op1;
        std::shared_ptr<DDSFilterValue> op2;

        ret = convert_tree<DDSFilterValue>(state, op1, and_node.left());
        if (RETCODE_OK == ret)
        {
            ret = convert_tree<DDSFilterValue>(state, op2, and_node.right());
        }

        if (RETCODE_OK == ret)
        {
            if (!check_value_compatibility(field->kind, op1->kind, false) ||
                    !check_value_compatibility(field->kind, op2->kind, false) ||
                    !check_value_compatibility(op1->kind, op2->kind, false))
            {
                return RETCODE_BAD_PARAMETER;
            }

            ret = transform_enums(field, node.left().type_id, op1, and_node.left().type_id);
            if (RETCODE_OK == ret)
            {
                ret = transform_enums(field, node.left().type_id, op2, and_node.right().type_id);
            }
        }

        if (RETCODE_OK == ret)
        {
            DDSFilterPredicate::OperationKind binary_op = node.is<between_op>() ?
                    DDSFilterPredicate::OperationKind::LESS_EQUAL :
                    DDSFilterPredicate::OperationKind::GREATER_THAN;
            DDSFilterCompoundCondition::OperationKind logical_op = node.is<between_op>() ?
                    DDSFilterCompoundCondition::OperationKind::AND :
                    DDSFilterCompoundCondition::OperationKind::OR;

            std::unique_ptr<DDSFilterCondition> left_cond(new DDSFilterPredicate(binary_op, op1, field));
            std::unique_ptr<DDSFilterCondition> right_cond(new DDSFilterPredicate(binary_op, field, op2));
            auto cond = new DDSFilterCompoundCondition(logical_op, std::move(left_cond), std::move(right_cond));
            condition.reset(cond);
        }
    }

    return ret;
}

template<>
ReturnCode_t DDSFilterFactory::convert_tree<DDSFilterCompoundCondition>(
        ExpressionParsingState& state,
        std::unique_ptr<DDSFilterCondition>& condition,
        const parser::ParseNode& node)
{
    ReturnCode_t ret = RETCODE_UNSUPPORTED;
    DDSFilterCompoundCondition::OperationKind op = DDSFilterCompoundCondition::OperationKind::NOT;
    std::unique_ptr<DDSFilterCondition> left;
    std::unique_ptr<DDSFilterCondition> right;

    if (node.is<not_op>())
    {
        op = DDSFilterCompoundCondition::OperationKind::NOT;
        ret = convert_tree<DDSFilterCondition>(state, left, node.left());
    }
    else if (node.is<and_op>())
    {
        op = DDSFilterCompoundCondition::OperationKind::AND;
        ret = convert_tree<DDSFilterCondition>(state, left, node.left());
        if (RETCODE_OK == ret)
        {
            ret = convert_tree<DDSFilterCondition>(state, right, node.right());
        }
    }
    else if (node.is<or_op>())
    {
        op = DDSFilterCompoundCondition::OperationKind::OR;
        ret = convert_tree<DDSFilterCondition>(state, left, node.left());
        if (RETCODE_OK == ret)
        {
            ret = convert_tree<DDSFilterCondition>(state, right, node.right());
        }
    }
    else
    {
        assert(false);
    }

    if (RETCODE_OK == ret)
    {
        condition.reset(new DDSFilterCompoundCondition(op, std::move(left), std::move(right)));
    }

    return ret;
}

template<>
ReturnCode_t DDSFilterFactory::convert_tree<DDSFilterCondition>(
        ExpressionParsingState& state,
        std::unique_ptr<DDSFilterCondition>& condition,
        const parser::ParseNode& node)
{
    if (node.is<and_op>() || node.is<or_op>() || node.is<not_op>())
    {
        return convert_tree<DDSFilterCompoundCondition>(state, condition, node);
    }
    else if (node.is<between_op>() || node.is<not_between_op>())
    {
        return convert_tree<between_op>(state, condition, node);
    }

    return convert_tree<DDSFilterPredicate>(state, condition, node);
}

DDSFilterFactory::~DDSFilterFactory()
{
    auto& pool = expression_pool_.collection();
    for (DDSFilterExpression* item : pool)
    {
        delete item;
    }
    pool.clear();
}

ReturnCode_t DDSFilterFactory::create_content_filter(
        const char* filter_class_name,
        const char* type_name,
        const TopicDataType* data_type,
        const char* filter_expression,
        const IContentFilterFactory::ParameterSeq& filter_parameters,
        IContentFilter*& filter_instance)
{
    static_cast<void>(data_type);

    ReturnCode_t ret = RETCODE_UNSUPPORTED;

    if (nullptr == filter_expression)
    {
        if (nullptr == filter_instance)
        {
            ret = RETCODE_BAD_PARAMETER;
        }
        else
        {
            ret = RETCODE_OK;
            if (&empty_expression_ != filter_instance)
            {
                auto expr = static_cast<DDSFilterExpression*>(filter_instance);
                auto n_params = static_cast<LoanableCollection::size_type>(expr->parameters.size());
                if (filter_parameters.length() < n_params)
                {
                    ret = RETCODE_BAD_PARAMETER;
                }
                else
                {
                    std::vector<DDSFilterValue> old_values(n_params);
                    LoanableCollection::size_type n = n_params;
                    while ((n > 0) && (RETCODE_OK == ret))
                    {
                        --n;
                        if (expr->parameters[n])
                        {
                            old_values[n].copy_from(*(expr->parameters[n]), true);
                            if (!expr->parameters[n]->set_value(filter_parameters[n]))
                            {
                                ret = RETCODE_BAD_PARAMETER;
                            }
                        }
                    }

                    if (RETCODE_OK != ret)
                    {
                        while (n < n_params)
                        {
                            expr->parameters[n]->copy_from(old_values[n], true);
                            ++n;
                        }
                    }
                }
            }
        }
    }
    else if (std::strlen(filter_expression) == 0)
    {
        delete_content_filter(filter_class_name, filter_instance);
        filter_instance = &empty_expression_;
        ret = RETCODE_OK;
    }
    else
    {
        std::shared_ptr<xtypes::TypeObjectPair> type_objects = std::make_shared<xtypes::TypeObjectPair>();
        ret = DomainParticipantFactory::get_instance()->type_object_registry().get_type_objects(
            type_name, *type_objects);
        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DDSSQLFILTER, "No TypeObject found for type " << type_name);
        }
        else
        {
            auto node =
                    parser::parse_filter_expression(filter_expression,
                            std::make_shared<xtypes::TypeObject>(type_objects->complete_type_object));
            if (node)
            {
                DynamicType::_ref_type dyn_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
                    type_objects->complete_type_object)->build();
                if (dyn_type)
                {
                    DDSFilterExpression* expr = get_expression();
                    expr->set_type(dyn_type);
                    size_t n_params = filter_parameters.length();
                    expr->parameters.reserve(n_params);
                    while (expr->parameters.size() < n_params)
                    {
                        expr->parameters.emplace_back();
                    }
                    ExpressionParsingState state{ std::make_shared<xtypes::TypeObject>(
                                                      type_objects->complete_type_object),
                                                  filter_parameters, expr };
                    ret = convert_tree<DDSFilterCondition>(state, expr->root, *(node->children[0]));
                    if (RETCODE_OK == ret)
                    {
                        delete_content_filter(filter_class_name, filter_instance);
                        filter_instance = expr;
                    }
                    else
                    {
                        delete_content_filter(filter_class_name, expr);
                    }
                }
                else
                {
                    ret = RETCODE_BAD_PARAMETER;
                }
            }
            else
            {
                ret = RETCODE_BAD_PARAMETER;
            }
        }
    }

    return ret;
}

ReturnCode_t DDSFilterFactory::delete_content_filter(
        const char* filter_class_name,
        IContentFilter* filter_instance)
{
    static_cast<void>(filter_class_name);

    if (nullptr == filter_instance)
    {
        return RETCODE_BAD_PARAMETER;
    }

    if (&empty_expression_ != filter_instance)
    {
        auto expr = static_cast<DDSFilterExpression*>(filter_instance);
        expr->clear();
        expression_pool_.put(expr);
    }
    return RETCODE_OK;
}

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
