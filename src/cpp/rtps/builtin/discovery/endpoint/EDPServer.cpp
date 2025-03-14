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
 * @file EDPServer.cpp
 *
 */
#include <rtps/builtin/discovery/endpoint/EDPServer.hpp>
#include <rtps/builtin/discovery/endpoint/EDPServerListeners.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>

#include <rtps/reader/StatefulReader.hpp>
#include <rtps/writer/StatefulWriter.hpp>

using namespace ::eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

bool EDPServer::createSEDPEndpoints()
{
    // Assert that PDP object is a PDP SERVER
    assert(dynamic_cast<PDPServer*>(mp_PDP));

    bool created = true;  // Return code

    /* EDP Readers attributes */
    ReaderAttributes ratt;
    HistoryAttributes reader_history_att;
    RTPSReader* raux = nullptr;
    set_builtin_reader_history_attributes(reader_history_att);
    set_builtin_reader_attributes(ratt);
    ratt.endpoint.durabilityKind = durability_;

#if HAVE_SQLITE3
    ratt.endpoint.properties.properties().push_back(Property("dds.persistence.plugin", "builtin.SQLITE3"));
    ratt.endpoint.properties.properties().push_back(Property("dds.persistence.sqlite3.filename",
            get_pdp()->get_reader_persistence_file_name()));
#endif // if HAVE_SQLITE3

    /* EDP Writers attributes */
    WriterAttributes watt;
    HistoryAttributes writer_history_att;
    RTPSWriter* waux = nullptr;
    set_builtin_writer_history_attributes(writer_history_att);
    set_builtin_writer_attributes(watt);

#if HAVE_SQLITE3
    watt.endpoint.properties.properties().push_back(Property("dds.persistence.plugin", "builtin.SQLITE3"));
    watt.endpoint.properties.properties().push_back(Property("dds.persistence.sqlite3.filename",
            get_pdp()->get_writer_persistence_file_name()));
#endif // if HAVE_SQLITE3

    watt.endpoint.durabilityKind = durability_;
    watt.mode = ASYNCHRONOUS_WRITER;

    /* EDP Listeners */
    publications_listener_ = new EDPServerPUBListener(this);
    subscriptions_listener_ = new EDPServerSUBListener(this);

    /* Manage publications */
    if (m_discovery.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
    {
        /* If the participant declares that it will have publications, then it needs a writer to announce them, and a
         * reader to receive information about subscriptions that might match the participant's publications.
         *    1. Create publications writer
         *       1.1. Enable separate sending
         *       1.2. Set writer's data filter
         *    2. Create subscriptions reader
         */

        // 1. Set publications writer history and create the writer. Set `created` to the result.
        publications_writer_.second = new WriterHistory(writer_history_att);
        // 1.1. Enable separate sending so the filter can be called for each change and reader proxy
        watt.separate_sending = true;

        created &= this->mp_RTPSParticipant->createWriter(&waux, watt, publications_writer_.second,
                        publications_listener_, c_EntityId_SEDPPubWriter, true);

        if (created)
        {
            // Cast publications writer to a StatefulWriter, since we now that's what it is
            publications_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            // 1.2. Set publications writer data filter
            IReaderDataFilter* edp_publications_filter =
                    static_cast<ddb::EDPDataFilter<ddb::DiscoveryDataBase,
                            true>*>(&dynamic_cast<PDPServer*>(mp_PDP)->discovery_db());
            publications_writer_.first->reader_data_filter(edp_publications_filter);
            EPROSIMA_LOG_INFO(RTPS_EDP, "SEDP Publications Writer created");

            // TODO check if this should be done here or below
            publications_writer_.second->remove_all_changes();
        }
        else
        {
            // Something went wrong. Delete publications writer history and set it to nullptr. Return false
            delete(publications_writer_.second);
            publications_writer_.second = nullptr;
            EPROSIMA_LOG_ERROR(RTPS_EDP, "Error creating SEDP Publications Writer");
            return false;
        }

        // 2. Set subscriptions reader history and create the reader. Set `created` to the result.
        subscriptions_reader_.second = new ReaderHistory(reader_history_att);

        created &= this->mp_RTPSParticipant->createReader(&raux, ratt, subscriptions_reader_.second,
                        subscriptions_listener_, c_EntityId_SEDPSubReader, true, false);

        if (created)
        {
            // Cast subscriptions reader to a StatefulReader, since we now that's what it is
            subscriptions_reader_.first = dynamic_cast<StatefulReader*>(raux);
            EPROSIMA_LOG_INFO(RTPS_EDP, "SEDP Subscriptions Reader created");
        }
        else
        {
            // Something went wrong. Delete subscriptions reader history and set it to nullptr. Return false
            delete(subscriptions_reader_.second);
            subscriptions_reader_.second = nullptr;
            EPROSIMA_LOG_ERROR(RTPS_EDP, "Error creating SEDP Subscriptions Reader");
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_EDP, "Server operation requires the presence of all 4 builtin endpoints");
        return false;
    }

    /* Manage subscriptions */
    if (m_discovery.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
    {
        /* If the participant declares that it will have subscriptions, then it needs a writer to announce them, and a
         * reader to receive information about publications that might match the participant's subscriptions.
         *    1. Create subscriptions writer
         *       1.1. Enable separate sending
         *       1.2. Set writer's data filter
         *    2. Create publications reader
         */

        // 1. Set subscriptions writer history and create the writer. Set `created` to the result.
        subscriptions_writer_.second = new WriterHistory(writer_history_att);
        // 1.1. Enable separate sending so the filter can be called for each change and reader proxy
        watt.separate_sending = true;
        created &= this->mp_RTPSParticipant->createWriter(&waux, watt, subscriptions_writer_.second,
                        subscriptions_listener_, c_EntityId_SEDPSubWriter, true);

        if (created)
        {
            // Cast subscriptions writer to a StatefulWriter, since we now that's what it is
            subscriptions_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            // 1.2. Set subscriptions writer data filter
            IReaderDataFilter* edp_subscriptions_filter =
                    static_cast<ddb::EDPDataFilter<ddb::DiscoveryDataBase,
                            false>*>(&dynamic_cast<PDPServer*>(mp_PDP)->discovery_db());
            subscriptions_writer_.first->reader_data_filter(edp_subscriptions_filter);
            EPROSIMA_LOG_INFO(RTPS_EDP, "SEDP Subscriptions Writer created");

            // TODO check if this should be done here or below
            subscriptions_writer_.second->remove_all_changes();
        }
        else
        {
            // Something went wrong. Delete subscriptions writer history and set it to nullptr. Return false
            delete(subscriptions_writer_.second);
            subscriptions_writer_.second = nullptr;
            EPROSIMA_LOG_ERROR(RTPS_EDP, "Error creating SEDP Subscriptions Writer");
            return false;
        }

        // 2. Set publications reader history and create the reader. Set `created` to the result.
        publications_reader_.second = new ReaderHistory(writer_history_att);
        created &= this->mp_RTPSParticipant->createReader(&raux, ratt, publications_reader_.second,
                        publications_listener_, c_EntityId_SEDPPubReader, true, false);

        if (created)
        {
            // Cast publications reader to a StatefulReader, since we now that's what it is
            publications_reader_.first = dynamic_cast<StatefulReader*>(raux);
            EPROSIMA_LOG_INFO(RTPS_EDP, "SEDP Publications Reader created");
        }
        else
        {
            // Something went wrong. Delete publications reader history and set it to nullptr. Return false
            delete(publications_reader_.second);
            publications_reader_.second = nullptr;
            EPROSIMA_LOG_ERROR(RTPS_EDP, "Error creating SEDP Publications Reader");
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_EDP, "Server operation requires the presence of all 4 builtin endpoints");
        return false;
    }

    EPROSIMA_LOG_INFO(RTPS_EDP, "Creation finished");
    return created;
}

bool EDPServer::remove_reader(
        RTPSReader* rtps_reader)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, "Removing local reader: " << rtps_reader->getGuid().entityId);

    // Get subscriptions writer and reader guid
    auto* writer = &subscriptions_writer_;
    GUID_t guid = rtps_reader->getGuid();

    // Recover reader information
    std::string topic_name;
    {
        auto temp_reader_proxy_data = get_temporary_reader_proxies_pool().get();
        mp_PDP->lookupReaderProxyData(guid, *temp_reader_proxy_data);
        topic_name = temp_reader_proxy_data->topic_name.to_string();
    }

    // Remove proxy data associated with the reader
    if (mp_PDP->removeReaderProxyData(guid)
            && writer->first != nullptr)
    {
        // We need to create a DATA(Ur) here to added it to the discovery database, so that the disposal can be
        // propagated to remote clients
        CacheChange_t* change = EDPUtils::create_change(*writer, NOT_ALIVE_DISPOSED_UNREGISTERED, guid,
                        mp_PDP->builtin_attributes().readerPayloadSize);

        // Populate the DATA(Ur)
        if (change != nullptr)
        {
            WriteParams& wp = change->write_params;
            SampleIdentity local;
            local.writer_guid(writer->first->getGuid());
            local.sequence_number(writer->second->next_sequence_number());
            wp.sample_identity(local);
            wp.related_sample_identity(local);

            // Notify the DiscoveryDataBase
            if (get_pdp()->discovery_db().update(change, topic_name))
            {
                // From here on, the discovery database takes ownership of the CacheChange_t. Henceforth there are no
                // references to the CacheChange_t.
                // Ensure processing time for the cache by triggering the Server thread (which process the updates)
                get_pdp()->awake_routine_thread();
            }
            else
            {
                // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
                get_pdp()->release_change_from_writer(change);
            }
            return true;
        }
    }
    return false;
}

