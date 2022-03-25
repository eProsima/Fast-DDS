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

#ifndef _FASTDDS_RTPS_PDPCLIENT_H_
#define _FASTDDS_RTPS_PDPCLIENT_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastdds/rtps/messages/RTPSMessageGroup.h>
#include <fastdds/rtps/builtin/discovery/participant/timedevent/DSClientEvent.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class StatefulWriter;
class StatefulReader;

/**
 * Class PDPClient manages client side of the discovery server mechanism
 *@ingroup DISCOVERY_MODULE
 */
class PDPClient : public PDP
{
    friend class DSClientEvent;

public:

    /**
     * Constructor
     * @param builtin Pointer to the BuiltinProcols object.
     * @param allocation Participant allocation parameters.
     */
    PDPClient(
            BuiltinProtocols* builtin,
            const RTPSParticipantAllocationAttributes& allocation,
            bool super_client = false);
    ~PDPClient();

    void initializeParticipantProxyData(
            ParticipantProxyData* participant_data) override;

    /**
     * Initialize the PDP.
     * @param part Pointer to the RTPSParticipant.
     * @return True on success
     */
    bool init(
            RTPSParticipantImpl* part) override;

    /**
     * Creates an initializes a new participant proxy from a DATA(p) raw info
     * @param p from DATA msg deserialization
     * @param writer_guid GUID of originating writer
     * @return new ParticipantProxyData * or nullptr on failure
     */
    ParticipantProxyData* createParticipantProxyData(
            const ParticipantProxyData& p,
            const GUID_t& writer_guid) override;

    /**
     * Create the SPDP Writer and Reader
     * @return True if correct.
     */
    bool createPDPEndpoints() override;

    /**
     * Check if all servers have acknowledge the client PDP data
     * This method must be called from a mutex protected context.
     * @return True if all can reach the client
     */
    bool all_servers_acknowledge_PDP();

    /**
     * Check if we have our PDP received data updated
     * This method must be called from a mutex protected context.
     * @return True if we known all the participants the servers are aware of
     */
    bool is_all_servers_PDPdata_updated();

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
            WriteParams& wparams = WriteParams::WRITE_PARAM_DEFAULT) override;

    /**
     * These methods wouldn't be needed under perfect server operation
     * (no need of dynamic endpoint allocation) but must be implemented
     * to solve server shutdown situations.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     */
    void assignRemoteEndpoints(
            ParticipantProxyData* pdata) override;
    void removeRemoteEndpoints(
            ParticipantProxyData* pdata) override;
    void notifyAboveRemoteEndpoints(
            const ParticipantProxyData& pdata) override;

    /**
     * This method removes a remote RTPSParticipant and all its writers and readers.
     * @param participant_guid GUID_t of the remote RTPSParticipant.
     * @param reason Why the participant is being removed (dropped vs removed)
     * @return true if correct.
     */
    bool remove_remote_participant(
            const GUID_t& participant_guid,
            ParticipantDiscoveryInfo::DISCOVERY_STATUS reason) override;

    /**
     * Matching server EDP endpoints
     * @return true if all servers have been discovered
     */
    bool match_servers_EDP_endpoints();

    /*
     * Update the list of remote servers
     */
    void update_remote_servers_list();

protected:

    /**
     * Manually match the local PDP reader with the PDP writer of a given server. The function is
     * not thread safe (nts) in the sense that it does not take the PDP mutex. It does however take
     * temp_data_lock_
     */
    void match_pdp_writer_nts_(
            const eprosima::fastdds::rtps::RemoteServerAttributes& server_att);

    /**
     * Manually match the local PDP writer with the PDP reader of a given server. The function is
     * not thread safe (nts) in the sense that it does not take the PDP mutex. It does however take
     * temp_data_lock_
     */
    void match_pdp_reader_nts_(
            const eprosima::fastdds::rtps::RemoteServerAttributes& server_att);

private:

    /**
     * TimedEvent for server synchronization:
     *   first stage: periodically resend the local RTPSParticipant information until
     *    all servers have acknowledge reception
     *   second stage: waiting PDP info is up to date before allowing EDP matching
     */
    DSClientEvent* mp_sync;

    //! flag to hightlight we need a server ping announcement
    bool _serverPing;

    //! flag to know this client must use super client participant type
    bool _super_client;
};

} // namespace rtps
} /* namespace fastrtps */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_PDPCLIENT_H_ */
