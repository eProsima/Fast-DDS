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

/*
 * @file RTPSWriter.cpp
 *
 */

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/messages/RTPSMessageCreator.h>
#include <fastrtps/log/Log.h>
#include "../participant/RTPSParticipantImpl.h"
#include "../flowcontrol/FlowController.h"

#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {


RTPSWriter::RTPSWriter(
        RTPSParticipantImpl* impl, 
        const GUID_t& guid, 
        const WriterAttributes& att, 
        WriterHistory* hist, 
        WriterListener* listen) 
    : Endpoint(impl, guid, att.endpoint)
    , m_pushMode(true)
    , m_cdrmessages(impl->getMaxMessageSize() > att.throughputController.bytesPerPeriod ?
        att.throughputController.bytesPerPeriod > impl->getRTPSParticipantAttributes().throughputController.bytesPerPeriod ?
        impl->getRTPSParticipantAttributes().throughputController.bytesPerPeriod :
        att.throughputController.bytesPerPeriod :
        impl->getMaxMessageSize() > impl->getRTPSParticipantAttributes().throughputController.bytesPerPeriod ?
        impl->getRTPSParticipantAttributes().throughputController.bytesPerPeriod :
        impl->getMaxMessageSize(), impl->getGuid().guidPrefix)
    , m_livelinessAsserted(false)
    , mp_history(hist)
    , mp_listener(listen)
    , is_async_(att.mode == SYNCHRONOUS_WRITER ? false : true)
    , m_separateSendingEnabled(false)
    , all_remote_readers_(att.matched_readers_allocation)
#if HAVE_SECURITY
    , encrypt_payload_(mp_history->getTypeMaxSerialized())
#endif
{
    mp_history->mp_writer = this;
    mp_history->mp_mutex = &mp_mutex;
    logInfo(RTPS_WRITER, "RTPSWriter created");
}

RTPSWriter::~RTPSWriter()
{
    logInfo(RTPS_WRITER, "RTPSWriter destructor");

    // Deletion of the events has to be made in child destructor.

    mp_history->mp_writer = nullptr;
    mp_history->mp_mutex = nullptr;
}

CacheChange_t* RTPSWriter::new_change(const std::function<uint32_t()>& dataCdrSerializedSize,
    ChangeKind_t changeKind, InstanceHandle_t handle)
{
    logInfo(RTPS_WRITER, "Creating new change");
    CacheChange_t* ch = nullptr;

    if (!mp_history->reserve_Cache(&ch, dataCdrSerializedSize))
    {
        logWarning(RTPS_WRITER, "Problem reserving Cache from the History");
        return nullptr;
    }

    ch->kind = changeKind;
    if (m_att.topicKind == WITH_KEY && !handle.isDefined())
    {
        logWarning(RTPS_WRITER, "Changes in KEYED Writers need a valid instanceHandle");
    }
    ch->instanceHandle = handle;
    ch->writerGUID = m_guid;
    return ch;
}

SequenceNumber_t RTPSWriter::get_seq_num_min()
{
    CacheChange_t* change;
    if (mp_history->get_min_change(&change) && change != nullptr)
        return change->sequenceNumber;
    else
        return c_SequenceNumber_Unknown;
}

SequenceNumber_t RTPSWriter::get_seq_num_max()
{
    CacheChange_t* change;
    if (mp_history->get_max_change(&change) && change != nullptr)
        return change->sequenceNumber;
    else
        return c_SequenceNumber_Unknown;
}

uint32_t RTPSWriter::getTypeMaxSerialized()
{
    return mp_history->getTypeMaxSerialized();
}


bool RTPSWriter::remove_older_changes(unsigned int max)
{
    logInfo(RTPS_WRITER, "Starting process clean_history for writer " << getGuid());
    std::lock_guard<std::recursive_timed_mutex> guard(mp_mutex);
    bool limit = (max != 0);

    bool remove_ret = mp_history->remove_min_change();
    bool at_least_one = remove_ret;
    unsigned int count = 1;

    while (remove_ret && (!limit || count < max))
    {
        remove_ret = mp_history->remove_min_change();
        ++count;
    }

    return at_least_one;
}