bool EDPServer::remove_writer(
        RTPSWriter* rtps_writer)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, "Removing local writer: " << rtps_writer->getGuid().entityId);

    // Get publications writer and writer guid
    auto* writer = &publications_writer_;
    GUID_t guid = rtps_writer->getGuid();

    // Recover writer information
    std::string topic_name;

    {
        auto temp_writer_proxy_data = get_temporary_writer_proxies_pool().get();
        mp_PDP->lookupWriterProxyData(guid, *temp_writer_proxy_data);
        topic_name = temp_writer_proxy_data->topic_name.to_string();
    }

    // Remove proxy data associated with the writer
    if (mp_PDP->removeWriterProxyData(guid)
            && writer->first != nullptr)
    {
        // We need to create a DATA(Uw) here to added it to the discovery database, so that the disposal can be
        // propagated to remote clients
        CacheChange_t* change = EDPUtils::create_change(*writer, NOT_ALIVE_DISPOSED_UNREGISTERED, guid,
                        mp_PDP->builtin_attributes().writerPayloadSize);

        // Populate the DATA(Uw)
        if (change != nullptr)
        {
            WriteParams& wp = change->write_params;
            SampleIdentity local;
            local.writer_guid(writer->first->getGuid());
            local.sequence_number(writer->second->next_sequence_number());
            wp.sample_identity(local);
            wp.related_sample_identity(local);

            // Notify the DiscoveryDataBase
            if (get_pdp()->discovery_db().update(change, topic_name))
            {
                // From here on, the discovery database takes ownership of the CacheChange_t. Henceforth there are no
                // references to the CacheChange_t.
                // Ensure processing time for the cache by triggering the Server thread (which process the updates)
                get_pdp()->awake_routine_thread();
            }
            else
            {
                // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
                get_pdp()->release_change_from_writer(change);
            }
            return true;
        }
    }
    return false;
}

