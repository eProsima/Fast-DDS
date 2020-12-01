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

/*
 * @file StatelessWriter.cpp
 *
 */

#include <fastdds/rtps/writer/StatelessWriter.h>
#include <fastdds/rtps/writer/WriterListener.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/resources/AsyncWriterThread.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/flowcontrol/FlowController.h>
#include <rtps/history/HistoryAttributesExtension.hpp>
#include <rtps/writer/RTPSWriterCollector.h>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>

#include <algorithm>
#include <mutex>
#include <set>
#include <vector>

#include <fastdds/dds/log/Log.hpp>
#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>
#include <rtps/DataSharing/DataSharingNotifier.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/RTPSDomainImpl.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

static bool add_change_to_rtps_group(
        RTPSMessageGroup& group,
        ChangeForReader_t* reader_change,
        bool inline_qos)
{
    try
    {
        CacheChange_t* change = reader_change->getChange();
        uint32_t n_fragments = change->getFragmentCount();
        if (n_fragments > 0)
        {
            for (uint32_t frag = 1; frag <= n_fragments; frag++)
            {
                if (!group.add_data_frag(*change, frag, inline_qos))
                {
                    logError(RTPS_WRITER, "Error sending fragment (" << change->sequenceNumber << ", " << frag << ")");
                }
            }
        }
        else
        {
            if (!group.add_data(*change, inline_qos))
            {
                logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
            }
        }
    }
    catch (const RTPSMessageGroup::timeout&)
    {
        logError(RTPS_WRITER, "Max blocking time reached");
        return false;
    }

    return true;
}

StatelessWriter::StatelessWriter(
        RTPSParticipantImpl* impl,
        const GUID_t& guid,
        const WriterAttributes& attributes,
        WriterHistory* history,
        WriterListener* listener)
    : RTPSWriter(impl, guid, attributes, history, listener)
    , matched_readers_(attributes.matched_readers_allocation)
    , late_joiner_guids_(attributes.matched_readers_allocation)
    , unsent_changes_(resource_limits_from_history(history->m_att))
    , last_intraprocess_sequence_number_(0)
    , matched_readers_pool_(attributes.matched_readers_allocation)
{
    init(impl, attributes);
}

StatelessWriter::StatelessWriter(
        RTPSParticipantImpl* impl,
        const GUID_t& guid,
        const WriterAttributes& attributes,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        WriterHistory* history,
        WriterListener* listener)
    : RTPSWriter(impl, guid, attributes, payload_pool, history, listener)
    , matched_readers_(attributes.matched_readers_allocation)
    , late_joiner_guids_(attributes.matched_readers_allocation)
    , unsent_changes_(resource_limits_from_history(history->m_att))
    , last_intraprocess_sequence_number_(0)
    , matched_readers_pool_(attributes.matched_readers_allocation)
{
    init(impl, attributes);
}

StatelessWriter::StatelessWriter(
        RTPSParticipantImpl* participant,
        const GUID_t& guid,
        const WriterAttributes& attributes,
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool,
        WriterHistory* history,
        WriterListener* listener)
    : RTPSWriter(participant, guid, attributes, payload_pool, change_pool, history, listener)
    , matched_readers_(attributes.matched_readers_allocation)
    , late_joiner_guids_(attributes.matched_readers_allocation)
    , unsent_changes_(resource_limits_from_history(history->m_att))
    , last_intraprocess_sequence_number_(0)
    , matched_readers_pool_(attributes.matched_readers_allocation)
{
    init(participant, attributes);
}

void StatelessWriter::init(
        RTPSParticipantImpl* participant,
        const WriterAttributes& attributes)
{
    get_builtin_guid();

    const RemoteLocatorsAllocationAttributes& loc_alloc =
            participant->getRTPSParticipantAttributes().allocation.locators;

    for (size_t i = 0; i < attributes.matched_readers_allocation.initial; ++i)
    {
        matched_readers_pool_.emplace_back(new ReaderLocator(
            this,
            loc_alloc.max_unicast_locators,
            loc_alloc.max_multicast_locators));
    }
}

