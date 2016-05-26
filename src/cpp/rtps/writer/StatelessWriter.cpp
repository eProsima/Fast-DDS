/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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

#include <fastrtps/utils/RTPSLog.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "StatelessWriter";

StatelessWriter::StatelessWriter(RTPSParticipantImpl* pimpl,GUID_t& guid,
		WriterAttributes& att,WriterHistory* hist,WriterListener* listen):
							RTPSWriter(pimpl,guid,att,hist,listen)
{

}

StatelessWriter::~StatelessWriter()
{
   AsyncWriterThread::removeWriter(*this);
	const char* const METHOD_NAME = "~StatelessWriter";
	logInfo(RTPS_WRITER,"StatelessWriter destructor";);
}

/*
 *	CHANGE-RELATED METHODS
 */

void StatelessWriter::unsent_change_added_to_history(CacheChange_t* cptr)
{
	const char* const METHOD_NAME = "unsent_change_added_to_history";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    if(!isAsync())
    {
        std::vector<CacheChangeForGroup_t> changes_to_send;
        changes_to_send.push_back(CacheChangeForGroup_t(cptr));
        LocatorList_t locList;
        LocatorList_t locList2;
        this->setLivelinessAsserted(true);

        if(!reader_locator.empty()) //TODO change to m_reader_locator.
        {
            for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
            {
                locList.push_back(rit->locator);
            }

            uint32_t bytesSent = RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages, (RTPSWriter*)this,
                    changes_to_send, c_GuidPrefix_Unknown,
                    this->m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER ? c_EntityId_SPDPReader : c_EntityId_Unknown,
                    locList, locList2, false);

            // Check send
            if(bytesSent == 0 || changes_to_send.size() > 0)
                logError(RTPS_WRITER, "Error sending change " << cptr->sequenceNumber);
        }
        else
        {
            logWarning(RTPS_WRITER, "No reader locator to send change");
        }
    }
    else
    {
        for(auto rit = reader_locator.begin(); rit != reader_locator.end(); ++rit)
        {
            rit->add_unsent_change(CacheChangeForGroup_t(cptr));
            AsyncWriterThread::wakeUp(this);
        }
    }
}

bool StatelessWriter::change_removed_by_history(CacheChange_t* change)
{
    bool returnedValue = false;

    // If the writer is asynchronous, find change in unsent list and remove it.
	 boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    for(auto rit = reader_locator.begin(); rit != reader_locator.end(); ++rit)
    {
        rit->remove_unsent_change(change);
    }

    return returnedValue;
}

uint32_t StatelessWriter::send_any_unsent_changes()
{
	const char* const METHOD_NAME = "send_any_unsent_changes";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
   uint32_t messagesToSend = 0;

	for(auto rit = reader_locator.begin(); rit != reader_locator.end(); ++rit)
	{
      // Shallow copy the list
      auto unsent_changes_copy = rit->get_unsent_changes_copy(); 

      // Clear through local filters
      for (auto& filter : m_filters)
          (*filter)(unsent_changes_copy);

      // Clear through parent filters
      for (auto& filter : mp_RTPSParticipant->getFlowFilters())
          (*filter)(unsent_changes_copy); 

      // Remove the messages selected for sending from the original list
      for (auto& change : unsent_changes_copy)
         rit->remove_unsent_change(change.getChange());

      messagesToSend += unsent_changes_copy.size();
		if(messagesToSend)
		{
			if(m_pushMode)
			{
             uint32_t bytesSent = 0;
             do
             {
                 bytesSent = RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages, (RTPSWriter*)this,
                         unsent_changes_copy, c_GuidPrefix_Unknown,
                         this->m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER ? c_EntityId_SPDPReader : c_EntityId_Unknown, rit->locator, rit->expectsInlineQos);
             } while(bytesSent > 0 && unsent_changes_copy.size() > 0);
			}
		}
	}
	logInfo(RTPS_WRITER, "Finish sending unsent changes";);
   return messagesToSend;
}


/*
 *	MATCHED_READER-RELATED METHODS
 */

bool StatelessWriter::matched_reader_add(RemoteReaderAttributes& rdata)
{
	const char* const METHOD_NAME = "matched_reader_add";
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
	const char* const METHOD_NAME = "add_locator";
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
		rit = reader_locator.end()-1;
	}

	if(rdata.endpoint.durabilityKind >= TRANSIENT_LOCAL)
	{
		//TODO check if the history needs to be protected,
		//Cuando un hilo añade un remote reader y otro hilo está añadiendo cambios a la historia.
		for(std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
				it!=mp_history->changesEnd();++it)
		{
         rit->add_unsent_change(CacheChangeForGroup_t((*it)));
         AsyncWriterThread::wakeUp(this);
		}
	}

	if(rit->count_unsent_changes())
		return true;

	return false;
}

bool StatelessWriter::matched_reader_remove(RemoteReaderAttributes& rdata)
{
	const char* const METHOD_NAME = "matched_reader_remove";
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
			}
			break;
		}
	}
	return true;
}

void StatelessWriter::unsent_changes_reset()
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
    {
		rit->remove_all_unsent_changes();

		for(std::vector<CacheChange_t*>::iterator cit=mp_history->changesBegin();
				cit!=mp_history->changesEnd();++cit)
      {
         rit->add_unsent_change(CacheChangeForGroup_t(*cit));    
         AsyncWriterThread::wakeUp(this);
		}
	}
	send_any_unsent_changes();
}

bool StatelessWriter::clean_history(unsigned int max)
{
    const char* const METHOD_NAME = "clean_history";
    logInfo(RTPS_WRITER, "Starting process clean_history for writer " << getGuid());
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    bool limit = (max != 0);

    bool remove_ret = mp_history->remove_min_change();
    bool at_least_one = remove_ret;
    unsigned int count = 1;

    while(remove_ret && (!limit || count < max))
    {
        remove_ret = mp_history->remove_min_change();
        ++count;
    }

    return at_least_one;
}

void StatelessWriter::add_flow_filter(std::unique_ptr<FlowFilter> filter)
{
   m_filters.push_back(std::move(filter));
}


} /* namespace rtps */
} /* namespace eprosima */


}
