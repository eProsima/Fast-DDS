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
 * @file EDPServer2.cpp
 *
 */

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastdds/rtps/reader/StatefulReader.h>

#include "./EDPServerListeners2.hpp"
#include "./EDPServer2.hpp"

using namespace ::eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

bool EDPServer2::createSEDPEndpoints()
{
    // Assert that there is PDP SERVER
    assert(dynamic_cast<PDPServer2*>(mp_PDP));

    bool created = true;  // Return code

    /* EDP Readers attributes */
    ReaderAttributes ratt;
    HistoryAttributes reader_history_att;
    RTPSReader* raux = nullptr;
    set_builtin_reader_history_attributes(reader_history_att);
    set_builtin_reader_attributes(ratt);
    ratt.endpoint.durabilityKind = VOLATILE;

    /* EDP Writers attributes */
    WriterAttributes watt;
    HistoryAttributes writer_history_att;
    RTPSWriter* waux = nullptr;
    set_builtin_writer_history_attributes(writer_history_att);
    set_builtin_writer_attributes(watt);
    watt.endpoint.durabilityKind = VOLATILE;
    watt.mode = ASYNCHRONOUS_WRITER;

    /* EDP Listeners */
    publications_listener_ = new EDPServerPUBListener2(this);
    subscriptions_listener_ = new EDPServerSUBListener2(this);

    /* Manage publications */
    if(m_discovery.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
    {
        /* If the participant declares that it will have publications, then it needs a writer to announce them, and a
         * reader to receive information about subscriptions that might match the participant's publications.
         *    1. Create publications writer
         *       1.1. Set writer's data filter
         *       1.2. Enable separate sending
         *    2. Create subscriptions reader
         */

        // 1. Set publications writer history and create the writer. Set `created` to the result.
        publications_writer_.second = new WriterHistory(writer_history_att);
        created &= this->mp_RTPSParticipant->createWriter(&waux, watt, publications_writer_.second,
                        publications_listener_, c_EntityId_SEDPPubWriter, true);

        if (created)
        {
            // Cast publications writer to a StatefulWriter, since we now that's what it is
            publications_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            // 1.1. Set publications writer data filter
            IReaderDataFilter* edp_publications_filter =
                    static_cast<ddb::EDPDataFilter<ddb::DiscoveryDataBase,
                            true>*>(&dynamic_cast<PDPServer2*>(mp_PDP)->discovery_db());
            publications_writer_.first->reader_data_filter(edp_publications_filter);
            // 1.2. Enable separate sending so the filter can be called for each change and reader proxy
            publications_writer_.first->set_separate_sending(true);
            logInfo(RTPS_EDP, "SEDP Publications Writer created");
        }
        else
        {
            // Something went wrong. Delete publications writer history and set it to nullptr. Return false
            delete(publications_writer_.second);
            publications_writer_.second = nullptr;
            logError(RTPS_EDP, "Error creating SEDP Publications Writer");
            return false;
        }

        // 2. Set subscriptions reader history and create the reader. Set `created` to the result.
        subscriptions_reader_.second = new ReaderHistory(reader_history_att);
        created &= this->mp_RTPSParticipant->createReader(&raux, ratt, subscriptions_reader_.second,
                        subscriptions_listener_, c_EntityId_SEDPSubReader, true);

        if (created)
        {
            // Cast subscriptions reader to a StatefulReader, since we now that's what it is
            subscriptions_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP, "SEDP Subscriptions Reader created");
        }
        else
        {
            // Something went wrong. Delete subscriptions reader history and set it to nullptr. Return false
            delete(subscriptions_reader_.second);
            subscriptions_reader_.second = nullptr;
            logError(RTPS_EDP, "Error creating SEDP Subscriptions Reader");
            return false;
        }
    }
    else
    {
        logError(RTPS_EDP, "Server operation requires the presence of all 4 builtin endpoints");
        return false;
    }

    /* Manage subscriptions */
    if(m_discovery.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
    {
        /* If the participant declares that it will have subscriptions, then it needs a writer to announce them, and a
         * reader to receive information about publications that might match the participant's subscriptions.
         *    1. Create subscriptions writer
         *       1.1. Set writer's data filter
         *       1.2. Enable separate sending
         *    2. Create publications reader
         */

        // 1. Set subscriptions writer history and create the writer. Set `created` to the result.
        subscriptions_writer_.second = new WriterHistory(writer_history_att);
        created &= this->mp_RTPSParticipant->createWriter(&waux, watt, subscriptions_writer_.second,
                        subscriptions_listener_, c_EntityId_SEDPSubWriter, true);

        if (created)
        {
            // Cast subscriptions writer to a StatefulWriter, since we now that's what it is
            subscriptions_writer_.first = dynamic_cast<StatefulWriter*>(waux);
            // 1.1. Set subscriptions writer data filter
            IReaderDataFilter* edp_subscriptions_filter =
                    static_cast<ddb::EDPDataFilter<ddb::DiscoveryDataBase,
                            false>*>(&dynamic_cast<PDPServer2*>(mp_PDP)->discovery_db());
            subscriptions_writer_.first->reader_data_filter(edp_subscriptions_filter);
            // 1.2. Enable separate sending so the filter can be called for each change and reader proxy
            publications_writer_.first->set_separate_sending(true);
            logInfo(RTPS_EDP, "SEDP Subscriptions Writer created");

        }
        else
        {
            // Something went wrong. Delete subscriptions writer history and set it to nullptr. Return false
            delete(subscriptions_writer_.second);
            subscriptions_writer_.second = nullptr;
            logError(RTPS_EDP, "Error creating SEDP Subscriptions Writer");
            return false;
        }

        // 2. Set publications reader history and create the reader. Set `created` to the result.
        publications_reader_.second = new ReaderHistory(writer_history_att);
        created &= this->mp_RTPSParticipant->createReader(&raux, ratt, publications_reader_.second,
                        publications_listener_, c_EntityId_SEDPPubReader, true);

        if (created)
        {
            // Cast publications reader to a StatefulReader, since we now that's what it is
            publications_reader_.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP, "SEDP Publications Reader created");
        }
        else
        {
            // Something went wrong. Delete publications reader history and set it to nullptr. Return false
            delete(publications_reader_.second);
            publications_reader_.second = nullptr;
            logError(RTPS_EDP, "Error creating SEDP Publications Reader");
            return false;
        }
    }
    else
    {
        logError(RTPS_EDP, "Server operation requires the presence of all 4 builtin endpoints");
        return false;
    }

    logInfo(RTPS_EDP, "Creation finished");
    return created;
}

