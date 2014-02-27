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
	// TODO Auto-generated constructor stub

}

HistoryCache::~HistoryCache() {
	// TODO Auto-generated destructor stub
}

bool HistoryCache::get_change(SequenceNumber_t seqNum,CacheChange_t** change) {
	std::vector<CacheChange_t>::iterator it;
	for(it = changes.begin();it!=changes.end();it++){
		if(it->sequenceNumber == seqNum){
			*change = &(*it);
			return true;
		}
	}
	return false;
}

bool HistoryCache::add_change(CacheChange_t a_change) {
	if(changes.size() == (size_t)historySize) //History is full
		return false;
	//make copy of change to save
	CacheChange_t* ch = new CacheChange_t();
	ch->copy(&a_change);
	if(historyKind == WRITER){
		rtpswriter->lastChangeSequenceNumber++;
		ch->sequenceNumber = rtpswriter->lastChangeSequenceNumber;
	}


	changes.push_back(*ch);
	maxSeqNum = ch->sequenceNumber;
	if(historyKind == WRITER){
		if(rtpswriter->stateType == STATELESS){
			((StatelessWriter*)rtpswriter)->unsent_change_add(ch->sequenceNumber);
		}
		else{

		}
	}
	else if(historyKind == READER){
		//Notify user... and maybe writer proxies
		if(rtpsreader->stateType == STATEFUL){
			//TODO Notify proxies
		}
	}
	return true;
}

bool HistoryCache::remove_change(CacheChange_t a_change) {
	return remove_change(a_change.sequenceNumber);
}

bool HistoryCache::remove_change(SequenceNumber_t seqnum) {

	std::vector<CacheChange_t>::iterator it;
	for(it = changes.begin();it!=changes.end();it++){
		if(it->sequenceNumber == seqnum){
			changes.erase(it);
			updateMaxMinSeqNum();
			return true;
		}
	}
	return false;
}



SequenceNumber_t HistoryCache::get_seq_num_min() {
	return minSeqNum;
}

SequenceNumber_t HistoryCache::get_seq_num_max() {
	return maxSeqNum;
}

void HistoryCache::updateMaxMinSeqNum() {
	std::vector<CacheChange_t>::iterator it;
	maxSeqNum = minSeqNum = changes[0].sequenceNumber;
	for(it = changes.begin();it!=changes.end();it++){
		if(it->sequenceNumber > maxSeqNum)
			maxSeqNum = it->sequenceNumber;
		if(it->sequenceNumber < minSeqNum)
			minSeqNum = it->sequenceNumber;
	}
}

} /* namespace rtps */
} /* namespace eprosima */
