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
            std::vector<std::pair<std::string, DynamicType::_ref_type>> members_by_name;
            ret = get_members_sorted(type, members_by_name);

            if (ret != RETCODE_OK)
            {
                return ret;
            }

            for (const auto& member : members_by_name)
            {
                ret = dyn_type_to_tree(member.second, member.first, node);

                if (ret != RETCODE_OK)
                {
                    return ret;
                }

                // Add each member with its name as a new node in a branch (recursion)
                parent.add_branch(node);
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
            ret = dyn_type_to_str(internal_type, internal_str);

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
            ret = dyn_type_to_str(type, type_str);

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

ReturnCode_t dyn_type_to_str(
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
        {
            type_str = dyn_type->get_name().to_string();
            break;
        }
        case TK_BITSET:
        case TK_BITMASK:
        case TK_NONE:
        case TK_ALIAS:
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

    ret = dyn_type_to_str(internal_type, array_str);

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
        array_str += bound;
        array_str += "]";
    }

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

    ret = dyn_type_to_str(internal_type, sequence_str);

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
        if (bound == static_cast<std::uint32_t>(LENGTH_UNLIMITED))
        {
            sequence_str += ", unbounded";
        }
        else
        {
            sequence_str += ", " + bound;
        }
    }

    sequence_str += ">";

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
    ret = dyn_type_to_str(key_type, key_str);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    std::string value_str;
    const auto value_type = type_descriptor->element_type();
    ret = dyn_type_to_str(value_type, value_str);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    std::stringstream ss;

    ss << "map<" << key_str << ", " << value_str << ">";

    map_str = ss.str();

    return ret;
}

ReturnCode_t get_members_sorted(
        const DynamicType::_ref_type& dyn_type,
        std::vector<std::pair<std::string, DynamicType::_ref_type>>& result) noexcept
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
            std::make_pair<std::string, DynamicType::_ref_type>(
                dyn_name.to_string(),
                std::move(member_descriptor->type())));
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
            case TK_STRUCTURE:
            {
                ret = struct_to_str(node, kind_str);
                break;
            }
            case TK_ENUM:
            {
                ret = enum_to_str(node, kind_str);
                break;
            }
            case TK_UNION:
            {
                ret = union_to_str(node, kind_str);
                break;
            }
            default:
                continue;
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

    std::map<MemberId, DynamicTypeMember::_ref_type> members;
    ret = node.info.dynamic_type->get_all_members(members);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    enum_str = "enum " + node.info.type_kind_name + TYPE_OPENING + TAB_SEPARATOR;
    bool first_iter = true;

    for (const auto& member : members)
    {
        if (!first_iter)
        {
            enum_str += ",\n";
            enum_str += TAB_SEPARATOR;
        }

        first_iter = false;

        enum_str += member.second->get_name();
    }

    // Close definition
    enum_str += "\n";
    enum_str += TYPE_CLOSURE;

    return ret;
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
    ret = dyn_type_to_str(type_descriptor->discriminator_type(), discriminant_type_str);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    union_str = "union " + node.info.type_kind_name + " switch (" + discriminant_type_str + ")" + TYPE_OPENING;

    std::map<MemberId, DynamicTypeMember::_ref_type> members;
    ret = node.info.dynamic_type->get_all_members(members);  // WARNING: Default case not included in this collection, and currently not available

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
        ret = dyn_type_to_str(member_descriptor->type(), member_str);

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        union_str += "\n";
        union_str += TAB_SEPARATOR;
        union_str += TAB_SEPARATOR;
        union_str += member_str + " ";
        union_str += member.second->get_name();
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

    if (node.info.dynamic_type->get_kind() == TK_ARRAY)
    {
        auto dim_pos = node.info.type_kind_name.find("[");
        auto kind_name_str = node.info.type_kind_name.substr(0, dim_pos);
        auto dim_str = node.info.type_kind_name.substr(dim_pos, std::string::npos);

        node_str += kind_name_str + " " + node.info.member_name + dim_str;
    }
    else
    {
        node_str += node.info.type_kind_name + " " + node.info.member_name;
    }

    return RETCODE_OK;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
