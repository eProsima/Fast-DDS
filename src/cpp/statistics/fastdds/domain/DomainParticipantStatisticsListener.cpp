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
}

void DomainParticipantStatisticsListener::on_statistics_data(
        const Data& statistics_data)
{
    std::lock_guard<std::mutex> guard(mtx_);

    DataWriter* writer = nullptr;
    EventKind data_kind = statistics_data._d();
    auto writer_it = writers_.find(data_kind);
    if (writer_it != writers_.end())
    {
        writer = writer_it->second;
    }

    if (nullptr != writer)
    {
        const void* data_sample = nullptr;

        switch (data_kind)
        {
            case EventKind::HISTORY2HISTORY_LATENCY:
                data_sample = &statistics_data.writer_reader_data();
                break;

            case EventKind::NETWORK_LATENCY:
                data_sample = &statistics_data.locator2locator_data();
                break;

            case EventKind::PUBLICATION_THROUGHPUT:
            case EventKind::SUBSCRIPTION_THROUGHPUT:
                data_sample = &statistics_data.entity_data();
                break;

            case EventKind::RTPS_SENT:
            case EventKind::RTPS_LOST:
                data_sample = &statistics_data.entity2locator_traffic();
                break;

            case EventKind::RESENT_DATAS:
            case EventKind::HEARTBEAT_COUNT:
            case EventKind::ACKNACK_COUNT:
            case EventKind::NACKFRAG_COUNT:
            case EventKind::GAP_COUNT:
            case EventKind::DATA_COUNT:
            case EventKind::PDP_PACKETS:
            case EventKind::EDP_PACKETS:
                data_sample = &statistics_data.entity_count();
                break;

            case EventKind::DISCOVERED_ENTITY:
                data_sample = &statistics_data.discovery_time();
                break;

            case EventKind::SAMPLE_DATAS:
                data_sample = &statistics_data.sample_identity_count();
                break;

            case EventKind::PHYSICAL_DATA:
                data_sample = &statistics_data.physical_data();
                break;
        }

        writer->write(const_cast<void*>(data_sample));
    }
}

} // dds
} // statistics
} // fastdds
} // eprosima
