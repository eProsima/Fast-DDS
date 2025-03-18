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

#include "StatelessWriter.hpp"

#include <algorithm>
#include <mutex>
#include <set>
#include <vector>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include "../flowcontrol/FlowController.hpp"
#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/liveliness/WLP.hpp>
#include <rtps/DataSharing/DataSharingNotifier.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>
#include <rtps/DataSharing/WriterPool.hpp>
#include <rtps/history/BasicPayloadPool.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/history/HistoryAttributesExtension.hpp>
#include <rtps/messages/RTPSMessageGroup.hpp>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/RTPSDomainImpl.hpp>
#include <rtps/writer/BaseWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Loops over all the readers in the vector, applying the given routine.
 * The loop continues until the result of the routine is true for any reader
 * or all readers have been processes.
 * The returned value is true if the routine returned true at any point,
 * or false otherwise.
 */
bool for_matched_readers(
        ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_1,
        std::function<bool(ReaderLocator&)> fun)
{
    for (auto& remote_reader : reader_vector_1)
    {
        if (fun(*remote_reader))
        {
            return true;
        }
    }

    return false;
}

bool for_matched_readers(
        ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_1,
        ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_2,
        std::function<bool(ReaderLocator&)> fun)
{
    if (for_matched_readers(reader_vector_1, fun))
    {
        return true;
    }
    return for_matched_readers(reader_vector_2, fun);
}

bool for_matched_readers(
        ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_1,
        ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_2,
        ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_3,
        std::function<bool(ReaderLocator&)> fun)
{
    if (for_matched_readers(reader_vector_1, reader_vector_2, fun))
    {
        return true;
    }
    return for_matched_readers(reader_vector_3, fun);
}

/**
 * Loops over all the readers in the vector, applying the given routine.
 * The loop continues until the result of the routine is true for any reader
 * or all readers have been processes.
 * The returned value is true if the routine returned true at any point,
 * or false otherwise.
 *
 * const version
 */
bool for_matched_readers(
        const ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_1,
        std::function<bool(const ReaderLocator)> fun)
{
    for (const auto& remote_reader : reader_vector_1)
    {
        if (fun(*remote_reader))
        {
            return true;
        }
    }

    return false;
}

bool for_matched_readers(
        const ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_1,
        const ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_2,
        std::function<bool(const ReaderLocator)> fun)
{
    if (for_matched_readers(reader_vector_1, fun))
    {
        return true;
    }
    return for_matched_readers(reader_vector_2, fun);
}

bool for_matched_readers(
        const ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_1,
        const ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_2,
        const ResourceLimitedVector<std::unique_ptr<ReaderLocator>>& reader_vector_3,
        std::function<bool(const ReaderLocator&)> fun)
{
    if (for_matched_readers(reader_vector_1, reader_vector_2, fun))
    {
        return true;
    }
    return for_matched_readers(reader_vector_3, fun);
}

StatelessWriter::StatelessWriter(
        RTPSParticipantImpl* impl,
        const GUID_t& guid,
        const WriterAttributes& attributes,
        FlowController* flow_controller,
        WriterHistory* history,
        WriterListener* listener)
    : BaseWriter(impl, guid, attributes, flow_controller, history, listener)
    , matched_remote_readers_(attributes.matched_readers_allocation)
    , matched_local_readers_(attributes.matched_readers_allocation)
    , matched_datasharing_readers_(attributes.matched_readers_allocation)
    , matched_readers_pool_(attributes.matched_readers_allocation)
    , locator_selector_(*this, attributes.matched_readers_allocation)
{
    init(impl, attributes);
}

void StatelessWriter::init(
        RTPSParticipantImpl* participant,
        const WriterAttributes& attributes)
{
    get_builtin_guid();

    const RemoteLocatorsAllocationAttributes& loc_alloc =
            participant->get_attributes().allocation.locators;

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
    EPROSIMA_LOG_INFO(RTPS_WRITER, "StatelessWriter destructor"; );
}

