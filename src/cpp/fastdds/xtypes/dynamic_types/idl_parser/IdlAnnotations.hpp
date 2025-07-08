// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLANNOTATIONS_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLANNOTATIONS_HPP

#include <algorithm>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/Types.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

#include "../TypeValueConverter.hpp"
#include "IdlParserTags.hpp"
#include "IdlParserUtils.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

/**
 * @brief Struct representing the input parameter values provided when annotating a type.
 *        They could be positional parameters (if the parameter's name is not specified) or
 *        keyword parameters (if the parameter's name is specified).
 */
struct AnnotationParameterValues
{
    // Constant expression representing the shortened parameter value in one-member annotations.
    std::string shortened_parameter;
    // Map containing the provided parameter values, where the key is the parameter's name
    std::map<std::string, std::string> keyword_parameters;

    /**
     * @brief Parse string input of parameter values separated by commas.
     */
    static AnnotationParameterValues from_string(
            const std::string& input)
    {
        AnnotationParameterValues result;

        std::stringstream ss(input);
        std::string param_token;

        while (getline(ss, param_token, ','))
        {
            param_token = utils::trim(param_token);
            if (param_token.empty())
            {
                continue;
            }

            auto it = param_token.find("=");
            if (it != std::string::npos)
            {
                // Keyword parameter
                assert(result.shortened_parameter.empty());
                std::string key = utils::trim(param_token.substr(0, it));
                std::string value = utils::trim(param_token.substr(it + 1));
                result.keyword_parameters[key] = value;
            }
            else
            {
                assert(result.keyword_parameters.empty());
                result.shortened_parameter = param_token;
            }
        }

        return result;
    }

};

/**
 * @brief Class representing a generic (user-defined or built-in) annotation building block,
 * as described in OMG IDL 4.2 specification, section 7.4.15.
 */
class Annotation
{
public:

    Annotation(
            const std::string& name)
    {
        TypeDescriptor::_ref_type annotation_type {traits<TypeDescriptor>::make_shared()};
        annotation_type->kind(TK_ANNOTATION);
        annotation_type->name(name);
        annotation_builder_ = DynamicTypeBuilderFactory::get_instance()->create_type(annotation_type);
    }

    virtual ~Annotation() = default;

    /**
     * @brief Getter for the annotation's name.
     */
    std::string get_name() const
    {
        return annotation_builder_->get_name().to_string();
    }

