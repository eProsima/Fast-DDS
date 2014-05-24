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
 *  Created on: Mar 17, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
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


StatefulWriter::~StatefulWriter() {

	pDebugInfo("StatefulWriter destructor"<<endl;);
}

StatefulWriter::StatefulWriter(const PublisherAttributes& param,const GuidPrefix_t&guidP, const EntityId_t& entId):
				RTPSWriter(guidP,entId,param.topic,STATEFUL,param.userDefinedId,param.historyMaxSize,param.payloadMaxSize),
				m_PubTimes(param.times)

{
	m_pushMode = param.pushMode;
	unicastLocatorList = param.unicastLocatorList;
	multicastLocatorList = param.multicastLocatorList;

	m_heartbeatCount = 0;
}


bool StatefulWriter::matched_reader_add(ReaderProxy_t& RPparam)
{

	for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		if((*it)->m_param.remoteReaderGuid == RPparam.remoteReaderGuid)
		{
			pWarning("Attempting to add existing reader" << endl);
			return false;
		}
	}
	ReaderProxy* rp = new ReaderProxy(RPparam,m_PubTimes,this);


	for(std::vector<CacheChange_t*>::iterator cit=m_writer_cache.m_changes.begin();cit!=m_writer_cache.m_changes.end();++cit)
	{
		ChangeForReader_t changeForReader;
		changeForReader.change = (*cit);
		changeForReader.is_relevant = rp->dds_is_relevant(*cit);

		if(m_pushMode)
			changeForReader.status = UNSENT;
		else
			changeForReader.status = UNACKNOWLEDGED;
		rp->m_changesForReader.push_back(changeForReader);
	}
	matched_readers.push_back(rp);
	pDebugInfo("Reader Proxy added" << endl);
	return true;
}

bool StatefulWriter::matched_reader_remove(ReaderProxy_t& Rp)
{
	return matched_reader_remove(Rp.remoteReaderGuid);
}

bool StatefulWriter::matched_reader_remove(GUID_t& readerGuid)
{
	for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		if((*it)->m_param.remoteReaderGuid == readerGuid)
		{
			delete(*it);
			matched_readers.erase(it);
			pDebugInfo("Reader Proxy removed" << endl);
			return true;
		}
	}
	pInfo("Reader Proxy doesn't exist in this writer" << endl)
	return false;
}

bool StatefulWriter::matched_reader_lookup(GUID_t& readerGuid,ReaderProxy** RP)
{
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
	if(!matched_readers.empty())
	{
		std::vector<ReaderProxy*>::iterator it;
		for(it=matched_readers.begin();it!=matched_readers.end();++it)
		{
			ChangeForReader_t changeForReader;
			changeForReader.change = change;
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
		pWarning("No reader proxy to add change." << endl);
	}
}

bool sort_changeForReader_ptr (ChangeForReader_t* c1,ChangeForReader_t* c2)
{
	return(c1->change->sequenceNumber.to64long() < c2->change->sequenceNumber.to64long());
}

bool sort_changeForReader(ChangeForReader_t c1,ChangeForReader_t c2)
{
	return(c1.change->sequenceNumber.to64long() < c2.change->sequenceNumber.to64long());
}

bool sort_changes (CacheChange_t* c1,CacheChange_t* c2)
{
	return(c1->sequenceNumber.to64long() < c2->sequenceNumber.to64long());
}



void StatefulWriter::unsent_changes_not_empty()
{
	std::vector<ReaderProxy*>::iterator rit;
	boost::lock_guard<ResourceSend> guard(*mp_send_thr);
	for(rit=matched_readers.begin();rit!=matched_readers.end();++rit)
	{
		boost::lock_guard<ReaderProxy> guard(*(*rit));
		std::vector<ChangeForReader_t*> ch_vec;
		if((*rit)->unsent_changes(&ch_vec))
		{
			std::sort(ch_vec.begin(),ch_vec.end(),sort_changeForReader_ptr);

			//Get relevant data cache changes
			std::vector<CacheChange_t*> relevant_changes;
			std::vector<CacheChange_t*> not_relevant_changes;
			std::vector<ChangeForReader_t*>::iterator cit;
			for(cit = ch_vec.begin();cit!=ch_vec.end();++cit)
			{
				(*cit)->status = UNDERWAY;
				if((*cit)->is_relevant)
				{
					relevant_changes.push_back((*cit)->change);
				}
				else
				{
					not_relevant_changes.push_back((*cit)->change);
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
					(*rit)->m_periodicHB.restart_timer();
				(*rit)->m_nackSupression.restart_timer();
			}
			else
			{
				SequenceNumber_t first,last;
				m_writer_cache.get_seq_num_min(&first,NULL);
				m_writer_cache.get_seq_num_max(&last,NULL);
				incrementHBCount();
				CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_fullmsg);
				RTPSMessageCreator::addMessageHeartbeat(&m_cdrmessages.m_rtpsmsg_fullmsg,m_guid.guidPrefix,
						ENTITYID_UNKNOWN,m_guid.entityId,first,last,m_heartbeatCount,true,false);
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
	SequenceNumber_t seq;
	GUID_t gui;
	m_writer_cache.get_seq_num_min(&seq,&gui);
	CacheChange_t* change=NULL;
	if(m_writer_cache.get_change(seq,gui,&change))
	{
		if(is_acked_by_all(change))
		{
			ReaderProxy* rp;
			for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
					it!=this->matched_readers.end();++it)
			{
				rp = *it;
				std::sort(rp->m_changesForReader.begin(),rp->m_changesForReader.end(),sort_changeForReader);
				rp->m_changesForReader.erase(rp->m_changesForReader.begin());
			}
			m_writer_cache.remove_change(change->sequenceNumber,change->writerGUID);
			return true;
		}
	}
	return false;
}

bool StatefulWriter::removeAllCacheChange(int32_t* removed)
{
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



} /* namespace rtps */
} /* namespace eprosima */
