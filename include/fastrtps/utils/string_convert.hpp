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

/*!
 * @file string_convert.hpp
 *
 * std::wstring_convert deprecation on c++17 forces us to provide our own conversions.
 */

#ifndef FASTRTPS_UTILS_STRING_CONVERT_HPP_
#define FASTRTPS_UTILS_STRING_CONVERT_HPP_

#include <string>

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
namespace eprosima {
namespace fastrtps {

std::wstring wstring_from_bytes(
        const std::string& str);
std::string wstring_to_bytes(
        const std::wstring& str);

} /* namespace fastrtps */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif /* FASTRTPS_UTILS_STRING_CONVERT_HPP_ */
