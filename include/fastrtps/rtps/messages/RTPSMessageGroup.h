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

#include <vector>
#include <cassert>

namespace eprosima {
    namespace fastrtps{
        namespace rtps {

            class CacheChangeForGroup_t
            {
                public:

                    CacheChangeForGroup_t(const CacheChange_t* change) : change_(change),
                    last_fragment_number_(0)
                {
                }

                    CacheChangeForGroup_t(const CacheChangeForGroup_t& c) : change_(c.change_),
                    last_fragment_number_(c.last_fragment_number_)
                {
                }

                    const CacheChange_t* getChange() const
                    {
                        return change_;
                    }

                    bool isFragmented() const
                    {
                        return change_->getFragmentSize() != 0;
                    }

                    uint32_t getLastFragmentNumber() const
                    {
                        return last_fragment_number_;
                    }

                    void setLastFragmentNumber(uint32_t fragment_number)
                    {
                        last_fragment_number_ = fragment_number;
                    }

                    uint32_t increaseLastFragmentNumber()
                    {
                        assert(isFragmented());
                        return ++last_fragment_number_;
                    }


                private:

                    const CacheChange_t* change_;
                    uint32_t last_fragment_number_;
            };

            /**
             * Class RTPSMessageGroup_t that contains the messages used to send multiples changes as one message.
             * @ingroup WRITER_MODULE
             */
            class RTPSMessageGroup_t{
                public:
                    CDRMessage_t m_rtpsmsg_header;
                    CDRMessage_t m_rtpsmsg_submessage;
                    CDRMessage_t m_rtpsmsg_fullmsg;
                    RTPSMessageGroup_t(uint32_t payload):
                        m_rtpsmsg_header(RTPSMESSAGE_HEADER_SIZE),
                        m_rtpsmsg_submessage(payload+RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE),
                        m_rtpsmsg_fullmsg(payload+RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE){};
            };

            class RTPSWriter;

            /**
             * RTPSMessageGroup class used to send multiple changes as a single CDRMessage.
             * @ingroup WRITER_MODULE
             */
            class RTPSMessageGroup {
                public:

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
                static bool send_Changes_AsGap(RTPSMessageGroup_t* msg_group,
                        RTPSWriter* W,
                        std::vector<SequenceNumber_t>* changesSeqNum,
                        const GuidPrefix_t& remoteGuidPrefix,
                        const EntityId_t& readerId,
                        LocatorList_t* unicast,
                        LocatorList_t* multicast);

                /**
                 * @param changesSeqNum
                 * @param Sequences
                 */
                static void prepare_SequenceNumberSet(std::vector<SequenceNumber_t>* changesSeqNum,
                        std::vector<std::pair<SequenceNumber_t,SequenceNumberSet_t>>* Sequences);

                /**
                 * @param msg_group
                 * @param W
                 * @param changes
                 * @param remoteGuidPrefix
                 * @param unicast
                 * @param multicast
                 * @param expectsInlineQos
                 * @param ReaderId
                 * @return 
                 */
                static uint32_t send_Changes_AsData(RTPSMessageGroup_t* msg_group,
                        RTPSWriter* W,
                        std::vector<CacheChangeForGroup_t>& changes,
                        const GuidPrefix_t& remoteGuidPrefix,
                        const EntityId_t& ReaderId,
                        LocatorList_t& unicast,
                        LocatorList_t& multicast,
                        bool expectsInlineQos);

                /**
                 * @param msg_group
                 * @param W
                 * @param changes
                 * @param remoteGuidPrefix
                 * @param loc
                 * @param expectsInlineQos
                 * @param ReaderId
                 * @return 
                 */
                static uint32_t send_Changes_AsData(RTPSMessageGroup_t* msg_group,
                        RTPSWriter* W,
                        std::vector<CacheChangeForGroup_t>& changes,
                        const GuidPrefix_t& remoteGuidPrefix,
                        const EntityId_t& ReaderId,
                        const Locator_t& loc,
                        bool expectsInlineQos);

                /**
                 * @param W
                 * @param submsg
                 * @param expectsInlineQos
                 * @param change
                 * @param ReaderId
                 * @return 
                 */
                static void prepareDataSubM(RTPSWriter* W, CDRMessage_t* submsg, bool expectsInlineQos, const CacheChange_t* change, const EntityId_t& ReaderId);

                static void prepareDataFragSubM(RTPSWriter* W, CDRMessage_t* submsg, bool expectsInlineQos, const CacheChange_t* change, const EntityId_t& ReaderId, uint32_t fragment_number);
            };
        } /* namespace rtps */
    } /* namespace fastrtps */
} /* namespace eprosima */

#endif
#endif /* RTPSMESSAGEGROUP_H_ */
