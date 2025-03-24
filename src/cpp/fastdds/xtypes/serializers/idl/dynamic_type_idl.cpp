// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cassert>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/core/Types.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

#include "dynamic_type_idl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

using namespace eprosima::utilities::collections;

constexpr auto TYPE_OPENING = "{\n";
constexpr auto TYPE_CLOSURE = "};\n";
constexpr auto TAB_SEPARATOR = "    ";
constexpr auto MODULE_SEPARATOR = "::";

//////////////////////////
// DYNAMIC TYPE TO TREE //
//////////////////////////

ReturnCode_t dyn_type_to_tree(
        const DynamicType::_ref_type& type,
        const std::string& member_name,
        TreeNode<TreeNodeType>& node) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    const auto kind = type->get_kind();

    if (kind == TK_STRUCTURE)
    {
        // If is struct, the call is recursive.
        // Create new tree node
        node = TreeNode<TreeNodeType>(member_name, type->get_name().to_string(), type);

        // Get its base class
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        ret = type->get_descriptor(type_descriptor);

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting type descriptor of " << node << ".");
            return ret;
        }

        // Add base class as a new branch
        const auto base_type = type_descriptor->base_type();
        std::uint32_t first_member = 0;

        if (nullptr != base_type)
        {
            TreeNode<TreeNodeType> base;
            ret = dyn_type_to_tree(base_type, "PARENT", base);

            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting base type of " << node << ".");
                return ret;
            }

            base.info.is_base = true;

            // If the struct is derived from a base, according to the xtypes standard, the first members of the
            // struct are the members of the base.
            first_member = base_type->get_member_count();

            node.add_branch(base);
        }

        // Add each member as a new branch except for the members of its base class
        for (std::uint32_t index = first_member; index < type->get_member_count(); index++)
        {
            traits<DynamicTypeMember>::ref_type member;
            ret = type->get_member_by_index(member, index);

            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                        "Error getting member of " << node << " at index " << index << ".");
                return ret;
            }

            MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
            ret = member->get_descriptor(member_descriptor);

            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                        "Error getting member descriptor of member " << member->get_name() <<
                        " of " << node << ".");
                return ret;
            }

            TreeNode<TreeNodeType> child;
            ret = dyn_type_to_tree(member_descriptor->type(), member_descriptor->name().to_string(), child);

            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                        "Error building tree of member " << member->get_name() <<
                        " of " << node << ".");
                return ret;
            }

            child.info.is_key = member_descriptor->is_key();

            // Add each member with its name as a new child in a branch (recursion)
            node.add_branch(child);
        }
    }
    else
    {
        std::stringstream idl;
        ret = type_kind_to_idl(type, idl);

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                    "Error getting IDL representation of " << type->get_name().to_string() << ".");
            return ret;
        }

        node = TreeNode<TreeNodeType>(member_name, idl.str(), type);

        if (kind == TK_ALIAS)
        {
            TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
            ret = node.info.dynamic_type->get_descriptor(type_descriptor);

            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting type descriptor of " << node << ".");
                return ret;
            }

            // Add a branch for the base type of the alias
            TreeNode<TreeNodeType> base;
            ret = dyn_type_to_tree(type_descriptor->base_type(), "BASE", base);

            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error building tree of " << base << ".");
                return ret;
            }

            node.add_branch(base);
        }
        else if (kind == TK_ARRAY || kind == TK_MAP || kind == TK_SEQUENCE)
        {
            // Add a branch for the element type of the container
            DynamicType::_ref_type element_type;
            ret = get_element_type(type, element_type);

            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting element type of " << node << ".");
                return ret;
            }

            TreeNode<TreeNodeType> child;
            ret = dyn_type_to_tree(element_type, "CONTAINER_MEMBER", child);

            if (RETCODE_OK != ret)
            {
                EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error building tree of " << child << ".");
                return ret;
            }

            node.add_branch(child);
        }
        else if (kind == TK_UNION)
        {
            // Add each member as a new branch
            for (std::uint32_t index = 1; index < type->get_member_count(); index++)
            {
                traits<DynamicTypeMember>::ref_type member;
                ret = type->get_member_by_index(member, index);

                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                            "Error getting member of " << node << " at index " << index << ".");
                    return ret;
                }

                MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
                ret = member->get_descriptor(member_descriptor);

                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                            "Error getting member descriptor of member " << member->get_name() <<
                            " of " << node << ".");
                    return ret;
                }

                TreeNode<TreeNodeType> child;
                ret = dyn_type_to_tree(member_descriptor->type(), member_descriptor->name().to_string(), child);

                if (RETCODE_OK != ret)
                {
                    EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                            "Error building tree of member " << member->get_name() <<
                            " of " << node << ".");
                    return ret;
                }

                node.add_branch(child);
            }
        }
    }

    return ret;
}

