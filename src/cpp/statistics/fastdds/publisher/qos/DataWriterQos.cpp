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
 * @file DataWriterQos.hpp
 */

#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>

#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

DataWriterQos::DataWriterQos()
{
    // Specific implementation for recommended statistics DataWriterQos
    reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    publish_mode().kind = eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE;
    publish_mode().flow_controller_name = eprosima::fastdds::rtps::FASTDDS_STATISTICS_FLOW_CONTROLLER_DEFAULT;
    history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
    history().depth = 10;
    properties().properties().emplace_back("fastdds.push_mode", "false");
}

} // dds
} // statistics
} // fastdds
} // eprosima
