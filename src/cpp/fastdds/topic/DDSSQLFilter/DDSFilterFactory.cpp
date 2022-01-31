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

#include <fastdds/dds/topic/IContentFilter.hpp>
#include <fastdds/dds/topic/IContentFilterFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>

#include <fastrtps/types/TypeObject.h>
#include <fastrtps/types/TypeObjectFactory.h>

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

static constexpr bool check_value_compatibility(
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
                   DDSFilterValue::ValueKind::FLOAT == right;

        case DDSFilterValue::ValueKind::CHAR:
        case DDSFilterValue::ValueKind::STRING:
            return DDSFilterValue::ValueKind::CHAR == right ||
                   DDSFilterValue::ValueKind::STRING == right;

        case DDSFilterValue::ValueKind::FLOAT:
            return DDSFilterValue::ValueKind::FLOAT == right ||
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

struct ExpressionParsingState
{
    const eprosima::fastrtps::types::TypeObject* type_object;
    const IContentFilterFactory::ParameterSeq& filter_parameters;
    DDSFilterExpression* filter;
};

template<>
IContentFilterFactory::ReturnCode_t DDSFilterFactory::convert_tree<DDSFilterValue>(
        ExpressionParsingState& state,
        std::shared_ptr<DDSFilterValue>& value,
        const parser::ParseNode& node)
{
    if (node.value)
    {
        value = std::make_shared<DDSFilterValue>(*node.value.get());
    }
    else if (nullptr != node.type_id)
    {
        std::string field_name = node.content();
        auto it = state.filter->fields.find(field_name);
        if (it == state.filter->fields.end())
        {
            value = state.filter->fields[field_name] =
                    std::make_shared<DDSFilterField>(state.type_object, node.field_access_path, node.field_kind);
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
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
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
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }
            value = state.filter->parameters[node.parameter_index] = param_value;
        }
    }

    return ReturnCode_t::RETCODE_OK;
}

template<>
IContentFilterFactory::ReturnCode_t DDSFilterFactory::convert_tree<DDSFilterPredicate>(
        ExpressionParsingState& state,
        std::unique_ptr<DDSFilterCondition>& condition,
        const parser::ParseNode& node)
{
    static_cast<void>(condition);

    std::shared_ptr<DDSFilterValue> left;
    std::shared_ptr<DDSFilterValue> right;
    ReturnCode_t ret = convert_tree<DDSFilterValue>(state, left, node.left());
    if (ReturnCode_t::RETCODE_OK == ret)
    {
        ret = convert_tree<DDSFilterValue>(state, right, node.right());
        if (ReturnCode_t::RETCODE_OK == ret)
        {
            if (node.is<like_op>())
            {
                // At least one fieldname should be a string
                if ( !( (node.left().is<fieldname>() && (DDSFilterValue::ValueKind::STRING == left->kind)) ||
                    (node.right().is<fieldname>() && (DDSFilterValue::ValueKind::STRING == right->kind)) ) )
                {
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
            }

            if ((DDSFilterValue::ValueKind::ENUM == left->kind) && (DDSFilterValue::ValueKind::ENUM == right->kind))
            {
                if (node.left().type_id != node.right().type_id)
                {
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
            }
            else if (!check_value_compatibility(left->kind, right->kind, node.is<like_op>()))
            {
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }
            // condition = new DDSFilterPredicate(left, right);
            // condition = new DDSFilterPredicate();
        }
    }

    return ret;
}

template<>
IContentFilterFactory::ReturnCode_t DDSFilterFactory::convert_tree<DDSFilterCompoundCondition>(
        ExpressionParsingState& state,
        std::unique_ptr<DDSFilterCondition>& condition,
        const parser::ParseNode& node)
{
    static_cast<void>(state);
    static_cast<void>(condition);
    static_cast<void>(node);

    // TODO (Miguel C): Compound conditions
    return ReturnCode_t::RETCODE_UNSUPPORTED;
}

template<>
IContentFilterFactory::ReturnCode_t DDSFilterFactory::convert_tree<DDSFilterCondition>(
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
        // TODO (Miguel C): BETWEEN ops
        return ReturnCode_t::RETCODE_UNSUPPORTED;
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

IContentFilterFactory::ReturnCode_t DDSFilterFactory::create_content_filter(
        const char* filter_class_name,
        const char* type_name,
        const TopicDataType* data_type,
        const char* filter_expression,
        const IContentFilterFactory::ParameterSeq& filter_parameters,
        IContentFilter*& filter_instance)
{
    using eprosima::fastrtps::types::TypeObjectFactory;

    static_cast<void>(filter_class_name);
    static_cast<void>(type_name);
    static_cast<void>(data_type);
    static_cast<void>(filter_expression);
    static_cast<void>(filter_parameters);
    static_cast<void>(filter_instance);

    ReturnCode_t ret = ReturnCode_t::RETCODE_UNSUPPORTED;

    if ((filter_expression == nullptr) || (std::strlen(filter_expression) == 0))
    {
        filter_instance = &empty_expression_;
        ret = ReturnCode_t::RETCODE_OK;
    }
    else
    {
        auto type_object = TypeObjectFactory::get_instance()->get_type_object(type_name, true);
        if (!type_object)
        {
            logError(DDSSQLFILTER, "No TypeObject found for type " << type_name);
            ret = ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        else
        {
            auto node = parser::parse_filter_expression(filter_expression, type_object);
            if (node)
            {
                DDSFilterExpression* expr = get_expression();
                size_t n_params = filter_parameters.length();
                expr->parameters.reserve(n_params);
                while (expr->parameters.size() < n_params)
                {
                    expr->parameters.emplace_back();
                }
                ExpressionParsingState state{ type_object, filter_parameters, expr };
                ret = convert_tree<DDSFilterCondition>(state, expr->root, *(node->children[0]));
                if (ReturnCode_t::RETCODE_OK == ret)
                {
                    filter_instance = expr;
                }
                else
                {
                    delete_content_filter(filter_class_name, expr);
                }
            }
            else
            {
                ret = ReturnCode_t::RETCODE_BAD_PARAMETER;
            }
        }
    }

    return ret;
}

IContentFilterFactory::ReturnCode_t DDSFilterFactory::delete_content_filter(
        const char* filter_class_name,
        IContentFilter* filter_instance)
{
    static_cast<void>(filter_class_name);
    static_cast<void>(filter_instance);

    if (&empty_expression_ != filter_instance)
    {
        expression_pool_.put(static_cast<DDSFilterExpression*>(filter_instance));
    }
    return ReturnCode_t::RETCODE_OK;
}

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
