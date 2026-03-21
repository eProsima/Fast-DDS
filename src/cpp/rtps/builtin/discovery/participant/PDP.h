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

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDP_H
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDP_H
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <fastdds/dds/core/Time_t.hpp>
#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <statistics/rtps/monitor-service/interfaces/IProxyObserver.hpp>
#include <statistics/rtps/monitor-service/interfaces/IProxyQueryable.hpp>
#include <utils/ProxyPool.hpp>

namespace eprosima {

namespace fastdds {
namespace statistics {
namespace rtps {

struct IProxyObserver;

} // namespace rtps
} // namespace statistics

namespace dds {
namespace xtypes {

class TypeObject;
class TypeIdentifier;

} // namespace xtypes
} // namespace dds

namespace rtps {

class BaseWriter;
class BaseReader;
class WriterHistory;
class ReaderHistory;
struct RTPSParticipantAllocationAttributes;
class RTPSParticipantImpl;
class RTPSParticipantListener;
class BuiltinProtocols;
class EDP;
class TimedEvent;
class ReaderProxyData;
class WriterProxyData;
class ParticipantProxyData;
class ReaderListener;
class PDPEndpoints;
class PDPListener;
class PDPServerListener;
class ITopicPayloadPool;

/**
 * Abstract class PDP that implements the basic interfaces for all Participant Discovery implementations
 * It also keeps the Participant Discovery Data and provides interfaces to access it
 *@ingroup DISCOVERY_MODULE
 */
class PDP : public fastdds::statistics::rtps::IProxyQueryable
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

    /**
     * @brief Perform actions before enabling the Participant Discovery Protocol if needed
     */
    virtual void pre_enable_actions();

    /**
     * @brief Disable the Participant Discovery Protocol
     */
    void disable();

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
            fastcdr::string_255& name);

    /**
     * This method removes and deletes a ReaderProxyData object from its corresponding RTPSParticipant.
     *
     * @param [in] reader_guid GUID_t of the reader to remove.
     * @return true if found and deleted.
     */
    bool removeReaderProxyData(
            const GUID_t& reader_guid);

    /**
     * This method removes and deletes a WriterProxyData object from its corresponding RTPSParticipant.
     *
     * @param [in] writer_guid GUID_t of the writer to remove.
     * @return true if found and deleted.
     */
    bool removeWriterProxyData(
            const GUID_t& writer_guid);

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
            ParticipantDiscoveryStatus reason);

    /**
     * This method returns the BuiltinAttributes of the local participant.
     * @return const reference to the BuiltinAttributes of the local participant.
     */
    const BuiltinAttributes& builtin_attributes() const;

    /**
     * Get a pointer to the local RTPSParticipant ParticipantProxyData object.
     * @return Pointer to the local RTPSParticipant ParticipantProxyData object.
     */
    ParticipantProxyData* getLocalParticipantProxyData() const
    {
        return participant_proxies_.empty() ? nullptr : participant_proxies_.front();
    }

