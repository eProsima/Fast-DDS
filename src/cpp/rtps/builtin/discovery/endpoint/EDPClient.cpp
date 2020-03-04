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

#include <fastdds/rtps/builtin/discovery/endpoint/EDPClient.h>
#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/log/Log.h>

#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace rtps {

bool EDPClient::processLocalReaderProxyData(
        RTPSReader* local_reader,
        ReaderProxyData* rdata)
{
    logInfo(RTPS_EDP, rdata->guid().entityId);
    (void)local_reader;

    auto* writer = &subscriptions_writer_;

#if HAVE_SECURITY
    if (local_reader->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &subscriptions_secure_writer_;
    }
#endif

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

bool EDPClient::processLocalWriterProxyData(
        RTPSWriter* local_writer,
        WriterProxyData* wdata)
{
    logInfo(RTPS_EDP, wdata->guid().entityId);
    (void)local_writer;

    auto* writer = &publications_writer_;

#if HAVE_SECURITY
    if (local_writer->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &publications_secure_writer_;
    }
#endif

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

bool EDPClient::removeLocalWriter(
        RTPSWriter* W)
{
    logInfo(RTPS_EDP, W->getGuid().entityId);

    auto* writer = &publications_writer_;

#if HAVE_SECURITY
    if (W->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &publications_secure_writer_;
    }
#endif

    if (writer->first != nullptr)
    {
        InstanceHandle_t iH;
        iH = W->getGuid();
        CacheChange_t* change = writer->first->new_change(
            [this]() -> uint32_t
            {
                return mp_PDP->builtin_attributes().writerPayloadSize;
            },
            NOT_ALIVE_DISPOSED_UNREGISTERED, iH);
        if (change != nullptr)
        {
            {
                std::lock_guard<RecursiveTimedMutex> guard(*writer->second->getMutex());
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
    return mp_PDP->removeWriterProxyData(W->getGuid());
}

bool EDPClient::removeLocalReader(
        RTPSReader* R)
{
    logInfo(RTPS_EDP, R->getGuid().entityId);

    auto* writer = &subscriptions_writer_;

#if HAVE_SECURITY
    if (R->getAttributes().security_attributes().is_discovery_protected)
    {
        writer = &subscriptions_secure_writer_;
    }
#endif

    if (writer->first != nullptr)
    {
        InstanceHandle_t iH;
        iH = (R->getGuid());
        CacheChange_t* change = writer->first->new_change(
            [this]() -> uint32_t
            {
                return mp_PDP->builtin_attributes().writerPayloadSize;
            },
            NOT_ALIVE_DISPOSED_UNREGISTERED, iH);
        if (change != nullptr)
        {
            {
                std::lock_guard<RecursiveTimedMutex> guard(*writer->second->getMutex());
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
    return mp_PDP->removeReaderProxyData(R->getGuid());
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
