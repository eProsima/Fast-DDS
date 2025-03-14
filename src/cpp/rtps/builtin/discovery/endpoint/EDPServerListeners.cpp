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
 * @file EDPServerListener.cpp
 *
 */
#include <rtps/builtin/discovery/endpoint/EDPServerListeners.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <rtps/builtin/discovery/endpoint/EDPServer.hpp>
#include <rtps/builtin/discovery/participant/PDPServer.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/writer/StatefulWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

PDPServer* EDPServerPUBListener::get_pdp()
{
    return sedp_->get_pdp();
}

EDPServerPUBListener::EDPServerPUBListener(
        EDPServer* sedp)
    : sedp_(sedp)
{
}

void EDPServerPUBListener::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "");
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "------------------ EDP PUB SERVER LISTENER START ------------------");
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER,
            "-------------------- " << sedp_->mp_RTPSParticipant->getGuid() << " --------------------");

    // Create a new change from the one received
    CacheChange_t* change = (CacheChange_t*)change_in;
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "EDP Server PUB Message received: " << change_in->instanceHandle);

    // DATA(w)s should have key
    if (!computeKey(change))
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP_LISTENER, "Received change with no Key");
    }

    // Get BaseReader
    BaseReader* base_reader = BaseReader::downcast(reader);
    // Get EDP publications' reader history
    ReaderHistory* reader_history = base_reader->get_history();

    // Related_sample_identity could be lost in message delivered, so we set as sample_identity
    // An empty related_sample_identity could lead into an empty sample_identity when resending this msg
    if (change->write_params.related_sample_identity() == SampleIdentity::unknown())
    {
        change->write_params.related_sample_identity(change->write_params.sample_identity());
    }

    // Reset the internal CacheChange_t union.
    change->writer_info.next = nullptr;
    change->writer_info.previous = nullptr;
    change->writer_info.num_sent_submessages = 0;

    // DATA(w) case: new writer or updated information about an existing writer
    if (change->kind == ALIVE)
    {
        EndpointAddedCallback writer_added_callback =
                std::bind(&EDPServerPUBListener::continue_with_writer, this, base_reader, change);

        // Note: add_writer_from_change() removes the change from the EDP publications' reader history, but it does not
        // return it to the pool
        add_writer_from_change(base_reader, reader_history, change, sedp_, false, writer_added_callback);

        // DATA(w) case: Retrieve the topic after creating the WriterProxyData (in add_writer_from_change()). This way, not matter
        // whether the DATA(w) is a new one or an update, the WriterProxyData exists, and so the topic can be retrieved

        // Stop and wait for callback in case of TypeLookupService needed time to process the types
        return;
    }
    // DATA(Uw) case
    else
    {
        EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "Disposed Remote Writer, removing...");

        // DATA(Uw) case: Retrieve the topic before removing the WriterProxyData. We need it to add the DATA(Uw) to the database
        GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
        std::string topic_name = get_writer_proxy_topic_name(auxGUID);

        // Remove WriterProxy data information
        get_pdp()->removeWriterProxyData(auxGUID);

        // Removing change from history, not returning the change to the pool, since the ownership will be yielded to the database
        reader_history->remove_change(reader_history->find_change(change), false);

        notify_discoverydatabase(topic_name, base_reader, change);
    }
}

std::string EDPServerPUBListener::get_writer_proxy_topic_name(
        GUID_t auxGUID)
{
    std::string topic_name = "";
    auto temp_writer_data = get_pdp()->get_temporary_writer_proxies_pool().get();
    if (get_pdp()->lookupWriterProxyData(auxGUID, *temp_writer_data))
    {
        topic_name = temp_writer_data->topic_name.to_string();
    }
    else
    {
        // This is a normal case in server redundancy scenarios
        EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "Writer Proxy Data missing for change " << auxGUID);
    }
    return topic_name;
}

void EDPServerPUBListener::notify_discoverydatabase(
        std::string topic_name,
        BaseReader* reader,
        CacheChange_t* change)
{
    // Notify the DiscoveryDataBase if it is enabled already
    // In case it is not enable, the change should not be updated or released because it is been
    // updated from a backup
    if (!get_pdp()->discovery_db().backup_in_progress())
    {
        if (topic_name.size() > 0 &&
                get_pdp()->discovery_db().update(change, topic_name))
        {
            // From here on, the discovery database takes ownership of the CacheChange_t. Henceforth there are no
            // references to the CacheChange_t.
            // Ensure processing time for the cache by triggering the Server thread (which process the updates
            get_pdp()->awake_routine_thread();
        }
        else
        {
            // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
            reader->release_cache(change);
        }
    }

    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER,
            "-------------------- " << sedp_->mp_RTPSParticipant->getGuid() << " --------------------");
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "------------------ EDP PUB SERVER LISTENER END ------------------");
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "");
}

