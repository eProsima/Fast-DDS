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

#include <fastrtps/log/Log.h>
#include "rtps/RTPSDomainImpl.hpp"

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
                if (group.add_data_frag(*change, frag, inline_qos))
                {
                    reader_change->markFragmentsAsSent(frag);
                }
                else
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
        RTPSParticipantImpl* participant,
        GUID_t& guid,
        WriterAttributes& attributes,
        WriterHistory* history,
        WriterListener* listener)
    : RTPSWriter(
        participant,
        guid,
        attributes,
        history,
        listener)
    , matched_readers_(attributes.matched_readers_allocation)
    , unsent_changes_(resource_limits_from_history(history->m_att))
    , last_intraprocess_sequence_number_(0)
{
    get_builtin_guid();

    const RemoteLocatorsAllocationAttributes& loc_alloc =
            participant->getRTPSParticipantAttributes().allocation.locators;
    for (size_t i = 0; i < attributes.matched_readers_allocation.initial; ++i)
    {
        matched_readers_.emplace_back(
            mp_RTPSParticipant,
            loc_alloc.max_unicast_locators,
            loc_alloc.max_multicast_locators);
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
#endif
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
#endif
    return false;
}

void StatelessWriter::update_reader_info(
        bool create_sender_resources)
{
    bool addGuid = !has_builtin_guid();
    is_inline_qos_expected_ = false;
    there_are_remote_readers_ = false;

    for (const ReaderLocator& reader : matched_readers_)
    {
        is_inline_qos_expected_ |= reader.expects_inline_qos();
        there_are_remote_readers_ |= !reader.is_local_reader();
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

// TODO(Ricardo) This function only can be used by history. Private it and frined History.
// TODO(Ricardo) Look for other functions
void StatelessWriter::unsent_change_added_to_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    if (!fixed_locators_.empty() || matched_readers_.size() > 0)
    {
#if HAVE_SECURITY
        encrypt_cachechange(change);
#endif

        if (!isAsync())
        {
            try
            {
                if (m_separateSendingEnabled)
                {
                    std::vector<GUID_t> guids(1);
                    for (ReaderLocator& it : matched_readers_)
                    {
                        if (it.is_local_reader())
                        {
                            intraprocess_delivery(change, it);
                        }
                        else
                        {
                            RTPSMessageGroup group(mp_RTPSParticipant, this, it, max_blocking_time);

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
                    for (ReaderLocator& it : matched_readers_)
                    {
                        if (it.is_local_reader())
                        {
                            intraprocess_delivery(change, it);
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

        if (liveliness_lease_duration_ < c_TimeInfinite)
        {
            mp_RTPSParticipant->wlp()->assert_liveliness(
                getGuid(),
                liveliness_kind_,
                liveliness_lease_duration_);
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

    unsent_changes_.remove_if(
        [change](ChangeForReader_t& cptr)
                {
                    return cptr.getChange() == change ||
                    cptr.getChange()->sequenceNumber == change->sequenceNumber;
                });

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
}

void StatelessWriter::send_all_unsent_changes()
{
    //TODO(Mcc) Separate sending for asynchronous writers

    RTPSMessageGroup group(mp_RTPSParticipant, this, *this);
    bool remote_destinations = there_are_remote_readers_ || !fixed_locators_.empty();
    bool bHasListener = mp_listener != nullptr;

    while (!unsent_changes_.empty())
    {
        ChangeForReader_t& unsentChange = unsent_changes_.front();
        CacheChange_t* cache_change = unsentChange.getChange();

        uint64_t sequence_number = cache_change->sequenceNumber.to64long();
        // Filter intraprocess unsent changes
        if (sequence_number > last_intraprocess_sequence_number_)
        {
            last_intraprocess_sequence_number_ = sequence_number;
            for (ReaderLocator& it : matched_readers_)
            {
                if (it.is_local_reader())
                {
                    intraprocess_delivery(cache_change, it);
                }
            }
        }

        if (remote_destinations)
        {
            if(!add_change_to_rtps_group(group, &unsentChange, is_inline_qos_expected_))
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
}

void StatelessWriter::send_unsent_changes_with_flow_control()
{
    //TODO(Mcc) Separate sending for asynchronous writers

    // There should be remote destinations
    assert(there_are_remote_readers_ || !fixed_locators_.empty());

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
                for (ReaderLocator& it : matched_readers_)
                {
                    if (it.is_local_reader())
                    {
                        intraprocess_delivery(cache_change, it);
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

            while (!changesToSend.empty())
            {
                RTPSWriterCollector<ReaderLocator*>::Item changeToSend = changesToSend.pop();

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
}

/*
 *	MATCHED_READER-RELATED METHODS
 */
bool StatelessWriter::matched_reader_add(
        const ReaderProxyData& data)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    assert(data.guid() != c_Guid_Unknown);

    for (ReaderLocator& reader : matched_readers_)
    {
        if (reader.remote_guid() == data.guid())
        {
            logWarning(RTPS_WRITER, "Attempting to add existing reader, updating information.");
            if (reader.update(data.remote_locators().unicast,
                    data.remote_locators().multicast,
                    data.m_expectsInlineQos))
            {
                update_reader_info(true);
            }
            return false;
        }
    }

    // Try to add entry on matched_readers_
    ReaderLocator* new_reader = nullptr;
    for (ReaderLocator& reader : matched_readers_)
    {
        if (reader.start(data.guid(),
                data.remote_locators().unicast,
                data.remote_locators().multicast,
                data.m_expectsInlineQos))
        {
            new_reader = &reader;
            break;
        }
    }
    if (new_reader == nullptr)
    {
        const RemoteLocatorsAllocationAttributes& loc_alloc =
                mp_RTPSParticipant->getRTPSParticipantAttributes().allocation.locators;
        new_reader = matched_readers_.emplace_back(
            mp_RTPSParticipant,
            loc_alloc.max_unicast_locators,
            loc_alloc.max_multicast_locators);
        if (new_reader != nullptr)
        {
            new_reader->start(data.guid(),
                    data.remote_locators().unicast,
                    data.remote_locators().multicast,
                    data.m_expectsInlineQos);
        }
        else
        {
            logWarning(RTPS_WRITER, "Couldn't add matched reader due to resource limits");
            return false;
        }
    }

    // Add info of new datareader.
    locator_selector_.clear();
    for (ReaderLocator& reader : matched_readers_)
    {
        locator_selector_.add_entry(reader.locator_selector_entry());
    }

    update_reader_info(true);

    if (data.m_qos.m_durability.kind >= TRANSIENT_LOCAL_DURABILITY_QOS)
    {
        unsent_changes_.assign(mp_history->changesBegin(), mp_history->changesEnd());
        mp_RTPSParticipant->async_thread().wake_up(this);
    }

    logInfo(RTPS_READER, "Reader " << data.guid() << " added to " << m_guid.entityId);
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
#endif

    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    fixed_locators_.push_back(locator_list);
    mp_RTPSParticipant->createSenderResources(fixed_locators_);

    return true;
}

bool StatelessWriter::matched_reader_remove(
        const GUID_t& reader_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    bool found = locator_selector_.remove_entry(reader_guid);
    if (found)
    {
        found = false;
        for (ReaderLocator& reader : matched_readers_)
        {
            if (reader.stop(reader_guid))
            {
                found = true;
                break;
            }
        }
        // guid should be both on locator_selector_ and matched_readers_
        assert(found);

        update_reader_info(false);
    }

    return found;
}

bool StatelessWriter::matched_reader_is_matched(
        const GUID_t& reader_guid)
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
    return std::any_of(matched_readers_.begin(), matched_readers_.end(),
                   [reader_guid](const ReaderLocator& item)
                {
                    return item.remote_guid() == reader_guid;
                });
}

void StatelessWriter::unsent_changes_reset()
{
    std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);

    unsent_changes_.assign(mp_history->changesBegin(), mp_history->changesEnd());
    mp_RTPSParticipant->async_thread().wake_up(this);
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

    for (const Locator_t& locator : fixed_locators_)
    {
        if (!mp_RTPSParticipant->sendSync(message, locator, max_blocking_time_point))
        {
            return false;
        }
    }

    return true;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
