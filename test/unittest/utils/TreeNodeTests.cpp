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

#include <array>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include <utils/collections/TreeNode.hpp>

using namespace eprosima;


struct TreeNodeTestType
{
    TreeNodeTestType(
            const std::string& name)
        : name(name)
    {
    }

    const std::string name;
};

/**
 * @brief Test that a tree can support one shallow branch.
 *
 *  0
 *  │
 *  ▼
 *  1
 */
TEST(TreeNodeTests, one_shallow_branch)
{
    constexpr auto DEPTH = 1U;
    constexpr auto NODE_COUNT = 2U;
    constexpr auto BRANCH_COUNT = 1U;

    std::vector<TreeNodeTestType> nodes;
    std::vector<utilities::collections::TreeNode<TreeNodeTestType>> trees;

    // Create the nodes and the trees
    for (unsigned int i = 0; i < NODE_COUNT; i++)
    {
        nodes.emplace_back(std::to_string(i));
        trees.emplace_back(nodes[i]);
    }

    // Build the tree
    auto& root = trees[0];

    root.add_branch(trees[1]);

    // Verify the depth
    const auto tree_depth = root.depth();
    ASSERT_EQ(tree_depth, DEPTH);

    // Verify the nodes are correct
    const auto tree_nodes = root.all_nodes();
    ASSERT_EQ(tree_nodes.size(), NODE_COUNT - 1);

    // Verify the branch count
    const auto tree_branches = root.branches();
    ASSERT_EQ(tree_branches.size(), BRANCH_COUNT);

    // Verify that the branch is a leaf
    ASSERT_TRUE(tree_branches.front().leaf());
}

/**
 * @brief Test that a tree can support one deep branch.
 *
 *  0
 *  │
 *  ▼
 *  1
 *  │
 *  ▼
 *  2
 *  │
 *  ▼
 *  3
 *  │
 *  ▼
 *  4
 *  │
 *  ▼
 *  5
 */
TEST(TreeNodeTests, one_deep_branch)
{
    constexpr auto DEPTH = 5U;
    constexpr auto NODE_COUNT = 6U;
    constexpr auto BRANCH_COUNT = 1U;

    std::vector<TreeNodeTestType> nodes;
    std::vector<utilities::collections::TreeNode<TreeNodeTestType>> trees;

    // Create the nodes and the trees
    for (unsigned int i = 0; i < NODE_COUNT; i++)
    {
        nodes.emplace_back(std::to_string(i));
        trees.emplace_back(nodes[i]);
    }

    // Build the tree
    for (int i = NODE_COUNT - 1; i >= 1; i--)
    {
        trees[i - 1].add_branch(trees[i]);
    }

    const auto& root = trees[0];

    // Verify the depth
    const auto tree_depth = root.depth();
    ASSERT_EQ(tree_depth, DEPTH);

    // Verify the nodes are correct
    const auto tree_nodes = root.all_nodes();
    ASSERT_EQ(tree_nodes.size(), NODE_COUNT - 1);

    // Verify the branch count
    const auto tree_branches = root.branches();
    ASSERT_EQ(tree_branches.size(), BRANCH_COUNT);

    // Verify that the branch is not a leaf
    ASSERT_FALSE(tree_branches.front().leaf());
}

/**
 * @brief Test that a tree can support many shallow branches.
 *
 * ┌───┬───0───┬───┐
 * │   │   │   │   │
 * ▼   ▼   ▼   ▼   ▼
 * 1   2   3   4   5
 */
