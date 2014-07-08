/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulWriter.cpp
 *
*/

#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/writer/ReaderProxy.h"

#include "eprosimartps/RTPSMessageCreator.h"

#include "eprosimartps/resources/ResourceSend.h"
#include "eprosimartps/resources/ResourceEvent.h"

//#include "eprosimartps/CDRMessage.h"
//#include "eprosimartps/qos/ParameterList.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {


StatefulWriter::~StatefulWriter()
{
	pDebugInfo("StatefulWriter destructor"<<endl;);
	if(mp_periodicHB !=NULL)
		delete(mp_periodicHB);
	for(std::vector<ReaderProxy*>::iterator it = matched_readers.begin();
			it!=matched_readers.end();++it)
	{
		delete(*it);
	}
}

StatefulWriter::StatefulWriter(const PublisherAttributes& param,const GuidPrefix_t&guidP, const EntityId_t& entId,DDSTopicDataType* ptype):
				RTPSWriter(guidP,entId,param,ptype,STATEFUL,param.userDefinedId,param.payloadMaxSize),
				m_PubTimes(param.times),
				mp_periodicHB(NULL)

{
	m_pushMode = param.pushMode;
	unicastLocatorList = param.unicastLocatorList;
	multicastLocatorList = param.multicastLocatorList;

	m_heartbeatCount = 0;
	if(entId == c_EntityId_SEDPPubWriter)
		m_HBReaderEntityId = c_EntityId_SEDPPubReader;
	else if(entId == c_EntityId_SEDPSubWriter)
		m_HBReaderEntityId = c_EntityId_SEDPSubReader;
	else if(entId == c_EntityId_WriterLiveliness)
		m_HBReaderEntityId= c_EntityId_ReaderLiveliness;
	else
		m_HBReaderEntityId = c_EntityId_Unknown;
}


bool StatefulWriter::matched_reader_add(ReaderProxy_t& RPparam)
{
	boost::lock_guard<Endpoint> guard(*this);
	for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		if((*it)->m_param.remoteReaderGuid == RPparam.remoteReaderGuid)
		{
			pWarning("Attempting to add existing reader" << endl);
			return false;
		}
	}
	ReaderProxy* rp = new ReaderProxy(RPparam,m_PubTimes,this);
	if(mp_periodicHB==NULL)
		mp_periodicHB = new PeriodicHeartbeat(this,boost::posix_time::milliseconds(Time_t2MilliSec(m_PubTimes.heartbeatPeriod)));
	if(rp->m_param.m_durabilityKind >= TRANSIENT_LOCAL_DURABILITY_QOS)
	{
		for(std::vector<CacheChange_t*>::iterator cit=m_writer_cache.changesBegin();cit!=m_writer_cache.changesEnd();++cit)
		{
			ChangeForReader_t changeForReader;
			changeForReader.setChange(*cit);
			changeForReader.is_relevant = rp->dds_is_relevant(*cit);

			if(m_pushMode)
				changeForReader.status = UNSENT;
			else
				changeForReader.status = UNACKNOWLEDGED;
			rp->m_changesForReader.push_back(changeForReader);
		}
	}
	matched_readers.push_back(rp);
	pDebugInfo("Reader Proxy added to StatefulWriter with " <<rp->m_param.unicastLocatorList.size()<<"(u)-"<<rp->m_param.multicastLocatorList.size()<<"(m) locators"<< endl);
	if(rp->m_changesForReader.size()>0)
		unsent_changes_not_empty();
	return true;
}

bool StatefulWriter::matched_reader_remove(ReaderProxy_t& Rp)
{
	return matched_reader_remove(Rp.remoteReaderGuid);
}

bool StatefulWriter::matched_reader_remove(GUID_t& readerGuid)
{
	boost::lock_guard<Endpoint> guard(*this);
	for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		if((*it)->m_param.remoteReaderGuid == readerGuid)
		{
			delete(*it);
			matched_readers.erase(it);
			pDebugInfo("Reader Proxy removed" << endl);
			if(matched_readers.size()==0)
				this->mp_periodicHB->stop_timer();
			return true;
		}
	}
	pInfo("Reader Proxy doesn't exist in this writer" << endl)
	return false;
}

bool StatefulWriter::matched_reader_lookup(GUID_t& readerGuid,ReaderProxy** RP)
{
	boost::lock_guard<Endpoint> guard(*this);
	std::vector<ReaderProxy*>::iterator it;
	for(it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		if((*it)->m_param.remoteReaderGuid == readerGuid)
		{
			*RP = *it;
			return true;
		}
	}
	return false;
}

bool StatefulWriter::is_acked_by_all(CacheChange_t* change)
{
	std::vector<ReaderProxy*>::iterator it;
	for(it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		ChangeForReader_t changeForReader;
		if((*it)->getChangeForReader(change,&changeForReader))
		{
			if(changeForReader.is_relevant)
			{
				if(changeForReader.status != ACKNOWLEDGED)
				{
					pDebugInfo("Change not acked. Relevant: " << changeForReader.is_relevant);
					pDebugInfo(" status: " << changeForReader.status << endl);
					return false;
				}
			}
		}
	}
	return true;
}

void StatefulWriter::unsent_change_add(CacheChange_t* change)
{
	boost::lock_guard<Endpoint> guard(*this);
	if(!matched_readers.empty())
	{
		std::vector<ReaderProxy*>::iterator it;
		for(it=matched_readers.begin();it!=matched_readers.end();++it)
		{
			ChangeForReader_t changeForReader;
			changeForReader.setChange(change);
			if(m_pushMode)
				changeForReader.status = UNSENT;
			else
				changeForReader.status = UNACKNOWLEDGED;
			changeForReader.is_relevant = (*it)->dds_is_relevant(change);
			(*it)->m_changesForReader.push_back(changeForReader);
		}
		unsent_changes_not_empty();
	}
	else
	{
		pInfo("No reader proxy to add change." << endl);
	}
}

