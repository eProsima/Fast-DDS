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

#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>


namespace eprosima {
namespace fastdds {
namespace dds {

constexpr auto TYPE_OPENING = "\n{\n";
constexpr auto TYPE_CLOSURE = "};\n";
constexpr auto TAB_SEPARATOR = "    ";


//////////////////////////
// DYNAMIC TYPE TO TREE //
//////////////////////////

ReturnCode_t dyn_type_to_tree(
        const DynamicType::_ref_type& type,
        const std::string& member_name,
        utilities::collections::TreeNode<TreeNodeType>& node) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    switch (type->get_kind())
    {
        case TK_STRUCTURE:
        {
            // If is struct, the call is recursive.
            // Create new tree node
            utilities::collections::TreeNode<TreeNodeType> parent(member_name, type->get_name().to_string(), type);

            // Get all members of this struct
            std::vector<std::pair<std::string, MemberDescriptor::_ref_type>> members_by_name;
            ret = get_members_sorted(type, members_by_name);

            if (ret != RETCODE_OK)
            {
                return ret;
            }

            for (const auto& member : members_by_name)
            {
                const auto& member_name = member.first;
                const auto& member_descriptor = member.second;

                utilities::collections::TreeNode<TreeNodeType> child;
                ret = dyn_type_to_tree(member_descriptor->type(), member_name, child);

                if (ret != RETCODE_OK)
                {
                    return ret;
                }

                child.info.is_key = member_descriptor->is_key();

                // Add each member with its name as a new child in a branch (recursion)
                parent.add_branch(child);
            }

            node = parent;
            break;
        }
        case TK_ARRAY:
        case TK_SEQUENCE:
        {
            // If container (array or struct) has exactly one branch
            // Calculate child branch
            DynamicType::_ref_type internal_type;
            ret = container_internal_type(type, internal_type);

            if (ret != RETCODE_OK)
            {
                return ret;
            }

            std::string internal_str;
            ret = type_kind_to_str(type, internal_str);

            if (ret != RETCODE_OK)
            {
                return ret;
            }

            // Create this node
            utilities::collections::TreeNode<TreeNodeType> container(member_name, internal_str, type);
            // Add branch
            ret = dyn_type_to_tree(internal_type, "CONTAINER_MEMBER", node);

            if (ret != RETCODE_OK)
            {
                return ret;
            }

            container.add_branch(node);

            node = container;
            break;
        }
        default:
        {
            std::string type_str;
            ret = type_kind_to_str(type, type_str);

            if (ret != RETCODE_OK)
            {
                return ret;
            }

            node = utilities::collections::TreeNode<TreeNodeType>(member_name, type_str, type);
            break;
        }
    }

    return ret;
}

ReturnCode_t type_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& type_str) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    switch (dyn_type->get_kind())
    {
        case TK_BOOLEAN:
        {
            type_str = "boolean";
            break;
        }
        case TK_BYTE:
        {
            type_str = "octet";
            break;
        }
        case TK_INT8:
        {
            type_str = "char";
            break;
        }
        case TK_INT16:
        {
            type_str = "short";
            break;
        }
        case TK_INT32:
        {
            type_str = "long";
            break;
        }
        case TK_INT64:
        {
            type_str = "long long";
            break;
        }
        case TK_UINT8:
        {
            type_str = "unsigned char";
            break;
        }
        case TK_UINT16:
        {
            type_str = "unsigned short";
            break;
        }
        case TK_UINT32:
        {
            type_str = "unsigned long";
            break;
        }
        case TK_UINT64:
        {
            type_str = "unsigned long long";
            break;
        }
        case TK_FLOAT32:
        {
            type_str = "float";
            break;
        }
        case TK_FLOAT64:
        {
            type_str = "double";
            break;
        }
        case TK_FLOAT128:
        {
            type_str = "long double";
            break;
        }
        case TK_CHAR8:
        {
            type_str = "char";
            break;
        }
        case TK_CHAR16:
        {
            type_str = "wchar";
            break;
        }
        case TK_STRING8:
        {
            type_str = "string";
            break;
        }
        case TK_STRING16:
        {
            type_str = "wstring";
            break;
        }
        case TK_ARRAY:
        {
            ret = array_kind_to_str(dyn_type, type_str);
            break;
        }
        case TK_SEQUENCE:
        {
            ret = sequence_kind_to_str(dyn_type, type_str);
            break;
        }
        case TK_MAP:
        {
            ret = map_kind_to_str(dyn_type, type_str);
            break;
        }
        case TK_STRUCTURE:
        case TK_ENUM:
        case TK_UNION:
        case TK_BITSET:
        case TK_BITMASK:
        case TK_ALIAS:
        {
            type_str = dyn_type->get_name().to_string();
            break;
        }
        case TK_NONE:
        {
            ret = RETCODE_UNSUPPORTED;
            break;
        }
        default:
        {
            ret = RETCODE_BAD_PARAMETER;
            break;
        }
    }

