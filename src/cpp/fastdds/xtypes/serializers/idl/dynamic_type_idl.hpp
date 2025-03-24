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
 * @file dynamic_type_idl.hpp
 */

#ifndef FASTDDS_DDS_XTYPES_SERIALIZERS_IDL__DYNAMIC_TYPE_IDL_HPP
#define FASTDDS_DDS_XTYPES_SERIALIZERS_IDL__DYNAMIC_TYPE_IDL_HPP

#include <iostream>
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

inline std::ostream& operator <<(
        std::ostream& output,
        const TreeNodeType& info)
{
    output << info.type_kind_name;
    return output;
}

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
 * @param idl The idl representation of the DynamicType.
 */
ReturnCode_t type_kind_to_idl(
        const DynamicType::_ref_type& dyn_type,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a DynamicType of \c TK_ARRAY kind to an idl.
 *
 * @param dyn_type The DynamicType kind to convert.
 * @param array_str The string representation of the DynamicType kind.
 */
ReturnCode_t array_kind_to_idl(
        const DynamicType::_ref_type& dyn_type,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a DynamicType of \c TK_MAP kind to an idl.
 *
 * @param dyn_type The DynamicType to convert.
 * @param idl The idl representation of the DynamicType.
 */
ReturnCode_t map_kind_to_idl(
        const DynamicType::_ref_type& dyn_type,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a DynamicType of \c TK_SEQUENCE kind to an idl.
 *
 * @param dyn_type The DynamicType to convert.
 * @param idl The idl representation of the DynamicType.
 */
ReturnCode_t sequence_kind_to_idl(
        const DynamicType::_ref_type& dyn_type,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a DynamicType of \c TK_STRING8 or \c TK_STRING16 kind to an idl.
 *
 * @param dyn_type The DynamicType to convert.
 * @param idl The idl representation of the DynamicType.
 */
ReturnCode_t string_kind_to_idl(
        const DynamicType::_ref_type& dyn_type,
        std::ostream& idl) noexcept;

//////////////////////////////
// DYNAMIC TYPE TREE TO IDL //
//////////////////////////////

/**
 * @brief Converts a tree to an IDL stream.
 *
 * @param root The root node of the tree.
 * @param idl The idl representation of the tree.
 */
ReturnCode_t dyn_type_tree_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& root,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a tree with a \c TK_ALIAS root to an IDL stream.
 *
 * @param node The root node of the tree.
 * @param idl The idl representation of the tree.
 */
ReturnCode_t alias_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a tree with a \c TK_BITMASK root to an IDL stream.
 *
 * @param node The root node of the tree.
 * @param idl The idl representation of the tree.
 */
ReturnCode_t bitmask_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a tree with a \c TK_BITSET root to an IDL stream.
 *
 * @param node The root node of the tree.
 * @param idl The idl representation of the tree.
 */
ReturnCode_t bitset_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a tree with a \c TK_ENUM root to an IDL stream.
 *
 * @param node The root node of the tree.
 * @param idl The idl representation of the tree.
 */
ReturnCode_t enum_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a tree with a \c TK_STRUCTURE root to an IDL stream.
 *
 * @param node The root node of the tree.
 * @param idl The idl representation of the tree.
 */
ReturnCode_t struct_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a tree with a \c TK_UNION root to an IDL stream.
 *
 * @param node The root node of the tree.
 * @param idl The idl representation of the tree.
 */
ReturnCode_t union_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept;

/**
 * @brief Converts a simple tree to an IDL string.
 *
 * @param node The root node of the tree.
 * @param idl The idl representation of the tree.
 */
ReturnCode_t node_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::ostream& idl) noexcept;

/**
 * @brief Resolves and inserts the opening modules definition of a type (if any), and removes them from \c type_name .
 *
 * @param type_name The full name of the type, including modules (if defined).
 * @param idl The idl representation of the tree.
 *
 * @return The number of modules resolved.
 */
unsigned int open_modules_definition(
        std::string& type_name,
        std::ostream& idl) noexcept;

/**
 * @brief Closes the modules definition for the processed type.
 *
 * @param n_modules The number of modules that need to be closed.
 * @param idl The idl representation of the tree.
 */
void close_modules_definition(
        unsigned int& n_modules,
        std::ostream& idl) noexcept;

/**
 * @brief Returns a string with \c n_tabs concatenated tabs.
 *
 * @param n_tabs The number of tabs to return.
 *
 * @return A string with \c n_tabs concatenated tabs.
 */
std::string tabulate_n(
        const unsigned int& n_tabs) noexcept;

///////////////////////
// AUXILIARY METHODS //
///////////////////////

/**
 * @brief Gathers the \c element_type of the @ref DynamicType.
 *
 * @param dyn_type The DynamicType to gather the \c element_type from.
 * @param element_type The element type of the DynamicType's \c type_descriptor.
 */
ReturnCode_t get_element_type(
        const DynamicType::_ref_type& dyn_type,
        DynamicType::_ref_type& element_type) noexcept;

/**
 * @brief Gathers the \c bounds of the @ref DynamicType.
 *
 * If there is a single bound of LENGTH_UNLIMITED, the bounds will be empty.
 *
 * @param dyn_type The DynamicType to gather the \c bounds from.
 * @param bounds The bounds of the DynamicType's \c type_descriptor.
 */
ReturnCode_t get_bounds(
        const DynamicType::_ref_type& dyn_type,
        BoundSeq& bounds) noexcept;

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

#endif // FASTDDS_DDS_XTYPES_SERIALIZERS_IDL__DYNAMIC_TYPE_IDL_HPP