    /**
     * @brief Add a new primitive member to the annotation.
     *
     * @param name Name of the member to add.
     * @param member_type DynamicType representing member's type.
     * @param default_value Optional default value for the member, if specified.
     *                      By default, it is an empty string, representing no default value.
     * @return true if the member was successfully added, false otherwise.
     */
    bool add_primitive_or_string_member(
            const std::string& name,
            DynamicType::_ref_type member_type,
            const std::string& default_value = "")
    {
        if (!is_primitive(member_type) && !is_string(member_type))
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Cannot add member '" << name
                                                                 << "' to annotation '" << annotation_builder_->get_name()
                                                                 << "': member type is not primitive.");
            return false;
        }

        MemberDescriptor::_ref_type annotation_param {traits<MemberDescriptor>::make_shared()};
        annotation_param->name(name);

        if (!default_value.empty())
        {
            annotation_param->default_value(default_value);
        }

        annotation_param->type(member_type);

        return (RETCODE_OK == annotation_builder_->add_member(annotation_param));
    }

    /**
     * @brief Add a new member of a previously declared type in the annotation's scope.
     *
     * @param member_name Name of the member to add.
     * @param type_name Name of the previously declared type.
     * @param default_value Optional default value for the member, if specified.
     *                      By default, it is an empty string, representing no default value.
     * @return true if the member was successfully added, false otherwise.
     */
    bool add_declared_type_member(
            const std::string& member_name,
            const std::string& type_name,
            const std::string& default_value = "")
    {
        DynamicType::_ref_type member_type = get_declared_type(type_name);
        if (!member_type)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Cannot add member '" << member_name
                                                                 << "' to annotation '" << annotation_builder_->get_name()
                                                                 << "': type '" << type_name << "' is not declared.");
            return false;
        }

        MemberDescriptor::_ref_type annotation_param {traits<MemberDescriptor>::make_shared()};
        annotation_param->name(member_name);

        if (!default_value.empty())
        {
            annotation_param->default_value(default_value);
        }

        annotation_param->type(member_type);

        return (RETCODE_OK == annotation_builder_->add_member(annotation_param));
    }

    /**
     * @brief Register a new type (enum or alias), declared inside the annotation body.
     *
     * @param name Name of the type to register.
     * @param type The type to add.
     * @param replace Flag indicating whether to replace an existing type with the same name.
     * @return true if the type was successfully registered, false in the following cases:
     *         - The type's name is empty.
     *         - A type with the same name and kind already exists in and `replace` is false.
     *         - A type with the same name but different kind already exists
     *         - The type is invalid.
     */
    bool add_declared_type(
            const std::string& name,
            DynamicType::_ref_type type,
            bool replace = false)
    {
        if (name.empty())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Cannot add a type with empty name.");
            return false;
        }

        // Check if there exists an inserted type with the same name but different kind
        DynamicType::_ref_type existing_type = get_declared_type(name);
        if (existing_type && (existing_type->get_kind() != type->get_kind()))
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Unable to add type '" << name
                                                                  << "': a type with the same name but different kind already exists.");

            return false;
        }

        bool is_inserted = false;
        auto insertion_handler =
                [&name, &type, &is_inserted,
                        replace](std::map<std::string, DynamicType::_ref_type>& container)
                {
                    if (replace)
                    {
                        container.erase(name);
                    }

                    auto result = container.emplace(name, type);
                    if (!result.second)
                    {
                        EPROSIMA_LOG_ERROR(IDL_PARSER, "Ignoring type '" << name
                                                                         << "': type is already declared.");
                    }

                    is_inserted = result.second;
                };

        if (is_enum(type))
        {
            insertion_handler(enums_);
        }
        else if (is_alias(type))
        {
            insertion_handler(aliases_);
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Cannot add type '" << name
                                                               << "': type is not an enum or an alias.");
            return false;
        }

        return is_inserted;
    }

    /**
     * @brief Register a new constant, declared inside the annotation body.
     *
     * @param name Name of the constant to register.
     * @param data The data of the declared constant.
     * @param replace Flag indicating whether to replace an existing constant with the same name.
     * @return true if the constant was successfully registered, false otherwise.
     */
    bool add_constant_type(
            const std::string& name,
            DynamicData::_ref_type data,
            bool replace = false)
    {
        if (name.empty())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Cannot add a type with empty name.");
            return false;
        }

        if (!data)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Cannot add constant '" << name
                                                                   << "': data is nullptr.");
            return false;
        }

        if (replace)
        {
            constants_.erase(name);
        }

        auto result = constants_.emplace(name, data);
        if (!result.second)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Ignoring constant '" << name
                                                                 << "': constant is already declared.");
        }

        return result.second;
    }

    /**
     * @brief Annotate a TypeDescriptor with the current annotation's information.
     * @note This method is used to apply builtin annotations to types,
     *       because TypeDescriptor cannot be modified after instantiating a DynamicTypeBuilder.
     *
     * @param[inout] descriptor The TypeDescriptor to annotate.
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the descriptor was successfully updated (i.e: if the annotation was applied or if no changes were needed),
     *         false otherwise.
     */
    bool annotate_descriptor(
            TypeDescriptor::_ref_type& descriptor,
            const AnnotationParameterValues& parameters) const
    {
        std::map<std::string, std::string> resolved_parameters;
        if (!resolve_parameter_values(parameters, resolved_parameters))
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to resolve annotation parameters for TypeDescriptor annotation.");
            return false;
        }

        if (!descriptor)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Cannot annotate a null TypeDescriptor.");
            return false;
        }

        return apply_to_type(descriptor, resolved_parameters);
    }

    /**
     * @brief Annotate a MemberDescriptor with the current annotation's information.
     * @note This method is used to apply builtin annotations to member types,
     *       because MemberDescriptor cannot be modified after instantiating a DynamicTypeBuilder.
     *
     * @param[inout] descriptor The MemberDescriptor to annotate.
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the descriptor was successfully updated (i.e: if the annotation was applied or if no changes were needed),
     *         false otherwise.
     */
    bool annotate_descriptor(
            MemberDescriptor::_ref_type& descriptor,
            const AnnotationParameterValues& parameters) const
    {
        std::map<std::string, std::string> resolved_parameters;
        if (!resolve_parameter_values(parameters, resolved_parameters))
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to resolve annotation parameters for MemberDescriptor annotation.");
            return false;
        }

        if (!descriptor)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Cannot annotate a null MemberDescriptor.");
            return false;
        }

        return apply_to_member(descriptor, resolved_parameters);
    }

    /**
     * @brief Annotate a type with the current annotation's information.
     *
     * @param[inout] type_builder The DynamicTypeBuilder to annotate.
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the type builder was successfully updated, false otherwise.
     */
    bool annotate_type(
            DynamicTypeBuilder::_ref_type& type_builder,
            const AnnotationParameterValues& parameters) const
    {
        AnnotationDescriptor::_ref_type descriptor = create_annotation_descriptor_from_params(parameters);

        if (!descriptor)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to create AnnotationDescriptor from parameters.");
            return false;
        }

        if (!type_builder)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Type Builder is nullptr.");
            return false;
        }

        ReturnCode_t ret = type_builder->apply_annotation(descriptor);
        return (RETCODE_OK == ret);
    }

    /**
     * @brief Annotate a member type with the current annotation's information.
     *
     * @param[inout] type_builder The DynamicTypeBuilder related to the type which contains the member to annotate.
     * @param[in] member_name The name of the type_builder's member to annotate.
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the member type builder was successfully updated, false otherwise.
     */
    bool annotate_member(
            DynamicTypeBuilder::_ref_type& type_builder,
            const std::string& member_name,
            const AnnotationParameterValues& parameters) const
    {
        DynamicTypeMember::_ref_type member;

        if (!type_builder)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Type Builder is nullptr.");
            return false;
        }

        ReturnCode_t ret = type_builder->get_member_by_name(member, member_name);
        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to get member '" << member_name
                                                                    << "' from DynamicTypeBuilder");
            return false;
        }

        return annotate_member(type_builder, member->get_id(), parameters);
    }

    /**
     * @brief Annotate a member type with the current annotation's information.
     *
     * @param[inout] type_builder The DynamicTypeBuilder related to the type which contains the member to annotate.
     * @param[in] member_id The id of the type_builder's member to annotate.
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the member type builder was successfully updated, false otherwise.
     */
    bool annotate_member(
            DynamicTypeBuilder::_ref_type& type_builder,
            MemberId member_id,
            const AnnotationParameterValues& parameters) const
    {
        AnnotationDescriptor::_ref_type descriptor = create_annotation_descriptor_from_params(parameters);

        if (!descriptor)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to create AnnotationDescriptor from parameters.");
            return false;
        }

        if (!type_builder)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Type Builder is nullptr.");
            return false;
        }

        ReturnCode_t ret = type_builder->apply_annotation_to_member(member_id, descriptor);

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to apply annotation to member with id " << member_id
                                                                                           << ": " << ret);
            return false;
        }

        return true;
    }

    /**
     * @brief Check if the annotation is a builtin annotation
     */
    virtual bool is_builtin() const
    {
        return false;
    }

