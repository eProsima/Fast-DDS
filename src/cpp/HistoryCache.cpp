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
#include "eprosimartps/writer/ReaderLocator.h"
#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/common/rtps_elem_seqnum.h"
#include "eprosimartps/common/rtps_elem_guid.h"



namespace eprosima {
namespace rtps {



HistoryCache::HistoryCache(uint16_t historysize,uint32_t payload_size):
		rtpswriter(NULL),rtpsreader(NULL),
		historyKind(UDEF),
		m_history_max_size(historysize),
		isHistoryFull(false),
		changePool(historysize,payload_size)

{
		SEQUENCENUMBER_UNKOWN(minSeqNum);
		SEQUENCENUMBER_UNKOWN(maxSeqNum);
		GUID_UNKNOWN(minSeqNumGuid);
		GUID_UNKNOWN(maxSeqNumGuid);
		pDebugInfo("History created"<<endl);
}

HistoryCache::~HistoryCache()
{
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
		if((*it)->sequenceNumber.to64long() == seqNum.to64long() &&
				(*it)->writerGUID == writerGuid)
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
		if((*it)->sequenceNumber.to64long() == seqNum.to64long() &&
				(*it)->writerGUID == writerGuid)
		{
			*ch_ptr = *it;

			return true;
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
	pDebugInfo ( "Trying to lock history " << endl);

	boost::lock_guard<HistoryCache> guard(*this);

	if(m_changes.size() == (size_t)m_history_max_size) //History is full
	{
		pWarning("Attempting to add change with Full History" << endl);
		return false;
	}

	//make copy of change to save

	if(historyKind == WRITER)
	{
		rtpswriter->m_lastChangeSequenceNumber++;
		a_change->sequenceNumber = rtpswriter->m_lastChangeSequenceNumber;
		m_changes.push_back(a_change);

	}
	else if(historyKind == READER)
	{
		//Check that the same change has not been already introduced
		std::vector<CacheChange_t*>::iterator it;
		for(it=m_changes.begin();it!=m_changes.end();++it)
		{
			if((*it)->sequenceNumber.to64long() == a_change->sequenceNumber.to64long() &&
					(*it)->writerGUID == a_change->writerGUID)
			{
				pWarning("Change with the same seqNum already in History" << endl);
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
	pDebugInfo("Cache added to History" << endl);

	return true;
}

bool HistoryCache::remove_change(SequenceNumber_t& seqnum, GUID_t& guid)
{
	boost::lock_guard<HistoryCache> guard(*this);
	for(std::vector<CacheChange_t*>::iterator it = m_changes.begin();
			it!=m_changes.end();++it)
	{
		if((*it)->sequenceNumber.to64long() == seqnum.to64long()
				&& (*it)->writerGUID == guid)
		{
			changePool.release_Cache(*it);
			m_changes.erase(it);
			isHistoryFull = false;
			pDebugInfo("Change removed"<<endl)
			return true;
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
	*seqnum = minSeqNum;
	*guid = minSeqNumGuid;
	return true;
}

bool HistoryCache::get_seq_num_max(SequenceNumber_t* seqnum,GUID_t* guid)
{
	boost::lock_guard<HistoryCache> guard(*this);
	updateMaxMinSeqNum();
	*seqnum = maxSeqNum;
	*guid = maxSeqNumGuid;
	return true;
}

void HistoryCache::updateMaxMinSeqNum()
{
	//boost::lock_guard<HistoryCache> guard(*this);
	if(!m_changes.empty())
	{
		maxSeqNum = minSeqNum = m_changes[0]->sequenceNumber;
		maxSeqNumGuid = minSeqNumGuid = m_changes[0]->writerGUID;

		for(std::vector<CacheChange_t*>::iterator it = m_changes.begin();
				it!=m_changes.end();++it){
			if((*it)->sequenceNumber.to64long() > maxSeqNum.to64long())
			{
				maxSeqNum = (*it)->sequenceNumber;
				maxSeqNumGuid = (*it)->writerGUID;
			}
			if((*it)->sequenceNumber.to64long() < minSeqNum.to64long())
			{
				minSeqNum = (*it)->sequenceNumber;
				minSeqNumGuid = (*it)->writerGUID;
			}
		}
	}
	else
	{
		SEQUENCENUMBER_UNKOWN(minSeqNum);
		SEQUENCENUMBER_UNKOWN(maxSeqNum);
		GUID_UNKNOWN(minSeqNumGuid);
		GUID_UNKNOWN(maxSeqNumGuid);
	}
	return;
}

CacheChange_t* HistoryCache::reserve_Cache()
{
	return changePool.reserve_Cache();
}

void HistoryCache::release_Cache(CacheChange_t* ch)
{
	return changePool.release_Cache(ch);
}



} /* namespace rtps */
} /* namespace eprosima */
