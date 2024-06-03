// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BuiltinEndpoints.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DATA__BUILTINENDPOINTS_HPP
#define FASTDDS_RTPS_BUILTIN_DATA__BUILTINENDPOINTS_HPP

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace rtps {

constexpr uint32_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER             = 0x00000001 << 0;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR              = 0x00000001 << 1;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER             = 0x00000001 << 2;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR              = 0x00000001 << 3;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER            = 0x00000001 << 4;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR             = 0x00000001 << 5;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_ANNOUNCER       = 0x00000001 << 6;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_DETECTOR        = 0x00000001 << 7;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_ANNOUNCER       = 0x00000001 << 8;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_DETECTOR        = 0x00000001 << 9;
constexpr uint32_t BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER        = 0x00000001 << 10;
constexpr uint32_t BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER        = 0x00000001 << 11;
constexpr uint32_t BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_WRITER = 0x00000001 << 12;
constexpr uint32_t BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_READER = 0x00000001 << 13;
constexpr uint32_t BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_WRITER   = 0x00000001 << 14;
constexpr uint32_t BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_READER   = 0x00000001 << 15;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER      = 0x00000001 << 16;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR       = 0x00000001 << 17;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER     = 0x00000001 << 18;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR      = 0x00000001 << 19;
constexpr uint32_t BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_WRITER = 0x00000001 << 20;
constexpr uint32_t BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_READER = 0x00000001 << 21;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER      = 0x00000001 << 26;
constexpr uint32_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR       = 0x00000001 << 27;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DATA__BUILTINENDPOINTS_HPP
