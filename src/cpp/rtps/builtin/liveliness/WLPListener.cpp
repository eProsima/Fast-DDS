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
namespace fastrtps{
namespace rtps {


WLPListener::WLPListener(WLP* plwp)
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
    if(!computeKey(change))
    {
        logWarning(RTPS_LIVELINESS,"Problem obtaining the Key");
        return;
    }
    //Check the serializedPayload:
    auto history = reader->getHistory();
    for(auto ch = history->changesBegin(); ch!=history->changesEnd(); ++ch)
    {
        if((*ch)->instanceHandle == change->instanceHandle && (*ch)->sequenceNumber < change->sequenceNumber)
        {
            history->remove_change(*ch);
            break;
        }
    }
    if (change->serializedPayload.length > 0)
    {
        if (PL_CDR_BE == change->serializedPayload.data[1])
        {
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
        }
        else
        {
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
        }

        for(size_t i = 0; i<12; ++i)
        {
            guidP.value[i] = change->serializedPayload.data[i + 4];
        }
        livelinessKind = (LivelinessQosPolicyKind)(change->serializedPayload.data[19]-0x01);

    }
    else
    {
        if(!separateKey(
                    change->instanceHandle,
                    &guidP,
                    &livelinessKind))
        {
            return;
        }
    }

    if(guidP == reader->getGuid().guidPrefix)
    {
        logInfo(RTPS_LIVELINESS,"Message from own RTPSParticipant, ignoring");
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
    for(uint8_t i=0;i<12;++i)
    {
        guidP->value[i] = key.value[i];
    }
    *liveliness = (LivelinessQosPolicyKind)key.value[15];
    return true;
}

bool WLPListener::computeKey(CacheChange_t* change)
{
    if(change->instanceHandle == c_InstanceHandle_Unknown)
    {
        SerializedPayload_t* pl = &change->serializedPayload;
        if(pl->length >= 20)
        {
            memcpy(change->instanceHandle.value, pl->data + 4, 16);
            return true;
        }
        return false;
    }
    return true;
}


} /* namespace rtps */
} /* namespace eprosima */
}
