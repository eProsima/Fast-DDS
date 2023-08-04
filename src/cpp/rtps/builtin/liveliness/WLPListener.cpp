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

#include <fastdds/rtps/builtin/liveliness/WLPListener.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>

#include <fastdds/rtps/history/ReaderHistory.h>

#include <fastdds/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>

#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/LivelinessManager.h>
#include <fastdds/dds/log/Log.hpp>

#include <mutex>



namespace eprosima {
namespace fastrtps {
namespace rtps {


WLPListener::WLPListener(
        WLP* plwp)
    : mp_WLP(plwp)
{
}

WLPListener::~WLPListener()
{
}

typedef std::vector<WriterProxy*>::iterator WPIT;

void WLPListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const changeIN)
{
    std::lock_guard<std::recursive_mutex> guard2(*mp_WLP->mp_builtinProtocols->mp_PDP->getMutex());

    GuidPrefix_t guidP;
    LivelinessQosPolicyKind livelinessKind;
    CacheChange_t* change = (CacheChange_t*)changeIN;
    if (!computeKey(change))
    {
        EPROSIMA_LOG_WARNING(RTPS_LIVELINESS, "Problem obtaining the Key");
        return;
    }
    //Check the serializedPayload:
    auto history = reader->getHistory();
    for (auto ch = history->changesBegin(); ch != history->changesEnd(); ++ch)
    {
        if ((*ch)->instanceHandle == change->instanceHandle && (*ch)->sequenceNumber < change->sequenceNumber)
        {
            history->remove_change(*ch);
            break;
        }
    }

    // Data should have at least 4 bytes of representation header, 12 of GuidPrefix, and 4 of kind.
    if (change->serializedPayload.length >= 20)
    {
        // Encapsulation in the second byte of the representation header.
        change->serializedPayload.encapsulation = (uint16_t)change->serializedPayload.data[1];

        // Extract GuidPrefix
        memcpy(guidP.value, change->serializedPayload.data + 4, 12);

        // Extract liveliness kind
        if (is_wlp_kind(&change->serializedPayload.data[16]))
        {
            // Adjust and cast to LivelinessQosPolicyKind enum, where AUTOMATIC_LIVELINESS_QOS == 0
            livelinessKind = (LivelinessQosPolicyKind)(change->serializedPayload.data[19] - 0x01);
        }
        else
        {
            logInfo(RTPS_LIVELINESS,"Ignoring not WLP ParticipantDataMessage");
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
            logInfo(RTPS_LIVELINESS,"Ignoring not WLP ParticipantDataMessage");
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
        mp_WLP->sub_liveliness_manager_->assert_liveliness(AUTOMATIC_LIVELINESS_QOS);
    }
    if (livelinessKind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        mp_WLP->sub_liveliness_manager_->assert_liveliness(MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    }
    mp_WLP->mp_builtinProtocols->mp_PDP->getMutex()->unlock();
    history->getMutex()->lock();
    mp_WLP->mp_builtinProtocols->mp_PDP->getMutex()->lock();
    return;
}

bool WLPListener::separateKey(
        InstanceHandle_t& key,
        GuidPrefix_t* guidP,
        LivelinessQosPolicyKind* liveliness)
{
    bool ret = false;
    if (is_wlp_kind(&key.value[12]))
    {
        // Extract GuidPrefix
        memcpy(guidP->value, key.value, 12);

        // Extract liveliness kind
        *liveliness = (LivelinessQosPolicyKind)key.value[15];
        ret = true;
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

bool WLPListener::is_wlp_kind(octet* kind)
{
    /*
    * From RTPS 2.5 9.6.3.1, the ParticipantMessageData kinds for WLP are:
    *   - PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE {0x00, 0x00, 0x00, 0x01}
    *   - PARTICIPANT_MESSAGE_DATA_KIND_MANUAL_LIVELINESS_UPDATE {0x00, 0x00, 0x00, 0x02}
    */
    bool is_wlp = true;
    is_wlp &= kind[0] == 0;
    is_wlp &= kind[1] == 0;
    is_wlp &= kind[2] == 0;
    is_wlp &= kind[3] == 0x01 || kind[3] == 0x02;
    return is_wlp;
}

} /* namespace rtps */
} /* namespace eprosima */
} // namespace eprosima
