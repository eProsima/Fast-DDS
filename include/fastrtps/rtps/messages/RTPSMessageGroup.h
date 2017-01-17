// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RTPSMessageGroup.h
 *
 */

#ifndef RTPSMESSAGEGROUP_H_
#define RTPSMESSAGEGROUP_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "../common/CDRMessage_t.h"
#include "../../qos/ParameterList.h"
#include <fastrtps/rtps/common/FragmentNumber.h>

#include <vector>
#include <cassert>

namespace eprosima {
namespace fastrtps{
namespace rtps {

class RTPSParticipantImpl;
class Endpoint;

// TODO(Ricardo) Remove
class CacheChangeForGroup_t
{
    public:

        CacheChangeForGroup_t(const CacheChange_t* change) : 
            change_(change)
    {
        fragments_cleared_for_sending_.base = 0;
        for (uint32_t i = 1; i != change->getFragmentCount() + 1; i++)
            fragments_cleared_for_sending_.add(i); // Indexed on 1

    }

        CacheChangeForGroup_t(const CacheChangeForGroup_t& c) : 
            change_(c.change_),
            fragments_cleared_for_sending_(c.fragments_cleared_for_sending_)
    {
    }

        CacheChangeForGroup_t(const ChangeForReader_t& c) : 
            change_(c.getChange()),
            fragments_cleared_for_sending_(c.getUnsentFragments())
    {
    }

        const CacheChangeForGroup_t& operator=(const CacheChangeForGroup_t& c)
        { 
            change_ = c.change_;
            fragments_cleared_for_sending_ = c.fragments_cleared_for_sending_;
            return *this;
        }

        const CacheChange_t* getChange() const
        {
            return change_;
        }

        bool isFragmented() const
        {
            return change_->getFragmentSize() != 0;
        }

        const FragmentNumberSet_t& getFragmentsClearedForSending() const
        {
            return fragments_cleared_for_sending_;
        }

        void setFragmentsClearedForSending(FragmentNumberSet_t fragments)
        {
            fragments_cleared_for_sending_ = fragments;
        }

    private:
        const CacheChange_t* change_;

        FragmentNumberSet_t fragments_cleared_for_sending_;
};

/**
 * Class RTPSMessageGroup_t that contains the messages used to send multiples changes as one message.
 * @ingroup WRITER_MODULE
 */
class RTPSMessageGroup_t
{
    public:

        RTPSMessageGroup_t(uint32_t payload):
            rtpsmsg_submessage_(payload),
            rtpsmsg_fullmsg_(payload) {}

        CDRMessage_t rtpsmsg_submessage_;

        CDRMessage_t rtpsmsg_fullmsg_;
};

class RTPSWriter;

/**
 * RTPSMessageGroup Class used to construct a RTPS message.
 * @ingroup WRITER_MODULE
 */
class RTPSMessageGroup
{
    public:

        RTPSMessageGroup(RTPSParticipantImpl* participant, Endpoint* endpoint,
                RTPSMessageGroup_t& msg_group);

        ~RTPSMessageGroup();

        bool add_data(const CacheChange_t& change, const GuidPrefix_t& remoteGuidPrefix, const EntityId_t& readerId,
                const LocatorList_t& locators, const std::vector<GuidPrefix_t>& remote_participants,
                bool expectsInlineQos);

        bool add_data_frag(const CacheChange_t& change, const uint32_t fragment_number,
                const GuidPrefix_t& remoteGuidPrefix, const EntityId_t& readerId,
                const LocatorList_t& locators, const std::vector<GuidPrefix_t>& remote_participants,
                bool expectsInlineQos);

        /**
         * @param msg_group
         * @param W
         * @param changesSeqNum
         * @param remoteGuidPrefix
         * @param readerId
         * @param unicast
         * @param multicast
         * @return
         */
        bool add_gap(std::vector<SequenceNumber_t>& changesSeqNum,
                const GuidPrefix_t& remoteGuidPrefix,
                const EntityId_t& readerId,
                LocatorList_t& locators);

    private:

        void reset_to_header();

        bool check_preconditions(const GuidPrefix_t& dst, const LocatorList_t& locator_list,
                const std::vector<GuidPrefix_t>& remote_participants) const;

        void flush_and_reset(const GuidPrefix_t& dst, const LocatorList_t& locator_list,
                const std::vector<GuidPrefix_t>& remote_participants);

        void flush();

        void send();

        void check_and_maybe_flush(const GuidPrefix_t& dst, const LocatorList_t& locator_list,
                const std::vector<GuidPrefix_t>& remote_participants);

        RTPSParticipantImpl* participant_;

        Endpoint* endpoint_;

        CDRMessage_t* full_msg_;

        CDRMessage_t* submessage_msg_;

        GuidPrefix_t current_dst_;

        LocatorList_t current_locators_;

        std::vector<GuidPrefix_t> current_remote_participants_;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
#endif /* RTPSMESSAGEGROUP_H_ */
