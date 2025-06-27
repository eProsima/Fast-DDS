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

#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include <fastdds/dds/core/ReturnCode.hpp>
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

#include "BuiltinTags.hpp"

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
    std::vector<std::string> positional_parameters;
    std::map<std::string, std::string> keyword_parameters;
};

/**
 * @brief Class representing a generic (user-defined or built-in) annotation building block,
 * as described in OMG IDL 4.2 specification, section 7.4.15.
 */
class Annotation
{

    using TypeDescriptorAnnotationHandler = std::function<bool(TypeDescriptor::ref_type& /* descriptor */, const std::map<std::string, std::string>&) /* annotation's member values */>;
    using MemberDescriptorAnnotationHandler = std::function<bool(MemberDescriptor::ref_type& /* descriptor */, const std::map<std::string, std::string>& /* annotation's member values */)>;

public:

    Annotation(
            const std::string& name)
        : Annotation(
            name,
            [](TypeDescriptor::ref_type&, const std::map<std::string, std::string>&){return true;},
            [](MemberDescriptor::ref_type&, const std::map<std::string, std::string>&){return true;})
    {
    }

    Annotation(
            const std::string& name,
            TypeDescriptorAnnotationHandler type_descriptor_handler,
            MemberDescriptorAnnotationHandler member_descriptor_handler)
        : type_descriptor_handler_(std::move(type_descriptor_handler))
        , member_descriptor_handler_(std::move(member_descriptor_handler))
    {
        TypeDescriptor::_ref_type annotation_type {traits<TypeDescriptor>::make_shared()};
        annotation_type->kind(TK_ANNOTATION);
        annotation_type->name(name);
        annotation_builder_ = DynamicTypeBuilderFactory::get_instance()->create_type(annotation_type);
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
            DynamicType::ref_type member_type,
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
        DynamicType::ref_type member_type = get_declared_type(type_name);
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
            DynamicType::ref_type type,
            bool replace = false)
    {
        if (name.empty())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Cannot add a type with empty name.");
            return false;
        }

