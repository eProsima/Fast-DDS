/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file StatelessWriter.cpp
 *
 */

#include "fastrtps/rtps/writer/StatelessWriter.h"
#include "fastrtps/rtps/history/WriterHistory.h"
#include "fastrtps/rtps/writer/timedevent/UnsentChangesNotEmptyEvent.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "fastrtps/utils/RTPSLog.h"

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
	const char* const METHOD_NAME = "~StatelessWriter";
	logInfo(RTPS_WRITER,"StatelessWriter destructor";);
}

/*
 *	CHANGE-RELATED METHODS
 */

void StatelessWriter::unsent_change_added_to_history(CacheChange_t* cptr)
{
	const char* const METHOD_NAME = "unsent_change_add";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(!reader_locator.empty())
	{
		for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
		{
			rit->unsent_changes.push_back(cptr);

			if(this->m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
			{
				RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)this,
						&rit->unsent_changes,rit->locator,rit->expectsInlineQos,c_EntityId_SPDPReader);
			}
			else
			{
				RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)this,
						&rit->unsent_changes,rit->locator,rit->expectsInlineQos,c_EntityId_Unknown);
			}
			rit->unsent_changes.clear();
		}
	}
	else
	{
		logWarning(RTPS_WRITER, "No reader locator to send change");
	}
}

bool StatelessWriter::change_removed_by_history(CacheChange_t* a_change)
{
	return true;
}

void StatelessWriter::unsent_changes_not_empty()
{
	const char* const METHOD_NAME = "unsent_changes_not_empty";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
	{
		if(!rit->unsent_changes.empty())
		{
			if(m_pushMode)
			{
				if(this->m_guid.entityId == ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER)
				{
					RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)this,
							&rit->unsent_changes,rit->locator,rit->expectsInlineQos,c_EntityId_SPDPReader);
				}
				else
				{
					RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)this,
							&rit->unsent_changes,rit->locator,rit->expectsInlineQos,c_EntityId_Unknown);
				}
				rit->unsent_changes.clear();
			}
			//			else
			//			{
			//				SequenceNumber_t first,last;
			//				m_writer_cache.get_seq_num_min(&first,NULL);
			//				m_writer_cache.get_seq_num_max(&last,NULL);
			//				m_heartbeatCount++;
			//				CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_fullmsg);
			//				RTPSMessageCreator::addMessageHeartbeat(&m_cdrmessages.m_rtpsmsg_fullmsg,m_guid.guidPrefix,
			//						ENTITYID_UNKNOWN,m_guid.entityId,first,last,m_heartbeatCount,true,false);
			//				mp_send_thr->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,&rit->locator);
			//				rit->unsent_changes.clear();
			//			}
		}
	}
	logInfo(RTPS_WRITER, "Finish sending unsent changes";);
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
	bool unsent_changes_not_empty = false;
	for(std::vector<Locator_t>::iterator lit = rdata.endpoint.unicastLocatorList.begin();
			lit!=rdata.endpoint.unicastLocatorList.end();++lit)
	{
		unsent_changes_not_empty |= add_locator(rdata,*lit);
	}
	for(std::vector<Locator_t>::iterator lit = rdata.endpoint.multicastLocatorList.begin();
			lit!=rdata.endpoint.multicastLocatorList.end();++lit)
	{
		unsent_changes_not_empty |= add_locator(rdata,*lit);
	}
	if(unsent_changes_not_empty)
	{
		//unsent_changes_not_empty();
		this->mp_unsetChangesNotEmpty = new UnsentChangesNotEmptyEvent(this,1.0);
		this->mp_unsetChangesNotEmpty->restart_timer();
		this->mp_unsetChangesNotEmpty = NULL;
	}
	this->m_matched_readers.push_back(rdata);
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
		for(std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
				it!=mp_history->changesEnd();++it)
		{
			rit->unsent_changes.push_back((*it));
		}
	}
	if(!rit->unsent_changes.empty())
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


//bool StatelessWriter::reader_locator_add(ReaderLocator& a_locator)
//{
//	boost::lock_guard<Endpoint> guard(*this);
//
//	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit){
//
//		if(rit->locator == a_locator.locator)
//		{
//			pWarning("Reader Locator: " << a_locator.locator.printIP4Port() <<" already in list"<<endl);
//			return false;
//		}
//	}
//	a_locator.requested_changes.clear();
//	a_locator.unsent_changes.clear();
//	if(a_locator.m_durabilityKind == TRANSIENT_LOCAL_DURABILITY_QOS)
//	{
//		for(std::vector<CacheChange_t*>::iterator it = m_writer_cache.changesBegin();
//				it!=m_writer_cache.changesEnd();++it)
//		{
//			a_locator.unsent_changes.push_back((*it));
//		}
//	}
//	reader_locator.push_back(a_locator);
//	pDebugInfo("Adding new Reader Locator to StatelessWriter: "<< a_locator.locator.printIP4Port()<<endl);
//	if(!a_locator.unsent_changes.empty())
//		this->unsent_changes_not_empty();
//	return true;
//}
//
//bool StatelessWriter::reader_locator_add(Locator_t& loc,bool expectsInlineQos)
//{
//	bool found = false;
//	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
//	{
//		if(rit->locator == loc)
//		{
//			rit->n_used++;
//			found = true;
//			break;
//		}
//	}
//	if(!found)
//	{
//		ReaderLocator rl;
//		rl.expectsInlineQos = expectsInlineQos;
//		rl.locator = loc;
//		reader_locator.push_back(rl);
//	}
//	return true;
//}


void StatelessWriter::unsent_changes_reset()
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit){
		rit->unsent_changes.clear();
		for(std::vector<CacheChange_t*>::iterator cit=mp_history->changesBegin();
				cit!=mp_history->changesEnd();++cit){
			rit->unsent_changes.push_back((*cit));
		}
	}
	unsent_changes_not_empty();
}
//
//bool sort_cacheChanges (CacheChange_t* c1,CacheChange_t* c2)
//{
//	return(c1->sequenceNumber.to64long() < c2->sequenceNumber.to64long());
//}






//bool StatelessWriter::removeMinSeqCacheChange()
//{
//	return mp_history->remove_min_change();
//}
//
//bool StatelessWriter::removeAllCacheChange(size_t* n_removed)
//{
//	size_t n_r=this->mp_history->getHistorySize();
//	if(this->mp_history->remove_all_changes())
//	{
//		*n_removed = n_r;
//		return true;
//	}
//	else
//		return false;
//}



} /* namespace rtps */
} /* namespace eprosima */


}