ReturnCode_t type_kind_to_idl(
        const DynamicType::_ref_type& dyn_type,
        std::ostream& idl) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    const auto kind = dyn_type->get_kind();

    switch (kind)
    {
        case TK_BOOLEAN:
        {
            idl << "boolean";
            break;
        }
        case TK_BYTE:
        {
            idl << "octet";
            break;
        }
        case TK_INT8:
        {
            idl << "int8";
            break;
        }
        case TK_INT16:
        {
            idl << "short";
            break;
        }
        case TK_INT32:
        {
            idl << "long";
            break;
        }
        case TK_INT64:
        {
            idl << "long long";
            break;
        }
        case TK_UINT8:
        {
            idl << "uint8";
            break;
        }
        case TK_UINT16:
        {
            idl << "unsigned short";
            break;
        }
        case TK_UINT32:
        {
            idl << "unsigned long";
            break;
        }
        case TK_UINT64:
        {
            idl << "unsigned long long";
            break;
        }
        case TK_FLOAT32:
        {
            idl << "float";
            break;
        }
        case TK_FLOAT64:
        {
            idl << "double";
            break;
        }
        case TK_FLOAT128:
        {
            idl << "long double";
            break;
        }
        case TK_CHAR8:
        {
            idl << "char";
            break;
        }
        case TK_CHAR16:
        {
            idl << "wchar";
            break;
        }
        case TK_STRING8:
        case TK_STRING16:
        {
            ret = string_kind_to_idl(dyn_type, idl);
            break;
        }
        case TK_ARRAY:
        {
            ret = array_kind_to_idl(dyn_type, idl);
            break;
        }
        case TK_SEQUENCE:
        {
            ret = sequence_kind_to_idl(dyn_type, idl);
            break;
        }
        case TK_MAP:
        {
            ret = map_kind_to_idl(dyn_type, idl);
            break;
        }
        case TK_ALIAS:
        case TK_BITMASK:
        case TK_BITSET:
        case TK_ENUM:
        case TK_STRUCTURE:
        case TK_UNION:
        {
            idl << dyn_type->get_name().to_string();
            break;
        }
        case TK_NONE:
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Failed to convert TK_NONE to stream.");
            ret = RETCODE_UNSUPPORTED;
            break;
        }
        default:
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Failed to convert unknown type (" << kind << ") to stream.");
            ret = RETCODE_BAD_PARAMETER;
            break;
        }
    }

    return ret;
}

ReturnCode_t array_kind_to_idl(
        const DynamicType::_ref_type& dyn_type,
        std::ostream& idl) noexcept
{
    assert(dyn_type->get_kind() == TK_ARRAY);

    ReturnCode_t ret = RETCODE_OK;

    DynamicType::_ref_type element_type;
    ret = get_element_type(dyn_type, element_type);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting element type of " << dyn_type->get_name().to_string() << ".");
        return ret;
    }

    ret = type_kind_to_idl(element_type, idl);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting IDL representation of " << element_type->get_name().to_string() << ".");
        return ret;
    }

    BoundSeq bounds;
    ret = get_bounds(dyn_type, bounds);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting bounds of " << dyn_type->get_name().to_string() << ".");
        return ret;
    }

    for (const auto& bound : bounds)
    {
        idl << "[" << std::to_string(bound) << "]";
    }

    return ret;
}

