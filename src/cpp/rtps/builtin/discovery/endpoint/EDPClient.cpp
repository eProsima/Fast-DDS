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
 * @file EDPClient.cpp
 *
 */

#include <rtps/builtin/discovery/endpoint/EDPClient.h>

#include <mutex>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/participant/PDP.h>
#if HAVE_SECURITY
#include <rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#endif // if HAVE_SECURITY
#include <rtps/writer/StatefulWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

bool EDPClient::process_reader_proxy_data(
        RTPSReader* local_reader,
        ReaderProxyData* rdata)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, rdata->guid.entityId);
    (void)local_reader;

    auto* writer = &subscriptions_writer_;

#if HAVE_SECURITY
    if (local_reader->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &subscriptions_secure_writer_;
    }
#endif // if HAVE_SECURITY

    CacheChange_t* change = nullptr;
    bool ret_val = serialize_reader_proxy_data(*rdata, *writer, true, &change);
    if (change != nullptr)
    {
        // We must key-signed the CacheChange_t to avoid duplications:
        WriteParams wp;
        SampleIdentity local;
        local.writer_guid(writer->first->getGuid());
        local.sequence_number(writer->second->next_sequence_number());
        wp.sample_identity(local);
        wp.related_sample_identity(local);

        writer->second->add_change(change, wp);
    }
    return ret_val;
}

bool EDPClient::process_writer_proxy_data(
        RTPSWriter* rtps_writer,
        WriterProxyData* wdata)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, wdata->guid.entityId);
    (void)rtps_writer;

    auto* writer = &publications_writer_;

#if HAVE_SECURITY
    if (rtps_writer->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &publications_secure_writer_;
    }
#endif // if HAVE_SECURITY

    CacheChange_t* change = nullptr;
    bool ret_val = serialize_writer_proxy_data(*wdata, *writer, true, &change);
    if (change != nullptr)
    {
        // We must key-signed the CacheChange_t to avoid duplications:
        WriteParams wp;
        SampleIdentity local;
        local.writer_guid(writer->first->getGuid());
        local.sequence_number(writer->second->next_sequence_number());
        wp.sample_identity(local);
        wp.related_sample_identity(local);

        writer->second->add_change(change, wp);
    }
    return ret_val;
}

bool EDPClient::remove_writer(
        RTPSWriter* rtps_writer)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, rtps_writer->getGuid().entityId);

    auto* writer = &publications_writer_;

#if HAVE_SECURITY
    if (rtps_writer->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &publications_secure_writer_;
    }
#endif // if HAVE_SECURITY

    if (writer->first != nullptr)
    {
        InstanceHandle_t iH;
        iH = rtps_writer->getGuid();
        CacheChange_t* change = EDPUtils::create_change(*writer, NOT_ALIVE_DISPOSED_UNREGISTERED, iH,
                        mp_PDP->builtin_attributes().writerPayloadSize);
        if (change != nullptr)
        {
            {
                std::lock_guard<fastdds::RecursiveTimedMutex> guard(*writer->second->getMutex());
                for (auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if ((*ch)->instanceHandle == change->instanceHandle)
                    {
                        writer->second->remove_change(*ch);
                        break;
                    }
                }

            }

            // We must key-signed the CacheChange_t to avoid duplications:
            WriteParams wp;
            SampleIdentity local;
            local.writer_guid(writer->first->getGuid());
            local.sequence_number(writer->second->next_sequence_number());
            wp.sample_identity(local);
            wp.related_sample_identity(local);

            writer->second->add_change(change, wp);
        }
    }
    return mp_PDP->removeWriterProxyData(rtps_writer->getGuid());
}

bool EDPClient::remove_reader(
        RTPSReader* rtps_reader)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, rtps_reader->getGuid().entityId);

    auto* writer = &subscriptions_writer_;

#if HAVE_SECURITY
    if (rtps_reader->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &subscriptions_secure_writer_;
    }
#endif // if HAVE_SECURITY

    if (writer->first != nullptr)
    {
        InstanceHandle_t iH;
        iH = (rtps_reader->getGuid());
        CacheChange_t* change = EDPUtils::create_change(*writer, NOT_ALIVE_DISPOSED_UNREGISTERED, iH,
                        mp_PDP->builtin_attributes().writerPayloadSize);
        if (change != nullptr)
        {
            {
                std::lock_guard<fastdds::RecursiveTimedMutex> guard(*writer->second->getMutex());
                for (auto ch = writer->second->changesBegin(); ch != writer->second->changesEnd(); ++ch)
                {
                    if ((*ch)->instanceHandle == change->instanceHandle)
                    {
                        writer->second->remove_change(*ch);
                        break;
                    }
                }
            }

            // We must key-signed the CacheChange_t to avoid duplications:
            WriteParams wp;
            SampleIdentity local;
            local.writer_guid(writer->first->getGuid());
            local.sequence_number(writer->second->next_sequence_number());
            wp.sample_identity(local);
            wp.related_sample_identity(local);

            writer->second->add_change(change, wp);
        }
    }
    return mp_PDP->removeReaderProxyData(rtps_reader->getGuid());
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