protected:

    /**
     * @brief Annotate a type descriptor with the provided annotation's resolved parameter values.
     *
     * @param[inout] descriptor The TypeDescriptor to annotate.
     * @param[in] parameters The resolved annotation's parameters values to use
     * @return true if the descriptor was successfully updated (i.e: if the annotation was applied or if no changes were needed), false otherwise.
     */
    virtual bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);
        return true;
    }

    /**
     * @brief Annotate a member descriptor with the provided annotation's resolved parameter values.
     *
     * @param[inout] descriptor The MemberDescriptor to annotate.
     * @param[in] parameters The resolved annotation's parameters values to use
     * @return true if the descriptor was successfully updated (i.e: if the annotation was applied or if no changes were needed), false otherwise.
     */
    virtual bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);
        return true;
    }

    /**
     * @brief Resolve the input parameters values according to the annotation's definition.
     *
     * @param input_parameters The input annotation's parameter values to resolve.
     * @param[out] resolved_parameters The resolved annotation's parameter values. Keys correspond to annotation's members names,
     *                                  and values correspond to the values of the annotation's members that should be considered.
     * @return true if the value of each annotation's member was successfully resolved, false otherwise.
     */
    bool resolve_parameter_values(
            const AnnotationParameterValues& input_parameters,
            std::map<std::string, std::string>& resolved_parameters) const
    {
        std::unordered_set<std::string> processed_params;

        // Collect annotation member descriptors in order
        DynamicTypeMembersById members_by_id;
        annotation_builder_->get_all_members(members_by_id);

        // Resolve shortened parameter for one-member annotations
        if (!input_parameters.shortened_parameter.empty())
        {
            if (members_by_id.size() != 1)
            {
                EPROSIMA_LOG_ERROR(IDL_PARSER,
                        "Shortened parameter '" << input_parameters.shortened_parameter
                                                << "' can only be used in annotations with a single member.");
                return false;
            }

            const auto& member = members_by_id.begin()->second;
            const std::string& name = member->get_name().to_string();
            processed_params.insert(name);
            resolved_parameters[name] = input_parameters.shortened_parameter;
        }
        else
        {
            // Resolve keyword parameters
            for (const auto& kv : input_parameters.keyword_parameters)
            {
                const std::string& name = kv.first;

                // Check if a member with the given name exists
                auto it = std::find_if(
                    members_by_id.begin(), members_by_id.end(),
                    [&](const std::pair<MemberId, traits<DynamicTypeMember>::ref_type>& m)
                    {
                        return m.second->get_name() == name;
                    });

                if (it == members_by_id.end())
                {
                    EPROSIMA_LOG_ERROR(IDL_PARSER,
                            "Unknown annotation member '" << name << "' in annotation '"
                                                          << annotation_builder_->get_name() << "'.");
                    return false;
                }

                // Check that there are no collisions
                if (!processed_params.insert(name).second)
                {
                    EPROSIMA_LOG_ERROR(IDL_PARSER,
                            "Parameter '" << name << "' is specified multiple times.");
                    return false;
                }

                resolved_parameters[name] = kv.second;
            }
        }


        // Fill missing parameters with default values
        for (const auto& member : members_by_id)
        {
            const std::string& name = member.second->get_name().to_string();

            // Is the member value specified?
            if (resolved_parameters.count(name) == 0)
            {
                MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                if (RETCODE_OK != member.second->get_descriptor(member_descriptor))
                {
                    EPROSIMA_LOG_ERROR(IDL_PARSER,
                            "Failed to get descriptor for member '" << name
                                                                    << "' in annotation '" << annotation_builder_->get_name() <<
                            "'.");
                    return false;
                }
                const std::string& default_value = member_descriptor->default_value();
                if (!default_value.empty())
                {
                    resolved_parameters[name] = default_value;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(IDL_PARSER,
                            "Missing required annotation parameter '" << name
                                                                      << "' in annotation '" << annotation_builder_->get_name() <<
                            "'.");
                    return false;
                }
            }
        }

        return true;
    }

    /**
     * @brief Create an AnnotationDescriptor from the given parameters.
     *
     * @param parameters The input annotation's parameter values to use
     * @return A reference to the created AnnotationDescriptor, or nullptr if the parameters are invalid.
     */
    AnnotationDescriptor::_ref_type create_annotation_descriptor_from_params(
            const AnnotationParameterValues& parameters) const
    {
        std::map<std::string, std::string> resolved_parameters;
        if (!resolve_parameter_values(parameters, resolved_parameters))
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to resolve annotation parameters for AnnotationDescriptor creation.");
            return nullptr;
        }

        if (!annotation_builder_)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Annotation builder is nullptr.");
            return nullptr;
        }

        AnnotationDescriptor::_ref_type descriptor {traits<AnnotationDescriptor>::make_shared()};
        descriptor->type(annotation_builder_->build());

        for (const auto& param : resolved_parameters)
        {
            descriptor->set_value(param.first, param.second);
        }

        return descriptor;
    }

    /**
     * @brief Check if the given type is a primitive type.
     *
     * @param type The type to check.
     * @return true if the type is a primitive type, false otherwise.
     */
    bool is_primitive(
            const DynamicType::_ref_type& type) const
    {
        if (!type)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Precondition failed: Type cannot be nullptr.");
            return false;
        }

        switch (type->get_kind())
        {
            case TK_BOOLEAN:
            case TK_CHAR8:
            case TK_CHAR16:
            case TK_BYTE:
            case TK_UINT8:
            case TK_INT8:
            case TK_INT16:
            case TK_UINT16:
            case TK_INT32:
            case TK_UINT32:
            case TK_INT64:
            case TK_UINT64:
            case TK_FLOAT32:
            case TK_FLOAT64:
            case TK_FLOAT128:
                return true;
            default:
                return false;
        }
    }

    /**
     * @brief Check if the given type is a string type.
     *
     * @param type The type to check.
     * @return true if the type is a string type, false otherwise.
     */
    bool is_string(
            const DynamicType::_ref_type& type) const
    {
        if (!type)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Precondition failed: Type cannot be nullptr.");
            return false;
        }

        return type->get_kind() == TK_STRING8 ||
               type->get_kind() == TK_STRING16;
    }

    /**
     * @brief Check if the given type is a enumeration type.
     *
     * @param type The type to check.
     * @return true if the type is a enumeration type, false otherwise.
     */
    bool is_enum(
            const DynamicType::_ref_type& type) const
    {
        if (!type)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Precondition failed: Type cannot be nullptr.");
            return false;
        }

        return type->get_kind() == TK_ENUM;
    }

    /**
     * @brief Check if the given type is an alias type.
     *
     * @param type The type to check.
     * @return true if the type is an alias type, false otherwise.
     */
    bool is_alias(
            const DynamicType::_ref_type& type) const
    {
        if (!type)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Precondition failed: Type cannot be nullptr.");
            return false;
        }

        return type->get_kind() == TK_ALIAS;
    }

    /**
     * @brief Get a declared type by its name.
     *
     * @param type_name The name of the type to retrieve.
     * @return A reference to the declared type with the given name if found, nullptr otherwise.
     */
    DynamicType::_ref_type get_declared_type(
            const std::string& type_name) const
    {
        auto it = enums_.find(type_name);
        if (it != enums_.end())
        {
            return it->second;
        }

        it = aliases_.find(type_name);
        if (it != aliases_.end())
        {
            return it->second;
        }

        return nullptr;
    }

    DynamicTypeBuilder::_ref_type annotation_builder_;
    std::map<std::string, DynamicType::_ref_type> enums_;
    std::map<std::string, DynamicType::_ref_type> aliases_;
    std::map<std::string, DynamicData::_ref_type> constants_;
};

