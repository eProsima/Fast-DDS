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
 * @file WLPListener.cpp
 *
 */
#include <rtps/builtin/liveliness/WLPListener.h>

#include <cstdint>
#include <cstring>
#include <mutex>
#include <vector>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/discovery/participant/PDPSimple.h>
#include <rtps/builtin/liveliness/WLP.hpp>
#include <rtps/messages/CDRMessage.hpp>
#include <rtps/writer/LivelinessManager.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

WLPListener::WLPListener(
        WLP* plwp)
    : mp_WLP(plwp)
{
}

WLPListener::~WLPListener()
{
}

void WLPListener::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const changeIN)
{
    std::lock_guard<std::recursive_mutex> guard2(*mp_WLP->mp_builtinProtocols->mp_PDP->getMutex());

    GuidPrefix_t guidP;
    dds::LivelinessQosPolicyKind livelinessKind = dds::AUTOMATIC_LIVELINESS_QOS;
    CacheChange_t* change = (CacheChange_t*)changeIN;
    if (!computeKey(change))
    {
        EPROSIMA_LOG_WARNING(RTPS_LIVELINESS, "Problem obtaining the Key");
        return;
    }
    //Check the serializedPayload:
    auto history = reader->get_history();
    for (auto ch = history->changesBegin(); ch != history->changesEnd(); ++ch)
    {
        if ((*ch)->instanceHandle == change->instanceHandle && (*ch)->sequenceNumber < change->sequenceNumber)
        {
            history->remove_change(*ch);
            break;
        }
    }

    // Serialized payload should have at least 4 bytes of representation header, 12 of GuidPrefix,
    // 4 of kind, and 4 of length.
    constexpr uint32_t participant_msg_data_kind_size = 4;
    constexpr uint32_t participant_msg_data_length_size = 4;
    constexpr uint32_t min_serialized_length = SerializedPayload_t::representation_header_size
            + GuidPrefix_t::size
            + participant_msg_data_kind_size
            + participant_msg_data_length_size;

    if (change->serializedPayload.length >= min_serialized_length)
    {
        constexpr uint32_t participant_msg_data_kind_pos = 16;
        constexpr uint32_t encapsulation_pos = 1;
        uint32_t data_length = 0;

        // Extract encapsulation from the second byte of the representation header. Done prior to
        // creating the CDRMessage_t, as the CDRMessage_t ctor uses it for its own state.
        change->serializedPayload.encapsulation =
                static_cast<uint16_t>(change->serializedPayload.data[encapsulation_pos]);

        // Create CDR message from buffer to deserialize contents for further validation
        CDRMessage_t cdr_message(change->serializedPayload);

        bool message_ok = (
            // Skip representation header
            CDRMessage::skip(&cdr_message, SerializedPayload_t::representation_header_size)
            // Extract GuidPrefix
            && CDRMessage::readData(&cdr_message, guidP.value, GuidPrefix_t::size)
            // Skip kind, it will be validated later
            && CDRMessage::skip(&cdr_message, participant_msg_data_kind_size)
            // Extract and validate liveliness kind
            && get_wlp_kind(&change->serializedPayload.data[participant_msg_data_kind_pos], livelinessKind)
            // Extract data length
            && CDRMessage::readUInt32(&cdr_message, &data_length)
            // Check that serialized length is correctly set
            && (change->serializedPayload.length >= min_serialized_length + data_length));

        if (!message_ok)
        {
            EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Ignoring incorrect WLP ParticipantDataMessage");
            history->remove_change(change);
            return;
        }
    }
    else
    {
        if (!separateKey(
                    change->instanceHandle,
                    &guidP,
                    &livelinessKind))
        {
            EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Ignoring not WLP ParticipantDataMessage");
            history->remove_change(change);
            return;
        }
    }

    if (guidP == reader->getGuid().guidPrefix)
    {
        EPROSIMA_LOG_INFO(RTPS_LIVELINESS, "Message from own RTPSParticipant, ignoring");
        history->remove_change(change);
        return;
    }

    history->getMutex()->unlock();
    if (mp_WLP->automatic_readers_)
    {
        mp_WLP->sub_liveliness_manager_->assert_liveliness(dds::AUTOMATIC_LIVELINESS_QOS, guidP);
    }
    if (livelinessKind == dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        mp_WLP->sub_liveliness_manager_->assert_liveliness(dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, guidP);
    }
    mp_WLP->mp_builtinProtocols->mp_PDP->getMutex()->unlock();
    history->getMutex()->lock();
    mp_WLP->mp_builtinProtocols->mp_PDP->getMutex()->lock();
    return;
}

bool WLPListener::separateKey(
        InstanceHandle_t& key,
        GuidPrefix_t* guidP,
        dds::LivelinessQosPolicyKind* liveliness)
{
    bool ret = get_wlp_kind(&key.value[12], *liveliness);
    if (ret)
    {
        // Extract GuidPrefix
        memcpy(guidP->value, key.value, 12);
    }
    return ret;
}

bool WLPListener::computeKey(
        CacheChange_t* change)
{
    if (change->instanceHandle == c_InstanceHandle_Unknown)
    {
        SerializedPayload_t* pl = &change->serializedPayload;
        if (pl->length >= 20)
        {
            memcpy(change->instanceHandle.value, pl->data + 4, 16);
            return true;
        }
        return false;
    }
    return true;
}

bool WLPListener::get_wlp_kind(
        const octet* serialized_kind,
        dds::LivelinessQosPolicyKind& liveliness_kind)
{
    /*
     * From RTPS 2.5 9.6.3.1, the ParticipantMessageData kinds for WLP are:
     *   - PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE {0x00, 0x00, 0x00, 0x01}
     *   - PARTICIPANT_MESSAGE_DATA_KIND_MANUAL_LIVELINESS_UPDATE {0x00, 0x00, 0x00, 0x02}
     */
    bool is_wlp = (
        serialized_kind[0] == 0
        && serialized_kind[1] == 0
        && serialized_kind[2] == 0
        && (serialized_kind[3] == 0x01 || serialized_kind[3] == 0x02));

    if (is_wlp)
    {
        // Adjust and cast to LivelinessQosPolicyKind enum, where AUTOMATIC_LIVELINESS_QOS == 0
        liveliness_kind = static_cast<dds::LivelinessQosPolicyKind>(serialized_kind[3] - 0x01);
    }

    return is_wlp;
}

} /* namespace rtps */
} /* namespace eprosima */
} // namespace eprosima
