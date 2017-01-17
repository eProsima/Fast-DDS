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
 * @file RTPSMessageGroup.cpp
 *
 */

#include <fastrtps/rtps/messages/RTPSMessageGroup.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include <fastrtps/rtps/flowcontrol/FlowController.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>
#include "../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

bool sort_changes_group (CacheChange_t* c1,CacheChange_t* c2)
{
    return(c1->sequenceNumber < c2->sequenceNumber);
}

bool sort_SeqNum(const SequenceNumber_t& s1,const SequenceNumber_t& s2)
{
    return(s1 < s2);
}

typedef std::pair<SequenceNumber_t,SequenceNumberSet_t> pair_T;

void prepare_SequenceNumberSet(std::vector<SequenceNumber_t>& changesSeqNum,
        std::vector<pair_T>& sequences)
{
    //First compute the number of GAP messages we need:
    std::sort(changesSeqNum.begin(), changesSeqNum.end(), sort_SeqNum);
    bool new_pair = true;
    bool seqnumset_init = false;
    uint32_t count = 0;
    for(auto it = changesSeqNum.begin();
            it!=changesSeqNum.end();++it)
    {
        if(new_pair)
        {
            SequenceNumberSet_t seqset;
            seqset.base = (*it) + 1; // IN CASE IN THIS SEQNUMSET there is only 1 number.
            pair_T pair(*it,seqset);
            sequences.push_back(pair);
            new_pair = false;
            seqnumset_init = true;
            count = 1;
            continue;
        }
        if((*it - sequences.back().first).low == count) //CONTINUOUS FROM THE START
        {
            ++count;
            sequences.back().second.base = (*it)+1;
            continue;
        }
        else
        {
            if(seqnumset_init) //FIRST TIME SINCE it was continuous
            {
                sequences.back().second.base = *(it-1);
                seqnumset_init = false;
            }
            // Try to add, If it fails the diference between *it and base is greater than 255.
            if(sequences.back().second.add((*it)))
                continue;
            else
            {
                // Process again the sequence number in a new pair in next loop.
                --it;
                new_pair = true;
            }
        }
    }
}

bool compare_remote_participants(const std::vector<GuidPrefix_t>& remote_participants1,
        const std::vector<GuidPrefix_t>& remote_participants2)
{
    if(remote_participants1.size() == remote_participants2.size())
    {
        bool returnedValue = true;

        for(auto it1 = remote_participants1.begin(); returnedValue && it1 != remote_participants1.end(); ++it1)
        {
            returnedValue = false;

            for(auto it2 = remote_participants2.begin(); !returnedValue && it2 != remote_participants2.end(); ++it2)
            {
                if(*it1 == *it2)
                    returnedValue = true;
            }
        }
    }

    return false;
}

RTPSMessageGroup::RTPSMessageGroup(RTPSParticipantImpl* participant, Endpoint* endpoint,
        RTPSMessageGroup_t& msg_group) :
    participant_(participant), endpoint_(endpoint), full_msg_(&msg_group.rtpsmsg_fullmsg_),
    submessage_msg_(&msg_group.rtpsmsg_submessage_), current_dst_(GuidPrefix_t::unknown())
{
    assert(participant);
    assert(endpoint);

    // Init RTPS message.
    reset_to_header();

    CDRMessage::initCDRMsg(submessage_msg_);
}

RTPSMessageGroup::~RTPSMessageGroup()
{
    send();
}

void RTPSMessageGroup::reset_to_header()
{
    CDRMessage::initCDRMsg(full_msg_);
    full_msg_->pos = RTPSMESSAGE_HEADER_SIZE;
    full_msg_->length = RTPSMESSAGE_HEADER_SIZE;
}

bool RTPSMessageGroup::check_preconditions(const GuidPrefix_t& dst, const LocatorList_t& locator_list,
        const std::vector<GuidPrefix_t>& remote_participants) const
{
    return current_dst_ == dst && locator_list == current_locators_ &&
        (!participant_->is_rtps_protected() || !endpoint_->supports_rtps_protection() ||
         compare_remote_participants(remote_participants, current_remote_participants_));
}

void RTPSMessageGroup::flush()
{
    send();

    reset_to_header();
}

void RTPSMessageGroup::send()
{
    // Reset this buffer because can be used as auxiliary buffer.
    CDRMessage::initCDRMsg(submessage_msg_);

    if(full_msg_->length > RTPSMESSAGE_HEADER_SIZE)
    {
        // TODO(Ricardo) Control message size if it will be encrypted.
        if(participant_->is_rtps_protected() && endpoint_->supports_rtps_protection())
        {
            participant_->security_manager().encode_rtps_message(*full_msg_, current_remote_participants_);
        }

        for(const auto& lit : current_locators_)
            participant_->sendSync(full_msg_, endpoint_, lit);
    }
}

void RTPSMessageGroup::flush_and_reset(const GuidPrefix_t& dst, const LocatorList_t& locator_list,
        const std::vector<GuidPrefix_t>& remote_participants)
{
    // Flush
    flush();

    // Reset
    current_dst_ = dst;
    current_locators_ = locator_list;
    current_remote_participants_ = remote_participants;

    // Maybe add INFO_DST
    if(current_dst_ != GuidPrefix_t::unknown())
        RTPSMessageCreator::addSubmessageInfoDST(full_msg_, current_dst_);
}

void RTPSMessageGroup::check_and_maybe_flush(const GuidPrefix_t& dst, const LocatorList_t& locator_list,
        const std::vector<GuidPrefix_t>& remote_participants)
{
    if(!check_preconditions(dst, locator_list, remote_participants))
        flush_and_reset(dst, locator_list, remote_participants);
}

