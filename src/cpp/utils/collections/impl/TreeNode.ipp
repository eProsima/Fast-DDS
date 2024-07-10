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

/**
 * @file TreeNode.ipp
 */

#ifndef FASTDDS_UTILS_COLLECTIONS_IMPL__TREE_NODE_IPP
#define FASTDDS_UTILS_COLLECTIONS_IMPL__TREE_NODE_IPP

#include <algorithm>


namespace eprosima {
namespace utilities {
namespace collections {

template <typename Info>
Node<Info>::Node()
{
    // Do nothing
}

template <typename Info>
Node<Info>::Node(
        const Info& inner_info)
    : info(inner_info)
{
    // Do nothing
}

template <typename Info>
Node<Info>::Node(
        Info&& inner_info)
    : info(std::move(inner_info))
{
    // Do nothing
}

template <typename Info>
template<typename ... Args>
Node<Info>::Node(
        Args... args)
    : info(args ...)
{
    // Do nothing
}

template <typename Info>
void TreeNode<Info>::add_branch(
        const Info& inner_info)
{
    branches_.push_back(TreeNode(inner_info));
}

template <typename Info>
void TreeNode<Info>::add_branch(
        Info&& inner_info)
{
    branches_.push_back(TreeNode(std::move(inner_info)));
}

template <typename Info>
void TreeNode<Info>::add_branch(
        const TreeNode& node)
{
    branches_.push_back(node);
}

template <typename Info>
void TreeNode<Info>::add_branch(
        TreeNode&& node)
{
    branches_.push_back(std::move(node));
}

template <typename Info>
bool TreeNode<Info>::leaf() const noexcept
{
    return branches_.empty();
}

template <typename Info>
unsigned int TreeNode<Info>::depth() const noexcept
{
    unsigned int max_child_depth = 0;

    for (const auto& branch : branches_)
    {
        const auto child_depth = branch.depth() + 1;
        max_child_depth = std::max(max_child_depth, child_depth);
    }

    return max_child_depth;
}

template <typename Info>
const std::list<TreeNode<Info>>& TreeNode<Info>::branches() const noexcept
{
    return branches_;
}

template <typename Info>
std::list<TreeNode<Info>> TreeNode<Info>::all_nodes() const noexcept
{
    std::list<TreeNode<Info>> nodes;

    for (const auto& child : branches_)
    {
        auto branch_nodes = child.all_nodes();
        nodes.splice(nodes.end(), branch_nodes);
        nodes.push_back(child);
    }

    return nodes;
}

} /* namespace collections */
} /* namespace utilities */
} /* namespace eprosima */

#endif  /* FASTDDS_UTILS_COLLECTIONS_IMPL__TREE_NODE_IPP */
