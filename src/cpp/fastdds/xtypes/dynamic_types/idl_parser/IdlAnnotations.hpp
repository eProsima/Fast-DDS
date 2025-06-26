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

#include <map>
#include <string>
#include <vector>

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

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
public:

    /**
     * @brief Add a new primitive member to the annotation.
     *
     * @param name Name of the member to add.
     * @param member_builder Builder for the member's type.
     * @param default_value Optional default value for the member, if specified.
     *                      By default, it is set to nullptr, representing no default value.
     * @param replace Flag indicating whether to replace an existing member with the same name.
     * @return true if the member was successfully added, false in the following cases:
     *         - The member's name is empty.
     *         - A member with the same name already exists and `replace` is false.
     *         - The member's type builder is invalid (nullptr or non-primitive).
     *         - The member's default value type and the member's builder type do not match.
     */
    bool add_primitive_member(
            const std::string& name,
            DynamicTypeBuilder::ref_type member_builder,
            DynamicData::ref_type default_value = nullptr,
            bool replace = false);

    /**
     * @brief Add a new enumeration member to the annotation.
     *
     * @param name Name of the member to add.
     * @param member_builder Builder for the member's type.
     * @param default_value Optional default value for the member, if specified.
     *                      By default, it is an empty string, representing no default value.
     * @param replace Flag indicating whether to replace an existing member with the same name.
     * @return true if the member was successfully added, false in the following cases:
     *         - The member's name is empty.
     *         - A member with the same name already exists and `replace` is false.
     *         - The member's type builder is invalid (nullptr, non-enum type or unregistered enum type).
     *         - The member's default value is not a valid enum value.
     */
    bool add_enum_member(
            const std::string& name,
            DynamicTypeBuilder::ref_type member_builder,
            std::string default_value = "",
            bool replace = false);

    /**
     * @brief Register a new enum type, declared inside the annotation body.
     *
     * @param name Name of the enum type to register.
     * @param enum_builder Builder for the enum type.
     * @param replace Flag indicating whether to replace an existing enum type with the same name.
     * @return true if the enum type was successfully registered, false in the following cases:
     *         - The enum type's name is empty.
     *         - An enum type with the same name already exists and `replace` is false.
     *         - The enum type's builder is invalid (nullptr or non-enum type).
     */
    bool add_enum_type(
            const std::string& name,
            DynamicTypeBuilder::ref_type enum_builder,
            bool replace = false);

    /**
     * @brief Register a new constant type, declared inside the annotation body.
     *
     * @param name Name of the constant type to register.
     * @param constant_builder Builder for the constant type.
     * @param replace Flag indicating whether to replace an existing constant type with the same name.
     * @return true if the constant type was successfully registered, false in the following cases:
     *         - The constant type's name is empty.
     *         - A constant type with the same name already exists and `replace` is false.
     *         - The constant type's builder is invalid (nullptr or non-constant type).
     */
    bool add_constant_type(
            const std::string& name,
            DynamicTypeBuilder::ref_type constant_builder,
            bool replace = false);

    /**
     * @brief Register a new aliased type, declared inside the annotation body.
     *
     * @param name Name of the aliased type to register.
     * @param alias_builder Builder for the aliased type.
     * @param replace Flag indicating whether to replace an existing aliased type with the same name.
     * @return true if the aliased type was successfully registered, false in the following cases:
     *         - The aliased type's name is empty.
     *         - An aliased type with the same name already exists and `replace` is false.
     *         - The aliased type's builder is invalid (nullptr or non-aliased type).
     */
    bool add_alias_type(
            const std::string& name,
            DynamicTypeBuilder::ref_type alias_builder,
            bool replace = false);

    /**
     * @brief Annotate a TypeDescriptor with the current annotation's information.
     * @note This method is used to apply builtin annotations to types,
     *       because TypeDescriptor cannot be modified after instantiating a DynamicTypeBuilder.
     *
     * @param[inout] descriptor The TypeDescriptor to annotate.
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the descriptor was successfully updated (i.e: if the annotation was applied or if no changes were needed),
     *         false if the input parameters are invalid.
     */
    bool annotate_descriptor(
            TypeDescriptor::ref_type& descriptor,
            const AnnotationParameterValues& parameters) const;

    /**
     * @brief Annotate a MemberDescriptor with the current annotation's information.
     * @note This method is used to apply builtin annotations to member types,
     *       because MemberDescriptor cannot be modified after instantiating a DynamicTypeBuilder.
     *
     * @param[inout] descriptor The MemberDescriptor to annotate.
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the descriptor was successfully updated (i.e: if the annotation was applied or if no changes were needed),
     *         false if the input parameters are invalid.
     */
    bool annotate_descriptor(
            MemberDescriptor::ref_type& descriptor,
            const AnnotationParameterValues& parameters) const;

    /**
     * @brief Annotate a type with the current annotation's information.
     *
     * @param[inout] type_builder The DynamicTypeBuilder to annotate.
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the type builder was successfully updated, false if the input parameters are invalid.
     */
    bool annotate_type(
            DynamicTypeBuilder::ref_type& type_builder,
            const AnnotationParameterValues& parameters) const;

    /**
     * @brief Annotate a member type with the current annotation's information.
     *
     * @param[inout] type_builder The DynamicTypeBuilder related to the type which contains the member to annotate.
     * @param[in] member_name The name of the type_builder's member to annotate.ç
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the member type builder was successfully updated, false if the input parameters, false in the following cases:Annotation
     *         - The member's name is invalid (empty or not found in the type_builder).
     *         - The member's type builder is invalid
     *         - The input parameters are invalid
     */
    bool annotate_member(
            DynamicTypeBuilder::ref_type& type_builder,
            const std::string& member_name,
            const AnnotationParameterValues& parameters) const;

    /**
     * @brief Annotate a member type with the current annotation's information.
     *
     * @param[inout] type_builder The DynamicTypeBuilder related to the type which contains the member to annotate.
     * @param[in] member_id The id of the type_builder's member to annotate.
     * @param[in] parameters The input parameters values to use for the annotation.
     * @return true if the member type builder was successfully updated, false if the input parameters, false in the following cases:Annotation
     *         - The member's id is invalid (empty or not found in the type_builder).
     *         - The member's type builder is invalid
     *         - The input parameters are invalid
     */
    bool annotate_member(
            DynamicTypeBuilder::ref_type& type_builder,
            uint32_t member_id,
            const AnnotationParameterValues& parameters) const;

    /**
     * @brief Check if the annotation is a builtin annotation
     */
    bool is_builtin() const;

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
            std::map<std::string, std::string>& resolved_parameters) const;

    DynamicTypeBuilder::ref_type annotation_builder_;
    std::map<std::string, DynamicTypeBuilder::ref_type> enums_;
    std::map<std::string, DynamicTypeBuilder::ref_type> constants_;
    std::map<std::string, DynamicTypeBuilder::ref_type> aliases_;
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
