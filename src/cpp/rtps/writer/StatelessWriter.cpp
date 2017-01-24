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
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include "../participant/RTPSParticipantImpl.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {


StatelessWriter::StatelessWriter(RTPSParticipantImpl* pimpl,GUID_t& guid,
        WriterAttributes& att,WriterHistory* hist,WriterListener* listen):
    RTPSWriter(pimpl,guid,att,hist,listen)
{}

StatelessWriter::~StatelessWriter()
{
    AsyncWriterThread::removeWriter(*this);
    logInfo(RTPS_WRITER,"StatelessWriter destructor";);
}

/*
 *	CHANGE-RELATED METHODS
 */

// TODO(Ricardo) This function only can be used by history. Private it and frined History.
// TODO(Ricardo) Look for other functions
void StatelessWriter::unsent_change_added_to_history(CacheChange_t* cptr)
{
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    // If payload protection, encode payload
    if(is_payload_protected())
    {
        getRTPSParticipant()->security_manager().encode_serialized_payload(cptr->serializedPayload,
                m_guid);
    }

    if(!isAsync())
    {
        this->setLivelinessAsserted(true);

        if(!reader_locator.empty()) //TODO change to m_reader_locator.
        {
            EntityId_t readerId;

            if(this->m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
                readerId = c_EntityId_SPDPReader;
            else if(this->m_guid.entityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER)
                readerId = participant_stateless_message_reader_entity_id;

            //TODO(Ricardo) Temporal.
            LocatorList_t locators(m_loc_list_1_for_sync_send);
            locators.push_back(m_loc_list_2_for_sync_send);

            //TODO(Ricardo) Temporal. This not works.
            std::vector<GuidPrefix_t> remote_participants{cptr->writerGUID.guidPrefix};
            std::vector<GUID_t> remote_readers{cptr->writerGUID};

            RTPSMessageGroup group(mp_RTPSParticipant, this,  m_cdrmessages);

            if(cptr->getFragmentSize() != 0)
            {
                for(uint32_t fragment = 1; fragment <= cptr->getFragmentCount(); ++fragment)
                {
                    group.add_data_frag(*cptr, fragment, c_GuidPrefix_Unknown, c_EntityId_Unknown,
                            locators, remote_participants, remote_readers, false);
                }
            }
            else
            {
                if(!group.add_data(*cptr, c_GuidPrefix_Unknown, c_EntityId_Unknown,
                            locators, remote_participants, remote_readers, false))
                {
                    logError(RTPS_WRITER, "Error sending change " << cptr->sequenceNumber);
                }
            }
        }
        else
        {
            logWarning(RTPS_WRITER, "No reader locator to send change");
        }
    }
    else
    {
        m_unsent_changes.push_back(cptr);
        AsyncWriterThread::wakeUp(this);
    }
}

bool StatelessWriter::change_removed_by_history(CacheChange_t* change)
{
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    auto newEnd = remove_if(m_unsent_changes.begin(),
            m_unsent_changes.end(),
            [change](const CacheChange_t* cptr)
            {
                return cptr == change ||
                    cptr->sequenceNumber == change->sequenceNumber;
            });
    bool removed = newEnd != m_unsent_changes.end();
    m_unsent_changes.erase(newEnd, m_unsent_changes.end());

    return removed;
}

void StatelessWriter::update_unsent_changes(const std::vector<const CacheChange_t*>& changes)
{
    for (const auto* change : changes)
    {
        auto it = std::find_if(m_unsent_changes.begin(),
                m_unsent_changes.end(),
                [&](const CacheChange_t* unsent_change)
                {
                    return change == unsent_change;
                });

        if (change->getFragmentSize() != 0)
        {
            // We remove the ones we are already sending.
            // TODO (Ricardo) Fixit
            /*
            it->setFragmentsClearedForSending(it->getFragmentsClearedForSending() - change.getFragmentsClearedForSending());
            if (it->getFragmentsClearedForSending().isSetEmpty())
                m_unsent_changes.erase(it);
                */
        }
        else
            m_unsent_changes.erase(it);
    }
}

void StatelessWriter::send_any_unsent_changes()
{
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    // Shallow copy the list
    auto changes_to_send = m_unsent_changes;

    // Clear through local controllers
    for (auto& controller : m_controllers)
        (*controller)(changes_to_send);

    // Clear through parent controllers
    for (auto& controller : mp_RTPSParticipant->getFlowControllers())
        (*controller)(changes_to_send);

    // Remove the messages selected for sending from the original list,
    // and update those that were fragmented with the new sent index
    update_unsent_changes(changes_to_send);

    if(!changes_to_send.empty())
    {
        if(m_pushMode)
        {
            RTPSMessageGroup group(mp_RTPSParticipant, this,  m_cdrmessages);

            //TODO(Ricardo) Temporal. This not works.
            std::vector<GuidPrefix_t> remote_participants;
            std::vector<GUID_t> remote_readers;
            for(const auto& change : changes_to_send)
            {
                remote_participants.push_back(change->writerGUID.guidPrefix);
                remote_readers.push_back(change->writerGUID);
            }

            for (const auto* change : changes_to_send)
            {
                // Notify the controllers
                FlowController::NotifyControllersChangeSent(change);

                EntityId_t readerId;

                if(this->m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
                    readerId = c_EntityId_SPDPReader;
                else if(this->m_guid.entityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER)
                    readerId = participant_stateless_message_reader_entity_id;

                // TODO(Ricardo) Temporal
                LocatorList_t locators(m_loc_list_1_for_sync_send);
                locators.push_back(m_loc_list_2_for_sync_send);

                if(change->getFragmentSize() != 0)
                {
                    for(uint32_t fragment = 1; fragment <= change->getFragmentCount(); ++fragment)
                    {
                        group.add_data_frag(*change, fragment, c_GuidPrefix_Unknown, readerId,
                                    locators, remote_participants, remote_readers, false);
                    }
                }
                else
                {
                    group.add_data(*change, c_GuidPrefix_Unknown, readerId,
                                locators, remote_participants, remote_readers, false);
                }
            }
        }
    }

    logInfo(RTPS_WRITER, "Finish sending unsent changes";);
}


/*
 *	MATCHED_READER-RELATED METHODS
 */

bool StatelessWriter::matched_reader_add(RemoteReaderAttributes& rdata)
{
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    if(rdata.guid != c_Guid_Unknown)
    {
        for(auto it=m_matched_readers.begin();it!=m_matched_readers.end();++it)
        {
            if((*it).guid == rdata.guid)
            {
                logWarning(RTPS_WRITER, "Attempting to add existing reader");
                return false;
            }
        }
    }
    bool send_any_unsent_changes = false;
    for(std::vector<Locator_t>::iterator lit = rdata.endpoint.unicastLocatorList.begin();
            lit!=rdata.endpoint.unicastLocatorList.end();++lit)
    {
        send_any_unsent_changes |= add_locator(rdata,*lit);
    }
    for(std::vector<Locator_t>::iterator lit = rdata.endpoint.multicastLocatorList.begin();
            lit!=rdata.endpoint.multicastLocatorList.end();++lit)
    {
        send_any_unsent_changes |= add_locator(rdata,*lit);
    }

    this->m_matched_readers.push_back(rdata);
    logInfo(RTPS_READER,"Reader " << rdata.guid << " added to "<<m_guid.entityId);
    return true;
}


bool StatelessWriter::add_locator(RemoteReaderAttributes& rdata,Locator_t& loc)
{
    logInfo(RTPS_WRITER, "Adding Locator: " << loc << " to StatelessWriter";);
    std::vector<ReaderLocator>::iterator rit;

    bool found = false;

    for(rit=reader_locator.begin();rit!=reader_locator.end();++rit)
    {
        if(rit->locator == loc)
        {
            rit->n_used++;
            found = true;
            break;
        }
    }

    if(!found)
    {
        ReaderLocator rl;
        rl.expectsInlineQos = rdata.expectsInlineQos;
        rl.locator = loc;
        reader_locator.push_back(rl);
        m_loc_list_1_for_sync_send.push_back(loc);
        rit = reader_locator.end()-1;
    }

    if(rdata.endpoint.durabilityKind >= TRANSIENT_LOCAL)
    {
        //TODO handle locators separately
        for(std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
                it!=mp_history->changesEnd();++it)
        {
            auto cit = std::find_if(m_unsent_changes.begin(),
                    m_unsent_changes.end(),
                    [&](const CacheChangeForGroup_t& unsent_change){ 
                    return *it == unsent_change.getChange();});

            if (cit == m_unsent_changes.end())
            {
                m_unsent_changes.emplace_back(*it);
                AsyncWriterThread::wakeUp(this);
            }
        }
    }

    return true;
}

bool StatelessWriter::matched_reader_remove(RemoteReaderAttributes& rdata)
{
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    bool found = false;
    if(rdata.guid == c_Guid_Unknown)
        found = true;
    else
    {
        for(auto rit = m_matched_readers.begin();
                rit!=m_matched_readers.end();++rit)
        {
            if((*rit).guid == rdata.guid)
            {
                found = true;
                m_matched_readers.erase(rit);
                break;
            }
        }
    }
    if(found)
    {
        logInfo(RTPS_WRITER, "Reader Proxy removed: " << rdata.guid;);
        for(std::vector<Locator_t>::iterator lit = rdata.endpoint.unicastLocatorList.begin();
                lit!=rdata.endpoint.unicastLocatorList.end();++lit)
        {
            remove_locator(*lit);
        }
        for(std::vector<Locator_t>::iterator lit = rdata.endpoint.multicastLocatorList.begin();
                lit!=rdata.endpoint.multicastLocatorList.end();++lit)
        {
            remove_locator(*lit);
        }
        return true;
    }
    return false;
}

bool StatelessWriter::matched_reader_is_matched(RemoteReaderAttributes& rdata)
{
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
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

bool StatelessWriter::remove_locator(Locator_t& loc)
{
    for(auto rit=reader_locator.begin();rit!=reader_locator.end();++rit)
    {
        if(rit->locator == loc)
        {
            rit->n_used--;
            if(rit->n_used == 0)
            {
                reader_locator.erase(rit);
                m_loc_list_1_for_sync_send.erase(loc);
            }
            break;
        }
    }
    return true;
}

void StatelessWriter::unsent_changes_reset()
{
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    m_unsent_changes.clear();

    for(std::vector<CacheChange_t*>::iterator cit = mp_history->changesBegin();
            cit!=mp_history->changesEnd();++cit)
    {
        m_unsent_changes.emplace_back(*cit);    
    }

    AsyncWriterThread::wakeUp(this);
}

void StatelessWriter::add_flow_controller(std::unique_ptr<FlowController> controller)
{
    m_controllers.push_back(std::move(controller));
}


} /* namespace rtps */
} /* namespace eprosima */
}
