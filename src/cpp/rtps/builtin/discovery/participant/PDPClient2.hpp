// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPClient2.hpp
 *
 */

#ifndef _FASTDDS_RTPS_PDPCLIENT2_H_
#define _FASTDDS_RTPS_PDPCLIENT2_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include "./DSClientEvent2.hpp"

namespace eprosima {
namespace fastrtps {
namespace rtps {

class StatefulWriter;
class StatefulReader;

} // namespace eprosima
} // namespace fastrtps
} // namespace rtps

// To be eventually removed together with eprosima::fastrtps
namespace aux = eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class PDPClient manages client side of the discovery server mechanism
 *@ingroup DISCOVERY_MODULE
 */
class PDPClient2 : public aux::PDP
{
    friend class DSClientEvent2;

public:

    /**
     * Constructor
     * @param builtin Pointer to the BuiltinProcols object.
     * @param allocation Participant allocation parameters.
     */
    PDPClient2(
            aux::BuiltinProtocols* builtin,
            const aux::RTPSParticipantAllocationAttributes& allocation);
    ~PDPClient2();

    void initializeParticipantProxyData(aux::ParticipantProxyData* participant_data) override;

    /**
     * Initialize the PDP.
     * @param part Pointer to the RTPSParticipant.
     * @return True on success
     */
    bool init(aux::RTPSParticipantImpl* part) override;

    /**
     * Creates an initializes a new participant proxy from a DATA(p) raw info
     * @param p from DATA msg deserialization
     * @param writer_guid GUID of originating writer
     * @return new ParticipantProxyData * or nullptr on failure
     */
    aux::ParticipantProxyData* createParticipantProxyData(
        const aux::ParticipantProxyData& p,
        const aux::GUID_t& writer_guid) override;

    /**
     * Create the SPDP Writer and Reader
     * @return True if correct.
     */
    bool createPDPEndpoints() override;

    /**
     * Force the sending of our local PDP to all servers
     * @param new_change If true a new change (with new seqNum) is created and sent;
     * if false the last change is re-sent
     * @param dispose Sets change kind to NOT_ALIVE_DISPOSED_UNREGISTERED
     * @param wparams allows to identify the change
     */
    void announceParticipantState(
        bool new_change,
        bool dispose = false,
        aux::WriteParams& wparams = aux::WriteParams::WRITE_PARAM_DEFAULT) override;

    void assignRemoteEndpoints(aux::ParticipantProxyData* pdata) override;
    void removeRemoteEndpoints(aux::ParticipantProxyData* pdata) override;
    void notifyAboveRemoteEndpoints(const aux::ParticipantProxyData& pdata) override;

    private:

    /**
    * TimedEvent for client synchronization:
    */
    DSClientEvent2* mp_sync;

};

} // namespace rtps
} // namepsace fastdds
} // namespace eprosima
#endif
#endif /* _FASTDDS_RTPS_PDPCLIENT2_H_ */