    return ret;
}

ReturnCode_t array_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& array_str) noexcept
{
    if (dyn_type->get_kind() != TK_ARRAY)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Type is not an array.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;

    DynamicType::_ref_type internal_type;
    ret = container_internal_type(dyn_type, internal_type);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    ret = type_kind_to_str(internal_type, array_str);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    BoundSeq bounds;
    ret = container_size(dyn_type, bounds);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    for (const auto& bound : bounds)
    {
        array_str += "[";
        array_str += std::to_string(bound);
        array_str += "]";
    }

    return ret;
}

ReturnCode_t map_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& map_str) noexcept
{
    if (dyn_type->get_kind() != TK_MAP)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Type is not a map.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = dyn_type->get_descriptor(type_descriptor);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    std::string key_str;
    const auto key_type = type_descriptor->key_element_type();
    ret = type_kind_to_str(key_type, key_str);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    std::string value_str;
    const auto value_type = type_descriptor->element_type();
    ret = type_kind_to_str(value_type, value_str);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    std::stringstream ss;

    ss << "map<" << key_str << ", " << value_str << ">";

    map_str = ss.str();

    return ret;
}

ReturnCode_t sequence_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& sequence_str) noexcept
{
    if (dyn_type->get_kind() != TK_SEQUENCE)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Type is not a sequence.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;

    DynamicType::_ref_type internal_type;
    ret = container_internal_type(dyn_type, internal_type);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    ret = type_kind_to_str(internal_type, sequence_str);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    sequence_str = "sequence<" + sequence_str;

    BoundSeq bounds;
    ret = container_size(dyn_type, bounds);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    for (const auto& bound : bounds)
    {
        const auto UNBOUNDED = static_cast<std::uint32_t>(LENGTH_UNLIMITED);

        if (bound != UNBOUNDED)
        {
            sequence_str += ", " + std::to_string(bound);
        }
    }

    sequence_str += ">";

    return ret;
}

ReturnCode_t get_members_sorted(
        const DynamicType::_ref_type& dyn_type,
        std::vector<std::pair<std::string, MemberDescriptor::_ref_type>>& result) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    std::map<MemberId, DynamicTypeMember::_ref_type> members;
    ret = dyn_type->get_all_members(members);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    for (const auto& member : members)
    {
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        ret = member.second->get_descriptor(member_descriptor);

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        const auto dyn_name = member.second->get_name();
        result.emplace_back(
            std::make_pair<std::string, MemberDescriptor::_ref_type>(
                dyn_name.to_string(),
                std::move(member_descriptor)));
    }

    return ret;
}

ReturnCode_t container_internal_type(
        const DynamicType::_ref_type& dyn_type,
        DynamicType::_ref_type& internal_type) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = dyn_type->get_descriptor(type_descriptor);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    internal_type = type_descriptor->element_type();

    return ret;
}

ReturnCode_t container_size(
        const DynamicType::_ref_type& dyn_type,
        BoundSeq& bounds) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = dyn_type->get_descriptor(type_descriptor);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    bounds = type_descriptor->bound();

    return ret;
}


//////////////////////////////
// DYNAMIC TYPE TREE TO IDL //
//////////////////////////////

ReturnCode_t dyn_type_tree_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& root,
        std::string& dyn_type_str) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    std::set<std::string> types_written;

    // For every Node, check if it is of a "writable" type (i.e. struct, enum or union).
    // If it is, check if it is not yet written
    // If it is not, write it down
    for (const auto& node : root.all_nodes())
    {
        if (types_written.find(node.info.type_kind_name) != types_written.end())
        {
            continue;
        }

        std::string kind_str;
        const auto kind = node.info.dynamic_type->get_kind();

        switch (kind)
        {
            case TK_ALIAS:
            {
                ret = alias_to_str(node, kind_str);
                break;
            }
            case TK_BITMASK:
            {
                ret = bitmask_to_str(node, kind_str);
                break;
            }
            case TK_BITSET:
            {
                ret = bitset_to_str(node, kind_str);
                break;
            }
            case TK_ENUM:
            {
                ret = enum_to_str(node, kind_str);
                break;
            }
            case TK_STRUCTURE:
            {
                ret = struct_to_str(node, kind_str);
                break;
            }
            case TK_UNION:
            {
                ret = union_to_str(node, kind_str);
                break;
            }
            default:
            {
                continue;
            }
        }

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        dyn_type_str += kind_str + "\n";
        types_written.insert(node.info.type_kind_name);
    }

    // Write struct parent node at last, after all its dependencies
    // NOTE: not a requirement for Foxglove IDL Parser, dependencies can be placed after parent
    std::string struct_str;
    ret = struct_to_str(root, struct_str);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    dyn_type_str += struct_str;

    return ret;
}