bool sort_changeForReader_ptr (ChangeForReader_t* c1,ChangeForReader_t* c2)
{
	return(c1->seqNum < c2->seqNum);
}

bool sort_changeForReader(ChangeForReader_t c1,ChangeForReader_t c2)
{
	return(c1.seqNum < c2.seqNum);
}

bool sort_changes (CacheChange_t* c1,CacheChange_t* c2)
{
	return(c1->sequenceNumber.to64long() < c2->sequenceNumber.to64long());
}



void StatefulWriter::unsent_changes_not_empty()
{
	boost::lock_guard<Endpoint> guard(*this);
	std::vector<ReaderProxy*>::iterator rit;
	boost::lock_guard<ResourceSend> guard2(*mp_send_thr);
	for(rit=matched_readers.begin();rit!=matched_readers.end();++rit)
	{
		boost::lock_guard<ReaderProxy> guard(*(*rit));
		std::vector<ChangeForReader_t*> ch_vec;
		if((*rit)->unsent_changes(&ch_vec))
		{
			//std::sort(ch_vec.begin(),ch_vec.end(),sort_changeForReader_ptr);
			//Get relevant data cache changes
			std::vector<CacheChange_t*> relevant_changes;
			std::vector<SequenceNumber_t> not_relevant_changes;
			std::vector<ChangeForReader_t*>::iterator cit;
			for(cit = ch_vec.begin();cit!=ch_vec.end();++cit)
			{
				(*cit)->status = UNDERWAY;
				if((*cit)->is_relevant && (*cit)->isValid())
				{
					relevant_changes.push_back((*cit)->getChange());
				}
				else
				{
					not_relevant_changes.push_back((*cit)->seqNum);
				}
			}
			if(m_pushMode)
			{
				if(!relevant_changes.empty())
					RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)this,
							&relevant_changes,
							(*rit)->m_param.unicastLocatorList,
							(*rit)->m_param.multicastLocatorList,
							(*rit)->m_param.expectsInlineQos,
							(*rit)->m_param.remoteReaderGuid.entityId);
				if(!not_relevant_changes.empty())
					RTPSMessageGroup::send_Changes_AsGap(&m_cdrmessages,(RTPSWriter*)this,
							&not_relevant_changes,
							(*rit)->m_param.remoteReaderGuid.entityId,
							&(*rit)->m_param.unicastLocatorList,
							&(*rit)->m_param.multicastLocatorList);
				if((*rit)->m_param.m_reliability == RELIABLE)
					this->mp_periodicHB->restart_timer();
				(*rit)->m_nackSupression.restart_timer();
			}
			else
			{
				CacheChange_t* first;
				CacheChange_t* last;
				m_writer_cache.get_min_change(&first);
				m_writer_cache.get_max_change(&last);
				incrementHBCount();
				CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_fullmsg);
				RTPSMessageCreator::addMessageHeartbeat(&m_cdrmessages.m_rtpsmsg_fullmsg,m_guid.guidPrefix,
						c_EntityId_Unknown,m_guid.entityId,first->sequenceNumber,last->sequenceNumber,m_heartbeatCount,true,false);
				std::vector<Locator_t>::iterator lit;
				for(lit = (*rit)->m_param.unicastLocatorList.begin();lit!=(*rit)->m_param.unicastLocatorList.end();++lit)
					mp_send_thr->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,(*lit));
				for(lit = (*rit)->m_param.multicastLocatorList.begin();lit!=(*rit)->m_param.multicastLocatorList.end();++lit)
					mp_send_thr->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,(*lit));
			}
		}
	}
	pDebugInfo("Finish sending unsent changes" << endl);
}

bool StatefulWriter::removeMinSeqCacheChange()
{
	CacheChange_t* change;
	m_writer_cache.get_min_change(&change);

	if(is_acked_by_all(change))
	{
		for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
				it!=this->matched_readers.end();++it)
		{
			if(!(*it)->m_changesForReader.empty())
				(*it)->m_changesForReader.erase((*it)->m_changesForReader.begin());
		}
		m_writer_cache.remove_min_change();
		return true;
	}

	return false;
}

bool StatefulWriter::removeAllCacheChange(size_t* removed)
{
	boost::lock_guard<Endpoint> guard(*this);
	int32_t n_count = 0;
	while(this->removeMinSeqCacheChange())
	{
		n_count++;
	}
	*removed = n_count;
	if(this->m_writer_cache.getHistorySize()==0)
		return true;
	else
		return false;
}

bool StatefulWriter::change_removed_by_history(CacheChange_t* a_change)
{
	boost::lock_guard<Endpoint> guard(*this);
	CacheChange_t* min_change;
	m_writer_cache.get_min_change(&min_change);
	if(a_change->sequenceNumber == min_change->sequenceNumber)
	{
		removeMinSeqCacheChange();
	}
	else
	{
		for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
				it!=this->matched_readers.end();++it)
		{
			for(std::vector<ChangeForReader_t>::iterator chit = (*it)->m_changesForReader.begin();
					chit!=(*it)->m_changesForReader.end();++chit)
			{
				if(chit->seqNum == a_change->sequenceNumber)
				{
					chit->notValid();
				}
			}
		}
		m_writer_cache.remove_change(a_change);
	}
	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
