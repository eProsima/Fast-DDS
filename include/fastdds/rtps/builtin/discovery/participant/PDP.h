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
 * @file PDP.h
 *
 */

#ifndef _FASTDDS_RTPS_PDP_H_
#define _FASTDDS_RTPS_PDP_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastdds/rtps/reader/ReaderDiscoveryInfo.h>
#include <fastdds/rtps/writer/WriterDiscoveryInfo.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/utils/ProxyPool.hpp>
#include <fastrtps/utils/collections/ResourceLimitedVector.hpp>

namespace eprosima {

namespace fastdds {
namespace rtps {

class PDPServerListener;
class PDPEndpoints;

} // namespace rtps
} // namespace fastdds

namespace fastrtps {
namespace types {

class TypeObject;
class TypeIdentifier;

} // namespace types
namespace rtps {

class RTPSWriter;
class RTPSReader;
class WriterHistory;
class ReaderHistory;
class RTPSParticipantImpl;
class RTPSParticipantListener;
class BuiltinProtocols;
class EDP;
class TimedEvent;
class ReaderProxyData;
class WriterProxyData;
class ParticipantProxyData;
class ReaderListener;
class PDPListener;
class PDPServerListener;
class ITopicPayloadPool;

/**
 * Abstract class PDP that implements the basic interfaces for all Participant Discovery implementations
 * It also keeps the Participant Discovery Data and provides interfaces to access it
 *@ingroup DISCOVERY_MODULE
 */
class PDP
{
    friend class PDPListener;
    friend class PDPServerListener;
    friend class fastdds::rtps::PDPServerListener;
    friend class PDPSecurityInitiatorListener;

public:

    /**
     * Constructor
     * @param builtin Pointer to the BuiltinProtocols object.
     * @param allocation Participant allocation parameters.
     */
    PDP(
            BuiltinProtocols* builtin,
            const RTPSParticipantAllocationAttributes& allocation);

    virtual ~PDP();

    virtual void initializeParticipantProxyData(
            ParticipantProxyData* participant_data);

    /**
     * Initialize the PDP.
     * @param part Pointer to the RTPSParticipant.
     * @return True on success
     */
    bool initPDP(
            RTPSParticipantImpl* part);

    /**
     * @brief Enable the Participant Discovery Protocol
     *
     * @return true if enabled correctly, or if already enabled; false otherwise
     */
    bool enable();

    virtual bool init(
            RTPSParticipantImpl* part) = 0;

    /**
     * Creates an initializes a new participant proxy from a DATA(p) raw info
     * @param p from DATA msg deserialization
     * @param writer_guid GUID of originating writer
     * @return new ParticipantProxyData * or nullptr on failure
     */
    virtual ParticipantProxyData* createParticipantProxyData(
            const ParticipantProxyData& p,
            const GUID_t& writer_guid) = 0;

    /**
     * Force the sending of our local DPD to all remote RTPSParticipants and multicast Locators.
     * @param new_change If true a new change (with new seqNum) is created and sent;If false the last change is re-sent
     * @param dispose sets change kind to NOT_ALIVE_DISPOSED_UNREGISTERED
     * @param wparams allows to identify the change
     */
    virtual void announceParticipantState(
            bool new_change,
            bool dispose,
            WriteParams& wparams) = 0;

    /**
     * \c announceParticipantState method without optional output parameter \c wparams .
     */
    virtual void announceParticipantState(
            bool new_change,
            bool dispose = false);

    //!Stop the RTPSParticipantAnnouncement (only used in tests).
    virtual void stopParticipantAnnouncement();

    //!Reset the RTPSParticipantAnnouncement (only used in tests).
    virtual void resetParticipantAnnouncement();

    /**
     * Add a ReaderProxyData to the correct ParticipantProxyData.
     * @param [in]  reader_guid       GUID of the reader to add.
     * @param [out] participant_guid  GUID of the ParticipantProxyData where the reader was added.
     * @param [in]  initializer_func  Function to be called in order to set the data of the ReaderProxyData.
     *
     * @return A pointer to the added ReaderProxyData (nullptr if it could not be added).
     */
    ReaderProxyData* addReaderProxyData(
            const GUID_t& reader_guid,
            GUID_t& participant_guid,
            std::function<bool(ReaderProxyData*, bool, const ParticipantProxyData&)> initializer_func);

    /**
     * Add a WriterProxyData to the correct ParticipantProxyData.
     * @param [in]  writer_guid       GUID of the writer to add.
     * @param [out] participant_guid  GUID of the ParticipantProxyData where the writer was added.
     * @param [in]  initializer_func  Function to be called in order to set the data of the WriterProxyData.
     *
     * @return A pointer to the added WriterProxyData (nullptr if it could not be added).
     */
    WriterProxyData* addWriterProxyData(
            const GUID_t& writer_guid,
            GUID_t& participant_guid,
            std::function<bool(WriterProxyData*, bool, const ParticipantProxyData&)> initializer_func);

