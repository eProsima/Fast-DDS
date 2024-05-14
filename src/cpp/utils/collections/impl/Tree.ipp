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
 * @file Tree.ipp
 *
 */

#ifndef SRC_CPP_UTILS_COLLECTIONS_IMPL_TREE_IPP_
#define SRC_CPP_UTILS_COLLECTIONS_IMPL_TREE_IPP_

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
        const Info& info)
    : info(info)
{
    // Do nothing
}

template <typename Info>
Node<Info>::Node(
        Info&& info)
    : info(std::move(info))
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
        const Info& info)
{
    branches_.push_back(TreeNode(info));
}

template <typename Info>
void TreeNode<Info>::add_branch(
        Info&& info)
{
    branches_.push_back(TreeNode(std::move(info)));
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
    unsigned int max_value = 0;
    for (const auto& b : branches_)
    {
        auto child_value = b.depth();
        max_value = MAX(max_value, child_value);
    }
    return max_value;
}

template <typename Info>
const std::list<TreeNode<Info>>& TreeNode<Info>::branches() const noexcept
{
    return branches_;
}

template <typename Info>
std::list<TreeNode<Info>> TreeNode<Info>::all_nodes() const noexcept
{
    std::list<TreeNode<Info>> result(branches_);
    for (const auto& b : branches_)
    {
        auto b_branches = b.all_nodes();
        result.splice(result.begin(), b_branches);
    }
    return result;
}

} /* namespace collections */
} /* namespace utilities */
} /* namespace eprosima */

#endif  /* SRC_CPP_UTILS_COLLECTIONS_IMPL_TREE_IPP_ */
