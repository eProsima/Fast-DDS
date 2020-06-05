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
 * @file set_node_size_impl.hpp
 */

#ifndef SRC_CPP_UTILS_COLLECTIONS_IMPL_FOONATHAN_SET_NODE_SIZE_IMPL_HPP_
#define SRC_CPP_UTILS_COLLECTIONS_IMPL_FOONATHAN_SET_NODE_SIZE_IMPL_HPP_

/* NOTE:
 *   The way foonathan_memory_node_size_debugger tool works may lead to wrong
 *   results on some platforms due to alignment / sizeof issues.
 *
 *   On MSVC in particular, it was detected that the value returned by
 *   foonathan::memory::set_node_size<std::set<uint32_t[2]>::value_type>
 *   was 4 bytes less than the size being asked to the allocator.
 *
 *   To solve this issue, we add a size_t before the value type, asking for the
 *   node size of std::pair<size_t, value_type>. This will consume more memory
 *   than strictly necessary but will avoid access violation exceptions.
 */

template <typename K>
struct set_node_size : foonathan::memory::set_node_size<std::pair<size_t, typename std::set<K>::value_type> >
{
};

#endif  /* SRC_CPP_UTILS_COLLECTIONS_IMPL_FOONATHAN_SET_NODE_SIZE_IMPL_HPP_ */