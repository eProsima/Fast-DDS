// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PortParameters.h
 */

#ifndef _FASTDDS_RTPS_PORT_PARAMETERS_H_
#define _FASTDDS_RTPS_PORT_PARAMETERS_H_

#include <fastdds/rtps/common/Types.h>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * Class PortParameters, to define the port parameters and gains related with the RTPS protocol.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class PortParameters
{
public:

    PortParameters()
        : portBase(7400)
        , domainIDGain(250)
        , participantIDGain(2)
        , offsetd0(0)
        , offsetd1(10)
        , offsetd2(1)
        , offsetd3(11)
    {
    }

    virtual ~PortParameters()
    {
    }

    bool operator ==(
            const PortParameters& b) const
    {
        return (this->portBase == b.portBase) &&
               (this->domainIDGain == b.domainIDGain) &&
               (this->participantIDGain == b.participantIDGain) &&
               (this->offsetd0 == b.offsetd0) &&
               (this->offsetd1 == b.offsetd1) &&
               (this->offsetd2 == b.offsetd2) &&
               (this->offsetd3 == b.offsetd3);
    }

    /**
     * Get a multicast port based on the domain ID.
     *
     * @param domainId Domain ID.
     * @return Multicast port
     */
    inline uint32_t getMulticastPort(
            uint32_t domainId) const
    {
        uint32_t port = portBase + domainIDGain * domainId + offsetd0;

        if (port > 65535)
        {
            logError(RTPS, "Calculated port number is too high. Probably the domainId is over 232 "
                    << "or portBase is too high.");
            std::cout << "Calculated port number is too high. Probably the domainId is over 232 "
                      << "or portBase is too high." << std::endl;
            std::cout.flush();
            exit(EXIT_FAILURE);
        }

        return port;
    }

    /**
     * Get a unicast port based on the domain ID and the participant ID.
     *
     * @param domainId Domain ID.
     * @param RTPSParticipantID Participant ID.
     * @return Unicast port
     */
    inline uint32_t getUnicastPort(
            uint32_t domainId,
            uint32_t RTPSParticipantID) const
    {
        uint32_t port = portBase + domainIDGain * domainId + offsetd1   + participantIDGain * RTPSParticipantID;

        if (port > 65535)
        {
            logError(RTPS, "Calculated port number is too high. Probably the domainId is over 232, there are "
                    << "too much participants created or portBase is too high.");
            std::cout << "Calculated port number is too high. Probably the domainId is over 232, there are "
                      << "too much participants created or portBase is too high." << std::endl;
            std::cout.flush();
            exit(EXIT_FAILURE);
        }

        return port;
    }

public:

    //!PortBase, default value 7400.
    uint16_t portBase;
    //!DomainID gain, default value 250.
    uint16_t domainIDGain;
    //!ParticipantID gain, default value 2.
    uint16_t participantIDGain;
    //!Offset d0, default value 0.
    uint16_t offsetd0;
    //!Offset d1, default value 10.
    uint16_t offsetd1;
    //!Offset d2, default value 1.
    uint16_t offsetd2;
    //!Offset d3, default value 11.
    uint16_t offsetd3;
};

} // namespace rtps
} /* namespace rtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_PORT_PARAMETERS_H_ */
