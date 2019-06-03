// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file EDPServerListener.cpp
 *
 */

#include "EDPServerListeners.h"
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPServer.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPServer.h>

#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include "../../../participant/RTPSParticipantImpl.h"
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/history/WriterHistory.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <mutex>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

void EDPServerPUBListener::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->sedp_->publications_reader_.first->getMutex());
    logInfo(RTPS_EDP,"");
    if(!computeKey(change))
    {
        logWarning(RTPS_EDP,"Received change with no Key");
    }

    ReaderHistory* reader_history = sedp_->publications_reader_.second;

    // update the PDP Writer with this reader info
    if (!sedp_->addPublisherFromHistory(*change))
    {
        reader_history->remove_change(change);
        return; // already there
    }

    if(change->kind == ALIVE)
    {
        //LOAD INFORMATION IN TEMPORAL WRITER PROXY DATA
        WriterProxyData writerProxyData;
        CDRMessage_t tempMsg(change_in->serializedPayload);

        if(writerProxyData.readFromCDRMessage(&tempMsg))
        {
            change->instanceHandle = writerProxyData.key();
            if(writerProxyData.guid().guidPrefix == sedp_->mp_RTPSParticipant->getGuid().guidPrefix)
            {
                logInfo(RTPS_EDP,"Message from own RTPSParticipant, ignoring");
                reader_history->remove_change(change);
                return;
            }

            //LOOK IF IS AN UPDATED INFORMATION
            ParticipantProxyData pdata;
            if(this->sedp_->mp_PDP->addWriterProxyData(&writerProxyData, pdata)) //ADDED NEW DATA
            {
                // At this point we can release reader lock, cause change is not used
                reader->getMutex().unlock();

                sedp_->pairing_writer_proxy_with_any_local_reader(&pdata, &writerProxyData);

                // Take again the reader lock.
                reader->getMutex().lock();
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
        this->sedp_->mp_PDP->removeWriterProxyData(auxGUID);
        sedp_->removePublisherFromHistory(change->instanceHandle);
    }

    //Removing change from history
    reader_history->remove_change(change);

    return;
}

void EDPServerPUBListener::onWriterChangeReceivedByAll(RTPSWriter* writer, CacheChange_t* change)
{
    (void)writer;

    if (ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED == change->kind)
    {
        WriterHistory* writer_history =
#if HAVE_SECURITY
            writer == sedp_->publications_secure_writer_.first ?
            sedp_->publications_secure_writer_.second :
#endif
            sedp_->publications_writer_.second;

        writer_history->remove_change(change);
    }
}

void EDPServerSUBListener::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->sedp_->subscriptions_reader_.first->getMutex());
    logInfo(RTPS_EDP,"");
    if(!computeKey(change))
    {
        logWarning(RTPS_EDP,"Received change with no Key");
    }

    ReaderHistory* reader_history = sedp_->subscriptions_reader_.second;

    // update the PDP Writer with this reader info
    if (!sedp_->addSubscriberFromHistory(*change))
    {
        reader_history->remove_change(change);
        return; // already there
    }

    if(change->kind == ALIVE)
    {
        //LOAD INFORMATION IN TEMPORAL WRITER PROXY DATA
        ReaderProxyData readerProxyData;
        CDRMessage_t tempMsg(change_in->serializedPayload);

        if(readerProxyData.readFromCDRMessage(&tempMsg))
        {
            change->instanceHandle = readerProxyData.key();
            if(readerProxyData.guid().guidPrefix == sedp_->mp_RTPSParticipant->getGuid().guidPrefix)
            {
                logInfo(RTPS_EDP,"From own RTPSParticipant, ignoring");
                reader_history->remove_change(change);
                return;
            }

            //LOOK IF IS AN UPDATED INFORMATION
            ParticipantProxyData pdata;
            if(this->sedp_->mp_PDP->addReaderProxyData(&readerProxyData, pdata)) //ADDED NEW DATA
            {
                // At this point we can release reader lock, cause change is not used
                reader->getMutex().unlock();

                sedp_->pairing_reader_proxy_with_any_local_writer(&pdata, &readerProxyData);

                // Take again the reader lock.
                reader->getMutex().lock();
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
        this->sedp_->mp_PDP->removeReaderProxyData(auxGUID);
        sedp_->removeSubscriberFromHistory(change->instanceHandle);
    }

    // Remove change from history.
    reader_history->remove_change(change);

    return;
}


void EDPServerSUBListener::onWriterChangeReceivedByAll(RTPSWriter* writer, CacheChange_t* change)
{
    (void)writer;

    if (ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED == change->kind)
    {
        WriterHistory* writer_history =
#if HAVE_SECURITY
            writer == sedp_->subscriptions_secure_writer_.first ?
            sedp_->subscriptions_secure_writer_.second :
#endif
            sedp_->subscriptions_writer_.second;

        writer_history->remove_change(change);
    }

}

} /* namespace rtps */
}
} /* namespace eprosima */