bool EDPServer2::removeLocalReader(
        RTPSReader* R)
{
    logInfo(RTPS_EDP, "Removing local reader: " << R->getGuid().entityId);

    // Get subscriptions writer and reader guid
    auto* writer = &subscriptions_writer_;
    GUID_t guid = R->getGuid();

    // Recover reader information
    std::string topic_name;
    {
        std::lock_guard<std::mutex> data_guard(temp_data_lock_);
        mp_PDP->lookupReaderProxyData(guid, temp_reader_proxy_data_);
        topic_name = temp_reader_proxy_data_.topicName().to_string();
    }

    // Remove proxy data associated with the reader
    if (mp_PDP->removeReaderProxyData(guid)
            && writer->first != nullptr)
    {
        // We need to create a DATA(Ur) here to added it to the discovery database, so that the disposal can be
        // propagated to remote clients
        CacheChange_t* change = writer->first->new_change(
            [this]() -> uint32_t
            {
                return mp_PDP->builtin_attributes().readerPayloadSize;
            },
            NOT_ALIVE_DISPOSED_UNREGISTERED, guid);

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
                get_pdp()->awakeServerThread();
            }
            else
            {
                // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
                get_pdp()->mp_PDPWriterHistory->release_Cache(change);
            }
            return true;
        }
    }
    return false;
}

bool EDPServer2::removeLocalWriter(
        RTPSWriter* W)
{
    logInfo(RTPS_EDP, "Removing local writer: " << W->getGuid().entityId);

    // Get publications writer and writer guid
    auto* writer = &publications_writer_;
    GUID_t guid = W->getGuid();

    // Recover writer information
    std::string topic_name;
    {
        std::lock_guard<std::mutex> data_guard(temp_data_lock_);
        mp_PDP->lookupWriterProxyData(guid, temp_writer_proxy_data_);
        topic_name = temp_writer_proxy_data_.topicName().to_string();
    }

    // Remove proxy data associated with the writer
    if (mp_PDP->removeWriterProxyData(guid)
            && writer->first != nullptr)
    {
        // We need to create a DATA(Uw) here to added it to the discovery database, so that the disposal can be
        // propagated to remote clients
        CacheChange_t* change = writer->first->new_change(
        [this]() -> uint32_t
        {
            return mp_PDP->builtin_attributes().writerPayloadSize;
        },
        NOT_ALIVE_DISPOSED_UNREGISTERED, guid);

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
                get_pdp()->awakeServerThread();
            }
            else
            {
                // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
                get_pdp()->mp_PDPWriterHistory->release_Cache(change);
            }
            return true;
        }
    }
    return false;
}

bool EDPServer2::processLocalWriterProxyData(
        RTPSWriter* local_writer,
        WriterProxyData* wdata)
{
    logInfo(RTPS_EDP, "Processing local writer: " << wdata->guid().entityId);
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
        if (get_pdp()->discovery_db().update(change, wdata->topicName().to_string()))
        {
            // From here on, the discovery database takes ownership of the CacheChange_t. Henceforth there are no
            // references to the CacheChange_t.
            // Ensure processing time for the cache by triggering the Server thread (which process the updates)
            get_pdp()->awakeServerThread();
        }
        else
        {
            // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
            get_pdp()->mp_PDPWriterHistory->release_Cache(change);
        }
        // Return whether the DATA(w) was generated correctly
        return ret_val;
    }

    // Return the change to the pool and return false
    get_pdp()->mp_PDPWriterHistory->release_Cache(change);
    return false;
}

bool EDPServer2::processLocalReaderProxyData(
        RTPSReader* local_reader,
        ReaderProxyData* rdata)
{
    logInfo(RTPS_EDP, "Processing local reader: " << rdata->guid().entityId);
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
        if (get_pdp()->discovery_db().update(change, rdata->topicName().to_string()))
        {
            // From here on, the discovery database takes ownership of the CacheChange_t. Henceforth there are no
            // references to the CacheChange_t.
            // Ensure processing time for the cache by triggering the Server thread (which process the updates)
            get_pdp()->awakeServerThread();
        }
        else
        {
            // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
            get_pdp()->mp_PDPWriterHistory->release_Cache(change);
        }
        // Return whether the DATA(w) was generated correctly
        return ret_val;
    }

    // Return the change to the pool and return false
    get_pdp()->mp_PDPWriterHistory->release_Cache(change);
    return false;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
