// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DomainParticipantExtendedQos.hpp
 *
 */

#ifndef FASTDDS_DDS_DOMAIN_QOS__PARTICIPANTEXTENDEDQOS_HPP
#define FASTDDS_DDS_DOMAIN_QOS__PARTICIPANTEXTENDEDQOS_HPP

#include <fastdds/fastdds_dll.hpp>

#include "DomainParticipantQos.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipantExtendedQos : public DomainParticipantQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API DomainParticipantExtendedQos()
    {
    }

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API virtual ~DomainParticipantExtendedQos()
    {
    }

    DomainParticipantExtendedQos& operator =(
            const DomainParticipantQos& qos)
    {
        static_cast<DomainParticipantQos&>(*this) = qos;

        return *this;
    }

    bool operator ==(
            const DomainParticipantExtendedQos& b) const
    {
        return (this->domainId_ == b.domainId()) &&
               (DomainParticipantQos::operator ==(b));
    }

    bool operator ==(
            const DomainParticipantQos& b) const override
    {
        return (DomainParticipantQos::operator ==(b));
    }

    /**
     * Getter for domainId
     *
     * @return domainId reference
     */
    const uint32_t& domainId() const
    {
        return domainId_;
    }

    /**
     * Getter for domainId
     *
     * @return domainId reference
     */
    uint32_t& domainId()
    {
        return domainId_;
    }

private:

    //! DomainId to be used by the associated DomainParticipant (default: 0)
    uint32_t domainId_ = 0;

};


} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_DOMAIN_QOS__PARTICIPANTEXTENDEDQOS_HPP
