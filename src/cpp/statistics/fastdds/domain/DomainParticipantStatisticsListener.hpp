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
 * @file DomainParticipantStatisticsListener.hpp
 */

#ifndef _STATISTICS_FASTDDS_DOMAIN_DOMAINPARTICIPANTSTATISTICSLISTENER_HPP
#define _STATISTICS_FASTDDS_DOMAIN_DOMAINPARTICIPANTSTATISTICSLISTENER_HPP

#include <fastdds/config.hpp>

#ifdef FASTDDS_STATISTICS

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>

#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/statistics/IListeners.hpp>

#include <statistics/types/types.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

struct DomainParticipantStatisticsListener : public IListener
{
    using DataWriter = eprosima::fastdds::dds::DataWriter;

    void set_datawriter(
            uint32_t kind,
            DataWriter* writer);

    void on_statistics_data(
            const Data& statistics_data) override;

    uint32_t enabled_writers_mask();

private:

    std::mutex mtx_;
    std::map<uint32_t, DataWriter*> writers_;
    std::atomic<uint32_t> enabled_writers_mask_{0};
};

} // dds
} // statistics
} // fastdds
} // eprosima

#endif  // FASTDDS_STATISTICS

#endif  // _STATISTICS_FASTDDS_DOMAIN_DOMAINPARTICIPANTSTATISTICSLISTENER_HPP