void StatelessWriter::local_actions_on_writer_removed()
{
    EPROSIMA_LOG_INFO(RTPS_WRITER, "StatelessWriter local_actions_on_writer_removed"; );
    BaseWriter::local_actions_on_writer_removed();
}

void StatelessWriter::get_builtin_guid()
{
    if (m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
    {
        add_guid(locator_selector_, GUID_t { GuidPrefix_t(), c_EntityId_SPDPReader });
    }
#if HAVE_SECURITY
    else if (m_guid.entityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER)
    {
        add_guid(locator_selector_, GUID_t { GuidPrefix_t(), participant_stateless_message_reader_entity_id });
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

    for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
            [this](const ReaderLocator& reader)
            {
                is_inline_qos_expected_ |= reader.expects_inline_qos();
                return false;
            }
            );

    update_cached_info_nts(locator_selector_);
    if (addGuid)
    {
        compute_selected_guids(locator_selector_);
    }

    if (create_sender_resources)
    {
        RTPSParticipantImpl* part = mp_RTPSParticipant;
        locator_selector_.locator_selector.for_each([part](const Locator_t& loc)
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
    auto pool = std::dynamic_pointer_cast<WriterPool>(history_->get_payload_pool());
    assert(pool != nullptr);

    pool->add_to_shared_history(change);
    EPROSIMA_LOG_INFO(RTPS_WRITER, "Notifying readers of cache change with SN " << change->sequenceNumber);
    for (std::unique_ptr<ReaderLocator>& reader : matched_datasharing_readers_)
    {
        if (!reader_data_filter_ || reader_data_filter_->is_relevant(*change, reader->remote_guid()))
        {
            reader->datasharing_notify();
        }
    }
    return true;
}

void StatelessWriter::unsent_change_added_to_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    auto payload_length = change->serializedPayload.length;

    if (liveliness_lease_duration_ < dds::c_TimeInfinite)
    {
        mp_RTPSParticipant->wlp()->assert_liveliness(
            getGuid(),
            liveliness_kind_,
            liveliness_lease_duration_);
    }

    // Notify the datasharing readers
    // This also prepares the metadata for late-joiners
    if (is_datasharing_compatible())
    {
        datasharing_delivery(change);
    }

    // Now for the rest of readers
    if (!fixed_locators_.empty() || get_matched_readers_size() > 0)
    {
        flow_controller_->add_new_sample(this, change, max_blocking_time);
    }
    else
    {
        EPROSIMA_LOG_INFO(RTPS_WRITER, "No reader to add change.");
        if (listener_ != nullptr)
        {
            listener_->on_writer_change_received_by_all(this, change);
        }
    }

    // Throughput should be notified even if no matches are available
    on_publish_throughput(payload_length);
}

bool StatelessWriter::intraprocess_delivery(
        CacheChange_t* change,
        ReaderLocator& reader_locator)
{
    LocalReaderPointer::Instance local_reader = reader_locator.local_reader();

    if (local_reader &&
            (!reader_data_filter_ || reader_data_filter_->is_relevant(*change, reader_locator.remote_guid())))
    {
        if (change->write_params.related_sample_identity() != SampleIdentity::unknown())
        {
            change->write_params.sample_identity(change->write_params.related_sample_identity());
        }
        return local_reader->process_data_msg(change);
    }

    return false;
}

bool StatelessWriter::change_removed_by_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    bool ret_value = false;
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    if (flow_controller_->remove_change(change, max_blocking_time))
    {

        // remove from datasharing pool history
        if (is_datasharing_compatible())
        {
            auto pool = std::dynamic_pointer_cast<WriterPool>(history_->get_payload_pool());
            assert (pool != nullptr);

            pool->remove_from_shared_history(change);
            EPROSIMA_LOG_INFO(RTPS_WRITER, "Removing shared cache change with SN " << change->sequenceNumber);
        }

        const uint64_t sequence_number = change->sequenceNumber.to64long();
        if (sequence_number > last_sequence_number_sent_)
        {
            unsent_changes_cond_.notify_all();
        }

        ret_value = true;
    }

    return ret_value;
}

bool StatelessWriter::has_been_fully_delivered(
        const SequenceNumber_t& seq_num) const
{
    // Sequence number has not been generated by this WriterHistory
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        if (seq_num >= history_->next_sequence_number())
        {
            return false;
        }
    }

    if (get_matched_readers_size() > 0)
    {
        return is_acked_by_all(seq_num);
    }
    return true;
}

