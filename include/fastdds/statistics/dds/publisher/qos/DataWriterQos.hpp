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

#ifndef FASTDDS_STATISTICS_DDS_PUBLISHER_QOS__DATAWRITERQOS_HPP
#define FASTDDS_STATISTICS_DDS_PUBLISHER_QOS__DATAWRITERQOS_HPP

#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

/**
 * Class DataWriterQos: extends standard DDS DataWriterQos class to include specific default constructor for the
 * recommended DataWriterQos profile.
 * @ingroup STATISTICS_MODULE
 */
class DataWriterQos : public eprosima::fastdds::dds::DataWriterQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API DataWriterQos();
};

//! Constant to access default Statistics DataWriter Qos
const eprosima::fastdds::statistics::dds::DataWriterQos STATISTICS_DATAWRITER_QOS;

} // dds
} // statistics
} // fastdds
} // eprosima

#endif // FASTDDS_STATISTICS_DDS_PUBLISHER_QOS__DATAWRITERQOS_HPP
