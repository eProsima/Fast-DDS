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

#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include "../participant/RTPSParticipantImpl.h"
#include "../flowcontrol/FlowController.h"
#include "../history/HistoryAttributesExtension.hpp"
#include "RTPSWriterCollector.h"

#include <algorithm>
#include <mutex>
#include <set>
#include <vector>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

StatelessWriter::StatelessWriter(
        RTPSParticipantImpl* participant,
        GUID_t& guid,
        WriterAttributes& attributes,
        WriterHistory* history,
        WriterListener* listener)
    : RTPSWriter(participant, guid, attributes, history, listener)
    , matched_readers_(attributes.matched_readers_allocation)
    , unsent_changes_(resource_limits_from_history(history->m_att))
{
    get_builtin_guid();

    for (size_t i = 0; i < attributes.matched_readers_allocation.initial; ++i)
    {
        // TODO (Miguel C): Use participant locators allocation policy
        matched_readers_.emplace_back(getRTPSParticipant(), 4u, 1u);
    }
}

StatelessWriter::~StatelessWriter()
{
    AsyncWriterThread::removeWriter(*this);
    logInfo(RTPS_WRITER,"StatelessWriter destructor";);
}

void StatelessWriter::get_builtin_guid()
{
    if (m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
    {
        add_guid(GUID_t{ GuidPrefix_t(), c_EntityId_SPDPReader });
    }
#if HAVE_SECURITY
    else if (m_guid.entityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER)
    {
        add_guid(GUID_t{ GuidPrefix_t(), participant_stateless_message_reader_entity_id });
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

/*
 *	CHANGE-RELATED METHODS
 */

// TODO(Ricardo) This function only can be used by history. Private it and frined History.
// TODO(Ricardo) Look for other functions
void StatelessWriter::unsent_change_added_to_history(CacheChange_t* change)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    if (!mAllShrinkedLocatorList.empty())
    {
#if HAVE_SECURITY
        encrypt_cachechange(change);
#endif

        if (!isAsync())
        {
            setLivelinessAsserted(true);

            if(m_separateSendingEnabled)
            {
                std::vector<GUID_t> guids(1);
                for (const ReaderLocator& it : matched_readers_)
                {
                    RTPSMessageGroup group(mp_RTPSParticipant, this, m_cdrmessages, it);
                    
                    if (!group.add_data(*change, it.expects_inline_qos()))
                    {
                        logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
                    }
                }
            }
            else
            {
                RTPSMessageGroup group(mp_RTPSParticipant, this, m_cdrmessages, *this);

                if (!group.add_data(*change, is_inline_qos_expected_))
                {
                    logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
                }
            }

            if (mp_listener != nullptr)
            {
                mp_listener->onWriterChangeReceivedByAll(this, change);
            }
        }
        else
        {
            unsent_changes_.push_back(ChangeForReader_t(change));
            AsyncWriterThread::wakeUp(this);
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

bool StatelessWriter::change_removed_by_history(CacheChange_t* change)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    unsent_changes_.remove_if(
        [change](ChangeForReader_t& cptr)
    {
        return cptr.getChange() == change ||
            cptr.getChange()->sequenceNumber == change->sequenceNumber;
    });

    return true;
}

bool StatelessWriter::is_acked_by_all(const CacheChange_t* change) const
{
    // Only asynchronous writers may have unacked (i.e. unsent changes)
    if (isAsync())
    {
        std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

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
    if(it != unsent_changes_.end())
    {
        bool should_remove = (frag_num == 0);
        if (!should_remove)
        {
            it->markFragmentsAsSent(frag_num);
            FragmentNumberSet_t fragment_sns = it->getUnsentFragments();
            should_remove = fragment_sns.empty();
        }

        if(should_remove)
        {
            unsent_changes_.remove_if(find_by_seq_num);
        }
    }
}

void StatelessWriter::send_any_unsent_changes()
{
    //TODO(Mcc) Separate sending for asynchronous writers
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    RTPSWriterCollector<ReaderLocator*> changesToSend;

    for (const ChangeForReader_t& unsentChange : unsent_changes_)
    {
        changesToSend.add_change(unsentChange.getChange(), nullptr, unsentChange.getUnsentFragments());
    }

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

    RTPSMessageGroup group(mp_RTPSParticipant, this, m_cdrmessages, *this);

    bool bHasListener = mp_listener != nullptr;
    while(!changesToSend.empty())
    {
        RTPSWriterCollector<ReaderLocator*>::Item changeToSend = changesToSend.pop();

        // Remove the messages selected for sending from the original list,
        // and update those that were fragmented with the new sent index
        update_unsent_changes(changeToSend.sequenceNumber, changeToSend.fragmentNumber);

        // Notify the controllers
        FlowController::NotifyControllersChangeSent(changeToSend.cacheChange);

        if(changeToSend.fragmentNumber != 0)
        {
            if(!group.add_data_frag(*changeToSend.cacheChange, changeToSend.fragmentNumber, is_inline_qos_expected_))
            {
                logError(RTPS_WRITER, "Error sending fragment (" << changeToSend.sequenceNumber <<
                        ", " << changeToSend.fragmentNumber << ")");
            }
        }
        else
        {
            if(!group.add_data(*changeToSend.cacheChange, is_inline_qos_expected_))
            {
                logError(RTPS_WRITER, "Error sending change " << changeToSend.sequenceNumber);
            }
        }

        if (bHasListener && is_acked_by_all(changeToSend.cacheChange))
        {
            mp_listener->onWriterChangeReceivedByAll(this, changeToSend.cacheChange);
        }
    }

    logInfo(RTPS_WRITER, "Finish sending unsent changes";);
}


/*
 *	MATCHED_READER-RELATED METHODS
 */
bool StatelessWriter::matched_reader_add(const ReaderProxyData& data)
{
    // TODO (Miguel C): Pending refactor for locator shrink
    RemoteReaderAttributes tmp(data);
    return matched_reader_add(tmp);
}

bool StatelessWriter::matched_reader_add(RemoteReaderAttributes& reader_attributes)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    assert(reader_attributes.guid != c_Guid_Unknown);

    bool addGuid = !has_builtin_guid();
    for(const ReaderLocator& reader : matched_readers_)
    {
        if(reader.remote_guid() == reader_attributes.guid)
        {
            logWarning(RTPS_WRITER, "Attempting to add existing reader");
            return false;
        }
    }

    // Try to add entry on matched_readers_
    ReaderLocator* new_reader = nullptr;
    for (ReaderLocator& reader : matched_readers_)
    {
        if (reader.start(reader_attributes.guid,
            reader_attributes.endpoint.unicastLocatorList,
            reader_attributes.endpoint.multicastLocatorList,
            reader_attributes.expectsInlineQos))
        {
            new_reader = &reader;
            break;
        }
    }
    if (new_reader == nullptr)
    {
        // TODO (Miguel C): Use participant locators allocation policy
        new_reader = matched_readers_.emplace_back(getRTPSParticipant(), 4u, 1u);
        if (new_reader != nullptr)
        {
            new_reader->start(reader_attributes.guid,
                reader_attributes.endpoint.unicastLocatorList,
                reader_attributes.endpoint.multicastLocatorList,
                reader_attributes.expectsInlineQos);
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

    update_cached_info_nts();

    is_inline_qos_expected_ |= reader_attributes.expectsInlineQos;

    if (addGuid)
    {
        compute_selected_guids();
    }

    if (reader_attributes.endpoint.durabilityKind >= TRANSIENT_LOCAL)
    {
        unsent_changes_.assign(mp_history->changesBegin(), mp_history->changesEnd());
        AsyncWriterThread::wakeUp(this);
    }

    // TODO (Miguel C): refactor with locator selector
    getRTPSParticipant()->createSenderResources(mAllShrinkedLocatorList);

    logInfo(RTPS_READER,"Reader " << reader_attributes.guid << " added to "<<m_guid.entityId);
    return true;
}

bool StatelessWriter::set_fixed_locators(const LocatorList_t& locator_list)
{
#if HAVE_SECURITY
    if (getAttributes().security_attributes().is_submessage_protected ||
        getAttributes().security_attributes().is_payload_protected)
    {
        logError(RTPS_WRITER, "A secure besteffort writer cannot add a lonely locator");
        return false;
    }
#endif

    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    fixed_locators_.push_back(locator_list);
    getRTPSParticipant()->createSenderResources(fixed_locators_);

    return true;
}

bool StatelessWriter::matched_reader_remove(const GUID_t& reader_guid)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    bool found = locator_selector_.remove_entry(reader_guid); 
    if(found)
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

        bool addGuid = !has_builtin_guid();
        is_inline_qos_expected_ = false;

        for (const ReaderLocator& reader : matched_readers_)
        {
            is_inline_qos_expected_ |= reader.expects_inline_qos();
        }

        update_cached_info_nts();
        if (addGuid)
        {
            compute_selected_guids();
        }
    }

    return found;
}

bool StatelessWriter::matched_reader_is_matched(const GUID_t& reader_guid)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    return std::any_of(matched_readers_.begin(), matched_readers_.end(), 
        [reader_guid](const ReaderLocator& item)
        {
            return item.remote_guid() == reader_guid;
        });
}

void StatelessWriter::unsent_changes_reset()
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    unsent_changes_.assign(mp_history->changesBegin(), mp_history->changesEnd());
    AsyncWriterThread::wakeUp(this);
}

void StatelessWriter::add_flow_controller(std::unique_ptr<FlowController> controller)
{
    flow_controllers_.push_back(std::move(controller));
}

void StatelessWriter::send(CDRMessage_t* message) const
{
    RTPSWriter::send(message);
    for (const Locator_t& locator : fixed_locators_)
    {
        getRTPSParticipant()->sendSync(message, locator);
    }
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
