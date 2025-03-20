// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RTPSReliableWriterQos.hpp
 */

#ifndef FASTDDS_DDS_CORE_POLICY__RTPSRELIABLEWRITERQOS_HPP
#define FASTDDS_DDS_CORE_POLICY__RTPSRELIABLEWRITERQOS_HPP

#include <fastdds/rtps/attributes/WriterAttributes.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

//! Qos Policy to configure the DisablePositiveACKsQos and the writer timing attributes
class RTPSReliableWriterQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API RTPSReliableWriterQos()
    {
    }

    /**
     * @brief Destructor
     */
    virtual FASTDDS_EXPORTED_API ~RTPSReliableWriterQos() = default;

    bool operator ==(
            const RTPSReliableWriterQos& b) const
    {
        return (this->times == b.times) &&
               (this->disable_positive_acks == b.disable_positive_acks) &&
               (this->disable_heartbeat_piggyback == b.disable_heartbeat_piggyback);
    }

    inline void clear()
    {
        *this = RTPSReliableWriterQos();
    }

    //!Writer Timing Attributes
    fastdds::rtps::WriterTimes times;

    //!Disable positive acks QoS, implemented in the library.
    DisablePositiveACKsQosPolicy disable_positive_acks;

    //! Disable heartbeat piggyback mechanism.
    bool disable_heartbeat_piggyback = false;
};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_DDS_CORE_POLICY__RTPSRELIABLEWRITERQOS_HPP
