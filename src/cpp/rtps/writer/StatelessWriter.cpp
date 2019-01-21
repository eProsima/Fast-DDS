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
#include "RTPSWriterCollector.h"

#include <algorithm>
#include <mutex>
#include <set>
#include <vector>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {


StatelessWriter::StatelessWriter(RTPSParticipantImpl* pimpl,GUID_t& guid,
        WriterAttributes& att,WriterHistory* hist,WriterListener* listen):
    RTPSWriter(pimpl,guid,att,hist,listen)
{
    get_builtin_guid(all_remote_readers_);
}

StatelessWriter::~StatelessWriter()
{
    AsyncWriterThread::removeWriter(*this);
    logInfo(RTPS_WRITER,"StatelessWriter destructor";);
}

void StatelessWriter::get_builtin_guid(ResourceLimitedVector<GUID_t>& guidVector)
{
    if(m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
        guidVector.emplace_back(GUID_t{GuidPrefix_t(), c_EntityId_SPDPReader});
#if HAVE_SECURITY
    else if(m_guid.entityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER)
        guidVector.emplace_back(GUID_t{GuidPrefix_t(), participant_stateless_message_reader_entity_id});
#endif
}

bool StatelessWriter::has_builtin_guid()
{
    if (m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
        return true;
#if HAVE_SECURITY
    if (m_guid.entityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER)
        return true;
#endif
    return false;
}

/*
 *	CHANGE-RELATED METHODS
 */

// TODO(Ricardo) This function only can be used by history. Private it and frined History.
// TODO(Ricardo) Look for other functions
void StatelessWriter::unsent_change_added_to_history(CacheChange_t* cptr)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    if (!mAllShrinkedLocatorList.empty())
    {
#if HAVE_SECURITY
        encrypt_cachechange(cptr);
#endif

        if (!isAsync())
        {
            setLivelinessAsserted(true);

            if(m_separateSendingEnabled)
            {
                std::vector<GUID_t> guids(1);
                for (auto it = matched_readers_.begin(); it != matched_readers_.end(); ++it)
                {
                    guids.at(0) = it->guid;
                    RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages,
                        it->endpoint.unicastLocatorList, guids);
                    
                    if (!group.add_data(*cptr, guids, it->endpoint.unicastLocatorList, it->expectsInlineQos))
                    {
                        logError(RTPS_WRITER, "Error sending change " << cptr->sequenceNumber);
                    }
                }
            }
            else
            {
                RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages,
                    mAllShrinkedLocatorList, all_remote_readers_);

                if (!group.add_data(*cptr, all_remote_readers_, mAllShrinkedLocatorList, is_inline_qos_expected_))
                {
                    logError(RTPS_WRITER, "Error sending change " << cptr->sequenceNumber);
                }
            }

            if (mp_listener != nullptr)
            {
                mp_listener->onWriterChangeReceivedByAll(this, cptr);
            }
        }
        else
        {
            unsent_changes_.push_back(ChangeForReader_t(cptr));
            AsyncWriterThread::wakeUp(this);
        }
    }
    else
    {
        logInfo(RTPS_WRITER, "No reader to add change.");
        if (mp_listener != nullptr)
        {
            mp_listener->onWriterChangeReceivedByAll(this, cptr);
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

void StatelessWriter::update_unsent_changes(const SequenceNumber_t& seqNum, const FragmentNumber_t fragNum)
{
    auto find_by_seq_num = [seqNum](const ChangeForReader_t& unsent_change)
    {
        return seqNum == unsent_change.getSequenceNumber();
    };

    auto it = std::find_if(unsent_changes_.begin(), unsent_changes_.end(), find_by_seq_num);
    if(it != unsent_changes_.end())
    {
        bool should_remove = fragNum == 0;
        if (!should_remove)
        {
            it->markFragmentsAsSent(fragNum);
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

    ReaderLocator tmp;
    RTPSWriterCollector<ReaderLocator*> changesToSend;

    for (auto unsentChange : unsent_changes_)
    {
        changesToSend.add_change(unsentChange.getChange(), &tmp, unsentChange.getUnsentFragments());
    }

    // Clear through local controllers
    for (auto& controller : m_controllers)
        (*controller)(changesToSend);

    // Clear through parent controllers
    for (auto& controller : mp_RTPSParticipant->getFlowControllers())
        (*controller)(changesToSend);

    RTPSMessageGroup group(mp_RTPSParticipant, this,  RTPSMessageGroup::WRITER, m_cdrmessages,
        mAllShrinkedLocatorList, all_remote_readers_);

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
            if(!group.add_data_frag(*changeToSend.cacheChange, changeToSend.fragmentNumber, all_remote_readers_,
                        mAllShrinkedLocatorList, is_inline_qos_expected_))
            {
                logError(RTPS_WRITER, "Error sending fragment (" << changeToSend.sequenceNumber <<
                        ", " << changeToSend.fragmentNumber << ")");
            }
        }
        else
        {
            if(!group.add_data(*changeToSend.cacheChange, all_remote_readers_,
                        mAllShrinkedLocatorList, is_inline_qos_expected_))
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

bool StatelessWriter::matched_reader_add(RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    std::vector<LocatorList_t> allLocatorLists;
    bool addGuid = !has_builtin_guid();

    for(auto it = matched_readers_.begin(); it != matched_readers_.end(); ++it)
    {
        if((*it).guid == rdata.guid)
        {
            logWarning(RTPS_WRITER, "Attempting to add existing reader");
            return false;
        }

        LocatorList_t locators((*it).endpoint.unicastLocatorList);
        locators.push_back((*it).endpoint.multicastLocatorList);
        allLocatorLists.push_back(locators);
    }

    // Add info of new datareader.
    if(addGuid)
        all_remote_readers_.push_back(rdata.guid);
    LocatorList_t locators(rdata.endpoint.unicastLocatorList);
    locators.push_back(rdata.endpoint.multicastLocatorList);
    allLocatorLists.push_back(locators);

    update_cached_info_nts(allLocatorLists);

    matched_readers_.push_back(rdata);
    is_inline_qos_expected_ |= rdata.expectsInlineQos;

    update_locators_nts_(rdata.endpoint.durabilityKind >= TRANSIENT_LOCAL ? rdata.guid : c_Guid_Unknown);

    getRTPSParticipant()->createSenderResources(mAllShrinkedLocatorList, false);

    logInfo(RTPS_READER,"Reader " << rdata.guid << " added to "<<m_guid.entityId);
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

    for (auto input_locator : locator_list)
    {
        if (std::find(fixed_locators_.begin(), fixed_locators_.end(), input_locator) == fixed_locators_.end())
        {
            fixed_locators_.push_back(input_locator);
            mAllShrinkedLocatorList.push_back(input_locator);
        }
    }

    return true;
}

void StatelessWriter::update_locators_nts_(const GUID_t& optionalGuid)
{
    // Update mAllShrinkedLocatorList because at this point it was only updated
    // with locators of matched_readers_, and not the fixed locators.
    mAllShrinkedLocatorList.push_back(fixed_locators_);

    for(auto it = mAllShrinkedLocatorList.begin(); it != mAllShrinkedLocatorList.end(); ++it)
    {
        // Find guids
        for(const RemoteReaderAttributes& remoteReader : matched_readers_)
        {
            bool found = false;

            for(auto loc = remoteReader.endpoint.unicastLocatorList.begin(); loc != remoteReader.endpoint.unicastLocatorList.end(); ++loc)
            {
                if(*loc == *it ||
                        (mp_RTPSParticipant->network_factory().is_local_locator(*loc) &&
                         mp_RTPSParticipant->network_factory().is_local_locator(*it)))
                {
                    found = true;
                    break;
                }
            }

            for(auto loc = remoteReader.endpoint.multicastLocatorList.begin(); !found && loc != remoteReader.endpoint.multicastLocatorList.end(); ++loc)
                if(*loc == *it)
                {
                    found = true;
                    break;
                }

            if(found)
            {
                if(remoteReader.guid == optionalGuid)
                {
                    unsent_changes_.assign(mp_history->changesBegin(), mp_history->changesEnd());
                    AsyncWriterThread::wakeUp(this);
                }
            }
        }
    }
}

bool StatelessWriter::matched_reader_remove(const RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    bool found = matched_readers_.remove_if(rdata.compare_guid_function());
    if (found)
    {
        std::vector<LocatorList_t> allLocatorLists;
        bool addGuid = !has_builtin_guid();
        is_inline_qos_expected_ = false;

        for (auto rit = matched_readers_.begin(); rit != matched_readers_.end(); ++rit)
        {
            LocatorList_t locators((*rit).endpoint.unicastLocatorList);
            locators.push_back((*rit).endpoint.multicastLocatorList);
            allLocatorLists.push_back(locators);
            is_inline_qos_expected_ |= rit->expectsInlineQos;
        }

        if (addGuid) all_remote_readers_.remove(rdata.guid);
        update_cached_info_nts(allLocatorLists);

        update_locators_nts_(c_Guid_Unknown);
    }

    return found;
}

bool StatelessWriter::matched_reader_is_matched(const RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    return std::any_of(matched_readers_.begin(), matched_readers_.end(), rdata.compare_guid_function());
}

void StatelessWriter::unsent_changes_reset()
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    unsent_changes_.assign(mp_history->changesBegin(), mp_history->changesEnd());
    AsyncWriterThread::wakeUp(this);
}

void StatelessWriter::add_flow_controller(std::unique_ptr<FlowController> controller)
{
    m_controllers.push_back(std::move(controller));
}


} /* namespace rtps */
} /* namespace eprosima */
}
