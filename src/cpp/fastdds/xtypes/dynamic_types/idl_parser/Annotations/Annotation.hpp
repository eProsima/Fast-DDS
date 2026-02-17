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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_ANNOTATION_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_ANNOTATION_HPP

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>

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

#include "AnnotationParameterValues.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

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

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_ANNOTATION_HPP