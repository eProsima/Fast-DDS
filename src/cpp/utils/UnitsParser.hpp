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
 * @file UnitsParser.hpp
 */

#ifndef _FASTDDS_UTILS_UNITS_PARSER_HPP_
#define _FASTDDS_UTILS_UNITS_PARSER_HPP_

#include <string>
#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace utils {

/**
 * Converts the string to uppercase.
 *
 * @param [in] st String to convert
 */
void to_uppercase(
        std::string& st) noexcept;

/**
 * Converts a numeric value with units to bytes.
 *
 * @param [in] value Numeric value to convert
 * @param [in] units Units to use for the conversion
 * @return The value in bytes
 */
uint32_t parse_value_and_units(
        std::string& value,
        std::string units);

} /* namespace utils */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif /* _FASTDDS_UTILS_UNITS_PARSER_HPP_ */