/**
 * @brief Base class for built-in annotations.
 */
class BuiltinAnnotation : public Annotation
{
public:

    BuiltinAnnotation(
            const std::string& name)
        : Annotation(name)
    {
    }

    ~BuiltinAnnotation() = default;

    bool is_builtin() const override
    {
        return true;
    }

    /**
     * @brief Configure the annotation's members and declared types.
     *
     * @return true if the annotation was successfully initialized, false otherwise.
     */
    virtual bool initialize() = 0;
};

/**
 * @brief @id annotation
 */
class IdAnnotation final : public BuiltinAnnotation
{
public:

    IdAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_ID_TAG)
        , initialized_(false)
    {
    }

    ~IdAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            initialized_ = add_primitive_or_string_member(IDL_VALUE_TAG,
                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @id annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(
                IDL_PARSER,
                "Missing required parameter '" << IDL_VALUE_TAG
                                               << "' for annotation '" << IDL_BUILTIN_ANN_ID_TAG << "'.");
            return false;
        }

        try
        {
            TypeForKind<TK_UINT32> value = TypeValueConverter::sto(parameters.at(IDL_VALUE_TAG));
            descriptor->id(value);
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(
                IDL_PARSER,
                "Failed to convert value '" << parameters.at(
                    IDL_VALUE_TAG)
                                            << "' for annotation '" << IDL_BUILTIN_ANN_ID_TAG << "': " <<
                            e.what());
            return false;
        }

        return true;
    }

    bool initialized_;
};

/**
 * @brief @optional annotation
 */
class OptionalAnnotation final : public BuiltinAnnotation
{
public:

    OptionalAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_OPTIONAL_TAG)
        , initialized_(false)
    {
    }

    ~OptionalAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            initialized_ = add_primitive_or_string_member(
                            IDL_VALUE_TAG,
                            DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN),
                            IDL_TRUE_TAG);
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @optional annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_OPTIONAL_TAG <<
                            "'.");
            return false;
        }

        try
        {
            TypeForKind<TK_BOOLEAN> value = TypeValueConverter::sto(utils::to_lower(parameters.at(IDL_VALUE_TAG)));
            descriptor->is_optional(value);
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to convert value '" << parameters.at(
                        IDL_VALUE_TAG)
                                                << "' for annotation '" << IDL_BUILTIN_ANN_OPTIONAL_TAG << "': " <<
                            e.what());
            return false;
        }

        return true;
    }

    bool initialized_;
};

/**
 * @brief @position annotation
 */
class PositionAnnotation final : public BuiltinAnnotation
{
public:

    PositionAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_POSITION_TAG)
        , initialized_(false)
    {
    }

    ~PositionAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            initialized_ = add_primitive_or_string_member(
                                IDL_VALUE_TAG,
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @position annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_POSITION_TAG <<
                            "'.");
            return false;
        }

        try
        {
            TypeForKind<TK_UINT16> value = TypeValueConverter::sto(parameters.at(IDL_VALUE_TAG));
            descriptor->id(value);
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to convert value '" << parameters.at(
                        IDL_VALUE_TAG)
                                                << "' for annotation '" << IDL_BUILTIN_ANN_POSITION_TAG << "': " <<
                            e.what());
            return false;
        }

        return true;
    }

    bool initialized_;
};

/**
 * @brief @extensibility annotation
 */
class ExtensibilityAnnotation final : public BuiltinAnnotation
{
public:

    ExtensibilityAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_EXTENSIBILITY_TAG)
        , initialized_(false)
    {
    }

    ~ExtensibilityAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            bool success {true};

            TypeDescriptor::_ref_type enum_type_descriptor {traits<TypeDescriptor>::make_shared()};
            enum_type_descriptor->kind(TK_ENUM);
            enum_type_descriptor->name(IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_TAG);
            DynamicTypeBuilder::_ref_type enum_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                            enum_type_descriptor)};
            MemberDescriptor::_ref_type enum_member_descriptor {traits<MemberDescriptor>::make_shared()};
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_FINAL_TAG);
            enum_builder->add_member(enum_member_descriptor);
            enum_member_descriptor = traits<MemberDescriptor>::make_shared();
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_APPENDABLE_TAG);
            enum_builder->add_member(enum_member_descriptor);
            enum_member_descriptor = traits<MemberDescriptor>::make_shared();
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_MUTABLE_TAG);
            enum_builder->add_member(enum_member_descriptor);

            success &= add_declared_type(IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_TAG, enum_builder->build());
            success &= add_declared_type_member(IDL_VALUE_TAG, IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_TAG);

            initialized_ = success;
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_EXTENSIBILITY_TAG <<
                            "'.");
            return false;
        }

        const std::string& value = parameters.at(IDL_VALUE_TAG);
        if (value == IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_FINAL_TAG)
        {
            descriptor->extensibility_kind(ExtensibilityKind::FINAL);
        }
        else if (value == IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_APPENDABLE_TAG)
        {
            descriptor->extensibility_kind(ExtensibilityKind::APPENDABLE);
        }
        else if (value == IDL_BUILTIN_ANN_EXTENSIBILITY_KIND_MUTABLE_TAG)
        {
            descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Invalid value '" << value
                                      << "' for annotation '" << IDL_BUILTIN_ANN_EXTENSIBILITY_TAG << "'.");
            return false;
        }

        return true;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @extensibility annotation to a MemberDescriptor.");
        return false;
    }

    bool initialized_;
};

/**
 * @brief @final annotation
 */
class FinalAnnotation final : public BuiltinAnnotation
{
public:

    FinalAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_FINAL_TAG)
    {
    }

    ~FinalAnnotation() = default;

    bool initialize() override
    {
        // Nothing to do
        return true;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(parameters);
        assert(descriptor != nullptr);

        descriptor->extensibility_kind(ExtensibilityKind::FINAL);
        return true;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @final annotation to a MemberDescriptor.");
        return false;
    }
};

/**
 * @brief @appendable annotation
 */
class AppendableAnnotation final : public BuiltinAnnotation
{
public:

    AppendableAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_APPENDABLE_TAG)
    {
    }

    ~AppendableAnnotation() = default;

    bool initialize() override
    {
        // Nothing to do
        return true;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(parameters);
        assert(descriptor != nullptr);

        descriptor->extensibility_kind(ExtensibilityKind::APPENDABLE);
        return true;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @appendable annotation to a MemberDescriptor.");
        return false;
    }
};

/**
 * @brief @mutable annotation
 */
class MutableAnnotation final : public BuiltinAnnotation
{
public:

    MutableAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_MUTABLE_TAG)
    {
    }

    ~MutableAnnotation() = default;

    bool initialize() override
    {
        // Nothing to do
        return true;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(parameters);
        assert(descriptor != nullptr);

        descriptor->extensibility_kind(ExtensibilityKind::MUTABLE);
        return true;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @mutable annotation to a MemberDescriptor.");
        return false;
    }
};

/**
 * @brief @key annotation
 */
class KeyAnnotation final : public BuiltinAnnotation
{
public:

    KeyAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_KEY_TAG)
        , initialized_(false)
    {
    }

    ~KeyAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            initialized_ = add_primitive_or_string_member(
                                IDL_VALUE_TAG,
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN),
                                IDL_TRUE_TAG);
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @key annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_KEY_TAG <<
                            "'.");
            return false;
        }

        try
        {
            TypeForKind<TK_BOOLEAN> value = TypeValueConverter::sto(utils::to_lower(parameters.at(IDL_VALUE_TAG)));
            descriptor->is_key(value);
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to convert value '" << parameters.at(
                        IDL_VALUE_TAG)
                                                << "' for annotation '" << IDL_BUILTIN_ANN_KEY_TAG << "': " <<
                            e.what());
            return false;
        }

        return true;
    }

    bool initialized_;
};

/**
 * @brief @default_literal annotation
 */
class DefaultLiteralAnnotation final : public BuiltinAnnotation
{
public:

    DefaultLiteralAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_DEFAULT_LITERAL_TAG)
    {
    }

    ~DefaultLiteralAnnotation() = default;

    bool initialize() override
    {
        // Nothing to do
        return true;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @default_literal annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(parameters);
        assert(descriptor != nullptr);

        descriptor->is_default_label(true);
        return true;
    }
};

/**
 * @brief @default annotation
 */
class DefaultAnnotation final : public BuiltinAnnotation
{
public:

    DefaultAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_DEFAULT_TAG)
        , initialized_(false)
    {
    }

    ~DefaultAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            // Note: The member is processed as a string, so it can be used to set the default value of any type.
            // In the future, it should be processed as a "any" type member.
            initialized_ = add_primitive_or_string_member(IDL_VALUE_TAG,
                                DynamicTypeBuilderFactory::get_instance()->create_string_type(
                                    static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @default annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_DEFAULT_TAG <<
                            "'.");
            return false;
        }

        descriptor->default_value(parameters.at(IDL_VALUE_TAG));
        return true;
    }

    bool initialized_;
};

/**
 * @brief @bit_bound annotation
 */
class BitBoundAnnotation final : public BuiltinAnnotation
{
public:

    BitBoundAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_BIT_BOUND_TAG)
        , initialized_(false)
    {
    }

    ~BitBoundAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            initialized_ = add_primitive_or_string_member(
                                IDL_VALUE_TAG,
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG <<
                            "'.");
            return false;
        }

        if (TK_BITSET != descriptor->kind() || TK_BITMASK != descriptor->kind())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "TypeDescriptor can only be annotated with '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG
                                                                  << "' for bitset/bitmask types.");
            return false;
        }

        try
        {
            TypeForKind<TK_UINT16> value = TypeValueConverter::sto(parameters.at(IDL_VALUE_TAG));
            descriptor->bound({value});
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to convert value '" << parameters.at(
                        IDL_VALUE_TAG)
                                                << "' for annotation '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG << "': " <<
                            e.what());
            return false;
        }

        return true;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG <<
                            "'.");
            return false;
        }

        if (TK_ENUM != descriptor->type()->get_kind())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "MemberDescriptor can only be annotated with '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG
                                                                    << "' for enumeration types.");
            return false;
        }

        TypeForKind<TK_UINT16> value;

        try
        {
            value = TypeValueConverter::sto(parameters.at(IDL_VALUE_TAG));
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to convert value '" << parameters.at(
                        IDL_VALUE_TAG)
                                                << "' for annotation '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG <<
                            "'.");
            return false;
        }

        DynamicType::_ref_type member_type;

        if (value == 8)
        {
            member_type = DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT8);
        }
        else if (value == 16)
        {
            member_type = DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16);
        }
        else if (value == 32)
        {
            member_type = DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32);
        }
        else if (value == 64)
        {
            member_type = DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT64);
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Invalid bit bound value '" << value << "' for annotation '"
                                                << IDL_BUILTIN_ANN_BIT_BOUND_TAG << "'.");
            return false;
        }

        descriptor->type(member_type);

        return true;
    }

    bool initialized_;
};

/**
 * @brief @external annotation
 */
class ExternalAnnotation final : public BuiltinAnnotation
{
public:

    ExternalAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_EXTERNAL_TAG)
        , initialized_(false)
    {
    }

    ~ExternalAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            initialized_ = add_primitive_or_string_member(
                                IDL_VALUE_TAG,
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN),
                                IDL_TRUE_TAG);
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @external annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_EXTERNAL_TAG <<
                            "'.");
            return false;
        }

        try
        {
            TypeForKind<TK_BOOLEAN> value = TypeValueConverter::sto(utils::to_lower(parameters.at(IDL_VALUE_TAG)));
            descriptor->is_shared(value);
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to convert value '" << parameters.at(
                        IDL_VALUE_TAG)
                                                << "' for annotation '" << IDL_BUILTIN_ANN_EXTERNAL_TAG <<
                            "'.");
            return false;
        }

        return true;
    }

    bool initialized_;
};

/**
 * @brief @nested annotation
 */
class NestedAnnotation final : public BuiltinAnnotation
{
public:

    NestedAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_NESTED_TAG)
        , initialized_(false)
    {
    }

    ~NestedAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            initialized_ = add_primitive_or_string_member(
                                IDL_VALUE_TAG,
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN),
                                IDL_TRUE_TAG);
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_NESTED_TAG <<
                            "'.");
            return false;
        }

        try
        {
            TypeForKind<TK_BOOLEAN> value = TypeValueConverter::sto(utils::to_lower(parameters.at(IDL_VALUE_TAG)));
            descriptor->is_nested(value);
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to convert value '" << parameters.at(
                        IDL_VALUE_TAG)
                                                << "' for annotation '" << IDL_BUILTIN_ANN_NESTED_TAG << "': " <<
                            e.what());
            return false;
        }

        return true;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @nested annotation to a MemberDescriptor.");
        return false;
    }

    bool initialized_;
};

/**
 * @brief @try_construct annotation
 */
class TryConstructAnnotation final : public BuiltinAnnotation
{
public:

    TryConstructAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_TRY_CONSTRUCT_TAG)
        , initialized_(false)
    {
    }

    ~TryConstructAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            bool success {true};

            TypeDescriptor::_ref_type enum_type_descriptor = traits<TypeDescriptor>::make_shared();
            enum_type_descriptor->kind(TK_ENUM);
            enum_type_descriptor->name(IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TAG);
            DynamicTypeBuilder::_ref_type enum_builder = DynamicTypeBuilderFactory::get_instance()->create_type(enum_type_descriptor);
            MemberDescriptor::_ref_type enum_member_descriptor = traits<MemberDescriptor>::make_shared();
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_DISCARD_TAG);
            enum_builder->add_member(enum_member_descriptor);
            enum_member_descriptor = traits<MemberDescriptor>::make_shared();
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_USE_DEFAULT_TAG);
            enum_builder->add_member(enum_member_descriptor);
            enum_member_descriptor = traits<MemberDescriptor>::make_shared();
            enum_member_descriptor->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32));
            enum_member_descriptor->name(IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TRIM_TAG);
            enum_builder->add_member(enum_member_descriptor);

            success &= add_declared_type(IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TAG, enum_builder->build());
            success &= add_declared_type_member(
                            IDL_VALUE_TAG,
                            IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TAG,
                            IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_USE_DEFAULT_TAG);

            initialized_ = success;
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @try_construct annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_TRY_CONSTRUCT_TAG <<
                            "'.");
            return false;
        }

        const std::string& value = parameters.at(IDL_VALUE_TAG);
        if (value == IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_DISCARD_TAG)
        {
            descriptor->try_construct_kind(TryConstructKind::DISCARD);
        }
        else if (value == IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_USE_DEFAULT_TAG)
        {
            descriptor->try_construct_kind(TryConstructKind::USE_DEFAULT);
        }
        else if (value == IDL_BUILTIN_ANN_TRY_CONSTRUCT_FAIL_ACTION_TRIM_TAG)
        {
            descriptor->try_construct_kind(TryConstructKind::TRIM);
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Invalid value '" << value
                                      << "' for annotation '" << IDL_BUILTIN_ANN_TRY_CONSTRUCT_TAG << "'.");
            return false;
        }

        return true;
    }

    bool initialized_;
};

/**
 * @brief @value annotation
 */
class ValueAnnotation final : public BuiltinAnnotation
{
public:

    ValueAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_VALUE_TAG)
        , initialized_(false)
    {
    }

    ~ValueAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            // Note: The member is processed as a string, so it can be used to set the default value of any type.
            // In the future, it should be processed as a "any" type member.
            initialized_ = add_primitive_or_string_member(
                                IDL_VALUE_TAG,
                                DynamicTypeBuilderFactory::get_instance()->create_string_type(
                                    static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        static_cast<void>(descriptor);
        static_cast<void>(parameters);

        EPROSIMA_LOG_ERROR(IDL_PARSER, "Trying to apply @value annotation to a TypeDescriptor.");
        return false;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_VALUE_TAG <<
                            "'.");
            return false;
        }

        descriptor->default_value(parameters.at(IDL_VALUE_TAG));

        // Check that the default value is consistent with the member type
        if (!descriptor->is_consistent())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Default value '" << parameters.at(IDL_VALUE_TAG)
                                      << "' is not consistent with the member type for annotation '"
                                      << IDL_BUILTIN_ANN_DEFAULT_TAG << "'.");
            return false;
        }

        return true;
    }

    bool initialized_;
};

/**
 * @brief Class used to manage a list of annotations
 */
class AnnotationList
{
public:

    AnnotationList() = default;

