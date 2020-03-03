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

#include <rtps/builtin/discovery/endpoint/EDPServerListeners.h>
#include <fastdds/rtps/builtin/discovery/endpoint/EDPServer.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPServer.h>

#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps_deprecated/participant/ParticipantImpl.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>

#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

#include <mutex>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

EDPServerPUBListener::EDPServerPUBListener(
        EDPServer* sedp)
    : EDPBasePUBListener(sedp->mp_RTPSParticipant->getAttributes().allocation.locators,
            sedp->mp_RTPSParticipant->getAttributes().allocation.data_limits)
    , sedp_(sedp)
{
}

void EDPServerPUBListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->sedp_->publications_reader_.first->getMutex());
    logInfo(RTPS_EDP, "");
    if (!computeKey(change))
    {
        logWarning(RTPS_EDP, "Received change with no Key");
    }

    ReaderHistory* reader_history = sedp_->publications_reader_.second;

    // update the PDP Writer with this reader info
    if (!sedp_->addPublisherFromHistory(*change))
    {
        reader_history->remove_change(change);
        return; // already there
    }

    if (change->kind == ALIVE)
    {
        add_writer_from_change(reader, change, sedp_);
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP, "Disposed Remote Writer, removing...");

        GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
        this->sedp_->mp_PDP->removeWriterProxyData(auxGUID);
        sedp_->removePublisherFromHistory(change->instanceHandle);
    }

    //Removing change from history
    reader_history->remove_change(change);

    return;
}

void EDPServerPUBListener::onWriterChangeReceivedByAll(
        RTPSWriter* writer,
        CacheChange_t* change)
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

EDPServerSUBListener::EDPServerSUBListener(
        EDPServer* sedp)
    : EDPBaseSUBListener(sedp->mp_RTPSParticipant->getAttributes().allocation.locators,
            sedp->mp_RTPSParticipant->getAttributes().allocation.data_limits)
    , sedp_(sedp)
{
}

void EDPServerSUBListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->sedp_->subscriptions_reader_.first->getMutex());
    logInfo(RTPS_EDP, "");
    if (!computeKey(change))
    {
        logWarning(RTPS_EDP, "Received change with no Key");
    }

    ReaderHistory* reader_history = sedp_->subscriptions_reader_.second;

    // update the PDP Writer with this reader info
    if (!sedp_->addSubscriberFromHistory(*change))
    {
        reader_history->remove_change(change);
        return; // already there
    }

    if (change->kind == ALIVE)
    {
        add_reader_from_change(reader, change, sedp_);
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP, "Disposed Remote Reader, removing...");

        GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
        this->sedp_->mp_PDP->removeReaderProxyData(auxGUID);
        sedp_->removeSubscriberFromHistory(change->instanceHandle);
    }

    // Remove change from history.
    reader_history->remove_change(change);

    return;
}

void EDPServerSUBListener::onWriterChangeReceivedByAll(
        RTPSWriter* writer,
        CacheChange_t* change)
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

