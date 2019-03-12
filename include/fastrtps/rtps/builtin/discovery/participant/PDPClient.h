// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPClient.h
 *
 */

#ifndef PDPCLIENT_H_
#define PDPCLIENT_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC


#include "PDP.h"
#include "timedevent/DSClientEvent.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatefulWriter;
class StatefulReader;

/**
 * Class PDPServer manages client side of the discovery server mechanism
 *@ingroup DISCOVERY_MODULE
 */
class PDPClient : public PDP
{
    friend class DSClientEvent;

    public:
    /**
     * Constructor
     * @param builtin Pointer to the BuiltinProcols object.
     */
    PDPClient(BuiltinProtocols* builtin);
    ~PDPClient();

    void initializeParticipantProxyData(ParticipantProxyData* participant_data) override;

    /**
     * Initialize the PDP.
     * @param part Pointer to the RTPSParticipant.
     * @return True on success
     */
    bool initPDP(RTPSParticipantImpl* part) override;

    /**
     * Create the SPDP Writer and Reader
     * @return True if correct.
     */
    bool createPDPEndpoints() override;

    /**
     * Check if all servers have acknowledge the client PDP data
     * @return True if all can reach the client
     */
    bool all_servers_acknowledge_PDP();

    /**
     * Check if we have our PDP received data updated
     * @return True if we known all the participants the servers are aware of
     */
    bool is_all_servers_PDPdata_updated();

    /**
     * Force the sending of our local DPD to all servers 
     * @param new_change If true a new change (with new seqNum) is created and sent; if false the last change is re-sent
     * @param dispose Sets change kind to NOT_ALIVE_DISPOSED_UNREGISTERED
     */
    void announceParticipantState(bool new_change, bool dispose = false) override;

    //! Not currently needed for DSClientEvent announcement
    void stopParticipantAnnouncement() override {};

    //! Not currently needed for DSClientEvent announcement
    void resetParticipantAnnouncement() override {};

    /**
     * These methods wouldn't be needed under perfect server operation (no need of dynamic endpoint allocation) but must be implemented
     * to solve server shutdown situations.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     */
    void assignRemoteEndpoints(ParticipantProxyData* pdata) override;
    void removeRemoteEndpoints(ParticipantProxyData * pdata) override;

    //!Matching server EDP endpoints
    void match_all_server_EDP_endpoints();

    // TODO: see if the liveliness mechanism is compatible with the DATA(p) driven one and make corrections if needed

    private:

    /**
    * TimedEvent for server synchronization: 
    *   first stage: periodically resend the local RTPSParticipant information until all servers have acknowledge reception
    *   second stage: waiting PDP info is up to date before allowing EDP matching
    */ 
    DSClientEvent * mp_sync;
};

}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* PDPCLIENT_H_ */
