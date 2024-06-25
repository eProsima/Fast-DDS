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

/*!
 * @file dyn_type_tree.hpp
 */

#ifndef _FASTDDS_DDS_XTYPES_DYN_TYPE_TREE_HPP_
#define _FASTDDS_DDS_XTYPES_DYN_TYPE_TREE_HPP_

#include <string>
#include <utility>
#include <vector>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>

#include "utils/collections/TreeNode.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

struct TreeNodeType
{
    TreeNodeType()
    {
    }

    TreeNodeType(
            const std::string& member_name,
            const std::string& type_kind_name,
            const DynamicType::_ref_type& dynamic_type,
            const bool is_base = false,
            const bool is_key = false)
        : member_name(member_name)
        , type_kind_name(type_kind_name)
        , dynamic_type(dynamic_type)
        , is_base(is_base)
        , is_key(is_key)
    {
    }

    std::string member_name;
    std::string type_kind_name;
    DynamicType::_ref_type dynamic_type;
    bool is_base;
    bool is_key;
};


//////////////////////////
// DYNAMIC TYPE TO TREE //
//////////////////////////

/**
 * @brief Converts a DynamicType to a tree.
 *
 * @param type The DynamicType to represent as a tree.
 * @param member_name The name of the root node.
 * @param node The root node of the tree.
 */
ReturnCode_t dyn_type_to_tree(
        const DynamicType::_ref_type& type,
        const std::string& member_name,
        utilities::collections::TreeNode<TreeNodeType>& node) noexcept;

/**
 * @brief Converts a DynamicType to a string.
 *
 * @param dyn_type The DynamicType to convert.
 * @param type_str The string representation of the DynamicType.
 */
ReturnCode_t type_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& type_str) noexcept;

/**
 * @brief Converts a DynamicType of \c TK_ARRAY kind to a string.
 *
 * @param dyn_type The DynamicType kind to convert.
 * @param array_str The string representation of the DynamicType kind.
 */
ReturnCode_t array_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& array_str) noexcept;

/**
 * @brief Converts a DynamicType of \c TK_MAP kind to a string.
 *
 * @param dyn_type The DynamicType to convert.
 * @param map_str The string representation of the DynamicType.
 */
ReturnCode_t map_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& map_str) noexcept;

/**
 * @brief Converts a DynamicType of \c TK_SEQUENCE kind to a string.
 *
 * @param dyn_type The DynamicType to convert.
 * @param sequence_str The string representation of the DynamicType.
 */
ReturnCode_t sequence_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& sequence_str) noexcept;

/**
 * @brief Gathers the members of a DynamicType.
 *
 * @param dyn_type The DynamicType to gather the members from.
 * @param result The vector of (name, member_descriptor) to store the members.
 */
ReturnCode_t get_members_sorted(
        const DynamicType::_ref_type& dyn_type,
        std::vector<std::pair<std::string, MemberDescriptor::_ref_type>>& result) noexcept;

/**
 * @brief Gathers the \c element_type of the DynamicType.
 *
 * @param dyn_type The DynamicType to gather the members from.
 * @param internal_type The internal type of the DynamicType's \c type_descriptor.
 */
ReturnCode_t container_internal_type(
        const DynamicType::_ref_type& dyn_type,
        DynamicType::_ref_type& internal_type) noexcept;

/**
 * @brief Gathers the \c bounds of the DynamicType.
 *
 * @param dyn_type The DynamicType to gather the members from.
 * @param bounds The vector to store the bounds in.
 */
ReturnCode_t container_size(
        const DynamicType::_ref_type& dyn_type,
        BoundSeq& bounds) noexcept;


//////////////////////////////
// DYNAMIC TYPE TREE TO IDL //
//////////////////////////////

/**
 * @brief Converts a tree to an IDL string.
 *
 * @param root The root node of the tree.
 * @param dyn_type_str The string representation of the tree.
 */
ReturnCode_t dyn_type_tree_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& root,
        std::string& dyn_type_str) noexcept;

/**
 * @brief Converts a tree with a \c TK_ALIAS root to an IDL string.
 *
 * @param node The root node of the tree.
 * @param alias_str The string representation of the tree.
 */
ReturnCode_t alias_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& alias_str) noexcept;

/**
 * @brief Converts a tree with a \c TK_BITMASK root to an IDL string.
 *
 * @param node The root node of the tree.
 * @param union_str The string representation of the tree.
 */
ReturnCode_t bitmask_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& bitset_str) noexcept;

/**
 * @brief Converts a tree with a \c TK_BITSET root to an IDL string.
 *
 * @param node The root node of the tree.
 * @param union_str The string representation of the tree.
 */
ReturnCode_t bitset_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& bitset_str) noexcept;

/**
 * @brief Converts a tree with a \c TK_ENUM root to an IDL string.
 *
 * @param node The root node of the tree.
 * @param enum_str The string representation of the tree.
 */
ReturnCode_t enum_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& enum_str) noexcept;

/**
 * @brief Converts a tree with a \c TK_STRUCTURE root to an IDL string.
 *
 * @param node The root node of the tree.
 * @param struct_str The string representation of the tree.
 */
ReturnCode_t struct_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& struct_str) noexcept;

/**
 * @brief Converts a tree with a \c TK_UNION root to an IDL string.
 *
 * @param node The root node of the tree.
 * @param union_str The string representation of the tree.
 */
ReturnCode_t union_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& union_str) noexcept;

/**
 * @brief Converts a simple tree to an IDL string.
 *
 * @param node The root node of the tree.
 * @param node_str The string representation of the tree.
 */
ReturnCode_t node_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& node_str) noexcept;

/**
 * @brief Finds the default type kind for a variable with a given size.
 *
 * @param size The size of the variable.
 * @param default_type The default type kind.
 */
ReturnCode_t get_default_type_kind(
        const std::uint32_t size,
        TypeKind& default_type) noexcept;

} // dds
} // fastdds
} // eprosima

#include "dyn_type_tree.ipp"

#endif // _FASTDDS_DDS_XTYPES_DYN_TYPE_TREE_HPP_