StatelessWriter::~StatelessWriter()
{
    logInfo(RTPS_WRITER, "StatelessWriter destructor"; );

    for (std::unique_ptr<FlowController>& controller : flow_controllers_)
    {
        controller->disable();
    }

    mp_RTPSParticipant->async_thread().unregister_writer(this);

    // After unregistering writer from AsyncWriterThread, delete all flow_controllers because they register the writer in
    // the AsyncWriterThread.
    flow_controllers_.clear();

    // Stop all active proxies
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        while (!matched_readers_.empty())
        {
            matched_readers_.back()->stop();
            matched_readers_.pop_back();
        }
    }

    // TOODO [ILG] Shold we force this on all cases?
    if(is_datasharing_compatible_)
    {
        //Release payloads orderly
        for (std::vector<CacheChange_t*>::iterator chit = mp_history->changesBegin();
                chit != mp_history->changesEnd(); ++chit)
        {
            IPayloadPool* pool = (*chit)->payload_owner();
            if (pool)
            {
                pool->release_payload(**chit);
            }
        }
    }
}

void StatelessWriter::get_builtin_guid()
{
    if (m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
    {
        add_guid(GUID_t { GuidPrefix_t(), c_EntityId_SPDPReader });
    }
#if HAVE_SECURITY
    else if (m_guid.entityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER)
    {
        add_guid(GUID_t { GuidPrefix_t(), participant_stateless_message_reader_entity_id });
    }
#endif // if HAVE_SECURITY
}

bool StatelessWriter::has_builtin_guid()
{
    if (m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
    {
        return true;
    }
#if HAVE_SECURITY
    if (m_guid.entityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER)
    {
        return true;
    }
#endif // if HAVE_SECURITY
    return false;
}

void StatelessWriter::update_reader_info(
        bool create_sender_resources)
{
    bool addGuid = !has_builtin_guid();
    is_inline_qos_expected_ = false;
    there_are_remote_readers_ = false;

    for (const std::unique_ptr<ReaderLocator>& reader : matched_readers_)
    {
        is_inline_qos_expected_ |= reader->expects_inline_qos();
        there_are_remote_readers_ |= !reader->is_local_reader();
    }

    update_cached_info_nts();
    if (addGuid)
    {
        compute_selected_guids();
    }

    if (create_sender_resources)
    {
        RTPSParticipantImpl* part = mp_RTPSParticipant;
        locator_selector_.for_each([part](const Locator_t& loc)
                {
                    part->createSenderResources(loc);
                });
    }
}

/*
 *	CHANGE-RELATED METHODS
 */
bool StatelessWriter::datasharing_delivery(
        CacheChange_t* change)
{
    if (is_datasharing_compatible_)
    {
        auto pool = std::dynamic_pointer_cast<DataSharingPayloadPool>(payload_pool_);
        if (pool)
        {
            pool->prepare_for_notification(change);
            logInfo(RTPS_WRITER, "Notifying readers of cache change with SN " << change->sequenceNumber);
            datasharing_notifier_->notify();
            return true;
        }
        else
        {
            logError(RTPS_WRITER, "Data sharing compatible writer but no data sharing payload pool.");
        }
    }
    return false;
}

// TODO(Ricardo) This function only can be used by history. Private it and frined History.
// TODO(Ricardo) Look for other functions
void StatelessWriter::unsent_change_added_to_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    if (liveliness_lease_duration_ < c_TimeInfinite)
    {
        mp_RTPSParticipant->wlp()->assert_liveliness(
            getGuid(),
            liveliness_kind_,
            liveliness_lease_duration_);
    }

    // Notify the datasharing readers
    datasharing_delivery(change);

    // Now for the rest of readers
    if (!fixed_locators_.empty() || matched_readers_.size() > 0)
    {
        if (!isAsync())
        {
            try
            {
                if (m_separateSendingEnabled)
                {
                    std::vector<GUID_t> guids(1);
                    for (std::unique_ptr<ReaderLocator>& it : matched_readers_)
                    {
                        if (it->is_local_reader())
                        {
                            intraprocess_delivery(change, *it);
                        }
                        else
                        {
                            RTPSMessageGroup group(mp_RTPSParticipant, this, *it, max_blocking_time);

                            uint32_t n_fragments = change->getFragmentCount();
                            if (n_fragments > 0)
                            {
                                for (uint32_t frag = 1; frag <= n_fragments; frag++)
                                {
                                    if (!group.add_data_frag(*change, frag, is_inline_qos_expected_))
                                    {
                                        logError(RTPS_WRITER, "Error sending fragment (" << change->sequenceNumber <<
                                                ", " << frag << ")");
                                    }
                                }
                            }
                            else
                            {
                                if (!group.add_data(*change, is_inline_qos_expected_))
                                {
                                    logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
                                }
                            }
                        }
                    }
                }
                else
                {
                    for (std::unique_ptr<ReaderLocator>& it : matched_readers_)
                    {
                        if (it->is_local_reader())
                        {
                            intraprocess_delivery(change, *it);
                        }
                    }

                    if (there_are_remote_readers_ || !fixed_locators_.empty())
                    {
                        RTPSMessageGroup group(mp_RTPSParticipant, this, *this, max_blocking_time);

                        uint32_t n_fragments = change->getFragmentCount();
                        if (n_fragments > 0)
                        {
                            for (uint32_t frag = 1; frag <= n_fragments; frag++)
                            {
                                if (!group.add_data_frag(*change, frag, is_inline_qos_expected_))
                                {
                                    logError(RTPS_WRITER, "Error sending fragment (" << change->sequenceNumber <<
                                            ", " << frag << ")");
                                }
                            }
                        }
                        else
                        {
                            if (!group.add_data(*change, is_inline_qos_expected_))
                            {
                                logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
                            }
                        }
                    }
                }

                if (mp_listener != nullptr)
                {
                    mp_listener->onWriterChangeReceivedByAll(this, change);
                }
            }
            catch (const RTPSMessageGroup::timeout&)
            {
                logError(RTPS_WRITER, "Max blocking time reached");
            }
        }
        else
        {
            unsent_changes_.push_back(ChangeForReader_t(change));
            mp_RTPSParticipant->async_thread().wake_up(this, max_blocking_time);
        }
    }
    else
    {
        logInfo(RTPS_WRITER, "No reader to add change.");
        if (mp_listener != nullptr)
        {
            mp_listener->onWriterChangeReceivedByAll(this, change);
        }
    }
}

bool StatelessWriter::intraprocess_delivery(
        CacheChange_t* change,
        ReaderLocator& reader_locator)
{
    RTPSReader* reader = reader_locator.local_reader();

    if (reader)
    {
        if (change->write_params.related_sample_identity() != SampleIdentity::unknown())
        {
            change->write_params.sample_identity(change->write_params.related_sample_identity());
        }
        return reader->processDataMsg(change);
    }

    return false;
}

bool StatelessWriter::change_removed_by_history(
        CacheChange_t* change)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    if (unsent_changes_.remove_if(
                [change](ChangeForReader_t& cptr)
                {
                    return cptr.getChange() == change ||
                    cptr.getChange()->sequenceNumber == change->sequenceNumber;
                }))
    {
        unsent_changes_cond_.notify_all();
    }

    return true;
}

