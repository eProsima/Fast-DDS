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
 * @file PDPServer.hpp
 *
 */

#ifndef _FASTDDS_RTPS_PDPSERVER2_H_
#define _FASTDDS_RTPS_PDPSERVER2_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/builtin/discovery/participant/PDP.h>

#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <fastdds/rtps/attributes/ServerAttributes.h>
#include <fastdds/rtps/history/History.h>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <rtps/builtin/discovery/database/DiscoveryDataBase.hpp>
#include <rtps/builtin/discovery/database/DiscoveryDataFilter.hpp>
#include <rtps/builtin/discovery/participant/timedevent/DServerEvent.hpp>
#include <rtps/builtin/discovery/participant/DS/DiscoveryServerPDPEndpointsSecure.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class PDPServer manages server side of the discovery server mechanism
 *@ingroup DISCOVERY_MODULE
 */
class PDPServer : public fastrtps::rtps::PDP
{
    friend class DServerRoutineEvent;
    friend class DServerPingEvent;
    friend class EDPServer;
    friend class PDPServerListener;
    friend class EDPServerListener2;

public:

    /**
     * Constructor
     * @param builtin Pointer to the BuiltinProcols object.
     * @param allocation participant's allocation settings
     * @param durability_kind the kind of persistence we want for the discovery data
     */
    PDPServer(
            fastrtps::rtps::BuiltinProtocols* builtin,
            const fastrtps::rtps::RTPSParticipantAllocationAttributes& allocation,
            fastrtps::rtps::DurabilityKind_t durability_kind = fastrtps::rtps::TRANSIENT_LOCAL);

    ~PDPServer();

    void initializeParticipantProxyData(
            fastrtps::rtps::ParticipantProxyData* participant_data) override;

    /**
     * Initialize the PDP.
     * @param part Pointer to the RTPSParticipant.
     * @return True on success
     */
    bool init(
            fastrtps::rtps::RTPSParticipantImpl* part) override;

    /**
     * Creates an initializes a new participant proxy from a DATA(p) raw info
     * @param p ParticipantProxyData from DATA msg deserialization
     * @param writer_guid GUID of originating writer
     * @return new ParticipantProxyData * or nullptr on failure
     */
    fastrtps::rtps::ParticipantProxyData* createParticipantProxyData(
            const fastrtps::rtps::ParticipantProxyData& p,
            const fastrtps::rtps::GUID_t& writer_guid) override;

    /**
     * Create the SPDP Writer and Reader
     * @return True if correct.
     */
    bool createPDPEndpoints() override;

    /**
     * This method removes a remote RTPSParticipant and all its writers and readers.
     * @param participant_guid GUID_t of the remote RTPSParticipant.
     * @param reason Why the participant is being removed (dropped vs removed)
     * @return true if correct.
     */
    bool remove_remote_participant(
            const fastrtps::rtps::GUID_t& participant_guid,
            fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERY_STATUS reason) override;

    /**
     * Force the sending of our local PDP to all servers
     * @param new_change If true a new change (with new seqNum) is created and sent; if false the last change is re-sent
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
            bool dispose = false,
            fastrtps::rtps::WriteParams& wparams = fastrtps::rtps::WriteParams::WRITE_PARAM_DEFAULT) override;

    // Force the sending of our DATA(p) to those servers that has not acked yet
    void ping_remote_servers();

    // send a specific Data to specific locators
    void send_announcement(
            fastrtps::rtps::CacheChange_t* change,
            std::vector<fastrtps::rtps::GUID_t> remote_readers,
            LocatorList locators,
            bool dispose = false);

    /**
     * These methods wouldn't be needed under perfect server operation (no need of dynamic endpoint allocation)
     * but must be implemented to solve server shutdown situations.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     */
    void assignRemoteEndpoints(
            fastrtps::rtps::ParticipantProxyData* pdata) override;
    void removeRemoteEndpoints(
            fastrtps::rtps::ParticipantProxyData* pdata) override;
    void notifyAboveRemoteEndpoints(
            const fastrtps::rtps::ParticipantProxyData& pdata,
            bool notify_secure_endpoints) override;

#if HAVE_SECURITY
    bool pairing_remote_writer_with_local_reader_after_security(
            const fastrtps::rtps::GUID_t& local_reader,
            const fastrtps::rtps::WriterProxyData& remote_writer_data) override;

    bool pairing_remote_reader_with_local_writer_after_security(
            const fastrtps::rtps::GUID_t& local_reader,
            const fastrtps::rtps::ReaderProxyData& remote_reader_data) override;
#endif // HAVE_SECURITY

    //! Get filename for writer persistence database file
    std::string get_writer_persistence_file_name() const;

    //! Get filename for reader persistence database file
    std::string get_reader_persistence_file_name() const;

    //! Get filename for discovery database file
    std::string get_ddb_persistence_file_name() const;

    //! Get filename for discovery database file
    std::string get_ddb_queue_persistence_file_name() const;

    /*
     * Wakes up the DServerRoutineEvent for new matching or trimming
     * By default the server execute the routine instantly
     */
    void awake_routine_thread(
            double interval_ms = 0);

