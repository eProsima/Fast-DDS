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
#include "../flowcontrol/FlowController.h"

#include <mutex>

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

std::vector<GUID_t> StatelessWriter::get_remote_readers()
{
    std::vector<GUID_t> remote_readers;

    if(this->m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
        remote_readers.emplace_back(GuidPrefix_t(), c_EntityId_SPDPReader);
#if HAVE_SECURITY
    else if(this->m_guid.entityId == ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER)
        remote_readers.emplace_back(GuidPrefix_t(), participant_stateless_message_reader_entity_id);
#endif
    else
        for(auto& reader : m_matched_readers)
            remote_readers.push_back(reader.guid);

    return remote_readers;
}

/*
 *	CHANGE-RELATED METHODS
 */

// TODO(Ricardo) This function only can be used by history. Private it and frined History.
// TODO(Ricardo) Look for other functions
void StatelessWriter::unsent_change_added_to_history(CacheChange_t* cptr)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    if(!isAsync())
    {
        this->setLivelinessAsserted(true);


        // TODO(Ricardo) ReaderLocators should store remote reader GUIDs
        std::vector<GUID_t> remote_readers = get_remote_readers();

        for(auto& reader_locator : reader_locators)
        {
            //TODO(Ricardo) Temporal.
            LocatorList_t locators;
            locators.push_back(reader_locator.locator);

            RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);

            if(!group.add_data(*cptr, remote_readers, locators, false))
            {
                logError(RTPS_WRITER, "Error sending change " << cptr->sequenceNumber);
            }
        }
    }
    else
    {
        for(auto& reader_locator : reader_locators)
            reader_locator.unsent_changes.push_back(ChangeForReader_t(cptr));
        AsyncWriterThread::wakeUp(this);
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

void StatelessWriter::update_unsent_changes(ReaderLocator& reader_locator,
        const std::vector<CacheChange_t*>& changes)
{
    //TODO(Ricardo)
    //for (const auto* change : changes)
    for (auto* change : changes)
    {
        auto it = std::find_if(reader_locator.unsent_changes.begin(),
                reader_locator.unsent_changes.end(),
                [change](const ChangeForReader_t& unsent_change)
                {
                    return change == unsent_change.getChange();
                });

        if (change->getFragmentSize() != 0)
        {
            FragmentNumberSet_t fragment_sns = it->getUnsentFragments();
            // We remove the ones we are already sending.
            auto frag_sn_it = fragment_sns.set.begin();
            while(frag_sn_it != fragment_sns.set.end())
            {
                if(change->getDataFragments()->at(*frag_sn_it - 1) == PRESENT)
                {
                    it->markFragmentsAsSent(*frag_sn_it++);
                }
                else
                    break;
            }

            if (frag_sn_it == fragment_sns.set.end())
                reader_locator.unsent_changes.erase(it);
        }
        else
            reader_locator.unsent_changes.erase(it);
    }
}

void StatelessWriter::send_any_unsent_changes()
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);

    std::vector<GUID_t> remote_readers = get_remote_readers();

    for(auto& reader_locator : reader_locators)
    {
        // Shallow copy the list
        std::vector<CacheChange_t*> changes_to_send;

        for(auto cit = reader_locator.unsent_changes.begin() ; cit != reader_locator.unsent_changes.end(); ++cit)
        {
            changes_to_send.push_back(cit->getChange());

            if(cit->getChange()->getFragmentSize() > 0)
            {
                cit->getChange()->getDataFragments()->assign(cit->getChange()->getDataFragments()->size(),
                        NOT_PRESENT);
                FragmentNumberSet_t frag_sns = cit->getUnsentFragments();

                for(auto sn = frag_sns.get_begin(); sn != frag_sns.get_end(); ++sn)
                {
                    assert(*sn <= cit->getChange()->getDataFragments()->size());
                    cit->getChange()->getDataFragments()->at(*sn - 1) = PRESENT;
                }
            }
        }

        // Clear through local controllers
        for (auto& controller : m_controllers)
            (*controller)(changes_to_send);

        // Clear through parent controllers
        for (auto& controller : mp_RTPSParticipant->getFlowControllers())
            (*controller)(changes_to_send);

        // Remove the messages selected for sending from the original list,
        // and update those that were fragmented with the new sent index
        update_unsent_changes(reader_locator, changes_to_send);

        if(!changes_to_send.empty())
        {
            if(m_pushMode)
            {
                RTPSMessageGroup group(mp_RTPSParticipant, this, RTPSMessageGroup::WRITER, m_cdrmessages);


                for (auto* change : changes_to_send)
                {
                    // Notify the controllers
                    FlowController::NotifyControllersChangeSent(change);

                    // TODO(Ricardo) Temporal
                    LocatorList_t locators;
                    locators.push_back(reader_locator.locator);

                    if(change->getFragmentSize() != 0)
                    {
                        for(uint32_t fragment  = 0; fragment < change->getDataFragments()->size(); ++fragment)
                        {
                            if(change->getDataFragments()->at(fragment) == PRESENT)
                            {
                                //TODO(Ricardo) Frag = 0
                                if(!group.add_data_frag(*change, fragment + 1, remote_readers, locators, false))
                                {
                                    logError(RTPS_WRITER, "Error sending fragment (" << change->sequenceNumber <<
                                            ", " << fragment + 1 << ")");
                                }
                            }
                        }
                    }
                    else
                    {
                        if(!group.add_data(*change, remote_readers, locators, false))
                        {
                            logError(RTPS_WRITER, "Error sending change " << change->sequenceNumber);
                        }
                    }
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
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
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

    auto rit = std::find_if(reader_locators.rbegin(), reader_locators.rend(),
            [loc](const ReaderLocator& reader_locator) {
                if(reader_locator.locator == loc)
                   return true;

                return false;
            });

    if(rit != reader_locators.rend())
    {
        ++rit->n_used;
    }
    else
    {
        ReaderLocator rl;
        rl.expectsInlineQos = rdata.expectsInlineQos;
        rl.locator = loc;
        reader_locators.push_back(rl);
        rit = reader_locators.rbegin();
    }

    if(rdata.endpoint.durabilityKind >= TRANSIENT_LOCAL)
    {
        rit->unsent_changes.assign(mp_history->changesBegin(), mp_history->changesEnd());
        AsyncWriterThread::wakeUp(this);
    }

    return true;
}

bool StatelessWriter::matched_reader_remove(RemoteReaderAttributes& rdata)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_mutex);
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

bool StatelessWriter::remove_locator(Locator_t& loc)
{
    for(auto rit = reader_locators.begin(); rit != reader_locators.end(); ++rit)
    {
        if(rit->locator == loc)
        {
            rit->n_used--;
            if(rit->n_used == 0)
            {
                reader_locators.erase(rit);
            }
            break;
        }
    }

    return true;
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