bool StatelessWriter::is_acked_by_all(
        const CacheChange_t* change) const
{
    // Only asynchronous writers may have unacked (i.e. unsent changes)
    if (isAsync())
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

        // Return false if change is pending to be sent
        auto it = std::find_if(unsent_changes_.begin(),
                        unsent_changes_.end(),
                        [change](const ChangeForReader_t& unsent_change)
                        {
                            return change == unsent_change.getChange();
                        });

        return it == unsent_changes_.end();
    }

    return true;
}

bool StatelessWriter::try_remove_change(
        const std::chrono::steady_clock::time_point&,
        std::unique_lock<RecursiveTimedMutex>&)
{
    return mp_history->remove_min_change();
}

bool StatelessWriter::wait_for_acknowledgement(
        const SequenceNumber_t& seq,
        const std::chrono::steady_clock::time_point& max_blocking_time_point,
        std::unique_lock<RecursiveTimedMutex>& lock)
{
    if (!isAsync())
    {
        return true;
    }

    auto change_is_unsent = [seq](const ChangeForReader_t& unsent_change)
            {
                return seq == unsent_change.getSequenceNumber();
            };
    auto change_is_acknowledged = [this, change_is_unsent]()
            {
                return unsent_changes_.end() ==
                       std::find_if(unsent_changes_.begin(), unsent_changes_.end(), change_is_unsent);
            };
    return unsent_changes_cond_.wait_until(lock, max_blocking_time_point, change_is_acknowledged);
}