        // Check if there exists an inserted type with the same name but different kind
        DynamicType::ref_type existing_type = get_declared_type(name);
        if (existing_type && (existing_type->get_kind() != type->get_kind()))
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Unable to add type '" << name
                << "': a type with the same name but different kind already exists.");

            return false;
        }

        bool is_inserted = false;
        auto insertion_handler = [&name, &type, &is_inserted, replace](auto& container)
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
            DynamicData::ref_type data,
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
            TypeDescriptor::ref_type& descriptor,
            const AnnotationParameterValues& parameters) const
    {
        std::map<std::string, std::string> resolved_parameters;
        if (!resolve_parameter_values(parameters, resolved_parameters))
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to resolve annotation parameters for TypeDescriptor annotation.");
            return false;
        }

        return type_descriptor_handler_(descriptor, resolved_parameters);
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
            MemberDescriptor::ref_type& descriptor,
            const AnnotationParameterValues& parameters) const
    {
        std::map<std::string, std::string> resolved_parameters;
        if (!resolve_parameter_values(parameters, resolved_parameters))
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to resolve annotation parameters for MemberDescriptor annotation.");
            return false;
        }

        return member_descriptor_handler_(descriptor, resolved_parameters);
    }

    /**
     * @brief Annotate a type with the current annotation's information.
     *
     * @param[inout] type_builder The DynamicTypeBuilder to annotate.
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the type builder was successfully updated, false otherwise.
     */
    bool annotate_type(
            DynamicTypeBuilder::ref_type& type_builder,
            const AnnotationParameterValues& parameters) const
    {
        AnnotationDescriptor::ref_type descriptor = create_annotation_descriptor_from_params(parameters);

        if (!descriptor)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to create AnnotationDescriptor from parameters.");
            return false;
        }

        return (RETCODE_OK == type_builder->apply_annotation(descriptor));
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
            DynamicTypeBuilder::ref_type& type_builder,
            const std::string& member_name,
            const AnnotationParameterValues& parameters) const
    {
        DynamicTypeMember::_ref_type member;
        ReturnCode_t ret = type_builder->get_member_by_name(member, member_name);
        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to get member '" << member_name
                    << "' from DynamicTypeBuilder: " << ret);
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
            DynamicTypeBuilder::ref_type& type_builder,
            MemberId member_id,
            const AnnotationParameterValues& parameters) const
    {
        AnnotationDescriptor::ref_type descriptor = create_annotation_descriptor_from_params(parameters);

        if (!descriptor)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to create AnnotationDescriptor from parameters.");
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
    bool is_builtin() const
    {
        const char* name = annotation_builder_->get_name().c_str();

        return strcmp(name, IDL_BUILTIN_ANN_ID_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_AUTOID_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_OPTIONAL_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_POSITION_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_VALUE_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_EXTENSIBILITY_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_FINAL_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_APPENDABLE_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_MUTABLE_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_KEY_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_MUST_UNDERSTAND_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_DEFAULT_LITERAL_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_DEFAULT_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_RANGE_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_MIN_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_MAX_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_UNIT_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_BIT_BOUND_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_EXTERNAL_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_NESTED_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_VERBATIM_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_SERVICE_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_ONEWAY_TAG) == 0 ||
                strcmp(name, IDL_BUILTIN_ANN_AMI_TAG) == 0;
    };

protected:

    /**
     * @brief Resolve the input parameters values according to the annotation's definition.
     *
     * @param input_parameters The input annotation's parameter values to resolve.
     * @param[out] resolved_parameters The resolved annotation's parameter values. Keys correspond to annotation's members names,
     *                                  and values correspond to the values of the annotation's members that should be considered.
     * @return true if the value of each annotation's member was successfully resolved, false otherwise.
     * @note Positional parameters should be provided before keyword parameters. If some positional parameter corresponds to a member id higher than
     *       the member id related to a keyword parameter, parameter resolution will fail.
     */
    bool resolve_parameter_values(
        const AnnotationParameterValues& input_parameters,
        std::map<std::string, std::string>& resolved_parameters) const
    {
        std::unordered_set<std::string> processed_params;

        // Collect annotation member descriptors in order
        DynamicTypeMembersById members_by_id;
        annotation_builder_->get_all_members(members_by_id);

        // Resolve positional parameters
        for (uint32_t i = 0; i < input_parameters.positional_parameters.size(); ++i)
        {
            if (i >= members_by_id.size())
            {
                EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Too many positional parameters for annotation " << annotation_builder_->get_name() << ".");
                return false;
            }

            const auto& member = members_by_id[i];
            const std::string& name = member->get_name().to_string();
            processed_params.insert(name);
            resolved_parameters[name] = input_parameters.positional_parameters[i];
        }

        // Resolve keyword parameters
        for (const auto& kv : input_parameters.keyword_parameters)
        {
            const std::string& name = kv.first;

            // Check if a member with the given name exists
            auto it = std::find_if(
                members_by_id.begin(), members_by_id.end(),
                [&](const auto& m) { return m->get_name() == name; });

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

        // Fill missing parameters with default values
        for (const auto& member : members_by_id)
        {
            const std::string& name = member.second->get_name();

            // Is the member value specified?
            if (resolved_parameters.count(name) == 0)
            {
                MemberDescriptor::_ref_type member_descriptor;
                member.second->get_descriptor(member_descriptor);
                const std::string& default_value = member_descriptor->default_value();
                if (!default_value.empty())
                {
                    resolved_parameters[name] = default_value;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(IDL_PARSER,
                        "Missing required annotation parameter '" << name
                        << "' in annotation '" << annotation_builder_->get_name() << "'.");
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
    AnnotationDescriptor::ref_type create_annotation_descriptor_from_params(
            const AnnotationParameterValues& parameters) const
    {
        std::map<std::string, std::string> resolved_parameters;
        if (!resolve_parameter_values(parameters, resolved_parameters))
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Failed to resolve annotation parameters for AnnotationDescriptor creation.");
            return nullptr;
        }

        if (!annotation_builder_)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Annotation builder is nullptr.");
            return nullptr;
        }

        AnnotationDescriptor::ref_type descriptor {traits<AnnotationDescriptor>::make_shared()};
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
            const DynamicType::ref_type& type) const
    {
        if (!type)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Precondition failed: Type cannot be nullptr.");
            return false;
        }

        switch (type->get_kind())
        {
            case TypeKind::TK_BOOLEAN:
            case TypeKind::TK_CHAR8:
            case TypeKind::TK_CHAR16:
            case TypeKind::TK_BYTE:
            case TypeKind::TK_UINT8:
            case TypeKind::TK_INT8:
            case TypeKind::TK_INT16:
            case TypeKind::TK_UINT16:
            case TypeKind::TK_INT32:
            case TypeKind::TK_UINT32:
            case TypeKind::TK_INT64:
            case TypeKind::TK_UINT64:
            case TypeKind::TK_FLOAT32:
            case TypeKind::TK_FLOAT64:
            case TypeKind::TK_FLOAT128:
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
            const DynamicType::ref_type& type) const
    {
        if (!type)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Precondition failed: Type cannot be nullptr.");
            return false;
        }

        return type->get_kind() == TypeKind::TK_STRING8 ||
               type->get_kind() == TypeKind::TK_STRING16;
    }

    /**
     * @brief Check if the given type is a enumeration type.
     *
     * @param type The type to check.
     * @return true if the type is a enumeration type, false otherwise.
     */
    bool is_enum(
            const DynamicType::ref_type& type) const
    {
        if (!type)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Precondition failed: Type cannot be nullptr.");
            return false;
        }

        return type->get_kind() == TypeKind::TK_ENUM;
    }

    /**
     * @brief Check if the given type is an alias type.
     *
     * @param type The type to check.
     * @return true if the type is an alias type, false otherwise.
     */
    bool is_alias(
            const DynamicType::ref_type& type) const
    {
        if (!type)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER, "Precondition failed: Type cannot be nullptr.");
            return false;
        }

        return type->get_kind() == TypeKind::TK_ALIAS;
    }

    /**
     * @brief Get a declared type by its name.
     *
     * @param type_name The name of the type to retrieve.
     * @return A reference to the declared type with the given name if found, nullptr otherwise.
     */
    DynamicType::ref_type get_declared_type(
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

    DynamicTypeBuilder::ref_type annotation_builder_;
    std::map<std::string, DynamicType::ref_type> enums_;
    std::map<std::string, DynamicType::ref_type> aliases_;
    std::map<std::string, DynamicData::ref_type> constants_;

    //! Callable representing how TypeDescriptor instances should be annotated.
    TypeDescriptorAnnotationHandler type_descriptor_handler_;

    //! Callable representing how MemberDescriptor instances should be annotated.
    MemberDescriptorAnnotationHandler member_descriptor_handler_;
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
    static AnnotationList from_builtin();

    /**
     * @brief Add a new annotation to the list.
     *
     * @param annotation The annotation to add.
     */
    void add_annotation(const Annotation& annotation);

    /**
     * @brief Delete an annotation from the list.
     *
     * @param annotation_name The name of the annotation to delete.
     */
    void delete_annotation(
            const std::string& annotation_name);

    /**
     * @brief Check if an annotation with the given name exists in the list.
     *
     * @param annotation_name The name of the annotation to check.
     */
    bool has_annotation(
            const std::string& annotation_name) const;

    /**
     * @brief Get an annotation by its name.
     *
     * @param annotation_name The name of the annotation to retrieve.
     * @return A reference to the annotation with the given name.
     */
    const Annotation& get_annotation(
            const std::string& annotation_name) const;

    /**
     * @brief Apply a callable type to each annotation in the list.
     */
    template<typename Func>
    void for_each(Func&& func) const
    {
        for (const auto& annotation : annotations_)
        {
            func(annotation);
        }
    }

    template<typename Func>
    void for_each(Func&& func)
    {
        for (auto& annotation : annotations_)
        {
            func(annotation);
        }
    }

private:

    std::vector<Annotation> annotations_;
};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLANNOTATIONS_HPP
