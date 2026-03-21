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

/*
 * UnitsParser.cpp
 *
 */

#include <utils/UnitsParser.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <limits>
#include <map>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace utils {

void to_uppercase(
        std::string& st) noexcept
{
    // These lambda has been taken from the notes in https://en.cppreference.com/w/cpp/string/byte/toupper
    // Note how the argument passed is unsigned char to avoid undefined behavior
    auto my_toupper = [](unsigned char c)
            {
                return static_cast<char>(std::toupper(c));
            };
    std::transform(st.begin(), st.end(), st.begin(), my_toupper);
}

uint32_t parse_value_and_units(
        std::string& value,
        std::string units)
{
    static const std::map<std::string, std::uint32_t> magnitudes = {
        {"", 1},
        {"B", 1},
        {"KB", 1000},
        {"MB", 1000 * 1000},
        {"GB", 1000 * 1000 * 1000},
        {"KIB", 1024},
        {"MIB", 1024 * 1024},
        {"GIB", 1024 * 1024 * 1024},
    };

    uint64_t num = 0;
    try
    {
        num = std::stoull(value);
    }
    catch (std::out_of_range&)
    {
        throw std::invalid_argument("Failed to parse value from string." \
                      " The number is too large to be converted to bytes (Max: (2^32)-1 Bytes).");
    }

    to_uppercase(units);

    uint32_t magnitude = 0;
    try
    {
        magnitude = magnitudes.at(units);
    }
    catch (std::out_of_range&)
    {
        throw std::invalid_argument(
                  "The units are not in the expected format. Use: {B, KB, MG, GB, KIB, MIB, GIB}.");
    }

    // Check whether the product of number * magnitude overflows
    if (num > std::numeric_limits<std::uint32_t>::max() / magnitude)
    {
        throw std::invalid_argument("The number is too large to be converted to bytes (Max: (2^32)-1 Bytes).");
    }

    // The explicit cast to uint32_t is safe since the number has already been checked to fit.
    // The product is also safe since the possible overflow has also been checked.
    const std::uint32_t bytes = static_cast<std::uint32_t>(num) * magnitude;

    return bytes;
}

} /* namespace utils */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