void StatelessWriter::update_unsent_changes(
        const SequenceNumber_t& seq_num,
        const FragmentNumber_t& frag_num)
{
    auto find_by_seq_num = [seq_num](const ChangeForReader_t& unsent_change)
            {
                return seq_num == unsent_change.getSequenceNumber();
            };

    auto it = std::find_if(unsent_changes_.begin(), unsent_changes_.end(), find_by_seq_num);
    if (it != unsent_changes_.end())
    {
        bool should_remove = (frag_num == 0);
        if (!should_remove)
        {
            it->markFragmentsAsSent(frag_num);
            FragmentNumberSet_t fragment_sns = it->getUnsentFragments();
            should_remove = fragment_sns.empty();
        }

        if (should_remove)
        {
            unsent_changes_.remove_if(find_by_seq_num);
        }
    }
}

void StatelessWriter::send_any_unsent_changes()
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    bool remote_destinations = there_are_remote_readers_ || !fixed_locators_.empty();
    bool no_flow_controllers = flow_controllers_.empty() && mp_RTPSParticipant->getFlowControllers().empty();
    if (!remote_destinations || no_flow_controllers)
    {
        send_all_unsent_changes();
    }
    else
    {
        send_unsent_changes_with_flow_control();
    }

    logInfo(RTPS_WRITER, "Finish sending unsent changes");

    // In case someone is waiting for changes to be sent
    unsent_changes_cond_.notify_all();
}