ReturnCode_t map_kind_to_idl(
        const DynamicType::_ref_type& dyn_type,
        std::ostream& idl) noexcept
{
    assert(dyn_type->get_kind() == TK_MAP);

    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = dyn_type->get_descriptor(type_descriptor);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting type descriptor of " << dyn_type->get_name().to_string() << ".");
        return ret;
    }

    idl << "map<";

    const auto key_type = type_descriptor->key_element_type();
    ret = type_kind_to_idl(key_type, idl);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting IDL representation of " << key_type->get_name().to_string() << ".");
        return ret;
    }

    idl << ", ";

    const auto value_type = type_descriptor->element_type();
    ret = type_kind_to_idl(value_type, idl);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting IDL representation of " << value_type->get_name().to_string() << ".");
        return ret;
    }

    BoundSeq bounds;
    ret = get_bounds(dyn_type, bounds);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting bounds of " << dyn_type->get_name().to_string() << ".");
        return ret;
    }

    assert(bounds.size() <= 1);

    if (1 == bounds.size())
    {
        idl << ", " << std::to_string(bounds[0]);
    }

    idl << ">";

    return ret;
}

ReturnCode_t sequence_kind_to_idl(
        const DynamicType::_ref_type& dyn_type,
        std::ostream& idl) noexcept
{
    assert(dyn_type->get_kind() == TK_SEQUENCE);

    ReturnCode_t ret = RETCODE_OK;

    DynamicType::_ref_type element_type;
    ret = get_element_type(dyn_type, element_type);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting element type of " << dyn_type->get_name().to_string() << ".");
        return ret;
    }

    idl << "sequence<";

    ret = type_kind_to_idl(element_type, idl);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting IDL representation of " << element_type->get_name().to_string() << ".");
        return ret;
    }

    BoundSeq bounds;
    ret = get_bounds(dyn_type, bounds);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting bounds of " << dyn_type->get_name().to_string() << ".");
        return ret;
    }

    assert(bounds.size() <= 1);

    if (1 == bounds.size())
    {
        idl << ", " << std::to_string(bounds[0]);
    }

    idl << ">";

    return ret;
}

ReturnCode_t string_kind_to_idl(
        const DynamicType::_ref_type& dyn_type,
        std::ostream& idl) noexcept
{
    assert(dyn_type->get_kind() == TK_STRING8 || dyn_type->get_kind() == TK_STRING16);

    ReturnCode_t ret = RETCODE_OK;

    if (dyn_type->get_kind() == TK_STRING16)
    {
        idl << "wstring";
    }
    else
    {
        idl << "string";
    }

    BoundSeq bounds;
    ret = get_bounds(dyn_type, bounds);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting bounds of " << dyn_type->get_name().to_string() << ".");
        return ret;
    }

    assert(bounds.size() <= 1);

    if (1 == bounds.size())
    {
        idl << "<" << std::to_string(bounds[0]) << ">";
    }

    return ret;
}

//////////////////////////////
// DYNAMIC TYPE TREE TO IDL //
//////////////////////////////