    /**
     * This method returns whether a ReaderProxyDataObject exists among the registered RTPSParticipants
     * (including the local RTPSParticipant).
     * @param [in] reader GUID_t of the reader we are looking for.
     * @return True if found.
     */
    bool has_reader_proxy_data(
            const GUID_t& reader);

    /**
     * This method gets a copy of a ReaderProxyData object if it is found among the registered RTPSParticipants
     * (including the local RTPSParticipant).
     * @param [in]  reader  GUID_t of the reader we are looking for.
     * @param [out] rdata   Reference to the ReaderProxyData object where data is to be returned.
     * @return True if found.
     */
    bool lookupReaderProxyData(
            const GUID_t& reader,
            ReaderProxyData& rdata);

    /**
     * This method returns whether a WriterProxyData exists among the registered RTPSParticipants
     * (including the local RTPSParticipant).
     * @param [in] writer GUID_t of the writer we are looking for.
     * @return True if found.
     */
    bool has_writer_proxy_data(
            const GUID_t& writer);

    /**
     * This method gets a copy of a WriterProxyData object if it is found among the registered RTPSParticipants
     * (including the local RTPSParticipant).
     * @param [in]  writer  GUID_t of the writer we are looking for.
     * @param [out] wdata   Reference to the WriterProxyData object where data is to be returned.
     * @return True if found.
     */
    bool lookupWriterProxyData(
            const GUID_t& writer,
            WriterProxyData& wdata);

    /**
     * This method returns the name of a participant if it is found among the registered RTPSParticipants.
     * @param [in]  guid  GUID_t of the RTPSParticipant we are looking for.
     * @param [out] name  Copy of name on ParticipantProxyData object.
     * @return True if found.
     */
    bool lookup_participant_name(
            const GUID_t& guid,
            string_255& name);

    /**
     * This method removes and deletes a ReaderProxyData object from its corresponding RTPSParticipant.
     *
     * @param[in] reader_guid GUID_t of the reader to remove.
     * @return true if found and deleted.
     */
    bool removeReaderProxyData(
            const GUID_t& reader_guid);

    /**
     * This method removes and deletes a ReaderProxyData object from its corresponding RTPSParticipant.
     *
     * @param[in] reader_guid GUID_t of the reader to remove.
     * @param[in] reason Why the reader is being removed (dropped, removed, or ignored)
     * @return true if found and deleted.
     */
    bool removeReaderProxyData(
            const GUID_t& reader_guid,
            ReaderDiscoveryInfo::DISCOVERY_STATUS reason);

    /**
     * This method removes and deletes a WriterProxyData object from its corresponding RTPSParticipant.
     *
     * @param[in] writer_guid GUID_t of the writer to remove.
     * @return true if found and deleted.
     */
    bool removeWriterProxyData(
            const GUID_t& writer_guid);

    /**
     * This method removes and deletes a WriterProxyData object from its corresponding RTPSParticipant.
     *
     * @param[in] writer_guid GUID_t of the writer to remove.
     * @param[in] reason Why the writer is being removed (dropped, removed, or ignored)
     * @return true if found and deleted.
     */
    bool removeWriterProxyData(
            const GUID_t& writer_guid,
            WriterDiscoveryInfo::DISCOVERY_STATUS reason);

    /**
     * Create the SPDP Writer and Reader
     * @return True if correct.
     */
    virtual bool createPDPEndpoints() = 0;

    /**
     * This method assigns remote endpoints to the builtin endpoints defined in this protocol. It also calls the corresponding methods in EDP and WLP.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     */
    virtual void assignRemoteEndpoints(
            ParticipantProxyData* pdata) = 0;

    /**
     * Override to match additional endpoints to PDP. Like EDP or WLP.
     * @param pdata Pointer to the ParticipantProxyData object.
     * @param notify_secure_endpoints Whether to try notifying secure endpoints.
     */
    virtual void notifyAboveRemoteEndpoints(
            const ParticipantProxyData& pdata,
            bool notify_secure_endpoints) = 0;

    /**
     * Some PDP classes require EDP matching with update PDP DATAs like EDPStatic
     * @return true if EDP endpoinst must be match
     */
    virtual bool updateInfoMatchesEDP()
    {
        return false;
    }

    /**
     * Remove remote endpoints from the participant discovery protocol
     * @param pdata Pointer to the ParticipantProxyData to remove
     */
    virtual void removeRemoteEndpoints(
            ParticipantProxyData* pdata) = 0;