void StatelessWriter::send_all_unsent_changes()
{
    //TODO(Mcc) Separate sending for asynchronous writers

    static constexpr uint32_t implicit_flow_controller_size = RTPSMessageGroup::get_max_fragment_payload_size();

    NetworkFactory& network = mp_RTPSParticipant->network_factory();
    RTPSMessageGroup group(mp_RTPSParticipant, this, *this);
    bool remote_destinations = locator_selector_.selected_size() > 0 || !fixed_locators_.empty();
    bool bHasListener = mp_listener != nullptr;

    uint32_t total_sent_size = 0;

    // Select late-joiners only
    if (!late_joiner_guids_.empty())
    {
        ignore_fixed_locators_ = true;
        locator_selector_.reset(false);
        for (const GUID_t& guid : late_joiner_guids_)
        {
            locator_selector_.enable(guid);
        }
        network.select_locators(locator_selector_);
        remote_destinations = locator_selector_.selected_size() > 0 || !fixed_locators_.empty();
        if (!has_builtin_guid())
        {
            compute_selected_guids();
        }
    }

    while (!unsent_changes_.empty() && (total_sent_size < implicit_flow_controller_size))
    {
        ChangeForReader_t& unsentChange = unsent_changes_.front();
        CacheChange_t* cache_change = unsentChange.getChange();

        total_sent_size += cache_change->serializedPayload.length;

        // Check if we finished with late-joiners only
        if (!late_joiner_guids_.empty() &&
                cache_change->sequenceNumber >= first_seq_for_all_readers_)
        {
            ignore_fixed_locators_ = false;
            late_joiner_guids_.clear();
            locator_selector_.reset(true);
            network.select_locators(locator_selector_);
            remote_destinations = locator_selector_.selected_size() > 0 || !fixed_locators_.empty();
            if (!has_builtin_guid())
            {
                compute_selected_guids();
            }
        }

        uint64_t sequence_number = cache_change->sequenceNumber.to64long();
        // Filter intraprocess unsent changes
        if (sequence_number > last_intraprocess_sequence_number_)
        {
            last_intraprocess_sequence_number_ = sequence_number;
            for (std::unique_ptr<ReaderLocator>& it : matched_readers_)
            {
                if (it->is_local_reader())
                {
                    intraprocess_delivery(cache_change, *it);
                }
            }
        }

        if (remote_destinations)
        {
            if (!add_change_to_rtps_group(group, &unsentChange, is_inline_qos_expected_))
            {
                break;
            }
        }

        unsent_changes_.erase(unsent_changes_.begin());
        if (bHasListener)
        {
            mp_listener->onWriterChangeReceivedByAll(this, cache_change);
        }
    }

    // Restore locator selector state
    ignore_fixed_locators_ = false;
    locator_selector_.reset(true);
    network.select_locators(locator_selector_);
    if (!has_builtin_guid())
    {
        compute_selected_guids();
    }

    if (!unsent_changes_.empty())
    {
        mp_RTPSParticipant->async_thread().wake_up(this);
    }
}

void StatelessWriter::send_unsent_changes_with_flow_control()
{
    //TODO(Mcc) Separate sending for asynchronous writers

    // There should be remote destinations
    assert(there_are_remote_readers_ || !fixed_locators_.empty());

    NetworkFactory& network = mp_RTPSParticipant->network_factory();
    bool flow_controllers_limited = false;
    while (!unsent_changes_.empty() && !flow_controllers_limited)
    {
        RTPSWriterCollector<ReaderLocator*> changesToSend;

        for (const ChangeForReader_t& unsentChange : unsent_changes_)
        {
            CacheChange_t* cache_change = unsentChange.getChange();
            changesToSend.add_change(cache_change, nullptr, unsentChange.getUnsentFragments());

            uint64_t sequence_number = cache_change->sequenceNumber.to64long();
            // Filter intraprocess unsent changes
            if (sequence_number > last_intraprocess_sequence_number_)
            {
                last_intraprocess_sequence_number_ = sequence_number;
                for (std::unique_ptr<ReaderLocator>& it : matched_readers_)
                {
                    if (it->is_local_reader())
                    {
                        intraprocess_delivery(cache_change, *it);
                    }
                }
            }
        }

        bool bHasListener = mp_listener != nullptr;
        size_t n_items = changesToSend.size();

        // Clear through local controllers
        for (auto& controller : flow_controllers_)
        {
            (*controller)(changesToSend);
        }

        // Clear through parent controllers
        for (auto& controller : mp_RTPSParticipant->getFlowControllers())
        {
            (*controller)(changesToSend);
        }

        flow_controllers_limited = n_items != changesToSend.size();

        try
        {
            RTPSMessageGroup group(mp_RTPSParticipant, this, *this);

            // Select late-joiners only
            if (!late_joiner_guids_.empty())
            {
                ignore_fixed_locators_ = true;
                locator_selector_.reset(false);
                for (const GUID_t& guid : late_joiner_guids_)
                {
                    locator_selector_.enable(guid);
                }
                network.select_locators(locator_selector_);
                if (!has_builtin_guid())
                {
                    compute_selected_guids();
                }
            }

            while (!changesToSend.empty())
            {
                RTPSWriterCollector<ReaderLocator*>::Item changeToSend = changesToSend.pop();

                // Check if we finished with late-joiners only
                if (!late_joiner_guids_.empty() &&
                        changeToSend.sequenceNumber >= first_seq_for_all_readers_)
                {
                    ignore_fixed_locators_ = false;
                    late_joiner_guids_.clear();
                    locator_selector_.reset(true);
                    network.select_locators(locator_selector_);
                    if (!has_builtin_guid())
                    {
                        compute_selected_guids();
                    }
                }

                // Remove the messages selected for sending from the original list,
                // and update those that were fragmented with the new sent index
                update_unsent_changes(changeToSend.sequenceNumber, changeToSend.fragmentNumber);

                // Notify the controllers
                FlowController::NotifyControllersChangeSent(changeToSend.cacheChange);

                if (changeToSend.fragmentNumber != 0)
                {
                    if (!group.add_data_frag(*changeToSend.cacheChange, changeToSend.fragmentNumber,
                            is_inline_qos_expected_))
                    {
                        logError(RTPS_WRITER, "Error sending fragment (" << changeToSend.sequenceNumber <<
                                ", " << changeToSend.fragmentNumber << ")");
                    }
                }
                else
                {
                    if (!group.add_data(*changeToSend.cacheChange, is_inline_qos_expected_))
                    {
                        logError(RTPS_WRITER, "Error sending change " << changeToSend.sequenceNumber);
                    }
                }

                if (bHasListener && is_acked_by_all(changeToSend.cacheChange))
                {
                    mp_listener->onWriterChangeReceivedByAll(this, changeToSend.cacheChange);
                }
            }
        }
        catch (const RTPSMessageGroup::timeout&)
        {
            logError(RTPS_WRITER, "Max blocking time reached");
        }
    }

    // Restore locator selector state
    ignore_fixed_locators_ = false;
    locator_selector_.reset(true);
    network.select_locators(locator_selector_);
    if (!has_builtin_guid())
    {
        compute_selected_guids();
    }
}

