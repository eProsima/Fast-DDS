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
 * @file PDPServer2.hpp
 *
 */

#ifndef _FASTDDS_RTPS_PDPSERVER2_H_
#define _FASTDDS_RTPS_PDPSERVER2_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastdds/rtps/history/History.h>
#include <fastdds/rtps/resources/ResourceEvent.h>

#include "../database/DiscoveryDataFilter.hpp"
#include "../database/DiscoveryDataBase.hpp"
#include "./DServerEvent2.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class PDPServer2 manages server side of the discovery server mechanism
 *@ingroup DISCOVERY_MODULE
 */
class PDPServer2 : public fastrtps::rtps::PDP
{
    friend class DServerRoutineEvent2;
    friend class DServerPingEvent2;
    friend class EDPServer2;
    friend class PDPServerListener2;

public:

    /**
     * Constructor
     * @param builtin Pointer to the BuiltinProcols object.
     * @param allocation participant's allocation settings
     * @param durability_kind the kind of persistence we want for the discovery data
     */
    PDPServer2(
            fastrtps::rtps::BuiltinProtocols* builtin,
            const fastrtps::rtps::RTPSParticipantAllocationAttributes& allocation);
    ~PDPServer2();

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
     * @param wparams allows to identify the change
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
            fastrtps::rtps::LocatorList_t locators);

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
            const fastrtps::rtps::ParticipantProxyData& pdata) override;

#if HAVE_SQLITE3
    //! Get filename for persistence database file
    std::string GetPersistenceFileName();
#endif // if HAVE_SQLITE3

    /*
     * Wakes up the DServerRoutineEvent2 for new matching or trimming
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

    fastdds::rtps::ddb::DiscoveryDataBase& discovery_db();

    const RemoteServerList_t& servers();

protected:

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


    // Remove from writer_history all the changes whose original sender was entity_guid_prefix
    void remove_related_alive_from_history_nts(
            fastrtps::rtps::WriterHistory* writer_history,
            const fastrtps::rtps::GuidPrefix_t& entity_guid_prefix);

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

    std::vector<fastrtps::rtps::GuidPrefix_t> servers_prefixes();

private:

    //! Server thread
    eprosima::fastrtps::rtps::ResourceEvent resource_event_thread_;

    /**
     * TimedEvent for server routine
     */
    DServerRoutineEvent2* routine_;

    /**
     * TimedEvent for server ping to other servers
     */
    DServerPingEvent2* ping_;


    //! Discovery database
    fastdds::rtps::ddb::DiscoveryDataBase discovery_db_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_PDPSERVER2_H_ */
