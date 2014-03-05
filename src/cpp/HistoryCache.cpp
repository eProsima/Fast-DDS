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



namespace eprosima {
namespace rtps {

HistoryCache::HistoryCache() {


}

HistoryCache::~HistoryCache() {

}

bool HistoryCache::get_change(SequenceNumber_t seqNum,CacheChange_t** change) {
	historyMutex.lock();
	std::vector<CacheChange_t*>::iterator it;
	for(it = changes.begin();it!=changes.end();it++){
		if((*it)->sequenceNumber == seqNum){
			*change = (*it);
			historyMutex.unlock();
			return true;
		}
	}
	historyMutex.unlock();
	return false;
}

bool HistoryCache::add_change(CacheChange_t a_change) {
	if(changes.size() == (size_t)historySize) //History is full
	{
		return false;
	}
	historyMutex.lock();
	//TODOG manage when a reader history is full
	//make copy of change to save
	CacheChange_t* ch = new CacheChange_t();
	ch->copy(&a_change);
	if(historyKind == WRITER){
		rtpswriter->lastChangeSequenceNumber++;
		ch->sequenceNumber = rtpswriter->lastChangeSequenceNumber;
		changes.push_back(ch);
		updateMaxMinSeqNum();
	}
	else if(historyKind == READER)
	{
		//Check that the same change has not been already introduced
		std::vector<CacheChange_t*>::iterator it;
		for(it=changes.begin();it!=changes.end();it++)
		{
			if((*it)->sequenceNumber == ch->sequenceNumber)
			{
				historyMutex.unlock();
				return false;
			}
		}
		changes.push_back(ch);
		updateMaxMinSeqNum();
	}
	maxSeqNum = ch->sequenceNumber;
	historyMutex.unlock();
	//DO SOMETHING ONCE THE NEW CHANGE HAS BEEN ADDED

	if(historyKind == WRITER)
	{
		if(rtpswriter->stateType == STATELESS)
		{
			((StatelessWriter*)rtpswriter)->unsent_change_add(ch->sequenceNumber);
		}
		else
		{

		}
	}
	else if(historyKind == READER)
	{
		//Notify user... and maybe writer proxies
		if(rtpsreader->newMessageCallback !=NULL)
			rtpsreader->newMessageCallback();
		//else //FIXME: removed for testing, put back.
			rtpsreader->newMessageSemaphore->post();

		if(rtpsreader->stateType == STATEFUL)
		{
			//TODOG Notify proxies
		}
	}

	return true;
}

bool HistoryCache::remove_change(CacheChange_t a_change) {
	return remove_change(a_change.sequenceNumber);
}

bool HistoryCache::remove_change(SequenceNumber_t seqnum) {
	historyMutex.lock();
	std::vector<CacheChange_t*>::iterator it;
	for(it = changes.begin();it!=changes.end();it++){
		if((*it)->sequenceNumber == seqnum){
			changes.erase(it);
			updateMaxMinSeqNum();
			delete (*it);
			updateMaxMinSeqNum();
			historyMutex.unlock();
			return true;
		}
	}
	historyMutex.unlock();
	return false;
}



SequenceNumber_t HistoryCache::get_seq_num_min() {
	return minSeqNum;
}

SequenceNumber_t HistoryCache::get_seq_num_max() {
	return maxSeqNum;
}

void HistoryCache::updateMaxMinSeqNum() {
	if(!changes.empty())
	{
		std::vector<CacheChange_t*>::iterator it;
		maxSeqNum = minSeqNum = changes[0]->sequenceNumber;
		//cout << "Seqnum init a " << maxSeqNum.to64long() << endl;
		for(it = changes.begin();it!=changes.end();it++){
			if((*it)->sequenceNumber.to64long() > maxSeqNum.to64long())
				maxSeqNum = (*it)->sequenceNumber;
			if((*it)->sequenceNumber.to64long() < minSeqNum.to64long())
				minSeqNum = (*it)->sequenceNumber;
			//cout << "New: " << (*it)->sequenceNumber.to64long() << " Max: " << maxSeqNum.to64long() << " Min: " << minSeqNum.to64long() << endl;
		}
	}
	return;
}

} /* namespace rtps */
} /* namespace eprosima */
