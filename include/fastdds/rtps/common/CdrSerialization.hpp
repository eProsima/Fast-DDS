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

/**
 * @file CdrSerialization.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__CDRSERIALIZATION_HPP
#define FASTDDS_RTPS_COMMON__CDRSERIALIZATION_HPP

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
//! Default XCDR encoding version used in Fast DDS.
constexpr eprosima::fastcdr::CdrVersion DEFAULT_XCDR_VERSION {eprosima::fastcdr::CdrVersion::XCDRv1};
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__CDRSERIALIZATION_HPP