bool StatelessWriter::is_acked_by_all(
        const SequenceNumber_t& seq_num) const
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    return seq_num.to64long() <= last_sequence_number_sent_;
}

bool StatelessWriter::wait_for_all_acked(
        const dds::Duration_t& max_wait)
{
    static_cast<void>(max_wait);
    return true;
}

bool StatelessWriter::get_disable_positive_acks() const
{
    return false;
}

bool StatelessWriter::matched_readers_guids(
        std::vector<GUID_t>& guids) const
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    guids.clear();
    guids.reserve(matched_local_readers_.size() + matched_datasharing_readers_.size() + matched_remote_readers_.size());
    for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
            [&guids](const ReaderLocator& reader)
            {
                guids.emplace_back(reader.remote_guid());
                return false;
            }
            );
    return true;
}

bool StatelessWriter::try_remove_change(
        const std::chrono::steady_clock::time_point&,
        std::unique_lock<RecursiveTimedMutex>&)
{
    return history_->remove_min_change();
}

bool StatelessWriter::wait_for_acknowledgement(
        const SequenceNumber_t& seq,
        const std::chrono::steady_clock::time_point& max_blocking_time_point,
        std::unique_lock<RecursiveTimedMutex>& lock)
{
    uint64_t seq_long_64 = seq.to64long();
    auto change_is_acknowledged = [this, seq, seq_long_64]()
            {
                bool ret = false;
                if (seq_long_64 <= last_sequence_number_sent_)
                {
                    // Stop waiting if the sequence number has been sent
                    ret = true;
                }
                else
                {
                    // If the sequence number has not been sent, stop waiting if it is not present in the history
                    CacheChange_t* change = nullptr;
                    ret = !history_->get_change(seq, m_guid, &change);
                }
                return ret;
            };
    return unsent_changes_cond_.wait_until(lock, max_blocking_time_point, change_is_acknowledged);
}

/*
 *	MATCHED_READER-RELATED METHODS
 */