bool EDPServer::process_writer_proxy_data(
        RTPSWriter* local_writer,
        WriterProxyData* wdata)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, "Processing local writer: " << wdata->guid.entityId);
    // We actually don't need the writer here
    (void)local_writer;

    // Get publications writer
    auto* writer = &publications_writer_;

    // Since the listeners will not be triggered for local writers, we need to manually create the DATA(w) and add it
    // to the discovery database.
    // Create an empty change add populate it with writer's information from its proxy
    CacheChange_t* change = nullptr;
    bool ret_val = serialize_writer_proxy_data(*wdata, *writer, true, &change);

    // If the was information about the writer, then fill some other DATA(w) fields and notify database
    if (change != nullptr)
    {
        // We must key-signed the CacheChange_t to avoid duplications:
        WriteParams& wp = change->write_params;
        SampleIdentity local;
        local.writer_guid(writer->first->getGuid());
        local.sequence_number(writer->second->next_sequence_number());
        wp.sample_identity(local);
        wp.related_sample_identity(local);

        // Notify the DiscoveryDataBase
        if (get_pdp()->discovery_db().update(change, wdata->topic_name.to_string()))
        {
            // From here on, the discovery database takes ownership of the CacheChange_t. Henceforth there are no
            // references to the CacheChange_t.
            // Ensure processing time for the cache by triggering the Server thread (which process the updates)
            get_pdp()->awake_routine_thread();
        }
        else
        {
            // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
            get_pdp()->release_change_from_writer(change);
        }
        // Return whether the DATA(w) was generated correctly
        return ret_val;
    }

    // Return the change to the pool and return false
    get_pdp()->release_change_from_writer(change);
    return false;
}

