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
#include <fastrtps/rtps/writer/RTPSWriter.h>
#include "../participant/RTPSParticipantImpl.h"
#include "../flowcontrol/FlowController.h"

#include <fastrtps/log/Log.h>

#include <algorithm>

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

void prepare_SequenceNumberSet(std::set<SequenceNumber_t>& changesSeqNum,
        std::vector<pair_T>& sequences)
{
    //First compute the number of GAP messages we need:
    bool new_pair = true;
    bool seqnumset_init = false;
    uint32_t count = 0;
    for(auto it = changesSeqNum.begin();
            it!=changesSeqNum.end();++it)
    {
        if(new_pair)
        {
            SequenceNumberSet_t seqset((*it) + 1); // IN CASE IN THIS SEQNUMSET there is only 1 number.
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
            sequences.back().second.base((*it)+1);
            continue;
        }
        else
        {
            if(seqnumset_init) //FIRST TIME SINCE it was continuous
            {
                sequences.back().second.base((*(std::prev(it)) + 1));
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

bool compare_remote_participants(const std::vector<GUID_t>& remote_participants1,
        const std::vector<GuidPrefix_t>& remote_participants2)
{
    if(remote_participants1.size() == remote_participants2.size())
    {
        for(auto& participant : remote_participants1)
        {
            if(std::find(remote_participants2.begin(), remote_participants2.end(), participant.guidPrefix) ==
                    remote_participants2.end())
                return false;
        }

        return true;
    }

    return false;
}

void get_participant_from_endpoint(const GUID_t& endpoint, std::vector<GuidPrefix_t>& participants)
{
    if (std::find(participants.begin(), participants.end(), endpoint.guidPrefix) == participants.end())
        participants.push_back(endpoint.guidPrefix);
}

void get_participants_from_endpoints(const std::vector<GUID_t>& endpoints, std::vector<GuidPrefix_t>& participants)
{
    participants.clear();

    for (const GUID_t& endpoint : endpoints)
    {
        if (std::find(participants.begin(), participants.end(), endpoint.guidPrefix) == participants.end())
            participants.push_back(endpoint.guidPrefix);
    }
}

const EntityId_t& get_entity_id(const std::vector<GUID_t>& endpoints)
{
    if(endpoints.size() == 0)
        return c_EntityId_Unknown;

    const EntityId_t& entityid = endpoints.at(0).entityId;

    for(auto it = endpoints.begin() + 1; it != endpoints.end(); ++it)
        if(entityid != it->entityId)
            return c_EntityId_Unknown;

    return entityid;
}

RTPSMessageGroup::RTPSMessageGroup(RTPSParticipantImpl* participant, Endpoint* endpoint, ENDPOINT_TYPE type,
        RTPSMessageGroup_t& msg_group) :
    participant_(participant), endpoint_(endpoint), full_msg_(&msg_group.rtpsmsg_fullmsg_),
    submessage_msg_(&msg_group.rtpsmsg_submessage_), currentBytesSent_(0),
    fixed_destination_(false), fixed_destination_locators_(nullptr), 
    fixed_destination_guids_(nullptr), fixed_destination_prefix_()
#if HAVE_SECURITY
    , encrypt_msg_(&msg_group.rtpsmsg_encrypt_)
#endif
{
    assert(participant);
    assert(endpoint);
    (void)type;

    // Init RTPS message.
    reset_to_header();

    CDRMessage::initCDRMsg(submessage_msg_);

#if HAVE_SECURITY
    CDRMessage::initCDRMsg(encrypt_msg_);
#endif
}

RTPSMessageGroup::RTPSMessageGroup(RTPSParticipantImpl* participant, Endpoint* endpoint, ENDPOINT_TYPE type,
    RTPSMessageGroup_t& msg_group, const LocatorList_t& locator_list,
    const std::vector<GUID_t>& remote_endpoints) : RTPSMessageGroup(participant, endpoint, type, msg_group)
{
#if HAVE_SECURITY
    if (participant_->security_attributes().is_rtps_protected && endpoint_->supports_rtps_protection())
    {
        get_participants_from_endpoints(remote_endpoints, current_remote_participants_);
    }
#endif

    size_t nEndpoints = remote_endpoints.size();
    if (nEndpoints > 0)
    {
        fixed_destination_prefix_ = remote_endpoints.at(0).guidPrefix;
        for (size_t i = 1; i < nEndpoints; ++i)
        {
            if (fixed_destination_prefix_ != remote_endpoints.at(i).guidPrefix)
            {
                fixed_destination_prefix_ = c_GuidPrefix_Unknown;
                break;
            }
        }
    }
    fixed_destination_locators_ = &locator_list;
    fixed_destination_guids_ = &remote_endpoints;
    fixed_destination_ = true;
}

/*
RTPSMessageGroup::RTPSMessageGroup(RTPSParticipantImpl* participant, Endpoint* endpoint, ENDPOINT_TYPE type,
    RTPSMessageGroup_t& msg_group, const LocatorList_t& locator_list,
    const GUID_t& remote_endpoint) : RTPSMessageGroup(participant, endpoint, type, msg_group)
{
#if HAVE_SECURITY
    if (participant_->security_attributes().is_rtps_protected && endpoint_->supports_rtps_protection())
    {
        get_participant_from_endpoint(remote_endpoint, current_remote_participants_);
    }
#endif

    fixed_destination_prefix_ = remote_endpoint.guidPrefix;
    fixed_destination_locators_ = &locator_list;
    fixed_destination_vector_.push_back(remote_endpoint);
    fixed_destination_ = true;
}
*/

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

bool RTPSMessageGroup::check_preconditions(const LocatorList_t& locator_list,
        const std::vector<GUID_t>& remote_endpoints) const
{
    (void)remote_endpoints;

    return fixed_destination_  ||
        (locator_list == current_locators_
#if HAVE_SECURITY
        && (!participant_->security_attributes().is_rtps_protected || !endpoint_->supports_rtps_protection() ||
         compare_remote_participants(remote_endpoints, current_remote_participants_))
#endif
        );
}

void RTPSMessageGroup::flush()
{
    send();

    reset_to_header();
}

void RTPSMessageGroup::send()
{
    CDRMessage_t* msgToSend = full_msg_;

    if(full_msg_->length > RTPSMESSAGE_HEADER_SIZE)
    {
#if HAVE_SECURITY
        // TODO(Ricardo) Control message size if it will be encrypted.
        if(participant_->security_attributes().is_rtps_protected && endpoint_->supports_rtps_protection())
        {
            CDRMessage::initCDRMsg(encrypt_msg_);
            full_msg_->pos = RTPSMESSAGE_HEADER_SIZE;
            encrypt_msg_->pos = RTPSMESSAGE_HEADER_SIZE;
            encrypt_msg_->length = RTPSMESSAGE_HEADER_SIZE;
            memcpy(encrypt_msg_->buffer, full_msg_->buffer, RTPSMESSAGE_HEADER_SIZE);

            if(!participant_->security_manager().encode_rtps_message(*full_msg_, *encrypt_msg_, current_remote_participants_))
            {
                logError(RTPS_WRITER,"Error encoding rtps message.");
                return;
            }

            msgToSend = encrypt_msg_;
        }
#endif
        const LocatorList_t & destinations =
            fixed_destination_ ? *fixed_destination_locators_ : current_locators_;
        for(const auto& lit : destinations)
        {
            participant_->sendSync(msgToSend, endpoint_, lit);
        }

        currentBytesSent_ += msgToSend->length;
    }
}

void RTPSMessageGroup::flush_and_reset(const LocatorList_t& locator_list,
    const std::vector<GUID_t>& remote_endpoints)
{
    (void)remote_endpoints;

    // Flush
    flush();

    // Reset
    if (!fixed_destination_)
    {
        current_locators_.assign(locator_list);
#if HAVE_SECURITY
        if (participant_->security_attributes().is_rtps_protected && endpoint_->supports_rtps_protection())
        {
            get_participants_from_endpoints(remote_endpoints, current_remote_participants_);
        }
#endif
    }

    current_dst_ = c_GuidPrefix_Unknown;
}

void RTPSMessageGroup::check_and_maybe_flush(const LocatorList_t& locator_list,
        const std::vector<GUID_t>& remote_endpoints)
{
    CDRMessage::initCDRMsg(submessage_msg_);

    if(!check_preconditions(locator_list, remote_endpoints))
        flush_and_reset(locator_list, remote_endpoints);

    add_info_dst_in_buffer(submessage_msg_, remote_endpoints);
}

bool RTPSMessageGroup::insert_submessage(const std::vector<GUID_t>& remote_endpoints)
{
    if(!CDRMessage::appendMsg(full_msg_, submessage_msg_))
    {
        // Retry
        flush();

        current_dst_ = c_GuidPrefix_Unknown;

        if(!add_info_dst_in_buffer(full_msg_, remote_endpoints))
        {
            logError(RTPS_WRITER,"Cannot add INFO_DST submessage to the CDRMessage. Buffer too small");
            return false;
        }

        if(!CDRMessage::appendMsg(full_msg_, submessage_msg_))
        {
            logError(RTPS_WRITER,"Cannot add RTPS submesage to the CDRMessage. Buffer too small");
            return false;
        }
    }

    return true;
}

bool RTPSMessageGroup::add_info_dst_in_buffer(CDRMessage_t* buffer, const std::vector<GUID_t>& remote_endpoints)
{
#if HAVE_SECURITY
    // Add INFO_SRC when we are at the beginning of the message and RTPS protection is enabled
    if ( (full_msg_->length == RTPSMESSAGE_HEADER_SIZE) &&
        participant_->security_attributes().is_rtps_protected && endpoint_->supports_rtps_protection())
    {
        RTPSMessageCreator::addSubmessageInfoSRC(buffer, c_ProtocolVersion, c_VendorId_eProsima, participant_->getGuid().guidPrefix);
    }
#endif

    if (fixed_destination_)
    {
        if (current_dst_ != fixed_destination_prefix_)
        {
            current_dst_ = fixed_destination_prefix_;
            RTPSMessageCreator::addSubmessageInfoDST(buffer, current_dst_);
        }
    }
    else if(remote_endpoints.size() == 1 && current_dst_ != remote_endpoints.at(0).guidPrefix)
    {
        current_dst_ = remote_endpoints.at(0).guidPrefix;
        RTPSMessageCreator::addSubmessageInfoDST(buffer, current_dst_);
    }
    else if(remote_endpoints.size() != 1 && current_dst_ != c_GuidPrefix_Unknown)
    {
        current_dst_ = c_GuidPrefix_Unknown;
        RTPSMessageCreator::addSubmessageInfoDST(buffer, current_dst_);
    }

    return true;
}

bool RTPSMessageGroup::add_info_ts_in_buffer(const std::vector<GUID_t>& remote_readers)
{
    (void)remote_readers;
    logInfo(RTPS_WRITER, "Sending INFO_TS message");

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif

    // Insert INFO_TS submessage.
    // TODO (Ricardo) Source timestamp maybe has marked when user call write function.
    if(!RTPSMessageCreator::addSubmessageInfoTS_Now(submessage_msg_, false)) //Change here to add a INFO_TS for DATA.
    {
        logError(RTPS_WRITER, "Cannot add INFO_TS submsg to the CDRMessage. Buffer too small");
        return false;
    }

#if HAVE_SECURITY
    if(endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if(!participant_->security_manager().encode_writer_submessage(*submessage_msg_, *encrypt_msg_,
                    endpoint_->getGuid(), fixed_destination_ ? *fixed_destination_guids_ : remote_readers))
        {
            logError(RTPS_WRITER, "Cannot encrypt DATA submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            logError(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif

    return true;
}

bool RTPSMessageGroup::add_data(const CacheChange_t& change, const std::vector<GUID_t>& remote_readers,
        const LocatorList_t& locators, bool expectsInlineQos)
{
    logInfo(RTPS_WRITER,"Sending relevant changes as DATA/DATA_FRAG messages");

    // Check preconditions. If fail flush and reset.
    check_and_maybe_flush(locators, remote_readers);

    add_info_ts_in_buffer(remote_readers);

    InlineQosWriter* inlineQos = nullptr;
    if(expectsInlineQos)
    {
        //TODOG INLINEQOS
        //inlineQos = W->getInlineQos();
    }

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif
    const EntityId_t& readerId = get_entity_id(remote_readers);

    // TODO (Ricardo). Check to create special wrapper.

    if(!RTPSMessageCreator::addSubmessageData(submessage_msg_, &change, endpoint_->getAttributes().topicKind,
                readerId, expectsInlineQos, inlineQos))
    {
        logError(RTPS_WRITER, "Cannot add DATA submsg to the CDRMessage. Buffer too small");
        return false;
    }

#if HAVE_SECURITY
    if(endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if(!participant_->security_manager().encode_writer_submessage(*submessage_msg_, *encrypt_msg_,
                    endpoint_->getGuid(), fixed_destination_ ? *fixed_destination_guids_ : remote_readers))
        {
            logError(RTPS_WRITER, "Cannot encrypt DATA submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            logError(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif

    return insert_submessage(remote_readers);
}

bool RTPSMessageGroup::add_data_frag(const CacheChange_t& change, const uint32_t fragment_number,
        const std::vector<GUID_t>& remote_readers, const LocatorList_t& locators, bool expectsInlineQos)
{
    logInfo(RTPS_WRITER,"Sending relevant changes as DATA/DATA_FRAG messages");

    // Check preconditions. If fail flush and reset.
    check_and_maybe_flush(locators, remote_readers);

    add_info_ts_in_buffer(remote_readers);

    InlineQosWriter* inlineQos = NULL;
    if(expectsInlineQos)
    {
        //TODOG INLINEQOS
        //inlineQos = W->getInlineQos();
    }

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif
    const EntityId_t& readerId = get_entity_id(remote_readers);

    // Calculate fragment start
    uint32_t fragment_start = change.getFragmentSize() * (fragment_number - 1);
    // Calculate fragment size. If last fragment, size may be smaller
    uint32_t fragment_size = fragment_number < change.getFragmentCount() ? change.getFragmentSize() :
        change.serializedPayload.length - fragment_start;

    // TODO (Ricardo). Check to create special wrapper.
    CacheChange_t change_to_add;
    change_to_add.copy_not_memcpy(&change);
    change_to_add.serializedPayload.data = change.serializedPayload.data + fragment_start;
    change_to_add.serializedPayload.length = fragment_size;

#if HAVE_SECURITY
    if(endpoint_->getAttributes().security_attributes().is_payload_protected)
    {
        SerializedPayload_t encrypt_payload;
        encrypt_payload.data = encrypt_msg_->buffer;
        encrypt_payload.max_size = encrypt_msg_->max_size;

        // If payload protection, encode payload
        if(!participant_->security_manager().encode_serialized_payload(change_to_add.serializedPayload,
                    encrypt_payload, endpoint_->getGuid()))
        {
            logError(RTPS_WRITER, "Error encoding change " << change.sequenceNumber);
            change_to_add.serializedPayload.data = nullptr;
            encrypt_payload.data = nullptr;
            return false;
        }

        change_to_add.serializedPayload.data = encrypt_msg_->buffer;
        encrypt_payload.data = nullptr;
        change_to_add.serializedPayload.length = encrypt_payload.length;
    }
#endif

    if(!RTPSMessageCreator::addSubmessageDataFrag(submessage_msg_, &change_to_add, fragment_number,
                change.serializedPayload.length, endpoint_->getAttributes().topicKind, readerId,
                expectsInlineQos, inlineQos))
    {
        logError(RTPS_WRITER, "Cannot add DATA_FRAG submsg to the CDRMessage. Buffer too small");
        change_to_add.serializedPayload.data = NULL;
        return false;
    }
    change_to_add.serializedPayload.data = NULL;

#if HAVE_SECURITY
    if(endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if(!participant_->security_manager().encode_writer_submessage(*submessage_msg_, *encrypt_msg_,
                    endpoint_->getGuid(), fixed_destination_ ? *fixed_destination_guids_ : remote_readers))
        {
            logError(RTPS_WRITER, "Cannot encrypt DATA submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            logError(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif

    return insert_submessage(remote_readers);
}

bool RTPSMessageGroup::add_heartbeat(const std::vector<GUID_t>& remote_readers, const SequenceNumber_t& firstSN,
        const SequenceNumber_t& lastSN, const Count_t count,
        bool isFinal, bool livelinessFlag, const LocatorList_t& locators)
{
    check_and_maybe_flush(locators, remote_readers);

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif

    const EntityId_t& readerId = get_entity_id(remote_readers);

    if(!RTPSMessageCreator::addSubmessageHeartbeat(submessage_msg_, readerId, endpoint_->getGuid().entityId,
                firstSN, lastSN, count, isFinal, livelinessFlag))
    {
        logError(RTPS_WRITER, "Cannot add HEARTBEAT submsg to the CDRMessage. Buffer too small");
        return false;
    }

#if HAVE_SECURITY
    if(endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if(!participant_->security_manager().encode_writer_submessage(*submessage_msg_, *encrypt_msg_,
                    endpoint_->getGuid(), fixed_destination_ ? *fixed_destination_guids_ : remote_readers))
        {
            logError(RTPS_WRITER, "Cannot encrypt HEARTBEAT submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            logError(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif

    return insert_submessage(remote_readers);
}

// TODO (Ricardo) Check with standard 8.3.7.4.5
bool RTPSMessageGroup::add_gap(std::set<SequenceNumber_t>& changesSeqNum,
        const std::vector<GUID_t>& remote_readers, const LocatorList_t& locators)
{
    std::vector<pair_T> Sequences;
    prepare_SequenceNumberSet(changesSeqNum, Sequences);
    std::vector<pair_T>::iterator seqit = Sequences.begin();

    uint16_t gap_n = 1;

    while(gap_n <= Sequences.size()) //There is still a message to add
    {
        // Check preconditions. If fail flush and reset.
        check_and_maybe_flush(locators, remote_readers);

#if HAVE_SECURITY
        uint32_t from_buffer_position = submessage_msg_->pos;
#endif

        const EntityId_t& readerId = get_entity_id(remote_readers);

        if(!RTPSMessageCreator::addSubmessageGap(submessage_msg_, seqit->first, seqit->second,
                readerId, endpoint_->getGuid().entityId))
        {
            logError(RTPS_WRITER, "Cannot add GAP submsg to the CDRMessage. Buffer too small");
            break;
        }

#if HAVE_SECURITY
        if(endpoint_->getAttributes().security_attributes().is_submessage_protected)
        {
            submessage_msg_->pos = from_buffer_position;
            CDRMessage::initCDRMsg(encrypt_msg_);
            if(!participant_->security_manager().encode_writer_submessage(*submessage_msg_, *encrypt_msg_,
                        endpoint_->getGuid(), fixed_destination_ ? *fixed_destination_guids_ : remote_readers))
            {
                logError(RTPS_WRITER, "Cannot encrypt DATA submessage for writer " << endpoint_->getGuid());
                return false;
            }

            if((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
            {
                memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
                submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
                submessage_msg_->pos = submessage_msg_->length;
            }
            else
            {
                logError(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
                return false;
            }
        }
#endif

        if(!insert_submessage(remote_readers))
            break;

        ++gap_n;
        ++seqit;
    }

    return true;
}

bool RTPSMessageGroup::add_acknack(const std::vector<GUID_t>& remote_writers, const SequenceNumberSet_t& SNSet,
        int32_t count, bool finalFlag, const LocatorList_t& locators)
{
    // A vector is used to avoid dynamic allocations, but only first item is used
    assert(remote_writers.size() == 1);

    check_and_maybe_flush(locators, remote_writers);

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif

    if(!RTPSMessageCreator::addSubmessageAcknack(submessage_msg_, endpoint_->getGuid().entityId,
                remote_writers.front().entityId, SNSet, count, finalFlag))
    {
        logError(RTPS_READER, "Cannot add ACKNACK submsg to the CDRMessage. Buffer too small");
        return false;
    }

#if HAVE_SECURITY
    if(endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if(!participant_->security_manager().encode_reader_submessage(*submessage_msg_, *encrypt_msg_,
                    endpoint_->getGuid(), remote_writers))
        {
            logError(RTPS_READER, "Cannot encrypt ACKNACK submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            logError(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif

    return insert_submessage(remote_writers);
}

bool RTPSMessageGroup::add_nackfrag(const std::vector<GUID_t>& remote_writers, SequenceNumber_t& writerSN,
        FragmentNumberSet_t fnState, int32_t count, const LocatorList_t locators)
{
    // A vector is used to avoid dynamic allocations, but only first item is used
    assert(remote_writers.size() == 1);

    check_and_maybe_flush(locators, remote_writers);

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif

    if(!RTPSMessageCreator::addSubmessageNackFrag(submessage_msg_, endpoint_->getGuid().entityId,
                remote_writers.front().entityId, writerSN, fnState, count))
    {
        logError(RTPS_READER, "Cannot add ACKNACK submsg to the CDRMessage. Buffer too small");
        return false;
    }

#if HAVE_SECURITY
    if(endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if(!participant_->security_manager().encode_reader_submessage(*submessage_msg_, *encrypt_msg_,
                    endpoint_->getGuid(), remote_writers))
        {
            logError(RTPS_READER, "Cannot encrypt ACKNACK submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            logError(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif

    return insert_submessage(remote_writers);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
