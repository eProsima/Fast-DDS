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
 * @file ProductVersion_t.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__PRODUCTVERSION_T_HPP
#define FASTDDS_RTPS_COMMON__PRODUCTVERSION_T_HPP

#include <cstdint>
#include <iomanip>
#include <iostream>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct ProductVersion_t
{
    uint8_t major {0};
    uint8_t minor {0};
    uint8_t patch {0};
    uint8_t tweak {0};
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

/**
 * @brief ostream operator<< for ProductVersion_t
 *
 * @param output: the output stream
 * @param product_version: the product version to append to the output stream
 */
inline std::ostream& operator <<(
        std::ostream& output,
        eprosima::fastdds::rtps::ProductVersion_t product_version)
{
    output << static_cast<uint32_t>(product_version.major)
           << "." << static_cast<uint32_t>(product_version.minor)
           << "." << static_cast<uint32_t>(product_version.patch)
           << "." << static_cast<uint32_t>(product_version.tweak);
    return output;
}

#endif /* FASTDDS_RTPS_COMMON__PRODUCTVERSION_T_HPP */