ReturnCode_t alias_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& alias_str) noexcept
{
    if (node.info.dynamic_type->get_kind() != TK_ALIAS)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Type is not an alias.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = node.info.dynamic_type->get_descriptor(type_descriptor);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    // Find the base type of the alias
    std::string base_type_kind_str;
    ret = type_kind_to_str(type_descriptor->base_type(), base_type_kind_str);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    alias_str = "typedef ";
    alias_str += base_type_kind_str + " ";
    alias_str += type_descriptor->name().to_string() + ";\n";

    return ret;
}

ReturnCode_t bitmask_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& bitset_str) noexcept
{
    if (node.info.dynamic_type->get_kind() != TK_BITMASK)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Type is not a bitmask.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = node.info.dynamic_type->get_descriptor(type_descriptor);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    if (type_descriptor->bound().size() != 1)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Bitmask type must have exactly one bound.");
        return RETCODE_BAD_PARAMETER;
    }

    // Annotation with the bitmask size
    static constexpr std::uint32_t DEFAULT_BITMASK_SIZE = 32;
    const auto bitmask_size = type_descriptor->bound()[0];

    if (bitmask_size != DEFAULT_BITMASK_SIZE)
    {
        bitset_str = "@bit_bound(" + std::to_string(bitmask_size) + ")\n";
    }

    bitset_str += "bitmask " + node.info.type_kind_name + TYPE_OPENING;

    const auto member_count = node.info.dynamic_type->get_member_count();

    std::uint32_t pos = 0;

    for (std::uint32_t index = 0; index < member_count; index++)
    {
        traits<DynamicTypeMember>::ref_type member;
        ret = node.info.dynamic_type->get_member_by_index(member, index);

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        bitset_str += TAB_SEPARATOR;

        // Annotation with the position
        const auto id = member->get_id();

        if (id != pos)
        {
            bitset_str += "@position(" + std::to_string(id) + ") ";
        }

        bitset_str += member->get_name().to_string();

        // Add comma if not last member
        if (index < member_count - 1)
        {
            bitset_str += ",";
        }

        bitset_str += "\n";

        // The position is always sequential
        pos = id + 1;
    }

    // Close definition
    bitset_str += TYPE_CLOSURE;

    return ret;
}

ReturnCode_t bitset_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& bitset_str) noexcept
{
    if (node.info.dynamic_type->get_kind() != TK_BITSET)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Type is not a bitset.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;

    bitset_str = "bitset " + node.info.type_kind_name + TYPE_OPENING;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = node.info.dynamic_type->get_descriptor(type_descriptor);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    // Find the bits that each bitfield occupies
    const auto bits = type_descriptor->bound();

    std::uint32_t bits_set = 0;

    for (std::uint32_t index = 0; index < node.info.dynamic_type->get_member_count(); index++)
    {
        traits<DynamicTypeMember>::ref_type member;
        ret = node.info.dynamic_type->get_member_by_index(member, index);

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        // The id of the member is the position in the bitset
        const auto id = member->get_id();

        if (id > bits_set)
        {
            // If the id is higher than the bits set, there must have been an empty bitfield (i.e. a gap)
            const auto bits = id - bits_set;
            bits_set += bits;

            bitset_str += TAB_SEPARATOR;
            bitset_str += "bitfield<";
            bitset_str += std::to_string(bits);
            bitset_str += ">;\n";
        }

        bitset_str += TAB_SEPARATOR;
        bitset_str += "bitfield<";
        bitset_str += std::to_string(bits[index]);

        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        ret = member->get_descriptor(member_descriptor);

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        TypeKind default_type_kind;
        ret = get_default_type_kind(bits[index], default_type_kind);

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        // WARNING: If a user had explicitly set the type to be the default type, the serialization to IDL will not
        // set it explicitly.
        if (member_descriptor->type()->get_kind() != default_type_kind)
        {
            // The type of the bitfield is not the default type. Write it.
            std::string type_str;
            type_kind_to_str(member_descriptor->type(), type_str);

            bitset_str += ", ";
            bitset_str += type_str;
        }

        bitset_str += "> ";
        bitset_str += member->get_name().to_string();
        bitset_str += ";\n";

        bits_set += bits[index];
    }

    // Close definition
    bitset_str += TYPE_CLOSURE;

    return ret;
}

