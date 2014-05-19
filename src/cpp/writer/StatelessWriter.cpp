/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * StatelessWriter.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com
 */

#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/writer/ReaderLocator.h"
#include "eprosimartps/qos/ParameterList.h"

namespace eprosima {
namespace rtps {



StatelessWriter::StatelessWriter(const PublisherAttributes* param,uint32_t payload_size):
		RTPSWriter(param->historyMaxSize,payload_size)
{
	m_pushMode = param->pushMode;
	//writer_cache.changes.reserve(param->historySize);

	m_stateType = STATELESS;
	//locator lists:
	unicastLocatorList = param->unicastLocatorList;
	multicastLocatorList = param->multicastLocatorList;
	topicKind = param->topic.topicKind;
	m_topicName = param->topic.topicName;
	m_topicDataType = param->topic.topicDataType;
	this->m_userDefinedId = param->userDefinedId;
}




StatelessWriter::~StatelessWriter()
{

	pDebugInfo("StatelessWriter destructor"<<endl;);
}

bool StatelessWriter::reader_locator_add(ReaderLocator& a_locator) {

	pDebugInfo("Adding new Reader Locator to StatelessWriter"<<endl);
	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit){

		if(rit->locator == a_locator.locator)
		{
			pWarning("Reader Locator already in list"<<endl);
			return false;
		}
	}
	a_locator.requested_changes.clear();
	a_locator.unsent_changes.clear();
	for(std::vector<CacheChange_t*>::iterator it = m_writer_cache.m_changes.begin();
			it!=m_writer_cache.m_changes.end();++it){
		a_locator.unsent_changes.push_back((*it));
	}
	reader_locator.push_back(a_locator);
	if(!a_locator.unsent_changes.empty())
		this->unsent_changes_not_empty();
	return true;
}

bool StatelessWriter::reader_locator_add(Locator_t& locator,bool expectsInlineQos)
{
	ReaderLocator a_locator;
	a_locator.expectsInlineQos = expectsInlineQos;
	a_locator.locator = locator;
	return reader_locator_add(a_locator);
}


bool StatelessWriter::reader_locator_remove(Locator_t& locator) {
	std::vector<ReaderLocator>::iterator it;
	for(it=reader_locator.begin();it!=reader_locator.end();++it){
		if(it->locator == locator){
			reader_locator.erase(it);
			return true;
		}
	}
	return false;
}

void StatelessWriter::unsent_changes_reset() {


	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit){
		rit->unsent_changes.clear();
		for(std::vector<CacheChange_t*>::iterator cit=m_writer_cache.m_changes.begin();
				cit!=m_writer_cache.m_changes.end();++cit){
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
	if(!reader_locator.empty())
	{
		for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
		{
			rit->unsent_changes.push_back(cptr);

//			if(m_pushMode)
//			{
//				std::sort(rit->unsent_changes.begin(),rit->unsent_changes.end(),sort_cacheChanges);
				RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)this,
						&rit->unsent_changes,&rit->locator,rit->expectsInlineQos,c_EntityId_Unknown);
				rit->unsent_changes.clear();
//			}
//			else
//			{
//					SequenceNumber_t first,last;
//				m_writer_cache.get_seq_num_min(&first,NULL);
//				m_writer_cache.get_seq_num_max(&last,NULL);
//				m_heartbeatCount++;
//				CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_fullmsg);
//				RTPSMessageCreator::addMessageHeartbeat(&m_cdrmessages.m_rtpsmsg_fullmsg,m_guid.guidPrefix,
//										ENTITYID_UNKNOWN,m_guid.entityId,first,last,m_heartbeatCount,true,false);
//				mp_send_thr->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,&rit->locator);
//				rit->unsent_changes.clear();
//			}
		}

	}
	else
	{
		pWarning( "No reader locator to add change" << std::endl);

	}

}

void StatelessWriter::unsent_changes_not_empty()
{

	for(std::vector<ReaderLocator>::iterator rit=reader_locator.begin();rit!=reader_locator.end();++rit)
	{
		if(!rit->unsent_changes.empty())
		{
			if(m_pushMode)
			{
				//	std::sort(rit->unsent_changes.begin(),rit->unsent_changes.end(),sort_cacheChanges);
				RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)this,
						&rit->unsent_changes,&rit->locator,rit->expectsInlineQos,c_EntityId_Unknown);
				rit->unsent_changes.clear();
			}
			else
			{
				SequenceNumber_t first,last;
				m_writer_cache.get_seq_num_min(&first,NULL);
				m_writer_cache.get_seq_num_max(&last,NULL);
				m_heartbeatCount++;
				CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_fullmsg);
				RTPSMessageCreator::addMessageHeartbeat(&m_cdrmessages.m_rtpsmsg_fullmsg,m_guid.guidPrefix,
						ENTITYID_UNKNOWN,m_guid.entityId,first,last,m_heartbeatCount,true,false);
				mp_send_thr->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,&rit->locator);
				rit->unsent_changes.clear();
			}
		}
	}
	pDebugInfo ( "Finish sending unsent changes" << endl);
}


bool StatelessWriter::removeMinSeqCacheChange()
{
	SequenceNumber_t seq;
	GUID_t gui;
	if(this->m_writer_cache.get_seq_num_min(&seq,&gui))
		return this->m_writer_cache.remove_change(seq,gui);
	else
		return false;
}

bool StatelessWriter::removeAllCacheChange(int32_t* n_removed)
{
	int32_t n_r=this->m_writer_cache.getHistorySize();
	if(this->m_writer_cache.remove_all_changes())
	{
		*n_removed = n_r;
		return true;
	}
	else
		return false;
}

} /* namespace rtps */
} /* namespace eprosima */