ReturnCode_t dyn_type_tree_to_idl(
        const TreeNode<TreeNodeType>& root,
        std::ostream& idl) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    std::set<std::string> types_written;

    // Write the dependencies of the root node
    for (const auto& node : root.all_nodes())
    {
        if (types_written.find(node.info.type_kind_name) != types_written.end())
        {
            // The type has already been written. Skip it.
            continue;
        }

        const auto kind = node.info.dynamic_type->get_kind();

        switch (kind)
        {
            case TK_ALIAS:
            {
                ret = alias_to_idl(node, idl);
                break;
            }
            case TK_BITMASK:
            {
                ret = bitmask_to_idl(node, idl);
                break;
            }
            case TK_BITSET:
            {
                ret = bitset_to_idl(node, idl);
                break;
            }
            case TK_ENUM:
            {
                ret = enum_to_idl(node, idl);
                break;
            }
            case TK_STRUCTURE:
            {
                ret = struct_to_idl(node, idl);
                break;
            }
            case TK_UNION:
            {
                ret = union_to_idl(node, idl);
                break;
            }
            default:
            {
                continue;
            }
        }

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error writing " << node.info.type_kind_name << " to IDL.");
            return ret;
        }

        idl << "\n";
        types_written.insert(node.info.type_kind_name);
    }

    // Write the struct root node at last, after all its dependencies
    ret = struct_to_idl(root, idl);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error writing " << root.info.type_kind_name << " to IDL.");
        return ret;
    }

    return ret;
}

ReturnCode_t alias_to_idl(
        const TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept
{
    assert(node.info.dynamic_type->get_kind() == TK_ALIAS);

    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = node.info.dynamic_type->get_descriptor(type_descriptor);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting type descriptor of " << node << ".");
        return ret;
    }

    // Open modules definition (if any) and get type name
    std::string type_name = node.info.type_kind_name;
    unsigned int n_modules = open_modules_definition(type_name, idl);

    idl << "typedef ";

    // Find the base type of the alias
    ret = type_kind_to_idl(type_descriptor->base_type(), idl);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting IDL representation of " << node << ".");
        return ret;
    }

    idl << " " << type_name << ";\n";

    // Close modules definition (if any)
    close_modules_definition(n_modules, idl);

    return ret;
}

ReturnCode_t bitmask_to_idl(
        const TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept
{
    assert(node.info.dynamic_type->get_kind() == TK_BITMASK);

    ReturnCode_t ret = RETCODE_OK;

    BoundSeq bounds;
    ret = get_bounds(node.info.dynamic_type, bounds);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting bounds of " << node << ".");
        return ret;
    }

    if (1 != bounds.size())
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Bitmask type has " << bounds.size() << " bounds instead of one.");
        return RETCODE_BAD_PARAMETER;
    }

    // Open modules definition (if any) and get type name
    std::string type_name = node.info.type_kind_name;
    unsigned int n_modules = open_modules_definition(type_name, idl);

    // Annotation with the bitmask size
    static constexpr std::uint32_t DEFAULT_BITMASK_SIZE = 32;
    const auto bitmask_size = bounds[0];

    if (DEFAULT_BITMASK_SIZE != bitmask_size)
    {
        idl << "@bit_bound(" << std::to_string(bitmask_size) << ")\n";
    }

    idl << tabulate_n(n_modules) << "bitmask " << type_name << "\n";

    idl << tabulate_n(n_modules) << TYPE_OPENING;

    const auto member_count = node.info.dynamic_type->get_member_count();

    std::uint32_t pos = 0;

    for (std::uint32_t index = 0; index < member_count; index++)
    {
        DynamicTypeMember::_ref_type member;
        ret = node.info.dynamic_type->get_member_by_index(member, index);

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting member of " << node << " at index " << index << ".");
            return ret;
        }

        idl << TAB_SEPARATOR << tabulate_n(n_modules);

        // Annotation with the position
        const auto id = member->get_id();

        if (id != pos)
        {
            idl << "@position(" << std::to_string(id) << ") ";
        }

        idl << member->get_name().to_string();

        // Add comma if not last member
        if (index < member_count - 1)
        {
            idl << ",";
        }

        idl << "\n";

        // The position is always sequential
        pos = id + 1;
    }

    // Close type definition
    idl << tabulate_n(n_modules) << TYPE_CLOSURE;

    // Close modules definition (if any)
    close_modules_definition(n_modules, idl);

    return ret;
}

