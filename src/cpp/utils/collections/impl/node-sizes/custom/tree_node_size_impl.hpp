// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file tree_node_size_impl.hpp
 *
 */

#ifndef SRC_CPP_UTILS_COLLECTIONS_IMPL_CUSTOM_TREE_NODE_SIZE_IMPL_HPP_
#define SRC_CPP_UTILS_COLLECTIONS_IMPL_CUSTOM_TREE_NODE_SIZE_IMPL_HPP_

template<typename T>
struct my_tree_node_type
{
    // There is an enum tree_color {false, true} here on libstdc++, we should include it here to
    // ensure there are no alignment issues
    enum color_t
    {
        RED = false,
        BLACK = true
    }
    color;

    // Three pointers on MSVC and libstdc++, two on libc++
    my_tree_node_type* parent;
    my_tree_node_type* left;
    my_tree_node_type* right;

    // There is a bool here on libc++, and two chars on MSVC.
    // We make room for 32 bits in order to be safe with alignments, even though we may consume more
    // memory than strictly required.
    uint32_t other_info;

    // All implementations have the node value at the end.
    T value;
};

template<typename T>
struct tree_node_size : std::integral_constant<size_t, sizeof(my_tree_node_type<T>)>
{
};

#endif  /* SRC_CPP_UTILS_COLLECTIONS_IMPL_CUSTOM_TREE_NODE_SIZE_IMPL_HPP_ */
