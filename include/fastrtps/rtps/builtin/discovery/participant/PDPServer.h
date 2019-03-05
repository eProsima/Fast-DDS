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
 * @file PDPServer.h
 *
 */

#ifndef PDPSERVER_H_
#define PDPSERVER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC


#include "PDP.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatefulWriter;
class StatefulReader;

/**
 * Class PDPServer manages client side of the discovery server mechanism
 *@ingroup DISCOVERY_MODULE
 */
class PDPServer : public PDP
{
    public:
    /**
     * Constructor
     * @param builtin Pointer to the BuiltinProcols object.
     */
    PDPServer(BuiltinProtocols* builtin);
    virtual ~PDPServer();

    void initializeParticipantProxyData(ParticipantProxyData* participant_data);

    /**
     * Initialize the PDP.
     * @param part Pointer to the RTPSParticipant.
     * @return True on success
     */
    bool initPDP(RTPSParticipantImpl* part);

    /**
     * Create the SPDP Writer and Reader
     * @return True if correct.
     */
    bool createPDPEndpoints();

    /**
     * This method assigns remote endpoints to the builtin endpoints defined in this protocol. It also calls the corresponding methods in EDP and WLP.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     */
    void assignRemoteEndpoints(ParticipantProxyData* pdata);

    void removeRemoteEndpoints(ParticipantProxyData * pdata);

    private:

    //!TimedEvent to periodically resend the local RTPSParticipant information.
    // TODO: Create a new TimedEvent associated with PDP servers acknowledgement

};

}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* PDPSERVER_H_ */
