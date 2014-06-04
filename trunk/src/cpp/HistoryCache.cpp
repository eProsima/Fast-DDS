/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * HistoryCache.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */
#include "eprosimartps/HistoryCache.h"
#include "eprosimartps/Endpoint.h"
#include "eprosimartps/common/CacheChange.h"
#include "eprosimartps/utils/RTPSLog.h"

//#include "eprosimartps/writer/ReaderLocator.h"
//#include "eprosimartps/writer/RTPSWriter.h"
//#include "eprosimartps/writer/StatelessWriter.h"
//#include "eprosimartps/reader/RTPSReader.h"
//#include "eprosimartps/reader/StatelessReader.h"



namespace eprosima {
namespace rtps {

bool sort_CacheChanges_History_SeqNum (CacheChange_t* c1,CacheChange_t* c2)
{
	return(c1->sequenceNumber.to64long() < c2->sequenceNumber.to64long());
}

HistoryCache::HistoryCache(Endpoint* endp, uint16_t historymaxsize, uint32_t payload_size):
		mp_Endpoint(endp),
		m_history_max_size(historymaxsize),
		isHistoryFull(false),
		changePool(historymaxsize,payload_size),
		m_isMaxMinUpdated(false)

{
		SEQUENCENUMBER_UNKOWN(m_minSeqNum);
		SEQUENCENUMBER_UNKOWN(m_maxSeqNum);
		GUID_UNKNOWN(m_minSeqNumGuid);
		GUID_UNKNOWN(m_maxSeqNumGuid);
		pDebugInfo("History created"<<std::endl;);
}

HistoryCache::~HistoryCache()
{
	pDebugInfo("HistoryCache destructor"<<endl;);
	for(std::vector<CacheChange_t*>::iterator it=m_changes.begin();
			it!=m_changes.end();++it)
	{
		changePool.release_Cache(*it);
	}

}

bool HistoryCache::get_change(SequenceNumber_t& seqNum,GUID_t& writerGuid,CacheChange_t** ch_ptr,uint16_t *ch_number) {
	boost::lock_guard<HistoryCache> guard(*this);

	(*ch_number)=0;
	for(	std::vector<CacheChange_t*>::iterator it = m_changes.begin();
			it!=m_changes.end();++it){
		if((*it)->sequenceNumber.to64long() == seqNum.to64long() )
			if(mp_Endpoint->getEndpointKind() == WRITER ||
					(mp_Endpoint->getEndpointKind() == READER && (*it)->writerGUID == writerGuid))

			{
				*ch_ptr = *it;
				return true;
			}
		(*ch_number)++;
	}
	return false;
}

bool HistoryCache::get_change(SequenceNumber_t& seqNum,GUID_t& writerGuid,CacheChange_t** ch_ptr) {
	boost::lock_guard<HistoryCache> guard(*this);
	std::vector<CacheChange_t*>::iterator it;
	for(it = m_changes.begin();it!=m_changes.end();++it){
		if((*it)->sequenceNumber.to64long() == seqNum.to64long())
		{
			if(mp_Endpoint->getEndpointKind() == WRITER ||
					(mp_Endpoint->getEndpointKind() == READER && (*it)->writerGUID == writerGuid))

			{
				*ch_ptr = *it;

				return true;
			}
	}
}
return false;
}

bool HistoryCache::get_last_added_cache(CacheChange_t** ch_ptr)
{
	boost::lock_guard<HistoryCache> guard(*this);
	if(!m_changes.empty())
	{
		*ch_ptr = *(m_changes.end()-1);
		return true;
	}
	else
	{
		*ch_ptr = NULL;
		return false;
	}
}

bool HistoryCache::add_change(CacheChange_t* a_change)
{
	boost::lock_guard<HistoryCache> guard(*this);
	if(m_changes.size() == (size_t)m_history_max_size) //History is full
	{
		pWarning("Change not added due to full History" << endl);
		return false;
	}

	//make copy of change to save

	if(mp_Endpoint->getEndpointKind() == WRITER)
	{
		m_lastChangeSequenceNumber++;
		a_change->sequenceNumber = m_lastChangeSequenceNumber;
		m_changes.push_back(a_change);
	}
	else if(mp_Endpoint->getEndpointKind() == READER)
	{
		//Check that the same change has not been already introduced
		for(std::vector<CacheChange_t*>::reverse_iterator it=m_changes.rbegin();it!=m_changes.rend();++it)
		{
			if((*it)->sequenceNumber.to64long() == a_change->sequenceNumber.to64long() &&
					(*it)->writerGUID == a_change->writerGUID)
			{
				pDebugInfo("Change with the same seqNum already in History" << endl);
				return false;
			}
		}
		m_changes.push_back(a_change);
	}
	else
	{
		pError(B_RED<<"HistoryType UNDEFINED"<<DEF <<endl);
		return false;
	}
	if(m_changes.size()==m_history_max_size)
		isHistoryFull = true;
	m_isMaxMinUpdated = false;
	pDebugInfo("Cache added to History (kind:"<<mp_Endpoint->getEndpointKind()<<") with seqNum: " << a_change->sequenceNumber.to64long()
			<< " from entityId: "<< a_change->writerGUID.entityId << endl);

	return true;
}

bool HistoryCache::remove_change(SequenceNumber_t& seqnum, GUID_t& guid)
{
	boost::lock_guard<HistoryCache> guard(*this);
	for(std::vector<CacheChange_t*>::iterator it = m_changes.begin();
			it!=m_changes.end();++it)
	{
		if((*it)->sequenceNumber.to64long() == seqnum.to64long())
		{
			if(mp_Endpoint->getEndpointKind() == WRITER ||
					(mp_Endpoint->getEndpointKind() == READER && (*it)->writerGUID == guid))
			{
				changePool.release_Cache(*it);
				m_changes.erase(it);
				isHistoryFull = false;
				m_isMaxMinUpdated = false;
				pDebugInfo("Change removed"<<endl)
				return true;
			}
		}
	}
	pWarning("Change NOT removed"<<endl);
	return false;
}

bool HistoryCache::remove_change(std::vector<CacheChange_t*>::iterator it)
{
	boost::lock_guard<HistoryCache> guard(*this);

	changePool.release_Cache(*it);
	m_changes.erase(it);
	isHistoryFull = false;
	m_isMaxMinUpdated = false;
	pDebugInfo("Change removed"<<endl);
	return true;

}



bool HistoryCache::remove_all_changes()
{
	boost::lock_guard<HistoryCache> guard(*this);
	if(!m_changes.empty())
	{
		for(std::vector<CacheChange_t*>::iterator it = m_changes.begin();it!=m_changes.end();++it)
		{
			changePool.release_Cache(*it);
		}
		m_changes.clear();
		isHistoryFull = false;
		m_isMaxMinUpdated = false;
		return true;
	}
	return false;
}

bool HistoryCache::isFull()
{
	//boost::lock_guard<HistoryCache> guard(*this);
	if(isHistoryFull)
		return true;
	else
		return false;
}

bool HistoryCache::get_seq_num_min(SequenceNumber_t* seqnum,GUID_t* guid)
{
	boost::lock_guard<HistoryCache> guard(*this);
		updateMaxMinSeqNum();
	*seqnum =m_minSeqNum;
	if(guid!=NULL)
		*guid = m_minSeqNumGuid;
	return true;
}

bool HistoryCache::get_seq_num_max(SequenceNumber_t* seqnum,GUID_t* guid)
{
	boost::lock_guard<HistoryCache> guard(*this);
	updateMaxMinSeqNum();
	*seqnum = m_maxSeqNum;
	if(guid!=NULL)
		*guid = m_maxSeqNumGuid;
	return true;
}



void HistoryCache::updateMaxMinSeqNum()
{
	//boost::lock_guard<HistoryCache> guard(*this);
	if(!m_changes.empty())
	{
		if(!m_isMaxMinUpdated)
		{
			std::sort(m_changes.begin(),m_changes.end(),sort_CacheChanges_History_SeqNum);
			m_maxSeqNum = (*(m_changes.end()-1))->sequenceNumber;
			m_maxSeqNumGuid = (*(m_changes.end()-1))->writerGUID;

			m_minSeqNum = (*m_changes.begin())->sequenceNumber;
			m_minSeqNumGuid = (*m_changes.begin())->writerGUID;

//			m_maxSeqNum = m_minSeqNum = m_changes[0]->sequenceNumber;
//			m_maxSeqNumGuid = m_minSeqNumGuid = m_changes[0]->writerGUID;
//
//			for(std::vector<CacheChange_t*>::iterator it = m_changes.begin();
//					it!=m_changes.end();++it){
//				if((*it)->sequenceNumber.to64long() > m_maxSeqNum.to64long())
//				{
//					m_maxSeqNum = (*it)->sequenceNumber;
//					m_maxSeqNumGuid = (*it)->writerGUID;
//				}
//				if((*it)->sequenceNumber.to64long() < m_minSeqNum.to64long())
//				{
//					m_minSeqNum = (*it)->sequenceNumber;
//					m_minSeqNumGuid = (*it)->writerGUID;
//				}
//			}
			m_isMaxMinUpdated = true;
		}
	}
	else
	{
		SEQUENCENUMBER_UNKOWN(m_minSeqNum);
		SEQUENCENUMBER_UNKOWN(m_maxSeqNum);
		GUID_UNKNOWN(m_minSeqNumGuid);
		GUID_UNKNOWN(m_maxSeqNumGuid);
	}
	return;
}




void HistoryCache::sortCacheChangesBySeqNum()
{
	std::sort(m_changes.begin(),m_changes.end(),sort_CacheChanges_History_SeqNum);
}


} /* namespace rtps */
} /* namespace eprosima */
