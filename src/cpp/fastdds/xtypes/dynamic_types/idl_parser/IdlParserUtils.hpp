// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSERUTILS_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSERUTILS_HPP

#include <algorithm>
#include <sstream>
#include <vector>
#include <cctype>
#include <string>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {
namespace utils {

/* Some generic utility functions to simplify IDL parsing */

//! Trim a string from start
std::string ltrim(
        const std::string& s)
{
    std::string result = s;
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }));

    return result;
}

//! Trim a string from end
std::string rtrim(
        const std::string& s)
{
    std::string result = s;
    result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }).base(), result.end());

    return result;
}

//! trim a string from both ends
std::string trim(
        const std::string& s)
{
    return rtrim(ltrim(s));
}

//! Convert a string to lower case
//! @warning If the string contains multi-byte characters, this will trigger undefined behavior.
std::string to_lower(
        const std::string& s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            });
    return result;
}

//! Remove a given character from a string
std::string remove_char(
        const std::string& s,
        char c)
{
    std::string result = s;
    result.erase(std::remove(result.begin(), result.end(), c), result.end());
    return result;
}

//! Split a string by a given delimiter
std::vector<std::string> split_string(
        const std::string& str,
        char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream ss(str);
    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

//! Join a vector of strings into a single string with a given delimiter
std::string join_strings(
        const std::vector<std::string>& strings,
        char delimiter)
{
    std::ostringstream oss;
    for (size_t i = 0; i < strings.size(); ++i)
    {
        oss << strings[i];
        oss << delimiter;
    }
    return oss.str();
}

} // namespace utils
} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDLPARSERUTILS_HPP