bool StatelessWriter::matched_reader_add_edp(
        const ReaderProxyData& data)
{
    using network::external_locators::filter_remote_locators;

    std::unique_lock<RecursiveTimedMutex> guard(mp_mutex);
    std::unique_lock<LocatorSelectorSender> locator_selector_guard(locator_selector_);

    assert(data.guid != c_Guid_Unknown);

    if (for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
            [this, &data](ReaderLocator& reader)
            {
                if (reader.remote_guid() == data.guid)
                {
                    EPROSIMA_LOG_WARNING(RTPS_WRITER, "Attempting to add existing reader, updating information.");
                    if (reader.update(data.remote_locators.unicast,
                    data.remote_locators.multicast,
                    data.expects_inline_qos))
                    {
                        filter_remote_locators(*reader.general_locator_selector_entry(),
                        m_att.external_unicast_locators, m_att.ignore_non_matching_locators);
                        mp_RTPSParticipant->createSenderResources(data.remote_locators, m_att);
                        update_reader_info(true);
                    }
                    return true;
                }
                return false;
            }
            ))
    {
        if (nullptr != listener_)
        {
            // call the listener without locks taken
            locator_selector_guard.unlock();
            guard.unlock();
            listener_->on_reader_discovery(this, ReaderDiscoveryStatus::CHANGED_QOS_READER, data.guid, &data);
        }

#ifdef FASTDDS_STATISTICS
        // notify monitor service so that the connectionlist for this entity
        // could be updated
        if (nullptr != mp_RTPSParticipant->get_connections_observer() && !m_guid.is_builtin())
        {
            mp_RTPSParticipant->get_connections_observer()->on_local_entity_connections_change(m_guid);
        }
#endif //FASTDDS_STATISTICS

        return false;
    }

    // Get a locator from the inactive pool (or create a new one if necessary and allowed)
    std::unique_ptr<ReaderLocator> new_reader;
    if (matched_readers_pool_.empty())
    {
        size_t max_readers = matched_readers_pool_.max_size();
        if (get_matched_readers_size() + matched_readers_pool_.size() < max_readers)
        {
            const RemoteLocatorsAllocationAttributes& loc_alloc =
                    mp_RTPSParticipant->get_attributes().allocation.locators;

            new_reader.reset(new ReaderLocator(
                        this,
                        loc_alloc.max_unicast_locators,
                        loc_alloc.max_multicast_locators));
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_WRITER, "Couldn't add matched reader due to resource limits");
            return false;
        }
    }
    else
    {
        new_reader = std::move(matched_readers_pool_.back());
        matched_readers_pool_.pop_back();
    }

    // Add info of new datareader.
    new_reader->start(data.guid,
            data.remote_locators.unicast,
            data.remote_locators.multicast,
            data.expects_inline_qos,
            is_datasharing_compatible_with(data.data_sharing));
    filter_remote_locators(*new_reader->general_locator_selector_entry(),
            m_att.external_unicast_locators, m_att.ignore_non_matching_locators);

    locator_selector_.locator_selector.add_entry(new_reader->general_locator_selector_entry());

    if (new_reader->is_local_reader())
    {
        matched_local_readers_.push_back(std::move(new_reader));
        EPROSIMA_LOG_INFO(RTPS_WRITER, "Adding reader " << data.guid << " to " << this->m_guid.entityId
                                                        << " as local reader");
    }
    else if (new_reader->is_datasharing_reader())
    {
        matched_datasharing_readers_.push_back(std::move(new_reader));
        EPROSIMA_LOG_INFO(RTPS_WRITER, "Adding reader " << data.guid << " to " << this->m_guid.entityId
                                                        << " as data sharing");
    }
    else
    {
        matched_remote_readers_.push_back(std::move(new_reader));
        EPROSIMA_LOG_INFO(RTPS_WRITER, "Adding reader " << data.guid << " to " << this->m_guid.entityId
                                                        << " as remote reader");
    }

    // Create sender resources for the case when we send to a single reader
    mp_RTPSParticipant->createSenderResources(data.remote_locators, m_att);

    // Create sender resources for the case when we send to all readers
    update_reader_info(true);

    if (nullptr != listener_)
    {
        // call the listener without locks taken
        locator_selector_guard.unlock();
        guard.unlock();
        listener_->on_reader_discovery(this, ReaderDiscoveryStatus::DISCOVERED_READER, data.guid, &data);
    }

#ifdef FASTDDS_STATISTICS
    // notify monitor service so that the connectionlist for this entity
    // could be updated
    if (nullptr != mp_RTPSParticipant->get_connections_observer() && !m_guid.is_builtin())
    {
        mp_RTPSParticipant->get_connections_observer()->on_local_entity_connections_change(m_guid);
    }
#endif //FASTDDS_STATISTICS

    return true;
}