ReturnCode_t bitset_to_idl(
        const TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept
{
    assert(node.info.dynamic_type->get_kind() == TK_BITSET);

    ReturnCode_t ret = RETCODE_OK;

    // Open modules definition (if any) and get type name
    std::string type_name = node.info.type_kind_name;
    unsigned int n_modules = open_modules_definition(type_name, idl);

    idl << "bitset " << type_name << "\n";

    idl << tabulate_n(n_modules) << TYPE_OPENING;

    // Find the bits that each bitfield occupies
    BoundSeq bounds;
    ret = get_bounds(node.info.dynamic_type, bounds);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting bounds of " << node.info.dynamic_type->get_name().to_string() << ".");
        return ret;
    }

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = node.info.dynamic_type->get_descriptor(type_descriptor);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting type descriptor of " << node << ".");
        return ret;
    }

    std::uint32_t bits_set = 0;

    for (std::uint32_t index = 0; index < node.info.dynamic_type->get_member_count(); index++)
    {
        traits<DynamicTypeMember>::ref_type member;
        ret = node.info.dynamic_type->get_member_by_index(member, index);

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting member of " << node << " at index " << index << ".");
            return ret;
        }

        // The id of the member is the position in the bitset
        const auto id = member->get_id();

        if (id > bits_set)
        {
            // If the id is higher than the bits set, there must have been an empty bitfield (i.e. a gap)
            const auto gap = id - bits_set;
            bits_set += gap;

            idl << tabulate_n(n_modules) << TAB_SEPARATOR << "bitfield<" << std::to_string(gap) << ">;\n";
        }

        idl << tabulate_n(n_modules) << TAB_SEPARATOR << "bitfield<" << std::to_string(bounds[index]);

        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        ret = member->get_descriptor(member_descriptor);

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting member descriptor of " << member->get_name() << ".");
            return ret;
        }

        TypeKind default_type_kind;
        ret = get_default_type_kind(bounds[index], default_type_kind);

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting default type kind.");
            return ret;
        }

        // WARNING: If a user had explicitly set the type to be the default type, the serialization to IDL will not
        // set it explicitly.
        if (member_descriptor->type()->get_kind() != default_type_kind)
        {
            idl << ", ";

            // The type of the bitfield is not the default type. Write it.
            type_kind_to_idl(member_descriptor->type(), idl);
        }

        idl << "> " << member->get_name().to_string() << ";\n";

        bits_set += bounds[index];
    }

    // Close type definition
    idl << tabulate_n(n_modules) << TYPE_CLOSURE;

    // Close modules definition (if any)
    close_modules_definition(n_modules, idl);

    return ret;
}

ReturnCode_t enum_to_idl(
        const TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept
{
    assert(node.info.dynamic_type->get_kind() == TK_ENUM);

    ReturnCode_t ret = RETCODE_OK;

    // Open modules definition (if any) and get type name
    std::string type_name = node.info.type_kind_name;
    unsigned int n_modules = open_modules_definition(type_name, idl);

    idl << "enum " << type_name << "\n";

    idl << tabulate_n(n_modules) << TYPE_OPENING << TAB_SEPARATOR;

    for (std::uint32_t index = 0; index < node.info.dynamic_type->get_member_count(); index++)
    {
        traits<DynamicTypeMember>::ref_type member;
        ret = node.info.dynamic_type->get_member_by_index(member, index);

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting member of " << node << " at index " << index << ".");
            return ret;
        }

        idl << tabulate_n(n_modules) << member->get_name().to_string();

        if (node.info.dynamic_type->get_member_count() - 1 != index)
        {
            idl << ",\n" << TAB_SEPARATOR;
        }
        else
        {
            idl << "\n";
        }
    }

    // Close type definition
    idl << tabulate_n(n_modules) << TYPE_CLOSURE;

    // Close modules definition (if any)
    close_modules_definition(n_modules, idl);

    return ret;
}