    /**
     * Get a pointer to the EDP object.
     * @return pointer to the EDP object.
     */
    inline EDP* get_edp()
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
     * Get the number of participant proxies.
     * @return size_t.
     */
    size_t participant_proxies_number()
    {
        return participant_proxies_number_;
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
     * Get the list of remote servers' locators to which the participant should connect
     * @return A reference to the LocatorList
     */
    fastdds::rtps::LocatorList& remote_server_locators();

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

    ReaderAttributes create_builtin_reader_attributes();

    WriterAttributes create_builtin_writer_attributes();

#if HAVE_SECURITY
    void add_builtin_security_attributes(
            ReaderAttributes& ratt,
            WriterAttributes& watt) const;

    virtual bool pairing_remote_writer_with_local_reader_after_security(
            const GUID_t& local_reader,
            const WriterProxyData& remote_writer_data);

    virtual bool pairing_remote_reader_with_local_writer_after_security(
            const GUID_t& local_writer,
            const ReaderProxyData& remote_reader_data);
#endif // HAVE_SECURITY

#ifdef FASTDDS_STATISTICS
    bool get_all_local_proxies(
            std::vector<GUID_t>& guids) override;

    bool get_serialized_proxy(
            const GUID_t& guid,
            CDRMessage_t* msg) override;

    void set_proxy_observer(
            const fastdds::statistics::rtps::IProxyObserver* proxy_observer);

    const fastdds::statistics::rtps::IProxyObserver* get_proxy_observer() const
    {
        return proxy_observer_.load();
    }

#else
    bool get_all_local_proxies(
            std::vector<GUID_t>&) override
    {
        return false;
    }

    bool get_serialized_proxy(
            const GUID_t&,
            CDRMessage_t*) override
    {
        return false;
    }

#endif // FASTDDS_STATISTICS

    virtual void local_participant_attributes_update_nts(
            const RTPSParticipantAttributes& new_atts);

    virtual void update_endpoint_locators_if_default_nts(
            const std::vector<BaseWriter*>& writers,
            const std::vector<BaseReader*>& readers,
            const RTPSParticipantAttributes& old_atts,
            const RTPSParticipantAttributes& new_atts);

    /**
     * @brief Notify monitor the IProxyObserver implementor about
     * any incompatible QoS matching between a local and a remote entity.
     *
     * @param local_guid GUID of the local entity.
     * @param remote_guid GUID of the remote entity.
     * @param incompatible_qos The PolicyMask with the incompatible QoS.
     */
    void notify_incompatible_qos_matching(
            const GUID_t& local_guid,
            const GUID_t& remote_guid,
            const fastdds::dds::PolicyMask& incompatible_qos) const;

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
     * Obtains the participant type based on a Parameters Properties list.
     *
     * @param properties Parameter Properties list to check
     *
     * @return a string indicating the participant type.
     */
    std::string check_participant_type(
            const fastdds::dds::ParameterPropertyList_t properties);

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
     * @param history history where the change should be added
     * @param new_change If true a new change (with new seqNum) is created and sent;If false the last change is re-sent
     * @param dispose sets change kind to NOT_ALIVE_DISPOSED_UNREGISTERED
     * @param wparams allows to identify the change
     */
    void announceParticipantState(
            WriterHistory& history,
            bool new_change,
            bool dispose = false,
            WriteParams& wparams = WriteParams::WRITE_PARAM_DEFAULT);

    /**
     * Called after creating the builtin endpoints to update the metatraffic unicast locators of BuiltinProtocols
     */
    virtual void update_builtin_locators() = 0;

    void notify_and_maybe_ignore_new_participant(
            ParticipantProxyData* pdata,
            bool& should_be_ignored);

    /**
     * Restores the `initial_announcements_` configuration to resend the initial announcements again.
     */
    void resend_ininitial_announcements();

#ifdef FASTDDS_STATISTICS

    std::atomic<const fastdds::statistics::rtps::IProxyObserver*> proxy_observer_;

#endif // FASTDDS_STATISTICS

private:

    //!TimedEvent to periodically resend the local RTPSParticipant information.
    TimedEvent* resend_participant_info_event_;

    //!Participant's initial announcements config
    InitialAnnouncementConfig initial_announcements_;

    void check_remote_participant_liveliness(
            ParticipantProxyData* remote_participant);

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

    /**
     * Performs all the necessary actions after removing a ParticipantProxyData from the
     * participant_proxies_ collection.
     *
     * @param pdata ParticipantProxyData that was removed.
     * @param partGUID GUID of the removed participant.
     * @param reason Reason why the participant was removed.
     * @param listener Listener to be notified of the unmatches / removal.
     */
    void actions_on_remote_participant_removed(
            ParticipantProxyData* pdata,
            const GUID_t& partGUID,
            ParticipantDiscoveryStatus reason,
            RTPSParticipantListener* listener);

};

// configuration values for PDP reliable entities.
extern const dds::Duration_t pdp_heartbeat_period;
extern const dds::Duration_t pdp_nack_response_delay;
extern const dds::Duration_t pdp_nack_supression_duration;
extern const dds::Duration_t pdp_heartbeat_response_delay;

extern const int32_t pdp_initial_reserved_caches;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT__PDP_H
