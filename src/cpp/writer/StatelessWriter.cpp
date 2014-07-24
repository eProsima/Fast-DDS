/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file StatelessWriter.cpp
 *
 */

#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/writer/ReaderProxyData.h"
#include "eprosimartps/utils/RTPSLog.h"

//#include "eprosimartps/qos/ParameterList.h"

namespace eprosima {
namespace rtps {



StatelessWriter::StatelessWriter(const PublisherAttributes& param,const GuidPrefix_t&guidP, const EntityId_t& entId,DDSTopicDataType* ptype):
		RTPSWriter(guidP,entId,param,ptype,STATELESS,param.userDefinedId,param.payloadMaxSize)
{
	m_pushMode = true;//TODOG, support pushmode false in best effort
	//locator lists:
	unicastLocatorList = param.unicastLocatorList;
	multicastLocatorList = param.multicastLocatorList;
}




StatelessWriter::~StatelessWriter()
{
	pDebugInfo("StatelessWriter destructor"<<endl;);
}

bool StatelessWriter::matched_reader_add(ReaderProxyData* rdata)
{
	boost::lock_guard<Endpoint> guard(*this);
	bool unsent_changes_not_empty = false;
	for(std::vector<Locator_t>::iterator lit = rdata->m_unicastLocatorList.begin();
			lit!=rdata->m_unicastLocatorList.end();++lit)
	{
		unsent_changes_not_empty |= add_locator(rdata,*lit);
	}
	for(std::vector<Locator_t>::iterator lit = rdata->m_multicastLocatorList.begin();
			lit!=rdata->m_multicastLocatorList.end();++lit)
	{
		unsent_changes_not_empty |= add_locator(rdata,*lit);
	}
	if(unsent_changes_not_empty)
		this->unsent_changes_not_empty();

	this->m_matched_readers.push_back(rdata);
	return true;
}


bool StatelessWriter::add_locator(ReaderProxyData* rdata,Locator_t& loc)
{
	pDebugInfo("Adding Locator: "<< loc.printIP4Port()<< " to StatelessWriter"<<endl;);
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
		rl.expectsInlineQos = rdata->m_expectsInlineQos;
		rl.locator = loc;
		reader_locator.push_back(rl);
		rit = reader_locator.end()-1;
	}
	if(rdata->m_qos.m_durability.kind >= TRANSIENT_LOCAL_DURABILITY_QOS)
	{
		for(std::vector<CacheChange_t*>::iterator it = m_writer_cache.changesBegin();
				it!=m_writer_cache.changesEnd();++it)
		{
			rit->unsent_changes.push_back((*it));
		}
	}
	if(!rit->unsent_changes.empty())
		return true;
	return false;
}

bool StatelessWriter::matched_reader_remove(ReaderProxyData* rdata)
{
	boost::lock_guard<Endpoint> guard(*this);
	bool found = false;
	for(std::vector<ReaderProxyData*>::iterator rit = m_matched_readers.begin();
			rit!=m_matched_readers.end();++rit)
	{
		if((*rit)->m_guid == rdata->m_guid)
		{
			found = true;
			m_matched_readers.erase(rit);
			break;
		}
	}
	if(found)
	{

		for(std::vector<Locator_t>::iterator lit = rdata->m_unicastLocatorList.begin();
				lit!=rdata->m_unicastLocatorList.end();++lit)
		{
			remove_locator(*lit);
		}
		for(std::vector<Locator_t>::iterator lit = rdata->m_multicastLocatorList.begin();
				lit!=rdata->m_multicastLocatorList.end();++lit)
		{
			remove_locator(*lit);
		}
		return true;
	}
	return false;
}

bool StatelessWriter::remove_locator(Locator_t& loc)
{
	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
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
bool StatelessWriter::reader_locator_add(Locator_t& loc,bool expectsInlineQos)
{
	bool found = false;
	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
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
		rl.expectsInlineQos = expectsInlineQos;
		rl.locator = loc;
		reader_locator.push_back(rl);
	}
	return true;
}
//
//
//bool StatelessWriter::reader_locator_remove(Locator_t& locator)
//{
//	boost::lock_guard<Endpoint> guard(*this);
//	for(std::vector<ReaderLocator>::iterator it=reader_locator.begin();it!=reader_locator.end();++it){
//		if(it->locator == locator){
//			reader_locator.erase(it);
//			return true;
//		}
//	}
//	return false;
//}

void StatelessWriter::unsent_changes_reset()
{

	boost::lock_guard<Endpoint> guard(*this);
	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit){
		rit->unsent_changes.clear();
		for(std::vector<CacheChange_t*>::iterator cit=m_writer_cache.changesBegin();
				cit!=m_writer_cache.changesEnd();++cit){
			rit->unsent_changes.push_back((*cit));
		}
	}
	unsent_changes_not_empty();
}

bool sort_cacheChanges (CacheChange_t* c1,CacheChange_t* c2)
{
	return(c1->sequenceNumber.to64long() < c2->sequenceNumber.to64long());
}

void StatelessWriter::unsent_change_add(CacheChange_t* cptr)
{
	boost::lock_guard<Endpoint> guard(*this);
	if(!reader_locator.empty())
	{
		for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
		{

			rit->unsent_changes.push_back(cptr);

			if(this->m_guid.entityId == ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER)
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
		pWarning( "No reader locator to add change" << std::endl);

	}

}

void StatelessWriter::unsent_changes_not_empty()
{
	boost::lock_guard<Endpoint> guard(*this);
	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
	{
		if(!rit->unsent_changes.empty())
		{
			if(m_pushMode)
			{
				if(this->m_guid.entityId == ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER)
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
	pDebugInfo ( "Finish sending unsent changes" << endl);
}


bool StatelessWriter::removeMinSeqCacheChange()
{
	return m_writer_cache.remove_min_change();
}

bool StatelessWriter::removeAllCacheChange(size_t* n_removed)
{
	size_t n_r=this->m_writer_cache.getHistorySize();
	if(this->m_writer_cache.remove_all_changes())
	{
		*n_removed = n_r;
		return true;
	}
	else
		return false;
}

bool StatelessWriter::change_removed_by_history(CacheChange_t* a_change)
{
	m_writer_cache.remove_change(a_change);
	return true;
}

} /* namespace rtps */
} /* namespace eprosima */