TEST(TreeNodeTests, many_shallow_branches)
{
    constexpr auto DEPTH = 1U;
    constexpr auto NODE_COUNT = 6U;
    constexpr auto BRANCH_COUNT = 5U;

    std::vector<TreeNodeTestType> nodes;
    std::vector<utilities::collections::TreeNode<TreeNodeTestType>> trees;

    // Create the nodes and the trees
    for (unsigned int i = 0; i < NODE_COUNT; i++)
    {
        nodes.emplace_back(std::to_string(i));
        trees.emplace_back(nodes[i]);
    }

    // Build the tree
    auto& root = trees[0];

    for (unsigned int i = 1; i < NODE_COUNT; i++)
    {
        root.add_branch(nodes[i]);
    }

    // Verify the depth
    const auto tree_depth = root.depth();
    ASSERT_EQ(tree_depth, DEPTH);

    // Verify the node count
    const auto tree_nodes = root.all_nodes();
    ASSERT_EQ(tree_nodes.size(), NODE_COUNT - 1);

    // Verify the node order
    static const std::array<std::string, NODE_COUNT - 1> NODES_ORDER{"1", "2", "3", "4", "5"};
    int i = 0;

    for (const auto& node : tree_nodes)
    {
        ASSERT_EQ(node.info.name, NODES_ORDER[i]);
        i++;
    }

    // Verify the branch count
    const auto tree_branches = root.branches();
    ASSERT_EQ(tree_branches.size(), BRANCH_COUNT);

    // Verify the branch order
    i = 0;

    for (const auto& child : tree_branches)
    {
        ASSERT_EQ(child.info.name, NODES_ORDER[i]);
        i++;
    }

    // Verify that every branch is a leaf
    for (const auto& branch : tree_branches)
    {
        ASSERT_TRUE(branch.leaf());
    }
}

/**
 * @brief Test that a tree can support a binary tree.
 *
 *     ┌───0───┐
 *     │       │
 *     ▼       ▼
 *  ┌──1──┐ ┌──2──┐
 *  │     │ │     │
 *  ▼     ▼ ▼     ▼
 *  3     4 5     6
 */
TEST(TreeNodeTests, binary_tree)
{
    constexpr auto DEPTH = 2U;
    constexpr auto NODE_COUNT = 7U;
    constexpr auto BRANCH_COUNT = 2U;

    std::vector<TreeNodeTestType> nodes;
    std::vector<utilities::collections::TreeNode<TreeNodeTestType>> trees;

    // Create the nodes and the trees
    for (unsigned int i = 0; i < NODE_COUNT; i++)
    {
        nodes.emplace_back(std::to_string(i));
        trees.emplace_back(nodes[i]);
    }

    // Build the tree
    trees[1].add_branch(trees[3]);
    trees[1].add_branch(trees[4]);

    trees[2].add_branch(trees[5]);
    trees[2].add_branch(trees[6]);

    trees[0].add_branch(trees[1]);
    trees[0].add_branch(trees[2]);

    const auto& root = trees[0];

    // Verify the depth
    const auto tree_depth = root.depth();
    ASSERT_EQ(tree_depth, DEPTH);

    // Verify the node count
    const auto tree_nodes = root.all_nodes();
    ASSERT_EQ(tree_nodes.size(), NODE_COUNT - 1);

    // Verify the node order
    const std::array<std::string, NODE_COUNT - 1> NODES_ORDER{"3", "4", "1", "5", "6", "2"};
    int i = 0;

    for (const auto& node : tree_nodes)
    {
        ASSERT_EQ(node.info.name, NODES_ORDER[i]);
        i++;
    }

    // Verify the branch count
    const auto tree_branches = root.branches();
    ASSERT_EQ(tree_branches.size(), BRANCH_COUNT);

    // Verify the branch order
    ASSERT_EQ(tree_branches.front().info.name, "1");
    ASSERT_EQ(tree_branches.back().info.name, "2");

    // Verify that every branch is not a leaf
    for (const auto& branch : tree_branches)
    {
        // Verify the branch count
        const auto branch_branches = branch.branches();
        ASSERT_EQ(branch_branches.size(), BRANCH_COUNT);

        // Verify that every branch is a leaf
        for (const auto& branch_branch : branch_branches)
        {
            ASSERT_TRUE(branch_branch.leaf());
        }
    }
}


int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
