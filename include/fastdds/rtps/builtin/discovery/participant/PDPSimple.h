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
 * @file PDPSimple.h
 *
 */

#ifndef _FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_PDPSIMPLE_H_
#define _FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_PDPSIMPLE_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/builtin/discovery/participant/PDP.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class StatelessWriter;
class StatelessReader;

/**
 * Class PDPSimple that implements the SimpleRTPSParticipantDiscoveryProtocol as defined in the RTPS specification.
 * @ingroup DISCOVERY_MODULE
 */
class PDPSimple : public PDP
{
public:

    /**
     * Constructor
     * @param builtin Pointer to the BuiltinProcols object.
     * @param allocation Participant allocation parameters.
     */
    PDPSimple(
            BuiltinProtocols* builtin,
            const RTPSParticipantAllocationAttributes& allocation);

    virtual ~PDPSimple();

    /**
     * Initialize the PDP.
     * @param part Pointer to the RTPSParticipant.
     * @return True on success
     */
    bool init(RTPSParticipantImpl* part) override;

    /**
     * Creates an initializes a new participant proxy from a DATA(p) raw info
     * @param p ParticipantProxyData from DATA msg deserialization
     * @param writer_guid GUID of originating writer
     * @return new ParticipantProxyData * or nullptr on failure
     */
    ParticipantProxyData* createParticipantProxyData(
        const ParticipantProxyData& p,
        const GUID_t& writer_guid) override;

    /**
     * Some PDP classes require EDP matching with update PDP DATAs like EDPStatic
     * @return true if EDP endpoinst must be match
     */
    bool updateInfoMatchesEDP() override;

    /**
     * Force the sending of our local DPD to all remote RTPSParticipants and multicast Locators.
     * @param new_change If true a new change (with new seqNum) is created and sent; if false the last change is re-sent
     * @param dispose Sets change kind to NOT_ALIVE_DISPOSED_UNREGISTERED
     * @param wparams allows to identify the change
     */
    void announceParticipantState(
        bool new_change,
        bool dispose = false,
        WriteParams& wparams = WriteParams::WRITE_PARAM_DEFAULT) override;

    /**
     * This method assigns remote endpoints to the builtin endpoints defined in this protocol. It also calls
     * the corresponding methods in EDP and WLP.
     * @param pdata Pointer to the ParticipantProxyData object.
     */
    void assignRemoteEndpoints(ParticipantProxyData* pdata) override;

    /**
     * Remove remote endpoints from the participant discovery protocol
     * @param pdata Pointer to the ParticipantProxyData to remove
     */
    void removeRemoteEndpoints(ParticipantProxyData * pdata) override;

    /**
     * This method notifies EDP and WLP of the existence of a new participant.
     * @param pdata
     */
    void notifyAboveRemoteEndpoints(const ParticipantProxyData& pdata) override;

    /**
     * Activate a new Remote Endpoint that has been statically discovered.
     * @param pguid GUID_t of the participant.
     * @param userDefinedId User Defined ID.
     * @param kind Kind of endpoint.
     */
    bool newRemoteEndpointStaticallyDiscovered(
            const GUID_t& pguid,
            int16_t userDefinedId,
            EndpointKind_t kind);


private:

    void initializeParticipantProxyData(ParticipantProxyData* participant_data) override;

    /**
     * Create the SPDP Writer and Reader
     * @return True if correct.
     */
    bool createPDPEndpoints() override;

};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
#endif //_FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_PDPSIMPLE_H_