bool StatelessWriter::matched_reader_remove(
        const GUID_t& reader_guid)
{
    std::unique_lock<RecursiveTimedMutex> guard(mp_mutex);
    std::unique_lock<LocatorSelectorSender> locator_selector_guard(locator_selector_);

    if (locator_selector_.locator_selector.remove_entry(reader_guid))
    {
        std::unique_ptr<ReaderLocator> reader;
        for (auto it = matched_local_readers_.begin();
                it != matched_local_readers_.end(); ++it)
        {
            if ((*it)->remote_guid() == reader_guid)
            {
                reader = std::move(*it);
                matched_local_readers_.erase(it);
                break;
            }
        }

        if (reader == nullptr)
        {
            for (auto it = matched_datasharing_readers_.begin();
                    it != matched_datasharing_readers_.end(); ++it)
            {
                if ((*it)->remote_guid() == reader_guid)
                {
                    reader = std::move(*it);
                    matched_datasharing_readers_.erase(it);
                    break;
                }
            }
        }

        if (reader == nullptr)
        {
            for (auto it = matched_remote_readers_.begin();
                    it != matched_remote_readers_.end(); ++it)
            {
                if ((*it)->remote_guid() == reader_guid)
                {
                    reader = std::move(*it);
                    matched_remote_readers_.erase(it);
                    break;
                }
            }
        }

        // guid should be both on locator_selector_ and matched_readers_ or in none
        assert(reader != nullptr);

        reader->stop();
        matched_readers_pool_.push_back(std::move(reader));
        update_reader_info(false);
        EPROSIMA_LOG_INFO(RTPS_WRITER, "Reader Proxy removed: " << reader_guid);
        if (nullptr != listener_)
        {
            // call the listener without locks taken
            locator_selector_guard.unlock();
            guard.unlock();

            listener_->on_reader_discovery(this, ReaderDiscoveryStatus::REMOVED_READER, reader_guid, nullptr);
        }

#ifdef FASTDDS_STATISTICS
        // notify monitor service so that the connectionlist for this entity
        // could be updated
        if (nullptr != mp_RTPSParticipant->get_connections_observer() && !m_guid.is_builtin())
        {
            mp_RTPSParticipant->get_connections_observer()->on_local_entity_connections_change(m_guid);
        }
#endif //FASTDDS_STATISTICS

        return true;
    }

    return false;
}

bool StatelessWriter::matched_reader_is_matched(
        const GUID_t& reader_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    return for_matched_readers(matched_local_readers_, matched_datasharing_readers_, matched_remote_readers_,
                   [reader_guid](const ReaderLocator& reader)
                   {
                       return reader.remote_guid() == reader_guid;
                   }
                   );
}

bool StatelessWriter::process_acknack(
        const GUID_t& writer_guid,
        const GUID_t& reader_guid,
        uint32_t ack_count,
        const SequenceNumberSet_t& sn_set,
        bool final_flag,
        bool& result,
        fastdds::rtps::VendorId_t origin_vendor_id)
{
    static_cast<void>(reader_guid);
    static_cast<void>(ack_count);
    static_cast<void>(sn_set);
    static_cast<void>(final_flag);
    static_cast<void>(origin_vendor_id);

    result = false;
    return writer_guid == m_guid;
}

bool StatelessWriter::process_nack_frag(
        const GUID_t& writer_guid,
        const GUID_t& reader_guid,
        uint32_t ack_count,
        const SequenceNumber_t& seq_num,
        const FragmentNumberSet_t& fragments_state,
        bool& result,
        fastdds::rtps::VendorId_t origin_vendor_id)
{
    static_cast<void>(reader_guid);
    static_cast<void>(ack_count);
    static_cast<void>(seq_num);
    static_cast<void>(fragments_state);
    static_cast<void>(origin_vendor_id);

    result = false;
    return writer_guid == m_guid;
}

bool StatelessWriter::send_nts(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
        const uint32_t& total_bytes,
        const LocatorSelectorSender& locator_selector,
        std::chrono::steady_clock::time_point& max_blocking_time_point) const
{
    if (!BaseWriter::send_nts(buffers, total_bytes, locator_selector, max_blocking_time_point))
    {
        return false;
    }

    return send_to_fixed_locators(buffers, total_bytes, max_blocking_time_point);
}

bool StatelessWriter::send_to_fixed_locators(
        const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
        const uint32_t& total_bytes,
        std::chrono::steady_clock::time_point& max_blocking_time_point) const
{
    return fixed_locators_.empty() ||
           mp_RTPSParticipant->sendSync(buffers, total_bytes, m_guid,
                   Locators(fixed_locators_.begin()), Locators(fixed_locators_.end()),
                   max_blocking_time_point);
}