    void awake_server_thread();

    /**
     * Check if all servers have acknowledge this server PDP data
     * This method must be called from a mutex protected context.
     * @return True if all can reach the client
     */
    bool all_servers_acknowledge_pdp();

    /* The server's main routine. This includes all the discovery related tasks that the server needs to run
     * periodically to keep the discovery graph updated.
     * @return: True if there is pending work, false otherwise.
     */
    bool server_update_routine();

    /*
     * Update the list of remote servers
     */
    void update_remote_servers_list();

    fastdds::rtps::ddb::DiscoveryDataBase& discovery_db();

    /**
     * Access to the remote servers list
     * This method is not thread safe.
     * The return reference may be invalidated if the user modifies simultaneously the remote server list.
     * @return constant reference to the remote servers list
     */
    const RemoteServerList_t& servers();

protected:

    void update_builtin_locators() override;

    /*
     * Get Pointer to the server resource event thread.
     */
    eprosima::fastrtps::rtps::ResourceEvent& get_resource_event_thread();

    // Check the messages in histories. Check which ones modify the database to unlock further messages
    // and clean them when not needed anymore
    bool process_writers_acknowledgements();

    bool process_history_acknowledgement(
            fastrtps::rtps::StatefulWriter* writer,
            fastrtps::rtps::WriterHistory* writer_history);

    fastrtps::rtps::History::iterator process_change_acknowledgement(
            fastrtps::rtps::History::iterator c,
            fastrtps::rtps::StatefulWriter* writer,
            fastrtps::rtps::WriterHistory* writer_history);

    bool process_data_queues();

    bool process_disposals();

    bool process_changes_release();

    void process_changes_release_(
            const std::vector<fastrtps::rtps::CacheChange_t*>& changes);

    bool remove_change_from_writer_history(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::WriterHistory* history,
            fastrtps::rtps::CacheChange_t* change,
            bool release_change = true);

    bool announcement_from_same_participant_in_disposals(
            const std::vector<fastrtps::rtps::CacheChange_t*>& disposals,
            const fastrtps::rtps::GuidPrefix_t& participant);

    bool process_to_send_lists();

    bool process_to_send_list(
            const std::vector<eprosima::fastrtps::rtps::CacheChange_t*>& send_list,
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::WriterHistory* history);

    bool remove_change_from_history_nts(
            fastrtps::rtps::WriterHistory* history,
            fastrtps::rtps::CacheChange_t* change,
            bool release_change = true);

    bool process_dirty_topics();

    bool pending_ack();

    // Method to restore de DiscoveryDataBase from a json object
    // This method reserve space for every cacheChange from the correspondent pool, and
    // sends these changes stored to the DDB for it to process them
    // This method must be called with the DDB variable backup_in_progress as true
    bool process_backup_discovery_database_restore(
            nlohmann::json& ddb_json);

    // Restore the backup file with the changes that were added to the DDB queues (and so acked)
    // It reserves memory for the changes depending the pool, and send them by the listener to the DDB
    // This method must be called with the DDB variable backup_in_progress as false
    bool process_backup_restore_queue(
            std::vector<nlohmann::json>& new_changes);

    // Reads the two backup files and stores each json objects in both arguments
    // The first argument has the json object to restore the DDB
    // The second argument has the json vector object to restore the changes that must be sent again to the queue
    bool read_backup(
            nlohmann::json& ddb_json,
            std::vector<nlohmann::json>& new_changes);

    std::set<fastrtps::rtps::GuidPrefix_t> servers_prefixes();

    // General file name for the prefix of every backup file
    std::ostringstream get_persistence_file_name_() const;

    // Erase the last file and store the backup info of the actual state of the DDB
    // Erase the content of the file with the changes in the queues
    // This method must be called after the whole DDB routine process has been finished and with the DDB
    // queues empty. If not, there will be some information that could be lost. For this, the lock_incoming_data()
    // from DDB must be called during this process
    void process_backup_store();

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

    /**
     * Release a change from the history of the PDP writer.
     *
     * @param change The CacheChange_t to be released.
     */
    void release_change_from_writer(
            eprosima::fastrtps::rtps::CacheChange_t* change);

private:

    using fastrtps::rtps::PDP::announceParticipantState;

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
            bool secure);

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
            const fastrtps::rtps::ParticipantProxyData& pdata);

    void match_reliable_pdp_endpoints(
            const fastrtps::rtps::ParticipantProxyData& pdata);

    //! Server thread
    eprosima::fastrtps::rtps::ResourceEvent resource_event_thread_;

    /**
     * TimedEvent for server routine
     */
    DServerRoutineEvent* routine_;

    /**
     * TimedEvent for server ping to other servers
     */
    DServerPingEvent* ping_;

    //! Discovery database
    fastdds::rtps::ddb::DiscoveryDataBase discovery_db_;

    //! TRANSIENT or TRANSIENT_LOCAL durability;
    fastrtps::rtps::DurabilityKind_t durability_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_PDPSERVER2_H_ */
