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
 * @file VendorId_t.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__VENDORID_T_HPP
#define FASTDDS_RTPS_COMMON__VENDORID_T_HPP

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace eprosima {
namespace fastdds {
namespace rtps {

//!@brief Structure VendorId_t, specifying the vendor Id of the implementation.
using VendorId_t = std::array<uint8_t, 2>;

const VendorId_t c_VendorId_Unknown = {0x00, 0x00};
const VendorId_t c_VendorId_eProsima = {0x01, 0x0F};
const VendorId_t c_VendorId_rti_connext = {0x01, 0x01};
const VendorId_t c_VendorId_opendds = {0x01, 0x03};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

/**
 * @brief ostream operator<< for VendorId_t
 *
 * @param output: the output stream
 * @param vendor_id: the vendor id to append to the output stream
 */
inline std::ostream& operator <<(
        std::ostream& output,
        eprosima::fastdds::rtps::VendorId_t vendor_id)
{
    output << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(vendor_id[0]) << " 0x" <<
        std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(vendor_id[1]);
    return output;
}

#endif // FASTDDS_RTPS_COMMON__VENDORID_T_HPP
