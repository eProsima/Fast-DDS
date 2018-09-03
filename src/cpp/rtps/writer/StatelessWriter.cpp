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

#include <mutex>
#include <vector>
#include <set>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {


StatelessWriter::StatelessWriter(RTPSParticipantImpl* pimpl,GUID_t& guid,
        WriterAttributes& att,WriterHistory* hist,WriterListener* listen):
    RTPSWriter(pimpl,guid,att,hist,listen)
{
    mAllRemoteReaders = get_builtin_guid();
}

StatelessWriter::~StatelessWriter()
{
    AsyncWriterThread::removeWriter(*this);
    logInfo(RTPS_WRITER,"StatelessWriter destructor";);
}

std::vector<GUID_t> StatelessWriter::get_builtin_guid()
{
    if(this->m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
        return {{GuidPrefix_t(), c_EntityId_SPDPReader}};
#if HAVE_SECURITY
    else if(this->m_guid.entityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER)
        return {{GuidPrefix_t(), participant_stateless_message_reader_entity_id}};
#endif

    return {};
}

/*
 *	CHANGE-RELATED METHODS
 */

// TODO(Ricardo) This function only can be used by history. Private it and frined History.
// TODO(Ricardo) Look for other functions
void StatelessWriter::unsent_change_added_to_history(CacheChange_t* cptr)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    if (!reader_locators.empty())
    {
#if HAVE_SECURITY
        encrypt_cachechange(cptr);
#endif

        if (!isAsync())
        {
            this->setLivelinessAsserted(true);

            RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);

            if (!group.add_data(*cptr, mAllRemoteReaders, mAllShrinkedLocatorList, false))
            {
                logError(RTPS_WRITER, "Error sending change " << cptr->sequenceNumber);
            }

            if (mp_listener != nullptr)
            {
                mp_listener->onWriterChangeReceivedByAll(this, cptr);
            }
        }
        else
        {
            for (auto& reader_locator : reader_locators)
                reader_locator.unsent_changes.push_back(ChangeForReader_t(cptr));
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

    for(auto& reader_locator : reader_locators)
        reader_locator.unsent_changes.erase(std::remove_if(
                    reader_locator.unsent_changes.begin(),
                    reader_locator.unsent_changes.end(),
                    [change](ChangeForReader_t& cptr)
                    {
                        return cptr.getChange() == change ||
                            cptr.getChange()->sequenceNumber == change->sequenceNumber;
                    }),
                    reader_locator.unsent_changes.end());

    return true;
}

bool StatelessWriter::is_acked_by_all(const CacheChange_t* change) const
{
    // Only asynchronous writers may have unacked (i.e. unsent changes)
    if (isAsync())
    {
        std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

        // Return false if change is pending to be sent
        for (auto& reader_locator : reader_locators)
        {
            auto it = std::find_if(reader_locator.unsent_changes.begin(),
                reader_locator.unsent_changes.end(),
                [change](const ChangeForReader_t& unsent_change)
            {
                return change == unsent_change.getChange();
            });

            if (it != reader_locator.unsent_changes.end())
            {
                return false;
            }
        }
    }

    return true;
}

void StatelessWriter::update_unsent_changes(ReaderLocator& reader_locator,
        const SequenceNumber_t& seqNum, const FragmentNumber_t fragNum)
{
    auto it = std::find_if(reader_locator.unsent_changes.begin(),
            reader_locator.unsent_changes.end(),
            [seqNum](const ChangeForReader_t& unsent_change)
            {
                return seqNum == unsent_change.getSequenceNumber();
            });

    if(it != reader_locator.unsent_changes.end())
    {
        if (fragNum != 0)
        {
            it->markFragmentsAsSent(fragNum);
            FragmentNumberSet_t fragment_sns = it->getUnsentFragments();
            if(fragment_sns.isSetEmpty())
                reader_locator.unsent_changes.erase(it);
        }
        else
            reader_locator.unsent_changes.erase(it);
    }
}

void StatelessWriter::send_any_unsent_changes()
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    RTPSWriterCollector<ReaderLocator*> changesToSend;

    for(auto& reader_locator : reader_locators)
    {
        for(auto unsentChange : reader_locator.unsent_changes)
        {
            changesToSend.add_change(unsentChange.getChange(), &reader_locator, unsentChange.getUnsentFragments());
        }
    }

    // Clear through local controllers
    for (auto& controller : m_controllers)
        (*controller)(changesToSend);

    // Clear through parent controllers
    for (auto& controller : mp_RTPSParticipant->getFlowControllers())
        (*controller)(changesToSend);

    RTPSMessageGroup group(mp_RTPSParticipant, this,  RTPSMessageGroup::WRITER, m_cdrmessages);
    bool bHasListener = mp_listener != nullptr;

    while(!changesToSend.empty())
    {
        RTPSWriterCollector<ReaderLocator*>::Item changeToSend = changesToSend.pop();
        std::vector<GUID_t> remote_readers = get_builtin_guid();
        std::set<GUID_t> remote_readers_aux;
        LocatorList_t locatorList;
        bool expectsInlineQos = false, addGuid = remote_readers.empty();

        for(auto* readerLocator : changeToSend.remoteReaders)
        {
            // Remove the messages selected for sending from the original list,
            // and update those that were fragmented with the new sent index
            update_unsent_changes(*readerLocator, changeToSend.sequenceNumber, changeToSend.fragmentNumber);

            if(addGuid)
                remote_readers_aux.insert(readerLocator->remote_guids.begin(), readerLocator->remote_guids.end());
            locatorList.push_back(readerLocator->locator);
            expectsInlineQos |= readerLocator->expectsInlineQos;
        }

        if(addGuid)
            remote_readers.assign(remote_readers_aux.begin(), remote_readers_aux.end());

        // Notify the controllers
        FlowController::NotifyControllersChangeSent(changeToSend.cacheChange);

        if(changeToSend.fragmentNumber != 0)
        {
            if(!group.add_data_frag(*changeToSend.cacheChange, changeToSend.fragmentNumber, remote_readers,
                        locatorList, expectsInlineQos))
            {
                logError(RTPS_WRITER, "Error sending fragment (" << changeToSend.sequenceNumber <<
                        ", " << changeToSend.fragmentNumber << ")");
            }
        }
        else
        {
            if(!group.add_data(*changeToSend.cacheChange, remote_readers,
                        locatorList, expectsInlineQos))
            {
                logError(RTPS_WRITER, "Error sending change " << changeToSend.sequenceNumber);
            }
        }

        if (bHasListener && this->is_acked_by_all(changeToSend.cacheChange))
        {
            mp_listener->onWriterChangeReceivedByAll(this, changeToSend.cacheChange);
        }
    }

    logInfo(RTPS_WRITER, "Finish sending unsent changes";);
}


