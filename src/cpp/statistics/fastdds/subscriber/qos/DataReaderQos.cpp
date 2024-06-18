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

#include <fastdds/statistics/dds/subscriber/qos/DataReaderQos.hpp>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/ResourceManagement.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

DataReaderQos::DataReaderQos()
{
    /* Specific implementation for recommended statistics DataReaderQos */
    reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
    history().depth = 100;
    // Setting history memory policy to PREALLOCATED_WITH_REALLOC_MEMORY_MODE allows for future type
    // extension with backwards compatibility
    endpoint().history_memory_policy = eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
}

MonitorServiceDataReaderQos::MonitorServiceDataReaderQos()
{
    /* Specific implementation for MonitorServiceDataReaderQos */
    reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
    history().depth = 1;

    resource_limits().max_instances = 1500;
    resource_limits().max_samples = 1600;
    resource_limits().max_samples_per_instance = 1;
}

} // dds
} // statistics
} // fastdds
} // eprosima
