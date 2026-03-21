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
 * @file list_node_size_impl.hpp
 *
 */

#ifndef SRC_CPP_UTILS_COLLECTIONS_IMPL_CUSTOM_LIST_NODE_SIZE_IMPL_HPP_
#define SRC_CPP_UTILS_COLLECTIONS_IMPL_CUSTOM_LIST_NODE_SIZE_IMPL_HPP_

// List nodes usually consist of two pointers followed by value
template<typename T>
struct my_list_node_type
{
    my_list_node_type* prev;
    my_list_node_type* next;
    T value;
};

template<typename T>
struct list_node_size : std::integral_constant<size_t, sizeof(my_list_node_type<T>)>
{
};

#endif  /* SRC_CPP_UTILS_COLLECTIONS_IMPL_CUSTOM_LIST_NODE_SIZE_IMPL_HPP_ */