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

#include <fastrtps/rtps/network/NetworkFactory.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPServer.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include "../../../participant/RTPSParticipantImpl.h"
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>

#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/history/WriterHistory.h>

#include <fastrtps/rtps/common/InstanceHandle.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <mutex>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

void EDPBasePUBListener::add_writer_from_change(
        RTPSReader* reader,
        CacheChange_t* change,
        EDP* edp)
{
    //LOAD INFORMATION IN DESTINATION WRITER PROXY DATA
    const NetworkFactory& network = edp->mp_RTPSParticipant->network_factory();
    CDRMessage_t tempMsg(change->serializedPayload);
    if (temp_writer_data_.readFromCDRMessage(&tempMsg, network))
    {
        change->instanceHandle = temp_writer_data_.key();
        if (temp_writer_data_.guid().guidPrefix == edp->mp_RTPSParticipant->getGuid().guidPrefix
                && !ongoingDeserialization(edp))
        {
            logInfo(RTPS_EDP, "Message from own RTPSParticipant, ignoring");
            return;
        }

        //LOAD INFORMATION IN DESTINATION WRITER PROXY DATA
        auto copy_data_fun = [this, &network](
            WriterProxyData* data,
            bool updating,
            const ParticipantProxyData& participant_data)
                {
                    if (!temp_writer_data_.has_locators())
                    {
                        temp_writer_data_.set_remote_locators(participant_data.default_locators, network, true);
                    }

                    if (updating && !data->is_update_allowed(temp_writer_data_))
                    {
                        logWarning(RTPS_EDP,
                                "Received incompatible update for WriterQos. writer_guid = " << data->guid());
                    }
                    *data = temp_writer_data_;
                    return true;
                };

        GUID_t participant_guid;
        WriterProxyData* writer_data =
                edp->mp_PDP->addWriterProxyData(temp_writer_data_.guid(), participant_guid, copy_data_fun);
        if (writer_data != nullptr)
        {
            // At this point we can release reader lock, cause change is not used
            reader->getMutex().unlock();

            edp->pairing_writer_proxy_with_any_local_reader(participant_guid, writer_data);

            // Take again the reader lock.
            reader->getMutex().lock();
        }
        else //NOT ADDED BECAUSE IT WAS ALREADY THERE
        {
            logWarning(RTPS_EDP, "Received message from UNKNOWN RTPSParticipant, removing");
        }
    }
}

void EDPSimplePUBListener::onNewCacheChangeAdded(
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

    ReaderHistory* reader_history =
#if HAVE_SECURITY
            reader == sedp_->publications_secure_reader_.first ?
            sedp_->publications_secure_reader_.second :
#endif // if HAVE_SECURITY
            sedp_->publications_reader_.second;

    if (change->kind == ALIVE)
    {
        add_writer_from_change(reader, change, sedp_);
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP, "Disposed Remote Writer, removing...");
        GUID_t writer_guid = iHandle2GUID(change->instanceHandle);
        this->sedp_->mp_PDP->removeWriterProxyData(writer_guid);
    }

    //Removing change from history
    reader_history->remove_change(change);

    return;
}

bool EDPListener::computeKey(
        CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, PID_ENDPOINT_GUID);
}

bool EDPListener::ongoingDeserialization(
        EDP* edp)
{
    EDPServer* pServer = dynamic_cast<EDPServer*>(edp);

    if (pServer)
    {
        return pServer->ongoingDeserialization();
    }

    return false;
}

void EDPBaseSUBListener::add_reader_from_change(
        RTPSReader* reader,
        CacheChange_t* change,
        EDP* edp)
{
    //LOAD INFORMATION IN TEMPORAL WRITER PROXY DATA
    const NetworkFactory& network = edp->mp_RTPSParticipant->network_factory();
    CDRMessage_t tempMsg(change->serializedPayload);
    if (temp_reader_data_.readFromCDRMessage(&tempMsg, network))
    {
        change->instanceHandle = temp_reader_data_.key();
        if (temp_reader_data_.guid().guidPrefix == edp->mp_RTPSParticipant->getGuid().guidPrefix
                && !ongoingDeserialization(edp))
        {
            logInfo(RTPS_EDP, "From own RTPSParticipant, ignoring");
            return;
        }

        auto copy_data_fun = [this, &network](
            ReaderProxyData* data,
            bool updating,
            const ParticipantProxyData& participant_data)
                {
                    if (!temp_reader_data_.has_locators())
                    {
                        temp_reader_data_.set_remote_locators(participant_data.default_locators, network, true);
                    }

                    if (updating && !data->is_update_allowed(temp_reader_data_))
                    {
                        logWarning(RTPS_EDP,
                                "Received incompatible update for ReaderQos. reader_guid = " << data->guid());
                    }
                    *data = temp_reader_data_;
                    return true;
                };

        //LOOK IF IS AN UPDATED INFORMATION
        GUID_t participant_guid;
        ReaderProxyData* reader_data =
                edp->mp_PDP->addReaderProxyData(temp_reader_data_.guid(), participant_guid, copy_data_fun);
        if (reader_data != nullptr) //ADDED NEW DATA
        {
            // At this point we can release reader lock, cause change is not used
            reader->getMutex().unlock();

            edp->pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);

            // Take again the reader lock.
            reader->getMutex().lock();
        }
        else
        {
            logWarning(RTPS_EDP, "From UNKNOWN RTPSParticipant, removing");
        }
    }
}

void EDPSimpleSUBListener::onNewCacheChangeAdded(
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

    ReaderHistory* reader_history =
#if HAVE_SECURITY
            reader == sedp_->subscriptions_secure_reader_.first ?
            sedp_->subscriptions_secure_reader_.second :
#endif // if HAVE_SECURITY
            sedp_->subscriptions_reader_.second;

    if (change->kind == ALIVE)
    {
        add_reader_from_change(reader, change, sedp_);
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP, "Disposed Remote Reader, removing...");

        GUID_t reader_guid = iHandle2GUID(change->instanceHandle);
        this->sedp_->mp_PDP->removeReaderProxyData(reader_guid);
    }

    // Remove change from history.
    reader_history->remove_change(change);

    return;
}

void EDPSimplePUBListener::onWriterChangeReceivedByAll(
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
#endif // if HAVE_SECURITY
                sedp_->publications_writer_.second;

        writer_history->remove_change(change);
    }
}

void EDPSimpleSUBListener::onWriterChangeReceivedByAll(
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
#endif // if HAVE_SECURITY
                sedp_->subscriptions_writer_.second;

        writer_history->remove_change(change);
    }

}

} /* namespace rtps */
} // namespace fastrtps
} /* namespace eprosima */