bool RTPSMessageGroup::add_data(const CacheChange_t& change, const GuidPrefix_t& remoteGuidPrefix,
        const EntityId_t& readerId, const LocatorList_t& locators,
        const std::vector<GuidPrefix_t>& remote_participants, bool expectsInlineQos)
{
    logInfo(RTPS_WRITER,"Sending relevant changes as DATA/DATA_FRAG messages");

    // Check preconditions. If fail flush and reset.
    check_and_maybe_flush(remoteGuidPrefix, locators, remote_participants);

    // Init submessage buffer.
    CDRMessage::initCDRMsg(submessage_msg_);

    // Insert INFO_TS submessage.
    // TODO (Ricardo) Source timestamp maybe has marked when user call write function.
    if(!RTPSMessageCreator::addSubmessageInfoTS_Now(submessage_msg_, false)) //Change here to add a INFO_TS for DATA.
    {
        logError(RTPS_WRITER, "Cannot add INFO_TS submsg to the CDRMessage. Buffer too small");
        return false;
    }

    ParameterList_t* inlineQos = NULL;
    if(expectsInlineQos)
    {
        //TODOG INLINEQOS
        //if(W->getInlineQos()->m_parameters.size()>0)
        //    inlineQos = W->getInlineQos();
    }

    if(!RTPSMessageCreator::addSubmessageData(submessage_msg_, &change, endpoint_->getAttributes()->topicKind,
                readerId, expectsInlineQos, inlineQos))
    {
        logError(RTPS_WRITER, "Cannot add DATA submsg to the CDRMessage. Buffer too small");
        return false;
    }

    if(!CDRMessage::appendMsg(full_msg_, submessage_msg_))
    {
        // Retry
        flush();

        if(!CDRMessage::appendMsg(full_msg_, submessage_msg_))
        {
            logError(RTPS_WRITER,"Cannot add RTPS submmesage to the CDRMessage. Buffer too small");
            return false;
        }
    }

    return true;
}

bool RTPSMessageGroup::add_data_frag(const CacheChange_t& change, const uint32_t fragment_number,
        const GuidPrefix_t& remoteGuidPrefix, const EntityId_t& readerId,
        const LocatorList_t& locators, const std::vector<GuidPrefix_t>& remote_participants, bool expectsInlineQos)
{
    logInfo(RTPS_WRITER,"Sending relevant changes as DATA/DATA_FRAG messages");

    // Check preconditions. If fail flush and reset.
    check_and_maybe_flush(remoteGuidPrefix, locators, remote_participants);

    // Init submessage buffer.
    CDRMessage::initCDRMsg(submessage_msg_);

    // Insert INFO_TS submessage.
    // TODO (Ricardo) Source timestamp maybe has marked when user call write function.
    if(!RTPSMessageCreator::addSubmessageInfoTS_Now(submessage_msg_, false)) //Change here to add a INFO_TS for DATA.
    {
        logError(RTPS_WRITER, "Cannot add INFO_TS submsg to the CDRMessage. Buffer too small");
        return false;
    }

    ParameterList_t* inlineQos = NULL;
    if(expectsInlineQos)
    {
        //TODOG INLINEQOS
        //if(W->getInlineQos()->m_parameters.size()>0)
        //    inlineQos = W->getInlineQos();
    }

    if(RTPSMessageCreator::addSubmessageDataFrag(submessage_msg_, &change, fragment_number,
                endpoint_->getAttributes()->topicKind, readerId, expectsInlineQos, inlineQos))
    {
        logError(RTPS_WRITER, "Cannot add DATA_FRAG submsg to the CDRMessage. Buffer too small");
        return false;
    }

    if(!CDRMessage::appendMsg(full_msg_, submessage_msg_))
    {
        // Retry
        flush();

        if(!CDRMessage::appendMsg(full_msg_, submessage_msg_))
        {
            logError(RTPS_WRITER,"Cannot add RTPS submmesage to the CDRMessage. Buffer too small");
            return false;
        }
    }

    return true;
}

// TODO (Ricardo) Check with standard 8.3.7.4.5
bool RTPSMessageGroup::add_gap(std::vector<SequenceNumber_t>& changesSeqNum,
        const GuidPrefix_t& remoteGuidPrefix, const EntityId_t& readerId,
        LocatorList_t& locators)
{
    // Check preconditions. If fail flush and reset.
    check_and_maybe_flush(remoteGuidPrefix, locators, std::vector<GuidPrefix_t>{remoteGuidPrefix});

    std::vector<pair_T> Sequences;
    prepare_SequenceNumberSet(changesSeqNum, Sequences);
    std::vector<pair_T>::iterator seqit = Sequences.begin();

    uint16_t gap_n = 1;
    bool returnedValue = true;

    while(gap_n < Sequences.size()) //There is still a message to add
    {
        CDRMessage::initCDRMsg(submessage_msg_);
        if(!RTPSMessageCreator::addSubmessageGap(submessage_msg_, seqit->first, seqit->second,
                readerId, endpoint_->getGuid().entityId))
        {
            logError(RTPS_WRITER, "Cannot add GAP submsg to the CDRMessage. Buffer too small");
            returnedValue = false;
            break;
        }

        if(!CDRMessage::appendMsg(full_msg_, submessage_msg_))
        {
            // Retry
            flush();

            if(!CDRMessage::appendMsg(full_msg_, submessage_msg_))
            {
                logError(RTPS_WRITER,"Cannot add RTPS submmesage to the CDRMessage. Buffer too small");
                returnedValue = false;
                break;
            }
        }

        ++gap_n;
        ++seqit;
    }

    return true;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