    /**
     * This method removes a remote RTPSParticipant and all its writers and readers.
     * @param participant_guid GUID_t of the remote RTPSParticipant.
     * @param reason Why the participant is being removed (dropped, removed, or ignored)
     * @return true if correct.
     */
    virtual bool remove_remote_participant(
            const GUID_t& participant_guid,
            ParticipantDiscoveryInfo::DISCOVERY_STATUS reason);

    /**
     * This method returns the BuiltinAttributes of the local participant.
     * @return const reference to the BuiltinAttributes of the local participant.
     */
    const BuiltinAttributes& builtin_attributes() const;

    /**
     * Get a pointer to the local RTPSParticipant ParticipantProxyData object.
     * @return Pointer to the local RTPSParticipant ParticipantProxyData object.
     */
    ParticipantProxyData* getLocalParticipantProxyData()
    {
        return participant_proxies_.front();
    }

    /**
     * Get a pointer to the EDP object.
     * @return pointer to the EDP object.
     */
    inline EDP* getEDP()
    {
        return mp_EDP;
    }

    /**
     * Get a const_iterator to the beginning of the RTPSParticipant Proxies.
     * @return const_iterator.
     */
    ResourceLimitedVector<ParticipantProxyData*>::const_iterator ParticipantProxiesBegin()
    {
        return participant_proxies_.begin();
    }

    /**
     * Get a const_iterator to the end of the RTPSParticipant Proxies.
     * @return const_iterator.
     */
    ResourceLimitedVector<ParticipantProxyData*>::const_iterator ParticipantProxiesEnd()
    {
        return participant_proxies_.end();
    }

    /**
     * Assert the liveliness of a Remote Participant.
     * @param remote_guid GuidPrefix_t of the participant whose liveliness is being asserted.
     */
    void assert_remote_participant_liveliness(
            const GuidPrefix_t& remote_guid);

    /**
     * Get the RTPS participant
     * @return RTPS participant
     */
    inline RTPSParticipantImpl* getRTPSParticipant() const
    {
        return mp_RTPSParticipant;
    }

    /**
     * Get the mutex.
     * @return Pointer to the Mutex
     */
    inline std::recursive_mutex* getMutex() const
    {
        return mp_mutex;
    }

    CDRMessage_t get_participant_proxy_data_serialized(
            Endianness_t endian);

    /**
     * Retrive the ParticipantProxyData of a participant
     * @param guid_prefix The GUID prefix of the participant of which the proxy data is retrieved
     * @return A pointer to the ParticipantProxyData. nullptr if there is no such ParticipantProxyData
     */
    ParticipantProxyData* get_participant_proxy_data(
            const GuidPrefix_t& guid_prefix);

    /**
     * Get the list of remote servers to which the client should connect
     * @return A reference to the list of RemoteServerAttributes
     */
    std::list<eprosima::fastdds::rtps::RemoteServerAttributes>& remote_server_attributes();

    /**
     * Access the temporary proxy pool for reader proxies
     * @return pool reference
     */
    ProxyPool<ReaderProxyData>& get_temporary_reader_proxies_pool()
    {
        return temp_reader_proxies_;
    }

    /**
     * Access the temporary proxy pool for writer proxies
     * @return pool reference
     */
    ProxyPool<WriterProxyData>& get_temporary_writer_proxies_pool()
    {
        return temp_writer_proxies_;
    }

#if HAVE_SECURITY
    virtual bool pairing_remote_writer_with_local_reader_after_security(
            const GUID_t& local_reader,
            const WriterProxyData& remote_writer_data);

    virtual bool pairing_remote_reader_with_local_writer_after_security(
            const GUID_t& local_writer,
            const ReaderProxyData& remote_reader_data);
#endif // HAVE_SECURITY

protected:

    //!Pointer to the builtin protocols object.
    BuiltinProtocols* mp_builtin;
    //!Pointer to the local RTPSParticipant.
    RTPSParticipantImpl* mp_RTPSParticipant;
    //!Discovery attributes.
    BuiltinAttributes m_discovery;
    //!Builtin PDP endpoints
    std::unique_ptr<fastdds::rtps::PDPEndpoints> builtin_endpoints_;
    //!Pointer to the EDP object.
    EDP* mp_EDP;
    //!Number of participant proxy data objects created
    size_t participant_proxies_number_;
    //!Registered RTPSParticipants (including the local one, that is the first one.)
    ResourceLimitedVector<ParticipantProxyData*> participant_proxies_;
    //!Pool of participant proxy data objects ready for reuse
    ResourceLimitedVector<ParticipantProxyData*> participant_proxies_pool_;
    //!Number of reader proxy data objects created
    size_t reader_proxies_number_;
    //!Pool of reader proxy data objects ready for reuse
    ResourceLimitedVector<ReaderProxyData*> reader_proxies_pool_;
    //!Number of writer proxy data objects created
    size_t writer_proxies_number_;
    //!Pool of writer proxy data objects ready for reuse
    ResourceLimitedVector<WriterProxyData*> writer_proxies_pool_;
    //!Variable to indicate if any parameter has changed.
    std::atomic_bool m_hasChangedLocalPDP;
    //!Listener for the SPDP messages.
    ReaderListener* mp_listener;
    //! ProxyPool for temporary reader proxies
    ProxyPool<ReaderProxyData> temp_reader_proxies_;
    //! ProxyPool for temporary writer proxies
    ProxyPool<WriterProxyData> temp_writer_proxies_;
    //!Participant data atomic access assurance
    std::recursive_mutex* mp_mutex;
    //!To protect callbacks (ParticipantProxyData&)
    std::mutex callback_mtx_;
    //!Tell if object is enabled
    std::atomic<bool> enabled_ {false};

    /**
     * Adds an entry to the collection of participant proxy information.
     * May use one of the entries present in the pool.
     *
     * @param participant_guid GUID of the participant for which to create the proxy object.
     * @param with_lease_duration indicates whether lease duration event should be created.
     * @param participant_proxy_data The participant proxy data from which the copy is made (if provided)
     *
     * @return pointer to the currently inserted entry, nullptr if allocation limits were reached.
     */
    ParticipantProxyData* add_participant_proxy_data(
            const GUID_t& participant_guid,
            bool with_lease_duration,
            const ParticipantProxyData* participant_proxy_data = nullptr);

    /**
     * Checks whether two participant prefixes are equal by calculating the mangled
     * GUID and comparing it with the remote participant prefix.
     *
     * @param guid_prefix the original desired guid_prefix to compare
     * @param participant_data The participant proxy data to compare against
     *
     * @return true when prefixes are equivalent
     */
    bool data_matches_with_prefix(
            const GuidPrefix_t& guid_prefix,
            const ParticipantProxyData& participant_data);

    /**
     * Gets the key of a participant proxy data.
     *
     * @param [in] participant_guid GUID of the participant to look for.
     * @param [out] key of the corresponding proxy object.
     *
     * @return true when input GUID is found.
     */
    bool lookup_participant_key(
            const GUID_t& participant_guid,
            InstanceHandle_t& key);

    /**
     * Force the sending of our local DPD to all remote RTPSParticipants and multicast Locators.
     * @param writer RTPSWriter to use for sending the announcement
     * @param history history where the change should be added
     * @param new_change If true a new change (with new seqNum) is created and sent;If false the last change is re-sent
     * @param dispose sets change kind to NOT_ALIVE_DISPOSED_UNREGISTERED
     * @param wparams allows to identify the change
     */
    void announceParticipantState(
            RTPSWriter& writer,
            WriterHistory& history,
            bool new_change,
            bool dispose = false,
            WriteParams& wparams = WriteParams::WRITE_PARAM_DEFAULT);

    /**
     * Called after creating the builtin endpoints to update the metatraffic unicast locators of BuiltinProtocols
     */
    virtual void update_builtin_locators() = 0;

private:

    //!TimedEvent to periodically resend the local RTPSParticipant information.
    TimedEvent* resend_participant_info_event_;

    //!Participant's initial announcements config
    InitialAnnouncementConfig initial_announcements_;

    void check_remote_participant_liveliness(
            ParticipantProxyData* remote_participant);

    void check_and_notify_type_discovery(
            RTPSParticipantListener* listener,
            const WriterProxyData& wdata) const;

    void check_and_notify_type_discovery(
            RTPSParticipantListener* listener,
            const ReaderProxyData& rdata) const;

    void check_and_notify_type_discovery(
            RTPSParticipantListener* listener,
            const string_255& topic_name,
            const string_255& type_name,
            const types::TypeIdentifier* type_id,
            const types::TypeObject* type_obj,
            const xtypes::TypeInformation* type_info) const;

    /**
     * Calculates the next announcement interval
     */
    void set_next_announcement_interval();

    /**
     * Calculates the initial announcement interval
     */
    void set_initial_announcement_interval();

    /**
     * Set to a Participant Proxy those properties from this participant that must be sent.
     */
    void set_external_participant_properties_(
            ParticipantProxyData* participant_data);
};


// configuration values for PDP reliable entities.
extern const Duration_t pdp_heartbeat_period;
extern const Duration_t pdp_nack_response_delay;
extern const Duration_t pdp_nack_supression_duration;
extern const Duration_t pdp_heartbeat_response_delay;

extern const int32_t pdp_initial_reserved_caches;

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_PDP_H_ */
