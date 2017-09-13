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

void EDPSimplePUBListener::onNewCacheChangeAdded(RTPSReader* /*reader*/, const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->mp_SEDP->mp_PubReader.first->getMutex());
    logInfo(RTPS_EDP,"");
    if(!computeKey(change))
    {
        logWarning(RTPS_EDP,"Received change with no Key");
    }
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
            WriterProxyData* wdata = nullptr;
            ParticipantProxyData* pdata = nullptr;
            if(this->mp_SEDP->mp_PDP->addWriterProxyData(&writerProxyData,true,&wdata,&pdata)) //ADDED NEW DATA
            {
                //CHECK the locators:
                pdata->mp_mutex->lock();
                if(wdata->unicastLocatorList().empty() && wdata->multicastLocatorList().empty())
                {
                    wdata->unicastLocatorList(pdata->m_defaultUnicastLocatorList);
                    wdata->multicastLocatorList(pdata->m_defaultMulticastLocatorList);
                }
                wdata->isAlive(true);
                pdata->mp_mutex->unlock();
                mp_SEDP->pairing_writer_proxy_with_any_local_reader(pdata, wdata);
            }
            else if(pdata == nullptr) //RTPSParticipant NOT FOUND
            {
                logWarning(RTPS_EDP,"Received message from UNKNOWN RTPSParticipant, removing");
            }
            else //NOT ADDED BECAUSE IT WAS ALREADY THERE
            {
                pdata->mp_mutex->lock();
                wdata->update(&writerProxyData);
                pdata->mp_mutex->unlock();
                mp_SEDP->pairing_writer_proxy_with_any_local_reader(pdata, wdata);
            }

            //Call the slave, if it exists
            attached_listener_mutex.lock();
            if(attached_listener != nullptr)
                attached_listener->onNewCacheChangeAdded(this->mp_SEDP->mp_PubReader.first, change_in);
            attached_listener_mutex.unlock();
        }
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP,"Disposed Remote Writer, removing...");

        //Call the slave, if it exists
        attached_listener_mutex.lock();
        if(attached_listener != nullptr)
            attached_listener->onNewCacheChangeAdded(this->mp_SEDP->mp_PubReader.first, change_in);
        attached_listener_mutex.unlock();

        GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
        this->mp_SEDP->removeWriterProxy(auxGUID);
    }

    //Removing change from history
    this->mp_SEDP->mp_PubReader.second->remove_change(change);

    return;
}

static inline bool compute_key(CDRMessage_t* aux_msg,CacheChange_t* change)
{
    if(change->instanceHandle == c_InstanceHandle_Unknown)
    {
        SerializedPayload_t* pl = &change->serializedPayload;
        aux_msg->buffer = pl->data;
        aux_msg->length = pl->length;
        aux_msg->max_size = pl->max_size;
        aux_msg->msg_endian = pl->encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
        bool valid = false;
        uint16_t pid;
        uint16_t plength;
        while(aux_msg->pos < aux_msg->length)
        {
            valid = true;
            valid&=CDRMessage::readUInt16(aux_msg,(uint16_t*)&pid);
            valid&=CDRMessage::readUInt16(aux_msg,&plength);
            if(pid == PID_SENTINEL)
            {
                break;
            }
            if(pid == PID_KEY_HASH)
            {
                valid &= CDRMessage::readData(aux_msg,change->instanceHandle.value,16);
                aux_msg->buffer = nullptr;
                return true;
            }
            if(pid == PID_ENDPOINT_GUID)
            {
                valid &= CDRMessage::readData(aux_msg,change->instanceHandle.value,16);
                aux_msg->buffer = nullptr;
                return true;
            }
            aux_msg->pos+=plength;
        }
        aux_msg->buffer = nullptr;
        return false;
    }
    aux_msg->buffer = nullptr;
    return true;
}

bool EDPSimplePUBListener::computeKey(CacheChange_t* change)
{
    CDRMessage_t aux_msg(0);
    return compute_key(&aux_msg,change);
}

bool EDPSimpleSUBListener::computeKey(CacheChange_t* change)
{
    CDRMessage_t aux_msg(0);
    return compute_key(&aux_msg,change);
}

void EDPSimpleSUBListener::onNewCacheChangeAdded(RTPSReader* /*reader*/, const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->mp_SEDP->mp_SubReader.first->getMutex());
    logInfo(RTPS_EDP,"");
    if(!computeKey(change))
    {
        logWarning(RTPS_EDP,"Received change with no Key");
    }
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
            ReaderProxyData* rdata = nullptr;
            ParticipantProxyData* pdata = nullptr;
            if(this->mp_SEDP->mp_PDP->addReaderProxyData(&readerProxyData,true,&rdata,&pdata)) //ADDED NEW DATA
            {
                pdata->mp_mutex->lock();
                //CHECK the locators:
                if(rdata->unicastLocatorList().empty() && rdata->multicastLocatorList().empty())
                {
                    rdata->unicastLocatorList(pdata->m_defaultUnicastLocatorList);
                    rdata->multicastLocatorList(pdata->m_defaultMulticastLocatorList);
                }
                rdata->isAlive(true);
                pdata->mp_mutex->unlock();
                mp_SEDP->pairing_reader_proxy_with_any_local_writer(pdata, rdata);
            }
            else if(pdata == nullptr) //RTPSParticipant NOT FOUND
            {
                logWarning(RTPS_EDP,"From UNKNOWN RTPSParticipant, removing");
            }
            else //NOT ADDED BECAUSE IT WAS ALREADY THERE
            {
                pdata->mp_mutex->lock();
                rdata->update(&readerProxyData);
                pdata->mp_mutex->unlock();
                mp_SEDP->pairing_reader_proxy_with_any_local_writer(pdata, rdata);
            }

            //Call the slave, if it exists
            attached_listener_mutex.lock();
            if(attached_listener != nullptr)
                attached_listener->onNewCacheChangeAdded(this->mp_SEDP->mp_SubReader.first, change);
            attached_listener_mutex.unlock();
        }
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP,"Disposed Remote Reader, removing...");

        //Call the slave, if it exists
        attached_listener_mutex.lock();
        if(attached_listener != nullptr)
            attached_listener->onNewCacheChangeAdded(this->mp_SEDP->mp_SubReader.first, change);
        attached_listener_mutex.unlock();

        GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
        this->mp_SEDP->removeReaderProxy(auxGUID);
    }

    // Remove change from history.
    this->mp_SEDP->mp_SubReader.second->remove_change(change);

    return;
}

} /* namespace rtps */
}
} /* namespace eprosima */