DeliveryRetCode StatelessWriter::deliver_sample_nts(
        CacheChange_t* cache_change,
        RTPSMessageGroup& group,
        LocatorSelectorSender& locator_selector, // Object locked by FlowControllerImpl
        const std::chrono::time_point<std::chrono::steady_clock>& /*TODO max_blocking_time*/)
{
    uint64_t change_sequence_number = cache_change->sequenceNumber.to64long();
    NetworkFactory& network = mp_RTPSParticipant->network_factory();
    DeliveryRetCode ret_code = DeliveryRetCode::DELIVERED;

    if (current_sequence_number_sent_ != change_sequence_number)
    {
        current_sequence_number_sent_ = change_sequence_number;
        current_fragment_sent_ = 0;
    }

    // Send the new sample to intra-process readers.
    if (0 == current_fragment_sent_)
    {
        for_matched_readers(matched_local_readers_, [&, cache_change](ReaderLocator& reader)
                {
                    intraprocess_delivery(cache_change, reader);
                    return false;
                });
    }

    try
    {
        uint32_t n_fragments = cache_change->getFragmentCount();

        if (separate_sending_enabled_)
        {
            if (0 < n_fragments)
            {
                for (FragmentNumber_t frag = current_fragment_sent_ + 1;
                        DeliveryRetCode::DELIVERED == ret_code && frag <= n_fragments; ++frag)
                {
                    for (std::unique_ptr<ReaderLocator>& it : matched_remote_readers_)
                    {
                        if ((nullptr == reader_data_filter_) ||
                                reader_data_filter_->is_relevant(*cache_change, it->remote_guid()))
                        {
                            group.sender(this, &*it);
                            size_t num_locators = it->locators_size();

                            if (group.add_data_frag(*cache_change, frag, is_inline_qos_expected_))
                            {
                                add_statistics_sent_submessage(cache_change, num_locators);
                            }
                            else
                            {
                                EPROSIMA_LOG_ERROR(RTPS_WRITER,
                                        "Error sending fragment (" << cache_change->sequenceNumber << ", " << frag <<
                                        ")");
                                ret_code = DeliveryRetCode::NOT_DELIVERED;
                            }
                        }
                    }

                    if (DeliveryRetCode::DELIVERED == ret_code)
                    {
                        current_fragment_sent_ = frag;
                    }
                }
            }
            else
            {
                for (std::unique_ptr<ReaderLocator>& it : matched_remote_readers_)
                {
                    if ((nullptr == reader_data_filter_) ||
                            reader_data_filter_->is_relevant(*cache_change, it->remote_guid()))
                    {
                        group.sender(this, &*it);
                        size_t num_locators = it->locators_size();

                        if (group.add_data(*cache_change, is_inline_qos_expected_))
                        {
                            add_statistics_sent_submessage(cache_change, num_locators);
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Error sending change " << cache_change->sequenceNumber);
                            ret_code = DeliveryRetCode::NOT_DELIVERED;
                        }
                    }
                }
            }
        }
        else
        {
            if (nullptr != reader_data_filter_)
            {
                locator_selector.locator_selector.reset(false);
                for (std::unique_ptr<ReaderLocator>& it : matched_remote_readers_)
                {
                    if (reader_data_filter_->is_relevant(*cache_change, it->remote_guid()))
                    {
                        locator_selector.locator_selector.enable(it->remote_guid());
                    }
                }
            }
            else
            {
                locator_selector.locator_selector.reset(true);
            }

            if (locator_selector.locator_selector.state_has_changed())
            {
                network.select_locators(locator_selector.locator_selector);
                if (!has_builtin_guid())
                {
                    compute_selected_guids(locator_selector);
                }
            }
            size_t num_locators = locator_selector.locator_selector.selected_size() + fixed_locators_.size();

            if (0 < num_locators)
            {
                if (0 < n_fragments)
                {
                    for (FragmentNumber_t frag = current_fragment_sent_ + 1;
                            DeliveryRetCode::DELIVERED == ret_code && frag <= n_fragments; ++frag)
                    {
                        if (group.add_data_frag(*cache_change, frag, is_inline_qos_expected_))
                        {
                            current_fragment_sent_ = frag;
                            add_statistics_sent_submessage(cache_change, num_locators);
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(RTPS_WRITER,
                                    "Error sending fragment (" << cache_change->sequenceNumber << ", " << frag << ")");
                            ret_code = DeliveryRetCode::NOT_DELIVERED;
                        }
                    }
                }
                else
                {
                    if (group.add_data(*cache_change, is_inline_qos_expected_))
                    {
                        add_statistics_sent_submessage(cache_change, num_locators);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Error sending change " << cache_change->sequenceNumber);
                        ret_code = DeliveryRetCode::NOT_DELIVERED;
                    }
                }
            }
        }

        // Do not send data without information (submessages)
        if (cache_change->writer_info.num_sent_submessages)
        {
            on_sample_datas(cache_change->write_params.sample_identity(),
                    cache_change->writer_info.num_sent_submessages);
            on_data_sent();
        }

    }
    catch (const RTPSMessageGroup::timeout&)
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Max blocking time reached");
        ret_code = DeliveryRetCode::NOT_DELIVERED;
    }
    catch (const RTPSMessageGroup::limit_exceeded&)
    {
        ret_code = DeliveryRetCode::EXCEEDED_LIMIT;
    }

    group.sender(this, &locator_selector);

    if (DeliveryRetCode::DELIVERED == ret_code &&
            change_sequence_number > last_sequence_number_sent_)
    {
        // This update must be done before calling the callback.
        last_sequence_number_sent_ = change_sequence_number;
        unsent_changes_cond_.notify_all();

        if (nullptr != listener_)
        {
            listener_->on_writer_change_received_by_all(this, cache_change);
        }
    }

    return ret_code;
}