ReturnCode_t struct_to_idl(
        const TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept
{
    assert(node.info.dynamic_type->get_kind() == TK_STRUCTURE);

    ReturnCode_t ret = RETCODE_OK;

    // Annotation with the extensibility kind
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = node.info.dynamic_type->get_descriptor(type_descriptor);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting type descriptor of " << node << ".");
        return ret;
    }

    // Open modules definition (if any) and get type name
    std::string type_name = node.info.type_kind_name;
    unsigned int n_modules = open_modules_definition(type_name, idl);

    switch (type_descriptor->extensibility_kind())
    {
        case ExtensibilityKind::FINAL:
        {
            idl << "@extensibility(FINAL)\n";
            break;
        }
        case ExtensibilityKind::MUTABLE:
        {
            idl << "@extensibility(MUTABLE)\n";
            break;
        }
        case ExtensibilityKind::APPENDABLE:
        {
            idl << "@extensibility(APPENDABLE)\n";
            break;
        }
        default:
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Extensibility kind not supported.");
            return RETCODE_BAD_PARAMETER;
        }
    }

    // Add type name
    idl << tabulate_n(n_modules) << "struct " << type_name;

    const auto base_type = type_descriptor->base_type();

    // Add inheritance
    if (nullptr != base_type)
    {
        idl << " : ";

        ret = type_kind_to_idl(base_type, idl);

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                    "Error getting IDL representation of " << base_type->get_name().to_string() << ".");
            return ret;
        }
    }

    idl << "\n" << tabulate_n(n_modules) << TYPE_OPENING;

    // Add struct attributes
    for (auto const& child : node.branches())
    {
        idl << tabulate_n(n_modules);

        if (child.info.is_base)
        {
            continue;
        }

        node_to_idl(child.info, idl);

        idl << ";\n";
    }

    // Close type definition
    idl << tabulate_n(n_modules) << TYPE_CLOSURE;

    // Close modules definition (if any)
    close_modules_definition(n_modules, idl);

    return ret;
}

ReturnCode_t union_to_idl(
        const TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept
{
    assert(node.info.dynamic_type->get_kind() == TK_UNION);

    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = node.info.dynamic_type->get_descriptor(type_descriptor);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting type descriptor of " << node << ".");
        return ret;
    }

    // Open modules definition (if any) and get type name
    std::string type_name = node.info.type_kind_name;
    unsigned int n_modules = open_modules_definition(type_name, idl);

    idl << "union " << type_name << " switch (";

    ret = type_kind_to_idl(type_descriptor->discriminator_type(), idl);

    if (RETCODE_OK != ret)
    {
        return ret;
    }

    idl << ")\n" << tabulate_n(n_modules) << TYPE_OPENING;

    for (std::uint32_t index = 1; index < node.info.dynamic_type->get_member_count(); index++)
    {
        traits<DynamicTypeMember>::ref_type member;
        ret = node.info.dynamic_type->get_member_by_index(member, index);

        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        ret = member->get_descriptor(member_descriptor);

        if (RETCODE_OK != ret)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Error getting member descriptor of " << member->get_name() << ".");
            return ret;
        }

        const auto labels = member_descriptor->label();  // WARNING: There might be casting issues as discriminant type is currently not taken into consideration

        for (const auto& label : labels)
        {
            idl << tabulate_n(n_modules) << TAB_SEPARATOR << "case " << std::to_string(label) << ":\n";
        }

        if (member_descriptor->is_default_label())
        {
            idl << tabulate_n(n_modules) << TAB_SEPARATOR << "default:\n";
        }

        idl << TAB_SEPARATOR << TAB_SEPARATOR << tabulate_n(n_modules);

        ret = type_kind_to_idl(member_descriptor->type(), idl);

        if (RETCODE_OK != ret)
        {
            return ret;
        }

        idl << " " << member->get_name().to_string() << ";\n";
    }

    // Close type definition
    idl << tabulate_n(n_modules) << TYPE_CLOSURE;

    // Close modules definition (if any)
    close_modules_definition(n_modules, idl);

    return ret;
}

