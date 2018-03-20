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
 * @file EDPSimpleListener.cpp
 *
 */

#include "EDPSimpleListeners.h"

#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include "../../../participant/RTPSParticipantImpl.h"
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/rtps/common/InstanceHandle.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <mutex>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

void EDPSimplePUBListener::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->mp_SEDP->mp_PubReader.first->getMutex());
    logInfo(RTPS_EDP,"");
    if(!computeKey(change))
    {
        logWarning(RTPS_EDP,"Received change with no Key");
    }

    //Call the slave, if it exists
    attached_listener_mutex.lock();
    if(attached_listener != nullptr)
        attached_listener->onNewCacheChangeAdded(this->mp_SEDP->mp_PubReader.first, change_in);
    attached_listener_mutex.unlock();

    if(change->kind == ALIVE)
    {
        //LOAD INFORMATION IN TEMPORAL WRITER PROXY DATA
        WriterProxyData writerProxyData;
        CDRMessage_t tempMsg(0);
        tempMsg.wraps = true;
        tempMsg.msg_endian = change_in->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
        tempMsg.length = change_in->serializedPayload.length;
        tempMsg.max_size = change_in->serializedPayload.max_size;
        tempMsg.buffer = change_in->serializedPayload.data;

        if(writerProxyData.readFromCDRMessage(&tempMsg))
        {
            change->instanceHandle = writerProxyData.key();
            if(writerProxyData.guid().guidPrefix == mp_SEDP->mp_RTPSParticipant->getGuid().guidPrefix)
            {
                logInfo(RTPS_EDP,"Message from own RTPSParticipant, ignoring");
                mp_SEDP->mp_PubReader.second->remove_change(change);
                return;
            }

            //LOOK IF IS AN UPDATED INFORMATION
            ParticipantProxyData pdata;
            if(this->mp_SEDP->mp_PDP->addWriterProxyData(&writerProxyData, pdata)) //ADDED NEW DATA
            {
                // At this point we can release reader lock, cause change is not used
                reader->getMutex()->unlock();

                mp_SEDP->pairing_writer_proxy_with_any_local_reader(&pdata, &writerProxyData);

                // Take again the reader lock.
                reader->getMutex()->lock();
            }
            else //NOT ADDED BECAUSE IT WAS ALREADY THERE
            {
                logWarning(RTPS_EDP,"Received message from UNKNOWN RTPSParticipant, removing");
            }
        }
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP,"Disposed Remote Writer, removing...");

        GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
        this->mp_SEDP->mp_PDP->removeWriterProxyData(auxGUID);
    }

    //Removing change from history
    this->mp_SEDP->mp_PubReader.second->remove_change(change);

    return;
}

bool EDPSimplePUBListener::computeKey(CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, PID_ENDPOINT_GUID);
}

bool EDPSimpleSUBListener::computeKey(CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, PID_ENDPOINT_GUID);
}

void EDPSimpleSUBListener::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->mp_SEDP->mp_SubReader.first->getMutex());
    logInfo(RTPS_EDP,"");
    if(!computeKey(change))
    {
        logWarning(RTPS_EDP,"Received change with no Key");
    }

    //Call the slave, if it exists
    attached_listener_mutex.lock();
    if(attached_listener != nullptr)
        attached_listener->onNewCacheChangeAdded(this->mp_SEDP->mp_SubReader.first, change);
    attached_listener_mutex.unlock();

    if(change->kind == ALIVE)
    {
        //LOAD INFORMATION IN TEMPORAL WRITER PROXY DATA
        ReaderProxyData readerProxyData;
        CDRMessage_t tempMsg(0);
        tempMsg.wraps = true;
        tempMsg.msg_endian = change_in->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
        tempMsg.length = change_in->serializedPayload.length;
        tempMsg.max_size = change_in->serializedPayload.max_size;
        tempMsg.buffer = change_in->serializedPayload.data;

        if(readerProxyData.readFromCDRMessage(&tempMsg))
        {
            change->instanceHandle = readerProxyData.key();
            if(readerProxyData.guid().guidPrefix == mp_SEDP->mp_RTPSParticipant->getGuid().guidPrefix)
            {
                logInfo(RTPS_EDP,"From own RTPSParticipant, ignoring");
                mp_SEDP->mp_SubReader.second->remove_change(change);
                return;
            }

            //LOOK IF IS AN UPDATED INFORMATION
            ParticipantProxyData pdata;
            if(this->mp_SEDP->mp_PDP->addReaderProxyData(&readerProxyData, pdata)) //ADDED NEW DATA
            {
                // At this point we can release reader lock, cause change is not used
                reader->getMutex()->unlock();

                mp_SEDP->pairing_reader_proxy_with_any_local_writer(&pdata, &readerProxyData);

                // Take again the reader lock.
                reader->getMutex()->lock();
            }
            else
            {
                logWarning(RTPS_EDP,"From UNKNOWN RTPSParticipant, removing");
            }
        }
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP,"Disposed Remote Reader, removing...");

        GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
        this->mp_SEDP->mp_PDP->removeReaderProxyData(auxGUID);
    }

    // Remove change from history.
    this->mp_SEDP->mp_SubReader.second->remove_change(change);

    return;
}

} /* namespace rtps */
}
} /* namespace eprosima */