ReturnCode_t enum_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& enum_str) noexcept
{
    if (node.info.dynamic_type->get_kind() != TK_ENUM)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Type is not an enum.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;

    enum_str = "enum " + node.info.type_kind_name + TYPE_OPENING + TAB_SEPARATOR;

    for (std::uint32_t index = 0; index < node.info.dynamic_type->get_member_count(); index++)
    {
        traits<DynamicTypeMember>::ref_type member;
        ret = node.info.dynamic_type->get_member_by_index(member, index);

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        if (index != 0)
        {
            enum_str += ",\n";
            enum_str += TAB_SEPARATOR;
        }

        enum_str += member->get_name().to_string();
    }

    // Close definition
    enum_str += "\n";
    enum_str += TYPE_CLOSURE;

    return ret;
}

ReturnCode_t struct_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& struct_str) noexcept
{
    if (node.info.dynamic_type->get_kind() != TK_STRUCTURE)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Type is not a struct.");
        return RETCODE_BAD_PARAMETER;
    }

    // Add types name
    struct_str = "struct " + node.info.type_kind_name + TYPE_OPENING;

    // Add struct attributes
    for (auto const& child : node.branches())
    {
        std::string child_str;
        node_to_str(child.info, child_str);

        struct_str += child_str + ";\n";
    }

    // Close definition
    struct_str += TYPE_CLOSURE;

    return RETCODE_OK;
}

ReturnCode_t union_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& union_str) noexcept
{
    if (node.info.dynamic_type->get_kind() != TK_UNION)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Type is not a union.");
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret = RETCODE_OK;

    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    ret = node.info.dynamic_type->get_descriptor(type_descriptor);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    std::string discriminant_type_str;
    ret = type_kind_to_str(type_descriptor->discriminator_type(), discriminant_type_str);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    union_str = "union " + node.info.type_kind_name + " switch (" + discriminant_type_str + ")" + TYPE_OPENING;

    for (std::uint32_t index = 1; index < node.info.dynamic_type->get_member_count(); index++)
    {
        traits<DynamicTypeMember>::ref_type member;
        ret = node.info.dynamic_type->get_member_by_index(member, index);

        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        ret = member->get_descriptor(member_descriptor);

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        const auto labels = member_descriptor->label();  // WARNING: There might be casting issues as discriminant type is currently not taken into consideration
        bool first_iter = true;

        for (const auto& label : labels)
        {
            if (first_iter)
            {
                union_str += TAB_SEPARATOR;
            }
            else
            {
                union_str += " ";
            }

            first_iter = false;

            union_str += "case " + std::to_string(label) + ":";
        }

        std::string member_str;
        ret = type_kind_to_str(member_descriptor->type(), member_str);

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        union_str += "\n";
        union_str += TAB_SEPARATOR;
        union_str += TAB_SEPARATOR;
        union_str += member_str + " ";
        union_str += member->get_name().to_string();
        union_str += ";\n";
    }

    // Close definition
    union_str += TYPE_CLOSURE;

    return ret;
}

ReturnCode_t node_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& node_str) noexcept
{
    node_str = TAB_SEPARATOR;

    if (node.info.is_key)
    {
        node_str += "@key ";
    }

    if (node.info.dynamic_type->get_kind() == TK_ARRAY)
    {
        const auto dim_pos = node.info.type_kind_name.find("[");

        if (dim_pos == std::string::npos)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Array type name is not well formed.");
            return RETCODE_BAD_PARAMETER;
        }

        const auto kind_name_str = node.info.type_kind_name.substr(0, dim_pos);
        const auto dim_str = node.info.type_kind_name.substr(dim_pos, std::string::npos);

        node_str += kind_name_str + " " + node.info.member_name + dim_str;
    }
    else
    {
        node_str += node.info.type_kind_name + " " + node.info.member_name;
    }

    return RETCODE_OK;
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
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Size is not supported.");
        return RETCODE_BAD_PARAMETER;
    }

    return RETCODE_OK;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