/*
 *	MATCHED_READER-RELATED METHODS
 */
bool StatelessWriter::matched_reader_add(
        const ReaderProxyData& data)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    assert(data.guid() != c_Guid_Unknown);

    for (std::unique_ptr<ReaderLocator>& reader : matched_readers_)
    {
        if (reader->remote_guid() == data.guid())
        {
            logWarning(RTPS_WRITER, "Attempting to add existing reader, updating information.");
            if (reader->update(data.remote_locators().unicast,
                    data.remote_locators().multicast,
                    data.m_expectsInlineQos))
            {
                update_reader_info(true);
            }
            return false;
        }
    }

    if (is_datasharing_compatible_with(data))
    {
        if (datasharing_notifier_->add_reader(data.guid()))
        {
            logInfo(RTPS_WRITER, "Reader " << data.guid() << " added to " << this->m_guid.entityId 
                                           << " with data sharing");
            return true;
        }

        logError(RTPS_WRITER, "Failed to add Reader Proxy " << data.guid()
                << " to " << this->m_guid.entityId 
                << " with data sharing.");
        return false;
    }

    // Get a locator from the inactive pool (or create a new one if necessary and allowed)
    std::unique_ptr<ReaderLocator> new_reader;
    if (matched_readers_pool_.empty())
    {
        size_t max_readers = matched_readers_pool_.max_size();
        if (matched_readers_.size() + matched_readers_pool_.size() < max_readers)
        {
            const RemoteLocatorsAllocationAttributes& loc_alloc =
                mp_RTPSParticipant->getRTPSParticipantAttributes().allocation.locators;

            new_reader.reset(new ReaderLocator(
                    this,
                    loc_alloc.max_unicast_locators,
                    loc_alloc.max_multicast_locators));
        }
        else
        {
            logWarning(RTPS_WRITER, "Couldn't add matched reader due to resource limits");
            return false;
        }
    }
    else
    {
        new_reader = std::move(matched_readers_pool_.back());
        matched_readers_pool_.pop_back();
    }

    // Add info of new datareader.
    new_reader->start(data.guid(),
        data.remote_locators().unicast,
        data.remote_locators().multicast,
        data.m_expectsInlineQos);
    locator_selector_.add_entry(new_reader->locator_selector_entry());
    matched_readers_.push_back(std::move(new_reader));

    update_reader_info(true);

    if ((mp_history->getHistorySize() > 0) &&
            (data.m_qos.m_durability.kind >= TRANSIENT_LOCAL_DURABILITY_QOS))
    {
        // Resend all changes
        unsent_changes_.assign(mp_history->changesBegin(), mp_history->changesEnd());
        // If a new change is added, should be sent to everyone
        first_seq_for_all_readers_ = mp_history->next_sequence_number();
        // Mark newcommer's guid as receiver of old changes
        late_joiner_guids_.emplace_back(data.guid());
        // History is always sent asynchronously to late joiners
        mp_RTPSParticipant->async_thread().wake_up(this);
    }

    logInfo(RTPS_WRITER, "Reader " << data.guid() << " added to " << m_guid.entityId);
    return true;
}

