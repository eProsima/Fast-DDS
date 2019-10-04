// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RequestHeader.hpp
 *
 */

#ifndef REPLYHEADER_HPP
#define REPLYHEADER_HPP

#include <fastrtps/rtps/common/SampleIdentity.h>
#include <fastdds/dds/builtin/common/Types.hpp>

namespace eprosima {

namespace fastcdr {
class Cdr;
} // fastcdr

namespace fastdds {
namespace dds {
namespace rpc {

struct ReplyHeader
{
    fastrtps::rtps::SampleIdentity relatedRequestId;

    RemoteExceptionCode_t remoteEx;

    RTPS_DllAPI static size_t getCdrSerializedSize(
            const ReplyHeader& data,
            size_t current_alignment = 0);

    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(
            eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined()
    {
        return false;
    }
};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // REPLYHEADER_HPP
