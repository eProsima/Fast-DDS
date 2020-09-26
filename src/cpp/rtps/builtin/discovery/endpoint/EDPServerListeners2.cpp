// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file EDPServerListener2.cpp
 *
 */

#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/history/ReaderHistory.h>

#include <fastdds/dds/log/Log.hpp>

#include "./EDPServerListeners2.hpp"
#include "./EDPServer2.hpp"
#include "../participant/PDPServer2.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace eprosima::fastrtps::rtps;

PDPServer2* EDPServerPUBListener2::get_pdp()
{
    return sedp_->get_pdp();
}

EDPServerPUBListener2::EDPServerPUBListener2(
        EDPServer2* sedp)
    : EDPBasePUBListener(sedp->mp_RTPSParticipant->getAttributes().allocation.locators,
            sedp->mp_RTPSParticipant->getAttributes().allocation.data_limits)
    , sedp_(sedp)
{
}

void EDPServerPUBListener2::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;

    logInfo(RTPS_EDP, "Server EDP listener received new publisher info");

    if (!computeKey(change))
    {
        logWarning(RTPS_EDP, "Received change with no Key");
    }

    // Retrieve the topic
    GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
    std::string topic_name;

    if (get_pdp()->lookupWriterProxyData(auxGUID, temp_writer_data_))
    {
        topic_name = temp_writer_data_.topicName().to_string();
    }

    ReaderHistory* reader_history = sedp_->publications_reader_.second;

    if (change->kind == ALIVE)
    {
        // Note: change is not removed from history inside this method.
        add_writer_from_change(reader, reader_history, change, sedp_, false);
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP, "Disposed Remote Writer, removing...");

        get_pdp()->removeWriterProxyData(auxGUID);

        //Removing change from history
        reader_history->remove_change_and_reuse(change);
    }

    // notify the DiscoveryDataBase
    if (!topic_name.empty() &&
            get_pdp()->discovery_db().update(change, topic_name))
    {
        // assure processing time for the cache
        get_pdp()->awakeServerThread();

        // the discovery database takes ownership of the CacheChange_t
        // henceforth there are no references to the CacheChange_t
    }
    else
    {
        // if the database doesn't take the ownership remove
        reader_history->release_Cache(change);
    }
}

PDPServer2* EDPServerSUBListener2::get_pdp()
{
    return sedp_->get_pdp();
}

EDPServerSUBListener2::EDPServerSUBListener2(
        EDPServer2* sedp)
    : EDPBaseSUBListener(sedp->mp_RTPSParticipant->getAttributes().allocation.locators,
            sedp->mp_RTPSParticipant->getAttributes().allocation.data_limits)
    , sedp_(sedp)
{
}

void EDPServerSUBListener2::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;

    logInfo(RTPS_EDP, "Server EDP listener received new subscriber info");

    if (!computeKey(change))
    {
        logWarning(RTPS_EDP, "Received change with no Key");
    }

    // Retrieve the topic
    GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
    std::string topic_name;

    if (get_pdp()->lookupReaderProxyData(auxGUID, temp_reader_data_))
    {
        topic_name = temp_reader_data_.topicName().to_string();
    }

    ReaderHistory* reader_history = sedp_->subscriptions_reader_.second;

    if (change->kind == ALIVE)
    {
        // Note: change is not removed from history inside this method.
        add_reader_from_change(reader, reader_history, change, sedp_, false);
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP, "Disposed Remote Reader, removing...");

        get_pdp()->removeReaderProxyData(auxGUID);

        //Removing change from history
        reader_history->remove_change_and_reuse(change);
    }

    // notify the DiscoveryDataBase
    if (!topic_name.empty() &&
            get_pdp()->discovery_db().update(change, topic_name))
    {
        // assure processing time for the cache
        get_pdp()->awakeServerThread();

        // the discovery database takes ownership of the CacheChange_t
        // henceforth there are no references to the CacheChange_t
    }
    else
    {
        // if the database doesn't take the ownership remove
        reader_history->release_Cache(change);
    }
}

} /* namespace rtps */
} // namespace fastdds
} /* namespace eprosima */
