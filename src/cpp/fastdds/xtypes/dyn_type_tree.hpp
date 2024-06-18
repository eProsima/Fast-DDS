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

#include "utils/collections/Tree.hpp"

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
            const DynamicType::_ref_type& dynamic_type)
        : member_name(member_name)
        , type_kind_name(type_kind_name)
        , dynamic_type(dynamic_type)
    {
    }

    std::string member_name;
    std::string type_kind_name;
    DynamicType::_ref_type dynamic_type;
};


//////////////////////////
// DYNAMIC TYPE TO TREE //
//////////////////////////

/**
 * TODO
 */
ReturnCode_t dyn_type_to_tree(
        const DynamicType::_ref_type& type,
        const std::string& member_name,
        utilities::collections::TreeNode<TreeNodeType>& node) noexcept;

/**
 * TODO
 */
ReturnCode_t type_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& type_str) noexcept;

/**
 * TODO
 */
ReturnCode_t array_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& array_str) noexcept;

/**
 * TODO
 */
ReturnCode_t sequence_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& sequence_str) noexcept;

/**
 * TODO
 */
ReturnCode_t map_kind_to_str(
        const DynamicType::_ref_type& dyn_type,
        std::string& map_str) noexcept;

/**
 * TODO
 */
ReturnCode_t get_members_sorted(
        const DynamicType::_ref_type& dyn_type,
        std::vector<std::pair<std::string, DynamicType::_ref_type>>& result) noexcept;

/**
 * TODO
 */
ReturnCode_t container_internal_type(
        const DynamicType::_ref_type& dyn_type,
        DynamicType::_ref_type& internal_type) noexcept;

/**
 * TODO
 */
ReturnCode_t container_size(
        const DynamicType::_ref_type& dyn_type,
        BoundSeq& bounds) noexcept;


//////////////////////////////
// DYNAMIC TYPE TREE TO IDL //
//////////////////////////////

/**
 * TODO
 */
ReturnCode_t dyn_type_tree_to_idl(
        const utilities::collections::TreeNode<TreeNodeType>& parent_node,
        std::string& dyn_type_idl) noexcept;

/**
 * TODO
 */
ReturnCode_t struct_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& struct_str) noexcept;

/**
 * TODO
 */
ReturnCode_t enum_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& enum_str) noexcept;

/**
 * TODO
 */
ReturnCode_t union_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& union_str) noexcept;

/**
 * TODO
 */
ReturnCode_t node_to_str(
        const utilities::collections::TreeNode<TreeNodeType>& node,
        std::string& node_str) noexcept;

} // dds
} // fastdds
} // eprosima

#include "dyn_type_tree.ipp"

#endif // _FASTDDS_DDS_XTYPES_DYN_TYPE_TREE_HPP_
