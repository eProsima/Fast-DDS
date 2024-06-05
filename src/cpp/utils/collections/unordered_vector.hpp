// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file unordered_vector.hpp
 */

#ifndef SRC_CPP_UTILS_COLLECTIONS_UNORDERED_VECTOR_HPP_
#define SRC_CPP_UTILS_COLLECTIONS_UNORDERED_VECTOR_HPP_

#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {
namespace utilities {
namespace collections {

template <
    typename _Ty,
    typename _Alloc = std::allocator<_Ty>>
using unordered_vector = eprosima::fastdds::ResourceLimitedVector<
    _Ty, std::false_type, eprosima::fastdds::ResourceLimitedContainerConfig, _Alloc>;

} // namespace collections
} // namespace utilities
} // namespace eprosima

#endif  /* SRC_CPP_UTILS_COLLECTIONS_NODE_SIZE_HELPERS_HPP_ */