    /**
     * @brief Create an AnnotationList with built-in annotations.
     *
     * @return An AnnotationList containing all the built-in annotations.
     */
    static AnnotationList from_builtin()
    {
        AnnotationList list;

        // @id
        auto id_annotation = std::make_shared<IdAnnotation>();
        if (!id_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @id annotation.");
            throw std::runtime_error("Failed to initialize @id annotation.");
        }
        list.add_annotation(id_annotation);

        // @optional
        auto optional_annotation = std::make_shared<OptionalAnnotation>();
        if (!optional_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @optional annotation.");
            throw std::runtime_error("Failed to initialize @optional annotation.");
        }
        list.add_annotation(optional_annotation);

        // @position
        auto position_annotation = std::make_shared<PositionAnnotation>();
        if (!position_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @position annotation.");
            throw std::runtime_error("Failed to initialize @position annotation.");
        }
        list.add_annotation(position_annotation);

        // @extensibility
        auto extensibility_annotation = std::make_shared<ExtensibilityAnnotation>();
        if (!extensibility_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @extensibility annotation.");
            throw std::runtime_error("Failed to initialize @extensibility annotation.");
        }
        list.add_annotation(extensibility_annotation);

        // @final
        list.add_annotation(std::make_shared<FinalAnnotation>());

        // @appendable
        list.add_annotation(std::make_shared<AppendableAnnotation>());

        // @mutable
        list.add_annotation(std::make_shared<MutableAnnotation>());

        // @key
        auto key_annotation = std::make_shared<KeyAnnotation>();
        if (!key_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @key annotation.");
            throw std::runtime_error("Failed to initialize @key annotation.");
        }
        list.add_annotation(key_annotation);

        // @default_literal
        list.add_annotation(std::make_shared<DefaultLiteralAnnotation>());

        // @default
        auto default_annotation = std::make_shared<DefaultAnnotation>();
        if (!default_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @default annotation.");
            throw std::runtime_error("Failed to initialize @default annotation.");
        }
        list.add_annotation(default_annotation);

        // @bit_bound
        auto bit_bound_annotation = std::make_shared<BitBoundAnnotation>();
        if (!bit_bound_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @bit_bound annotation.");
            throw std::runtime_error("Failed to initialize @bit_bound annotation.");
        }
        list.add_annotation(bit_bound_annotation);

        // @external
        auto external_annotation = std::make_shared<ExternalAnnotation>();
        if (!external_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @external annotation.");
            throw std::runtime_error("Failed to initialize @external annotation.");
        }
        list.add_annotation(external_annotation);

        // @nested
        auto nested_annotation = std::make_shared<NestedAnnotation>();
        if (!nested_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @nested annotation.");
            throw std::runtime_error("Failed to initialize @nested annotation.");
        }
        list.add_annotation(nested_annotation);

        // @try_construct
        auto try_construct_annotation = std::make_shared<TryConstructAnnotation>();
        if (!try_construct_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @try_construct annotation.");
            throw std::runtime_error("Failed to initialize @try_construct annotation.");
        }
        list.add_annotation(try_construct_annotation);

        // @value
        auto value_annotation = std::make_shared<ValueAnnotation>();
        if (!value_annotation->initialize())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to initialize @value annotation.");
            throw std::runtime_error("Failed to initialize @value annotation.");
        }
        list.add_annotation(value_annotation);

        // TODO: Add other built-in annotations when supported
        // @autoid (Unsupported)
        // @must_understand (Unsupported)
        // @range (Unsupported)
        // @min (Unsupported)
        // @max (Unsupported)
        // @unit (Unsupported)
        // @verbatim (Unsupported)
        // @service (Unsupported)
        // @oneway (Unsupported)
        // @ami (Unsupported)

        return list;
    }

    /**
     * @brief Add a new annotation to the list.
     *
     * @param annotation The annotation to add.
     */
    void add_annotation(
            const std::shared_ptr<Annotation>& annotation)
    {
        assert(annotation != nullptr);
        assert(!annotation->get_name().empty());

        const std::string& name = annotation->get_name();

        if (has_annotation(name))
        {
            EPROSIMA_LOG_WARNING(IDL_PARSER,
                    "Ignoring annotation '" << name
                                            << "': already exists in the list.");
        }
        else
        {
            annotations_[name] = annotation;
            EPROSIMA_LOG_INFO(IDL_PARSER, "Added annotation '" << name << "'");
        }
    }

    /**
     * @brief Delete an annotation from the list.
     *
     * @param annotation_name The name of the annotation to delete.
     */
    void delete_annotation(
            const std::string& annotation_name)
    {
        auto it = annotations_.find(annotation_name);

        if (it != annotations_.end())
        {
            EPROSIMA_LOG_INFO(IDL_PARSER, "Deleting annotation '" << annotation_name << "'");
            annotations_.erase(it);
        }
        else
        {
            EPROSIMA_LOG_WARNING(IDL_PARSER, "Annotation '" << annotation_name
                                                            << "' not found in the list.");
        }
    }

    /**
     * @brief Check if an annotation with the given name exists in the list.
     *
     * @param annotation_name The name of the annotation to check.
     */
    bool has_annotation(
            const std::string& annotation_name) const
    {
        auto it = annotations_.find(annotation_name);
        return it != annotations_.end();
    }

    /**
     * @brief Get an annotation by its name.
     *
     * @param annotation_name The name of the annotation to retrieve.
     * @return A reference to the annotation with the given name, or `nullptr` if not found.
     */
    const Annotation* get_annotation(
            const std::string& annotation_name) const
    {
        auto it = annotations_.find(annotation_name);

        if (it != annotations_.end())
        {
            return it->second.get();
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Annotation '" << annotation_name
                                                          << "' not found in the list.");
            return nullptr;
        }
    }

    /**
     * @brief Apply a callable type to each annotation in the list.
     */
    template<typename Func>
    void for_each(
            Func&& func) const
    {
        for (const auto& annotation : annotations_)
        {
            func(annotation.second);
        }
    }

    template<typename Func>
    void for_each(
            Func&& func)
    {
        for (auto& annotation : annotations_)
        {
            func(annotation.second);
        }
    }

protected:

    std::map<std::string, std::shared_ptr<Annotation>> annotations_;
};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLANNOTATIONS_HPP