bool StatelessWriter::set_fixed_locators(
        const LocatorList_t& locator_list)
{
#if HAVE_SECURITY
    if (getAttributes().security_attributes().is_submessage_protected ||
            getAttributes().security_attributes().is_payload_protected)
    {
        logError(RTPS_WRITER, "A secure besteffort writer cannot add a lonely locator");
        return false;
    }
#endif // if HAVE_SECURITY

    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    fixed_locators_.push_back(locator_list);
    mp_RTPSParticipant->createSenderResources(fixed_locators_);

    return true;
}

bool StatelessWriter::matched_reader_remove(
        const GUID_t& reader_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    bool found = false;
    if (is_datasharing_compatible_)
    {
        found = datasharing_notifier_->remove_reader(reader_guid);
    }

    if (!found)
    {
        found = locator_selector_.remove_entry(reader_guid);
        if (found)
        {
            found = false;
            for (auto it = matched_readers_.begin(); it != matched_readers_.end(); ++it)
            {
                if ((*it)->remote_guid() == reader_guid)
                {
                    (*it)->stop();
                    matched_readers_pool_.push_back(std::move((*it)));
                    matched_readers_.erase(it);
                    found = true;
                    break;
                }
            }
            // guid should be both on locator_selector_ and matched_readers_
            assert(found);

            late_joiner_guids_.remove(reader_guid);
            update_reader_info(false);
        }
    }

    return found;
}

bool StatelessWriter::matched_reader_is_matched(
        const GUID_t& reader_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    return std::any_of(matched_readers_.begin(), matched_readers_.end(),
                   [reader_guid](const std::unique_ptr<ReaderLocator>& item)
                   {
                       return item->remote_guid() == reader_guid;
                   });
}

void StatelessWriter::unsent_changes_reset()
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    // Request to send all changes to all readers
    if (mp_history->getHistorySize() > 0)
    {
        // Mark all changes as pending
        unsent_changes_.assign(mp_history->changesBegin(), mp_history->changesEnd());
        // Send to all from the beginning
        first_seq_for_all_readers_ = unsent_changes_.front().getSequenceNumber();
        // Do it asynchronously
        mp_RTPSParticipant->async_thread().wake_up(this);
    }
}

void StatelessWriter::add_flow_controller(
        std::unique_ptr<FlowController> controller)
{
    flow_controllers_.push_back(std::move(controller));
}

bool StatelessWriter::send(
        CDRMessage_t* message,
        std::chrono::steady_clock::time_point& max_blocking_time_point) const
{
    if (!RTPSWriter::send(message, max_blocking_time_point))
    {
        return false;
    }

    return ignore_fixed_locators_ ||
           fixed_locators_.empty() ||
           mp_RTPSParticipant->sendSync(message, Locators(fixed_locators_.begin()), Locators(
                       fixed_locators_.end()), max_blocking_time_point);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