#ifdef FASTDDS_STATISTICS

bool StatelessWriter::get_connections(
        fastdds::statistics::rtps::ConnectionList& connection_list)
{
    connection_list.reserve(matched_local_readers_.size() +
            matched_datasharing_readers_.size() +
            matched_remote_readers_.size());

    fastdds::statistics::Connection connection;

    {
        std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

        //! intraprocess
        for_matched_readers(matched_local_readers_, [&connection, &connection_list](ReaderLocator& reader)
                {
                    connection.guid(fastdds::statistics::to_statistics_type(reader.remote_guid()));
                    connection.mode(fastdds::statistics::ConnectionMode::INTRAPROCESS);
                    connection_list.push_back(connection);

                    return false;
                });
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

        //! datasharing
        for_matched_readers(matched_datasharing_readers_, [&connection, &connection_list](ReaderLocator& reader)
                {
                    connection.guid(fastdds::statistics::to_statistics_type(reader.remote_guid()));
                    connection.mode(fastdds::statistics::ConnectionMode::DATA_SHARING);
                    connection_list.push_back(connection);

                    return false;
                });
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(mp_mutex);

        //! remote
        for_matched_readers(matched_remote_readers_, [&connection, &connection_list](ReaderLocator& reader)
                {
                    //! Announced locators is, for the moment,
                    //! equal to the used_locators
                    LocatorSelectorEntry* loc_selector_entry = reader.general_locator_selector_entry();

                    connection.announced_locators().reserve(reader.locators_size());
                    connection.used_locators().reserve(reader.locators_size());

                    std::vector<fastdds::statistics::detail::Locator_s> statistics_locators;
                    std::for_each(loc_selector_entry->multicast.begin(), loc_selector_entry->multicast.end(),
                    [&statistics_locators](const Locator_t& locator)
                    {
                        statistics_locators.push_back(fastdds::statistics::to_statistics_type(locator));
                    });

                    std::for_each(loc_selector_entry->unicast.begin(), loc_selector_entry->unicast.end(),
                    [&statistics_locators](const Locator_t& locator)
                    {
                        statistics_locators.push_back(fastdds::statistics::to_statistics_type(locator));
                    });

                    connection.guid(fastdds::statistics::to_statistics_type(reader.remote_guid()));
                    connection.mode(fastdds::statistics::ConnectionMode::TRANSPORT);
                    connection.announced_locators(statistics_locators);
                    connection.used_locators(statistics_locators);
                    connection_list.push_back(connection);

                    return false;
                });
    }

    return true;
}

#endif // ifdef FASTDDS_STATISTICS

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
