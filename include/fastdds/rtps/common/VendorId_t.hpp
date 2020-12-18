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

#ifndef _FASTDDS_RTPS_COMMON_VENDORIDT_HPP_
#define _FASTDDS_RTPS_COMMON_VENDORIDT_HPP_

#include <array>
#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace rtps {

//!@brief Structure VendorId_t, specifying the vendor Id of the implementation.
using VendorId_t = std::array<uint8_t, 2>;

const VendorId_t c_VendorId_Unknown = {0x00, 0x00};
const VendorId_t c_VendorId_eProsima = {0x01, 0x0F};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_RTPS_COMMON_VENDORIDT_HPP_ */
