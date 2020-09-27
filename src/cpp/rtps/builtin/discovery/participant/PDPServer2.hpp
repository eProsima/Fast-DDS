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

#include "../database/DiscoveryDataFilter.hpp"
#include "../database/DiscoveryDataBase.hpp"
#include "./DServerEvent2.hpp"

// To be eventually removed together with eprosima::fastrtps
namespace aux = ::eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class PDPServer2 manages server side of the discovery server mechanism
 *@ingroup DISCOVERY_MODULE
 */
class PDPServer2 : public aux::PDP
{
    friend class DServerEvent2;
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
            aux::BuiltinProtocols* builtin,
            const aux::RTPSParticipantAllocationAttributes& allocation);
    ~PDPServer2();

    void initializeParticipantProxyData(
            aux::ParticipantProxyData* participant_data) override;

    /**
     * Initialize the PDP.
     * @param part Pointer to the RTPSParticipant.
     * @return True on success
     */
    bool init(
            aux::RTPSParticipantImpl* part) override;

    /**
     * Creates an initializes a new participant proxy from a DATA(p) raw info
     * @param p ParticipantProxyData from DATA msg deserialization
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
     * This method removes a remote RTPSParticipant and all its writers and readers.
     * @param participant_guid GUID_t of the remote RTPSParticipant.
     * @param reason Why the participant is being removed (dropped vs removed)
     * @return true if correct.
     */
    bool remove_remote_participant(
            const aux::GUID_t& participant_guid,
            aux::ParticipantDiscoveryInfo::DISCOVERY_STATUS reason) override;

    /**
     * Force the sending of our local PDP to all servers
     * @param new_change If true a new change (with new seqNum) is created and sent; if false the last change is re-sent
     * @param dispose Sets change kind to NOT_ALIVE_DISPOSED_UNREGISTERED
     * @param wparams allows to identify the change
     */
    void announceParticipantState(
            bool new_change,
            bool dispose = false,
            aux::WriteParams& wparams = aux::WriteParams::WRITE_PARAM_DEFAULT) override;

    /**
     * These methods wouldn't be needed under perfect server operation (no need of dynamic endpoint allocation)
     * but must be implemented to solve server shutdown situations.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     */
    void assignRemoteEndpoints(
            aux::ParticipantProxyData* pdata) override;
    void removeRemoteEndpoints(
            aux::ParticipantProxyData* pdata) override;
    void notifyAboveRemoteEndpoints(
            const aux::ParticipantProxyData& pdata) override;

#if HAVE_SQLITE3
    //! Get filename for persistence database file
    std::string GetPersistenceFileName();
#endif // if HAVE_SQLITE3

    //! Wakes up the DServerEvent2 for new matching or trimming
    void awakeServerThread()
    {
        mp_sync->restart_timer();
    }

    /* The server's main routine. This includes all the discovery related tasks that the server needs to run
     * periodically to keep the discovery graph updated.
     * @return: True if there is pending work, false otherwise.
     */
    bool server_update_routine();

    fastdds::rtps::ddb::DiscoveryDataBase& discovery_db();

protected:

    // Check the messages in histories. Check which ones modify the database to unlock further messages
    // and clean them when not needed anymore
    bool process_writers_acknowledgements();

    bool process_history_acknowledgement(
            fastrtps::rtps::StatefulWriter* writer,
            fastrtps::rtps::WriterHistory* writer_history);

    bool process_change_acknowledgement(
            fastrtps::rtps::CacheChange_t* c,
            fastrtps::rtps::StatefulWriter* writer,
            fastrtps::rtps::WriterHistory* writer_history);

    bool process_data_queue();

    bool process_disposals();

    bool process_changes_release();

    bool remove_change_from_writer_history(
            fastrtps::rtps::RTPSWriter* writer,
            fastrtps::rtps::WriterHistory* history,
            fastrtps::rtps::CacheChange_t* change);


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
            fastrtps::rtps::CacheChange_t* change);

    bool process_dirty_topics();

private:

    /**
     * TimedEvent for server synchronization
     */
    DServerEvent2* mp_sync;

    //! Discovery database
    fastdds::rtps::ddb::DiscoveryDataBase discovery_db_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_PDPSERVER2_H_ */
