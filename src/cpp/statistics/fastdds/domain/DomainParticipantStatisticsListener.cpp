// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DomainParticipantStatisticsListener.cpp
 */

#include "DomainParticipantStatisticsListener.hpp"

#include <mutex>

#include <fastdds/dds/publisher/DataWriter.hpp>

#include <statistics/types/types.h>
namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

void DomainParticipantStatisticsListener::set_datawriter(
        EventKind kind,
        DataWriter* writer)
{
    std::lock_guard<std::mutex> guard(mtx_);
    writers_[kind] = writer;

    // If the writer is being activated then activate the corresponding bit in mask,
    // else deactivate it.
    if (writer)
    {
        enabled_writers_mask_ |= kind;
    }
    else
    {
        enabled_writers_mask_ &= ~kind;
    }
}

void DomainParticipantStatisticsListener::on_statistics_data(
        const Data& statistics_data)
{
    DataWriter* writer = nullptr;
    EventKind data_kind = statistics_data._d();

    // Find corresponding writer
    {
        std::lock_guard<std::mutex> lock(mtx_);
        auto writer_it = writers_.find(data_kind);
        if (writer_it != writers_.end())
        {
            writer = writer_it->second;
        }
    }

    if (nullptr != writer)
    {
        const void* data_sample = nullptr;

        switch (data_kind)
        {
            case EventKindBits::HISTORY2HISTORY_LATENCY:
                data_sample = &statistics_data.writer_reader_data();
                break;

            case EventKindBits::NETWORK_LATENCY:
                data_sample = &statistics_data.locator2locator_data();
                break;

            case EventKindBits::PUBLICATION_THROUGHPUT:
            case EventKindBits::SUBSCRIPTION_THROUGHPUT:
                data_sample = &statistics_data.entity_data();
                break;

            case EventKindBits::RTPS_SENT:
            case EventKindBits::RTPS_LOST:
                data_sample = &statistics_data.entity2locator_traffic();
                break;

            case EventKindBits::RESENT_DATAS:
            case EventKindBits::HEARTBEAT_COUNT:
            case EventKindBits::ACKNACK_COUNT:
            case EventKindBits::NACKFRAG_COUNT:
            case EventKindBits::GAP_COUNT:
            case EventKindBits::DATA_COUNT:
            case EventKindBits::PDP_PACKETS:
            case EventKindBits::EDP_PACKETS:
                data_sample = &statistics_data.entity_count();
                break;

            case EventKindBits::DISCOVERED_ENTITY:
                data_sample = &statistics_data.discovery_time();
                break;

            case EventKindBits::SAMPLE_DATAS:
                data_sample = &statistics_data.sample_identity_count();
                break;

            case EventKindBits::PHYSICAL_DATA:
                data_sample = &statistics_data.physical_data();
                break;
        }

        writer->write(const_cast<void*>(data_sample));
    }
}

uint32_t DomainParticipantStatisticsListener::enabled_writers_mask()
{
    return enabled_writers_mask_;
}

} // dds
} // statistics
} // fastdds
} // eprosima