/*
 *	MATCHED_READER-RELATED METHODS
 */

bool StatelessWriter::matched_reader_add(const RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    std::vector<GUID_t> allRemoteReaders = get_builtin_guid();
    std::vector<LocatorList_t> allLocatorLists;
    bool addGuid = allRemoteReaders.empty();

    for(auto it = m_matched_readers.begin(); it != m_matched_readers.end(); ++it)
    {
        if((*it).guid == rdata.guid)
        {
            logWarning(RTPS_WRITER, "Attempting to add existing reader");
            return false;
        }

        if(addGuid)
            allRemoteReaders.push_back((*it).guid);
        LocatorList_t locators((*it).endpoint.unicastLocatorList);
        locators.push_back((*it).endpoint.multicastLocatorList);
        allLocatorLists.push_back(locators);
    }

    // Add info of new datareader.
    if(addGuid)
        allRemoteReaders.push_back(rdata.guid);
    LocatorList_t locators(rdata.endpoint.unicastLocatorList);
    locators.push_back(rdata.endpoint.multicastLocatorList);
    allLocatorLists.push_back(locators);

    update_cached_info_nts(std::move(allRemoteReaders), allLocatorLists);

    this->m_matched_readers.push_back(rdata);

    update_locators_nts_(rdata.endpoint.durabilityKind >= TRANSIENT_LOCAL ? rdata.guid : c_Guid_Unknown);

    logInfo(RTPS_READER,"Reader " << rdata.guid << " added to "<<m_guid.entityId);
    return true;
}

bool StatelessWriter::add_locator(Locator_t& loc)
{
#if HAVE_SECURITY
    if(!getAttributes()->security_attributes().is_submessage_protected &&
            !getAttributes()->security_attributes().is_payload_protected)
#endif
    {
        std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

        for(auto readerLocator : fixed_locators)
            if(readerLocator.locator == loc)
            {
                logInfo(RTPS_WRITER, "Already registered locator");
                return false;
            }

        ReaderLocator newLoc;
        newLoc.locator = loc;
        fixed_locators.push_back(newLoc);

        mAllShrinkedLocatorList.push_back(loc);

        for(auto readerLocator : reader_locators)
            if(readerLocator.locator == newLoc.locator)
                return true;

        reader_locators.push_back(newLoc);
        return true;
    }
#if HAVE_SECURITY
    else
    {
        logError(RTPS_WRITER, "A secure besteffort writer cannot add a lonely locator");
        return false;
    }
#endif
}

