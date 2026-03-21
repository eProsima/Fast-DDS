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
 * @file RTPSReliableReaderQos.hpp
 */

#ifndef FASTDDS_DDS_CORE_POLICY__RTPSRELIABLEREADERQOS_HPP
#define FASTDDS_DDS_CORE_POLICY__RTPSRELIABLEREADERQOS_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

//! Qos Policy to configure the DisablePositiveACKsQos and the reader attributes
class RTPSReliableReaderQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API RTPSReliableReaderQos()
    {
    }

    /**
     * @brief Destructor
     */
    virtual FASTDDS_EXPORTED_API ~RTPSReliableReaderQos() = default;

    bool operator ==(
            const RTPSReliableReaderQos& b) const
    {
        return (this->times == b.times) &&
               (this->disable_positive_acks == b.disable_positive_acks);
    }

    inline void clear()
    {
        *this = RTPSReliableReaderQos();
    }

    /*!
     * @brief Times associated with the Reliable Readers events.
     */
    fastdds::rtps::ReaderTimes times;

    /*!
     * @brief Control the sending of positive ACKs
     */
    DisablePositiveACKsQosPolicy disable_positive_acks;
};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_DDS_CORE_POLICY__RTPSRELIABLEREADERQOS_HPP
