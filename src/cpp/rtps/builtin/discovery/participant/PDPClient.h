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

#include <rtps/attributes/ServerAttributes.hpp>
#include <rtps/builtin/discovery/participant/DS/DiscoveryServerPDPEndpoints.hpp>
#include <rtps/builtin/discovery/participant/DS/DiscoveryServerPDPEndpointsSecure.hpp>
#include <rtps/builtin/discovery/participant/PDP.h>
#include <rtps/builtin/discovery/participant/timedevent/DSClientEvent.h>
#include <rtps/messages/RTPSMessageGroup.hpp>

namespace eprosima {
namespace fastdds {
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
     * @param wparams allows to identify the change [unused]
     *
     * @warning this method uses the static variable reference as it does not use the parameter \c wparams
     * and this avoids a creation of an object WriteParams.
     * However, if in future versions this method uses this argument, it must change to the function
     * \c write_params_default for thread safety reasons.
     */
    void announceParticipantState(
            bool new_change,
            bool dispose,
            WriteParams& wparams) override;

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
            const ParticipantProxyData& pdata,
            bool notify_secure_endpoints) override;

    /**
     * This method removes a remote RTPSParticipant and all its writers and readers.
     * @param participant_guid GUID_t of the remote RTPSParticipant.
     * @param reason Why the participant is being removed (dropped vs removed)
     * @return true if correct.
     */
    bool remove_remote_participant(
            const GUID_t& participant_guid,
            ParticipantDiscoveryStatus reason) override;

#if HAVE_SECURITY
    bool pairing_remote_writer_with_local_reader_after_security(
            const GUID_t& local_reader,
            const WriterProxyData& remote_writer_data) override;

    bool pairing_remote_reader_with_local_writer_after_security(
            const GUID_t& local_reader,
            const ReaderProxyData& remote_reader_data) override;
#endif // HAVE_SECURITY

    /*
     * Update the list of remote servers
     */
    void update_remote_servers_list();

    /**
     * Get the list of remote servers to which the client is already connected.
     * @return A reference to the list of RemoteServerAttributes
     */
    const fastdds::rtps::RemoteServerList_t& connected_servers();

protected:

    void update_builtin_locators() override;

    /**
     * Manually match the local PDP reader with the PDP writer of a given server. The function is
     * not thread safe (nts) in the sense that it does not take the PDP mutex. It does however take
     * temp_data_lock_
     * @param server_att Remote server attributes
     * @param from_this_host Whether the server is from this host or not
     */
    void match_pdp_writer_nts_(
            const eprosima::fastdds::rtps::RemoteServerAttributes& server_att,
            bool from_this_host);

    /**
     * Manually match the local PDP writer with the PDP reader of a given server. The function is
     * not thread safe (nts) in the sense that it does not take the PDP mutex. It does however take
     * temp_data_lock_
     * @param server_att Remote server attributes
     * @param from_this_host Whether the server is from this host or not
     */
    void match_pdp_reader_nts_(
            const eprosima::fastdds::rtps::RemoteServerAttributes& server_att,
            bool from_this_host);

private:

    using PDP::announceParticipantState;

    /**
     * Manually match the local PDP reader with the PDP writer of a given server. The function is
     * not thread safe (nts) in the sense that it does not take the PDP mutex. It does however take
     * temp_data_lock_
     * @param server_att Remote server attributes
     * @param prefix_override GUID prefix of the server
     * @param from_this_host Whether the server is from this host or not
     */
    void match_pdp_writer_nts_(
            const eprosima::fastdds::rtps::RemoteServerAttributes& server_att,
            const eprosima::fastdds::rtps::GuidPrefix_t& prefix_override,
            bool from_this_host);

    /**
     * Manually match the local PDP writer with the PDP reader of a given server. The function is
     * not thread safe (nts) in the sense that it does not take the PDP mutex. It does however take
     * temp_data_lock_
     * @param server_att Remote server attributes
     * @param prefix_override GUID prefix of the server
     * @param from_this_host Whether the server is from this host or not
     */
    void match_pdp_reader_nts_(
            const eprosima::fastdds::rtps::RemoteServerAttributes& server_att,
            const eprosima::fastdds::rtps::GuidPrefix_t& prefix_override,
            bool from_this_host);

#if HAVE_SECURITY
    /**
     * Returns whether discovery should be secured
     */
    bool should_protect_discovery();

    /**
     * Performs creation of secured DS PDP endpoints
     */
    bool create_secure_ds_pdp_endpoints();

#endif  // HAVE_SECURITY

    /**
     * Performs creation of standard DS PDP endpoints
     */
    bool create_ds_pdp_endpoints();

    /**
     * Performs creation of DS (reliable) PDP endpoints.
     *
     * @param [in,out]  endpoints  Container where the created resources should be kept.
     * @param [in]      secure     Whether the created endpoints should be secure.
     *
     * @return whether the endpoints were successfully created.
     */
    bool create_ds_pdp_reliable_endpoints(
            DiscoveryServerPDPEndpoints& endpoints,
            bool is_discovery_protected);

    /**
     * Performs creation of DS best-effort PDP reader.
     *
     * @param [in,out]  endpoints  Container where the created resources should be kept.
     *
     * @return whether the reader was successfully created.
     */
    bool create_ds_pdp_best_effort_reader(
            DiscoveryServerPDPEndpointsSecure& endpoints);

    /**
     * Provides the functionality of notifyAboveRemoteEndpoints without being an override of that method.
     */
    void perform_builtin_endpoints_matching(
            const ParticipantProxyData& pdata);

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

    //! List of real connected servers
    std::list<eprosima::fastdds::rtps::RemoteServerAttributes> connected_servers_;
};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_PDPCLIENT_H_ */
