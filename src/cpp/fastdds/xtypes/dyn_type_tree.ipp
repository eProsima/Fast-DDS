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

#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <fastdds/dds/core/ReturnCode.hpp>
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
            ret = type_kind_to_str(internal_type, internal_str);

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
        array_str += bound;
        array_str += "]";
    }

    return ret;
}

ReturnCode_t sequence_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& sequence_str) noexcept
{
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
        sequence_str += ", " + bound;
    }

    sequence_str += ">";

    return ret;
}

ReturnCode_t map_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& map_str) noexcept
{
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

ReturnCode_t get_members_sorted(
        const DynamicType::_ref_type& dyn_type,
        std::vector<std::pair<std::string, DynamicType::_ref_type>>& result) noexcept
{
    std::map<MemberId, DynamicTypeMember::_ref_type> members;
    dyn_type->get_all_members(members);

    for (const auto& member : members)
    {
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        const auto ret = member.second->get_descriptor(member_descriptor);

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

    return RETCODE_OK;
}

ReturnCode_t container_internal_type(
        const DynamicType::_ref_type& dyn_type,
        DynamicType::_ref_type& internal_type) noexcept
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    const auto ret = dyn_type->get_descriptor(type_descriptor);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    internal_type = type_descriptor->element_type();

    return RETCODE_OK;
}

ReturnCode_t container_size(
        const DynamicType::_ref_type& dyn_type,
        BoundSeq& bounds) noexcept
{
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    const auto ret = dyn_type->get_descriptor(type_descriptor);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    bounds = type_descriptor->bound();

    return RETCODE_OK;
}


//////////////////////////////
// DYNAMIC TYPE TREE TO IDL //
//////////////////////////////

ReturnCode_t dyn_type_tree_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& parent_node,
        std::string& dyn_type_idl) noexcept
{
    std::set<std::string> types_written;

    std::stringstream ss;

    // For every Node, check if it is of a "writable" type (i.e. struct, enum or union).
    // If it is, check if it is not yet written
    // If it is not, write it down
    for (const auto& node : parent_node.all_nodes())
    {
        if (types_written.find(node.info.type_kind_name) != types_written.end())
        {
            continue;
        }

        ReturnCode_t ret;
        const auto kind = node.info.dynamic_type->get_kind();

        switch (kind)
        {
            case TK_STRUCTURE:
            {
                ret = struct_to_str(ss, node);
                break;
            }
            case TK_ENUM:
            {
                ret = enum_to_str(ss, node);
                break;
            }
            case TK_UNION:
            {
                ret = union_to_str(ss, node);
                break;
            }
            default:
                continue;
        }

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        ss << "\n"; // Introduce blank line between type definitions
        types_written.insert(node.info.type_kind_name);
    }

    // Write struct parent node at last, after all its dependencies
    // NOTE: not a requirement for Foxglove IDL Parser, dependencies can be placed after parent
    const auto ret = struct_to_str(ss, parent_node);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    dyn_type_idl = ss.str();

    return RETCODE_OK;
}

ReturnCode_t struct_to_str(
        std::ostream& os,
        const utilities::collections::TreeNode<TreeNodeType>& node) noexcept
{
    // Add types name
    os << "struct " << node.info.type_kind_name << TYPE_OPENING;

    // Add struct attributes
    for (auto const& child : node.branches())
    {
        node_to_str(os, child.info);
        os << ";\n";
    }

    // Close definition
    os << TYPE_CLOSURE;

    return RETCODE_OK;
}

ReturnCode_t enum_to_str(
        std::ostream& os,
        const utilities::collections::TreeNode<TreeNodeType>& node) noexcept
{
    os << "enum " << node.info.type_kind_name << TYPE_OPENING << TAB_SEPARATOR;

    std::map<MemberId, DynamicTypeMember::_ref_type> members;
    node.info.dynamic_type->get_all_members(members);
    bool first_iter = true;
    for (const auto& member : members)
    {
        if (!first_iter)
        {
            os << ",\n" << TAB_SEPARATOR;
        }
        first_iter = false;

        os << member.second->get_name();
    }

    // Close definition
    os << "\n" << TYPE_CLOSURE;

    return RETCODE_OK;
}

ReturnCode_t union_to_str(
        std::ostream& os,
        const utilities::collections::TreeNode<TreeNodeType>& node) noexcept
{
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

    os << "union " << node.info.type_kind_name << " switch (" << discriminant_type_str << ")" << TYPE_OPENING;

    std::map<MemberId, DynamicTypeMember::_ref_type> members;
    node.info.dynamic_type->get_all_members(members);  // WARNING: Default case not included in this collection, and currently not available

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
                os << TAB_SEPARATOR;
            }
            else
            {
                os << " ";
            }
            first_iter = false;

            os << "case " << std::to_string(label) << ":";
        }

        std::string member_str;
        ret = type_kind_to_str(member_descriptor->type(), member_str);

        if (ret != RETCODE_OK)
        {
            return ret;
        }

        os << "\n" << TAB_SEPARATOR << TAB_SEPARATOR << member_str << " " << member.second->get_name() << ";\n";
    }

    // Close definition
    os << TYPE_CLOSURE;

    return ret;
}

ReturnCode_t node_to_str(
        std::ostream& os,
        const utilities::collections::TreeNode<TreeNodeType>& node) noexcept
{
    os << TAB_SEPARATOR;

    if (node.info.dynamic_type->get_kind() == TK_ARRAY)
    {
        auto dim_pos = node.info.type_kind_name.find("[");
        auto kind_name_str = node.info.type_kind_name.substr(0, dim_pos);
        auto dim_str = node.info.type_kind_name.substr(dim_pos, std::string::npos);

        os << kind_name_str << " " << node.info.member_name << dim_str;
    }
    else
    {
        os << node.info.type_kind_name << " " << node.info.member_name;
    }

    return RETCODE_OK;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
