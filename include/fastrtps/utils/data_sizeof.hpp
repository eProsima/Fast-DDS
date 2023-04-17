// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file fixed_size_string.hpp
 *
 */

#ifndef FASTRTPS_UTILS_DATA_OFFSET_HPP_
#define FASTRTPS_UTILS_DATA_OFFSET_HPP_

#include <cstddef>

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
namespace eprosima {
namespace fastrtps {

template<typename T, typename U, typename M>
static constexpr size_t size_of_()
{
    return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(U())))) + sizeof(M);
}

} // namespace fastrtps
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // FASTRTPS_UTILS_DATA_OFFSET_HPP_
