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
 * @file DataReaderQos.hpp
 */

#ifndef FASTDDS_STATISTICS_DDS_SUBSCRIBER_QOS__DATAREADERQOS_HPP
#define FASTDDS_STATISTICS_DDS_SUBSCRIBER_QOS__DATAREADERQOS_HPP

#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

/**
 * Class DataReaderQos: extends standard DDS DataReaderQos class to include specific default constructor for the
 * recommended DataReaderQos profile.
 * @ingroup STATISTICS_MODULE
 */
class DataReaderQos : public eprosima::fastdds::dds::DataReaderQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API DataReaderQos();
};

/**
 * Class MonitorServiceDataReaderQos: extends standard DDS DataReaderQos class to include specific default constructor for the
 * recommended MonitorServiceDataReaderQos profile.
 * @ingroup STATISTICS_MODULE
 */
class MonitorServiceDataReaderQos : public eprosima::fastdds::dds::DataReaderQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API MonitorServiceDataReaderQos();
};

//! Constant to access default Statistics DataReader Qos
const eprosima::fastdds::statistics::dds::DataReaderQos STATISTICS_DATAREADER_QOS;

//! Constant to access default Monitor Service Statistics DataReader Qos
const eprosima::fastdds::statistics::dds::MonitorServiceDataReaderQos MONITOR_SERVICE_DATAREADER_QOS;

} // dds
} // statistics
} // fastdds
} // eprosima

#endif // FASTDDS_STATISTICS_DDS_SUBSCRIBER_QOS__DATAREADERQOS_HPP
