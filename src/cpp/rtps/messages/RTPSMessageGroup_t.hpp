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
 * @file RTPSMessageGroup_t.hpp
 */

 #ifndef RTPS_MESSAGES_RTPSMESSAGEGROUP_T_HPP
 #define RTPS_MESSAGES_RTPSMESSAGEGROUP_T_HPP

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/rtps/common/CDRMessage_t.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

 /**
 * Class RTPSMessageGroup_t that contains the messages used to send multiples changes as one message.
 * @ingroup WRITER_MODULE
 */
class RTPSMessageGroup_t
{
public:

    RTPSMessageGroup_t(
#if HAVE_SECURITY
            bool has_security,
#endif
            uint32_t payload,
            const GuidPrefix_t& participant_guid)
        : rtpsmsg_submessage_(0u)
        , rtpsmsg_fullmsg_(0u)
#if HAVE_SECURITY
        , rtpsmsg_encrypt_(0u)
#endif
    {
        rtpsmsg_fullmsg_.reserve(payload);
        rtpsmsg_submessage_.reserve(payload);

#if HAVE_SECURITY
        if (has_security)
        {
            rtpsmsg_encrypt_.reserve(payload);
        }
#endif

        init(participant_guid);
    }

    RTPSMessageGroup_t(
            octet* buffer_ptr,
#if HAVE_SECURITY
            bool has_security,
#endif
            uint32_t payload,
            const GuidPrefix_t& participant_guid)
        : rtpsmsg_submessage_(0u)
        , rtpsmsg_fullmsg_(0u)
#if HAVE_SECURITY
        , rtpsmsg_encrypt_(0u)
#endif
    {
        rtpsmsg_fullmsg_.init(buffer_ptr, payload);
        buffer_ptr += payload;
        rtpsmsg_submessage_.init(buffer_ptr, payload);

#if HAVE_SECURITY
        if (has_security)
        {
            buffer_ptr += payload;
            rtpsmsg_encrypt_.init(buffer_ptr, payload);
        }
#endif

        init(participant_guid);
    }

    inline void init(
            const GuidPrefix_t& participant_guid)
    {
        CDRMessage::initCDRMsg(&rtpsmsg_fullmsg_);
        RTPSMessageCreator::addHeader(&rtpsmsg_fullmsg_, participant_guid);
    }

    CDRMessage_t rtpsmsg_submessage_;

    CDRMessage_t rtpsmsg_fullmsg_;

#if HAVE_SECURITY
    CDRMessage_t rtpsmsg_encrypt_;
#endif
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // RTPS_MESSAGES_RTPSMESSAGEGROUP_T_HPP

