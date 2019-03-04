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

#ifndef PDPSIMPLE_H_
#define PDPSIMPLE_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC


#include "PDP.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatelessWriter;
class StatelessReader;
class ResendParticipantProxyDataPeriod;

/**
 * Class PDPSimple that implements the SimpleRTPSParticipantDiscoveryProtocol as defined in the RTPS specification.
 *@ingroup DISCOVERY_MODULE
 */
class PDPSimple : public PDP
{
    friend class ResendRTPSParticipantProxyDataPeriod;

    public:
    /**
     * Constructor
     * @param builtin Pointer to the BuiltinProcols object.
     */
    PDPSimple(BuiltinProtocols* builtin);
    virtual ~PDPSimple();

    void initializeParticipantProxyData(ParticipantProxyData* participant_data);

    /**
     * Initialize the PDP.
     * @param part Pointer to the RTPSParticipant.
     * @return True on success
     */
    bool initPDP(RTPSParticipantImpl* part);

    /**
     * Force the sending of our local DPD to all remote RTPSParticipants and multicast Locators.
     * @param new_change If true a new change (with new seqNum) is created and sent; if false the last change is re-sent
     * @param dispose Sets change kind to NOT_ALIVE_DISPOSED_UNREGISTERED
     */
    virtual void announceParticipantState(bool new_change, bool dispose = false);

    //!Stop the RTPSParticipantAnnouncement (only used in tests).
    void stopParticipantAnnouncement();
    //!Reset the RTPSParticipantAnnouncement (only used in tests).
    void resetParticipantAnnouncement();

    /**
     * This method assigns remtoe endpoints to the builtin endpoints defined in this protocol. It also calls the corresponding methods in EDP and WLP.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     */
    void assignRemoteEndpoints(ParticipantProxyData* pdata);

    void removeRemoteEndpoints(ParticipantProxyData * pdata);

    private:

    //!TimedEvent to periodically resend the local RTPSParticipant information.
    ResendParticipantProxyDataPeriod* mp_resendParticipantTimer;

    /**
     * Create the SPDP Writer and Reader
     * @return True if correct.
     */
    bool createPDPEndpoints();

};

}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* PDPSIMPLE_H_ */