CONSTEXPR uint32_t info_dst_message_length = 16;
CONSTEXPR uint32_t info_ts_message_length = 12;
CONSTEXPR uint32_t data_frag_submessage_header_length = 36;

uint32_t RTPSWriter::getMaxDataSize()
{
    return calculateMaxDataSize(mp_RTPSParticipant->getMaxMessageSize());
}

uint32_t RTPSWriter::calculateMaxDataSize(uint32_t length)
{
    uint32_t maxDataSize = mp_RTPSParticipant->calculateMaxDataSize(length);

    maxDataSize -= info_dst_message_length +
        info_ts_message_length +
        data_frag_submessage_header_length;

    //TODO(Ricardo) inlineqos in future.

#if HAVE_SECURITY
    if (getAttributes().security_attributes().is_submessage_protected)
    {
        maxDataSize -= mp_RTPSParticipant->security_manager().calculate_extra_size_for_rtps_submessage(m_guid);
    }

    if (getAttributes().security_attributes().is_payload_protected)
    {
        maxDataSize -= mp_RTPSParticipant->security_manager().calculate_extra_size_for_encoded_payload(m_guid);
    }
#endif

    return maxDataSize;
}

void RTPSWriter::update_cached_info_nts(std::vector<LocatorList_t>& allLocatorLists)
{
    mAllShrinkedLocatorList.clear();
    mAllShrinkedLocatorList.push_back(mp_RTPSParticipant->network_factory().ShrinkLocatorLists(allLocatorLists));
}

#if HAVE_SECURITY
bool RTPSWriter::encrypt_cachechange(CacheChange_t* change)
{
    if (getAttributes().security_attributes().is_payload_protected && change->getFragmentCount() == 0)
    {
        if (encrypt_payload_.max_size < change->serializedPayload.length +
            // In future v2 changepool is in writer, and writer set this value to cachechagepool.
            +20 /*SecureDataHeader*/ + 4 + ((2 * 16) /*EVP_MAX_IV_LENGTH max block size*/ - 1) /* SecureDataBodey*/
            + 16 + 4 /*SecureDataTag*/ &&
            (mp_history->m_att.memoryPolicy == MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE ||
                mp_history->m_att.memoryPolicy == MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE))
        {
            encrypt_payload_.data = (octet*)realloc(encrypt_payload_.data, change->serializedPayload.length +
                    // In future v2 changepool is in writer, and writer set this value to cachechagepool.
                +20 /*SecureDataHeader*/ + 4 + ((2 * 16) /*EVP_MAX_IV_LENGTH max block size*/ - 1) /* SecureDataBodey*/
                + 16 + 4 /*SecureDataTag*/);
            encrypt_payload_.max_size = change->serializedPayload.length +
                // In future v2 changepool is in writer, and writer set this value to cachechagepool.
                +20 /*SecureDataHeader*/ + 4 + ((2 * 16) /*EVP_MAX_IV_LENGTH max block size*/ - 1) /* SecureDataBodey*/
                + 16 + 4 /*SecureDataTag*/;
        }

        if (!mp_RTPSParticipant->security_manager().encode_serialized_payload(change->serializedPayload,
            encrypt_payload_, m_guid))
        {
            logError(RTPS_WRITER, "Error encoding change " << change->sequenceNumber);
            return false;
        }

        octet* data = change->serializedPayload.data;
        uint32_t max_size = change->serializedPayload.max_size;

        change->serializedPayload.length = encrypt_payload_.length;
        change->serializedPayload.data = encrypt_payload_.data;
        change->serializedPayload.max_size = encrypt_payload_.max_size;
        change->serializedPayload.pos = encrypt_payload_.pos;

        encrypt_payload_.data = data;;
        encrypt_payload_.length = 0;
        encrypt_payload_.max_size = max_size;
        encrypt_payload_.pos = 0;

        change->setFragmentSize(change->getFragmentSize());
    }

    return true;
}
#endif

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
