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
 * @file Tree.hpp
 *
 */

#ifndef FASTDDS_UTILS_COLLECTIONS_TREE_HPP_
#define FASTDDS_UTILS_COLLECTIONS_TREE_HPP_

#include <list>

namespace eprosima {
namespace utilities {
namespace collections {

/**
 * Generic data struct of with an internal value of type \c Info .
 */
template <typename Info>
struct Node
{
public:

    Node();
    Node(
            const Info& info);
    Node(
            Info&& info);

    template<typename ... Args>
    Node(
            Args... args);

    Info info;
};

/**
 * Class that represents each of the Nodes that form a Tree, including the source and the leaves.
 */
template <typename Info>
class TreeNode : public Node<Info>
{
public:

    using Node<Info>::Node;

    void add_branch(
            const Info& info);
    void add_branch(
            Info&& info);
    void add_branch(
            const TreeNode& node);
    void add_branch(
            TreeNode&& node);

    bool leaf() const noexcept;

    unsigned int depth() const noexcept;

    const std::list<TreeNode>& branches() const noexcept;

    std::list<TreeNode> all_nodes() const noexcept;

protected:

    std::list<TreeNode> branches_;
};

} /* namespace collections */
} /* namespace utilities */
} /* namespace eprosima */

// Include implementation template file
#include "impl/Tree.ipp"

#endif /* FASTDDS_UTILS_COLLECTIONS_TREE_HPP_ */