ReturnCode_t node_to_idl(
        const TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept
{
    idl << TAB_SEPARATOR;

    if (node.info.is_key)
    {
        idl << "@key ";
    }

    if (TK_ARRAY == node.info.dynamic_type->get_kind())
    {
        const auto dim_pos = node.info.type_kind_name.find("[");

        if (std::string::npos == dim_pos)
        {
            EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Array type name is not well formed.");
            return RETCODE_BAD_PARAMETER;
        }

        const auto kind_name_str = node.info.type_kind_name.substr(0, dim_pos);
        const auto dim_str = node.info.type_kind_name.substr(dim_pos, std::string::npos);

        idl << kind_name_str << " " << node.info.member_name << dim_str;
    }
    else
    {
        idl << node.info.type_kind_name << " " << node.info.member_name;
    }

    return RETCODE_OK;
}

unsigned int open_modules_definition(
        std::string& type_name,
        std::ostream& idl) noexcept
{
    unsigned int n_modules = 0;

    while (type_name.find(MODULE_SEPARATOR) != std::string::npos)
    {
        size_t pos_start = 0;
        size_t pos_end = type_name.find(MODULE_SEPARATOR);

        std::string module_name = type_name.substr(0, pos_end);
        type_name.erase(pos_start, pos_end - pos_start + std::strlen(MODULE_SEPARATOR));

        idl << tabulate_n(n_modules) << "module " << module_name << "\n";

        idl << tabulate_n(n_modules) << TYPE_OPENING;

        n_modules++;
    }

    idl << tabulate_n(n_modules);

    return n_modules;
}

void close_modules_definition(
        unsigned int& n_modules,
        std::ostream& idl) noexcept
{
    while (n_modules > 0)
    {
        idl << tabulate_n(--n_modules) << TYPE_CLOSURE;
    }
}

std::string tabulate_n(
        const unsigned int& n_tabs) noexcept
{
    std::string tabs;

    tabs.reserve(std::strlen(TAB_SEPARATOR) * n_tabs);
    for (unsigned int i = 0; i < n_tabs; i++)
    {
        tabs += TAB_SEPARATOR;
    }

    return tabs;
}

///////////////////////
// AUXILIARY METHODS //
///////////////////////

ReturnCode_t get_element_type(
        const DynamicType::_ref_type& dyn_type,
        DynamicType::_ref_type& element_type) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = dyn_type->get_descriptor(type_descriptor);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting type descriptor of " << dyn_type->get_name().to_string() << ".");
        return ret;
    }

    element_type = type_descriptor->element_type();

    return ret;
}

ReturnCode_t get_bounds(
        const DynamicType::_ref_type& dyn_type,
        BoundSeq& bounds) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = dyn_type->get_descriptor(type_descriptor);

    if (RETCODE_OK != ret)
    {
        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL,
                "Error getting type descriptor of " << dyn_type->get_name().to_string() << ".");
        return ret;
    }

    bounds = type_descriptor->bound();

    static constexpr auto UNBOUNDED = static_cast<std::uint32_t>(LENGTH_UNLIMITED);

    if (1 == bounds.size() && UNBOUNDED == bounds[0])
    {
        bounds.clear();
    }

    return ret;
}

ReturnCode_t get_default_type_kind(
        const std::uint32_t size,
        TypeKind& default_type) noexcept
{
    if (size == 1)
    {
        default_type = TK_BOOLEAN;
    }
    else if (size <= 8)
    {
        default_type = TK_UINT8;
    }
    else if (size <= 16)
    {
        default_type = TK_UINT16;
    }
    else if (size <= 32)
    {
        default_type = TK_UINT32;
    }
    else if (size <= 64)
    {
        default_type = TK_UINT64;
    }
    else
    {
        // Set to none to avoid `may be used uninitialized` error
        default_type = TK_NONE;

        EPROSIMA_LOG_ERROR(DYNAMIC_TYPE_IDL, "Size " << size << " is not supported.");
        return RETCODE_BAD_PARAMETER;
    }

    return RETCODE_OK;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