void StatelessWriter::update_locators_nts_(const GUID_t& optionalGuid)
{
    std::vector<ReaderLocator> backup(std::move(reader_locators));

    // Update mAllShrinkedLocatorList because at this point it was only updated
    // with locators of reader_locators, and not the fixed locators.
    for(auto fixedLocator : fixed_locators)
            mAllShrinkedLocatorList.push_back(fixedLocator.locator);

    for(auto it = mAllShrinkedLocatorList.begin(); it != mAllShrinkedLocatorList.end(); ++it)
    {
        auto readerLocator = std::find_if(backup.begin(), backup.end(),
                [it](const ReaderLocator& reader_locator) {
                    if(reader_locator.locator == *it)
                        return true;

                    return false;
                });

        if(readerLocator != backup.end())
        {
            reader_locators.push_back(std::move(*readerLocator));
            //backup.erase(readerLocator);
        }
        else
        {
            logInfo(RTPS_WRITER, "Adding Locator: " << *it << " to StatelessWriter";);

            ReaderLocator rl;
            rl.locator = *it;
            reader_locators.push_back(std::move(rl));
        }

        reader_locators.back().remote_guids.clear();
        reader_locators.back().expectsInlineQos = false;

        // Find guids
        for(auto remoteReader = m_matched_readers.begin(); remoteReader != m_matched_readers.end(); ++remoteReader)
        {
            bool found = false;

            for(auto loc = remoteReader->endpoint.unicastLocatorList.begin(); loc != remoteReader->endpoint.unicastLocatorList.end(); ++loc)
            {
                if(*loc == reader_locators.back().locator ||
                        (mp_RTPSParticipant->network_factory().is_local_locator(*loc) &&
                         mp_RTPSParticipant->network_factory().is_local_locator(reader_locators.back().locator)))
                {
                    found = true;
                    break;
                }
            }

            for(auto loc = remoteReader->endpoint.multicastLocatorList.begin(); !found && loc != remoteReader->endpoint.multicastLocatorList.end(); ++loc)
                if(*loc == reader_locators.back().locator)
                {
                    found = true;
                    break;
                }

            if(found)
            {
                reader_locators.back().remote_guids.push_back(remoteReader->guid);
                reader_locators.back().expectsInlineQos |= remoteReader->expectsInlineQos;

                if(remoteReader->guid == optionalGuid)
                {
                    reader_locators.back().unsent_changes.assign(mp_history->changesBegin(), mp_history->changesEnd());
                    AsyncWriterThread::wakeUp(this);
                }
            }
        }
    }
}

bool StatelessWriter::matched_reader_remove(const RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    std::vector<GUID_t> allRemoteReaders = get_builtin_guid();
    std::vector<LocatorList_t> allLocatorLists;
    bool found = false, addGuid = allRemoteReaders.empty();

    auto rit = m_matched_readers.begin();
    while(rit!=m_matched_readers.end())
    {
        if((*rit).guid == rdata.guid)
        {
            rit = m_matched_readers.erase(rit);
            found = true;
            continue;
        }

        if(addGuid)
            allRemoteReaders.push_back((*rit).guid);
        LocatorList_t locators((*rit).endpoint.unicastLocatorList);
        locators.push_back((*rit).endpoint.multicastLocatorList);
        allLocatorLists.push_back(locators);
        ++rit;
    }

    update_cached_info_nts(std::move(allRemoteReaders), allLocatorLists);

    update_locators_nts_(c_Guid_Unknown);

    return found;
}

bool StatelessWriter::matched_reader_is_matched(const RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
    for(auto rit = m_matched_readers.begin();
            rit!=m_matched_readers.end();++rit)
    {
        if((*rit).guid == rdata.guid)
        {
            return true;
        }
    }
    return false;
}

void StatelessWriter::unsent_changes_reset()
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    for(auto& reader_locator : reader_locators)
        reader_locator.unsent_changes.assign(mp_history->changesBegin(),
                mp_history->changesEnd());

    AsyncWriterThread::wakeUp(this);
}

void StatelessWriter::add_flow_controller(std::unique_ptr<FlowController> controller)
{
    m_controllers.push_back(std::move(controller));
}


} /* namespace rtps */
} /* namespace eprosima */
}