bool EDPServer::process_reader_proxy_data(
        RTPSReader* local_reader,
        ReaderProxyData* rdata)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, "Processing local reader: " << rdata->guid.entityId);
    // We actually don't need the reader here
    (void)local_reader;

    // Get subscriptions writer
    auto* writer = &subscriptions_writer_;

    // Since the listeners will not be triggered for local readers, we need to manually create the DATA(r) and add it
    // to the discovery database.
    // Create an empty change add populate it with readers's information from its proxy
    CacheChange_t* change = nullptr;
    bool ret_val = serialize_reader_proxy_data(*rdata, *writer, true, &change);

    // If the was information about the reader, then fill some other DATA(r) fields and notify database
    if (change != nullptr)
    {
        // We must key-signed the CacheChange_t to avoid duplications:
        WriteParams& wp = change->write_params;
        SampleIdentity local;
        local.writer_guid(writer->first->getGuid());
        local.sequence_number(writer->second->next_sequence_number());
        wp.sample_identity(local);
        wp.related_sample_identity(local);

        // Notify the DiscoveryDataBase
        if (get_pdp()->discovery_db().update(change, rdata->topic_name.to_string()))
        {
            // From here on, the discovery database takes ownership of the CacheChange_t. Henceforth there are no
            // references to the CacheChange_t.
            // Ensure processing time for the cache by triggering the Server thread (which process the updates)
            get_pdp()->awake_routine_thread();
        }
        else
        {
            // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
            get_pdp()->release_change_from_writer(change);
        }
        // Return whether the DATA(w) was generated correctly
        return ret_val;
    }

    // Return the change to the pool and return false
    get_pdp()->release_change_from_writer(change);
    return false;
}

bool EDPServer::process_disposal(
        fastdds::rtps::CacheChange_t* disposal_change,
        fastdds::rtps::ddb::DiscoveryDataBase& discovery_db,
        fastdds::rtps::GuidPrefix_t& change_guid_prefix,
        bool should_publish_disposal)
{
    bool ret_val = false;
    eprosima::fastdds::rtps::WriteParams wp = disposal_change->write_params;

    // DATA(Uw) or DATA(Ur) cases
    if (discovery_db.is_writer(disposal_change) || discovery_db.is_reader(disposal_change))
    {
        EPROSIMA_LOG_INFO(RTPS_PDP_SERVER_DISPOSAL, "Disposal is: " <<
                (discovery_db.is_writer(
                    disposal_change) ? "writer" : "reader") << " handle: " << disposal_change->instanceHandle);

        auto builtin_pair = get_builtin_writer_history_pair_by_entity(disposal_change->writerGUID.entityId);

        if (nullptr != builtin_pair.first && nullptr != builtin_pair.second)
        {
            // Lock EDP writer
            std::unique_lock<fastdds::RecursiveTimedMutex> lock(builtin_pair.first->getMutex());

            // Remove all DATA(w/r) with the same sample identity as the DATA(Uw/Ur) from EDP PUBs/Subs writer's history
            discovery_db.remove_related_alive_from_history_nts(builtin_pair.second, change_guid_prefix);

            if (should_publish_disposal)
            {
                disposal_change->writerGUID.entityId = builtin_pair.first->getGuid().entityId;
                builtin_pair.second->add_change(disposal_change, wp);
            }

            ret_val = true;
        }
    }

    return ret_val;
}

bool EDPServer::process_and_release_change(
        fastdds::rtps::CacheChange_t* change,
        bool release_from_reader)
{
    bool ret_val = false;

    auto pdp = get_pdp();

    auto builtin_to_remove_from = get_builtin_writer_history_pair_by_entity(change->writerGUID.entityId);

    if (nullptr != builtin_to_remove_from.first && nullptr != builtin_to_remove_from.second)
    {
        pdp->remove_change_from_writer_history(
            builtin_to_remove_from.first,
            builtin_to_remove_from.second,
            change,
            false);

        if (release_from_reader)
        {
            auto builtin_to_release = get_builtin_reader_history_pair_by_entity(change->writerGUID.entityId);

            if (nullptr != builtin_to_release.first)
            {
                builtin_to_release.first->release_cache(change);
                ret_val = true;
            }
        }
        else
        {
            auto builtin_to_release = builtin_to_remove_from;

            builtin_to_release.second->release_change(change);
            ret_val = true;
        }
    }

    return ret_val;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
