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
#include "eprosimartps/ReaderLocator.h"
#include "eprosimartps/RTPSWriter.h"
#include "eprosimartps/StatelessWriter.h"
#include "eprosimartps/RTPSReader.h"
#include "eprosimartps/StatelessReader.h"
#include "eprosimartps/common/rtps_elem_seqnum.h"
#include "eprosimartps/common/rtps_elem_guid.h"



namespace eprosima {
namespace rtps {

HistoryCache::HistoryCache() {
	SEQUENCENUMBER_UNKOWN(minSeqNum);
	SEQUENCENUMBER_UNKOWN(maxSeqNum);
	GUID_UNKNOWN(minSeqNumGuid);
	GUID_UNKNOWN(maxSeqNumGuid);


}

HistoryCache::~HistoryCache() {

}

bool HistoryCache::get_change(SequenceNumber_t seqNum,GUID_t writerGuid,CacheChange_t** change) {
	boost::lock_guard<HistoryCache> guard(*this);
	std::vector<CacheChange_t*>::iterator it;
	for(it = changes.begin();it!=changes.end();it++){
		if((*it)->sequenceNumber.to64long() == seqNum.to64long() &&
				(*it)->writerGUID == writerGuid)
		{
			(*change)->copy(*it);
			return true;
		}
	}
	return false;
}

bool HistoryCache::add_change(CacheChange_t* a_change)
{
	pDebugInfo ( "Trying to lock history " << endl);

	boost::lock_guard<HistoryCache> guard(*this);

	if(changes.size() == (size_t)historySize) //History is full
	{
		pWarning("Attempting to add change with Full History" << endl);
		return false;
	}

	//make copy of change to save

	if(historyKind == WRITER)
	{
		rtpswriter->lastChangeSequenceNumber++;
		a_change->sequenceNumber = rtpswriter->lastChangeSequenceNumber;
		changes.push_back(a_change);
		updateMaxMinSeqNum();
	}
	else if(historyKind == READER)
	{

		//Check that the same change has not been already introduced
		std::vector<CacheChange_t*>::iterator it;
		for(it=changes.begin();it!=changes.end();it++)
		{
			if((*it)->sequenceNumber.to64long() == a_change->sequenceNumber.to64long() &&
					(*it)->writerGUID == a_change->writerGUID)
			{
				pWarning("Change with the same seqNum already in History" << endl);
				return false;
			}
		}
		changes.push_back(a_change);
		updateMaxMinSeqNum();
	}
	if(changes.size()==historySize)
		isHistoryFull = true;
	pDebugInfo("Cache added to History" << endl);

	return true;
}

bool HistoryCache::remove_change(SequenceNumber_t seqnum, GUID_t guid)
{
	boost::lock_guard<HistoryCache> guard(*this);
	std::vector<CacheChange_t*>::iterator it;
	for(it = changes.begin();it!=changes.end();it++){
		if((*it)->sequenceNumber.to64long() == seqnum.to64long()
				&& (*it)->writerGUID == guid)
		{
			delete (*it);
			changes.erase(it);
			isHistoryFull = false;
			updateMaxMinSeqNum();
			return true;
		}
	}
	return false;
}

bool HistoryCache::remove_change(std::vector<CacheChange_t*>::iterator it)
{
	boost::lock_guard<HistoryCache> guard(*this);

	delete(*it);
	changes.erase(it);
	isHistoryFull = false;
	updateMaxMinSeqNum();

	return true;

}



bool HistoryCache::remove_all_changes()
{
	boost::lock_guard<HistoryCache> guard(*this);
	if(!changes.empty())
	{
		std::vector<CacheChange_t*>::iterator it;
		for(it = changes.begin();it!=changes.end();it++)
		{
			delete (*it);
		}
		changes.clear();
		updateMaxMinSeqNum();
		isHistoryFull = false;
		return true;
	}
	return false;
}

bool HistoryCache::isFull()
{
	boost::lock_guard<HistoryCache> guard(*this);
	if(isHistoryFull)
		return true;
	else
		return false;
}

bool HistoryCache::get_seq_num_min(SequenceNumber_t* seqnum,GUID_t* guid)
{
	*seqnum = minSeqNum;
	*guid = minSeqNumGuid;
	return true;
}

bool HistoryCache::get_seq_num_max(SequenceNumber_t* seqnum,GUID_t* guid)
{
	*seqnum = maxSeqNum;
	*guid = maxSeqNumGuid;
	return true;
}

void HistoryCache::updateMaxMinSeqNum()
{
	boost::lock_guard<HistoryCache> guard(*this);
	if(!changes.empty())
	{
		std::vector<CacheChange_t*>::iterator it;
		maxSeqNum = minSeqNum = changes[0]->sequenceNumber;
		maxSeqNumGuid = minSeqNumGuid = changes[0]->writerGUID;
		//cout << "Seqnum init a " << maxSeqNum.to64long() << endl;
		for(it = changes.begin();it!=changes.end();it++){
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

} /* namespace rtps */
} /* namespace eprosima */