void EDPServerPUBListener::continue_with_writer(
        BaseReader* reader,
        CacheChange_t* change)
{
    std::string topic_name = get_writer_proxy_topic_name(iHandle2GUID(change->instanceHandle));
    notify_discoverydatabase(topic_name, reader, change);
}

PDPServer* EDPServerSUBListener::get_pdp()
{
    return sedp_->get_pdp();
}

EDPServerSUBListener::EDPServerSUBListener(
        EDPServer* sedp)
    : sedp_(sedp)
{
}

void EDPServerSUBListener::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "");
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "------------------ EDP SUB SERVER LISTENER START ------------------");
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER,
            "-------------------- " << sedp_->mp_RTPSParticipant->getGuid() << " --------------------");

    // Create a new change from the one received
    CacheChange_t* change = (CacheChange_t*)change_in;
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "EDP Server SUB Message received: " << change_in->instanceHandle);

    // DATA(r)s should have key
    if (!computeKey(change))
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP_LISTENER, "Received change with no Key");
    }

    // Related_sample_identity could be lost in message delivered, so we set as sample_identity
    // An empty related_sample_identity could lead into an empty sample_identity when resending this msg
    if (change->write_params.related_sample_identity() == SampleIdentity::unknown())
    {
        change->write_params.related_sample_identity(change->write_params.sample_identity());
    }

    // Reset the internal CacheChange_t union.
    change->writer_info.next = nullptr;
    change->writer_info.previous = nullptr;
    change->writer_info.num_sent_submessages = 0;

    // Get BaseReader
    BaseReader* base_reader = BaseReader::downcast(reader);
    // Get EDP subscriptions' reader history
    ReaderHistory* reader_history = base_reader->get_history();

    // DATA(r) case: new reader or updated information about an existing reader
    if (change->kind == ALIVE)
    {
        EndpointAddedCallback reader_added_callback =
                std::bind(&EDPServerSUBListener::continue_with_reader, this, base_reader, change);

        // Note: add_reader_from_change() removes the change from the EDP subscriptions' reader history, but it does not
        // return it to the pool
        add_reader_from_change(base_reader, reader_history, change, sedp_, false, reader_added_callback);

        // DATA(w) case: Retrieve the topic after creating the ReaderProxyData (in add_reader_from_change()). This way, not matter
        // whether the DATA(r) is a new one or an update, the ReaderProxyData exists, and so the topic can be retrieved

        // Stop and wait for callback in case of TypeLookupService needed time to process the types
        return;
    }
    // DATA(Ur) case
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "Disposed Remote Reader, removing...");

        // DATA(Uw) case: Retrieve the topic before removing the ReaderProxyData. We need it to add the DATA(Ur) to the database
        GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
        std::string topic_name = get_reader_proxy_topic_name(auxGUID);

        // Remove ReaderProxy data information
        get_pdp()->removeReaderProxyData(auxGUID);

        // Removing change from history, not returning the change to the pool, since the ownership will be yielded to
        // the database
        reader_history->remove_change(reader_history->find_change(change), false);

        notify_discoverydatabase(topic_name, base_reader, change);
    }
}

std::string EDPServerSUBListener::get_reader_proxy_topic_name(
        GUID_t auxGUID)
{
    std::string topic_name = "";
    auto temp_reader_data = get_pdp()->get_temporary_reader_proxies_pool().get();
    if (get_pdp()->lookupReaderProxyData(auxGUID, *temp_reader_data))
    {
        topic_name = temp_reader_data->topic_name.to_string();
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP_LISTENER, "Reader Proxy Data missing for change " << auxGUID);
    }
    return topic_name;
}

void EDPServerSUBListener::notify_discoverydatabase(
        std::string topic_name,
        BaseReader* reader,
        CacheChange_t* change)
{
    // Notify the DiscoveryDataBase if it is enabled already
    // In case it is not enable, the change should not be updated or released because it is been
    // updated from a backup
    if (!get_pdp()->discovery_db().backup_in_progress())
    {
        if (topic_name.size() > 0 &&
                get_pdp()->discovery_db().update(change, topic_name))
        {
            // From here on, the discovery database takes ownership of the CacheChange_t. Henceforth there are no
            // references to the CacheChange_t.
            // Ensure processing time for the cache by triggering the Server thread (which process the updates
            get_pdp()->awake_routine_thread();
        }
        else
        {
            // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
            reader->release_cache(change);
        }
    }

    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER,
            "-------------------- " << sedp_->mp_RTPSParticipant->getGuid() << " --------------------");
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "------------------ EDP SUB SERVER LISTENER END ------------------");
    EPROSIMA_LOG_INFO(RTPS_EDP_LISTENER, "");
}

void EDPServerSUBListener::continue_with_reader(
        BaseReader* reader,
        CacheChange_t* change)
{
    std::string topic_name = get_reader_proxy_topic_name(iHandle2GUID(change->instanceHandle));
    notify_discoverydatabase(topic_name, reader, change);
}

} /* namespace rtps */
} // namespace fastdds
} /* namespace eprosima */
