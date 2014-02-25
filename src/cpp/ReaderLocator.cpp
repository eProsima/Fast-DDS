/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ReaderLocator.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/ReaderLocator.h"

namespace eprosima {
namespace rtps {

ReaderLocator::ReaderLocator() {
	// TODO Auto-generated constructor stub

}

ReaderLocator::ReaderLocator(Locator_t a_locator, bool expectsQos){
	locator = a_locator;
	expectsInlineQos = expectsQos;
}

ReaderLocator::~ReaderLocator() {
	// TODO Auto-generated destructor stub
}

CacheChange_t* ReaderLocator::next_requested_change() {
	std::vector<CacheChange_t*>::iterator it;
	SequenceNumber_t minseqnum = requested_changes[0]->sequenceNumber;
	CacheChange_t* cpoin;
	for(it=requested_changes.begin();it!=requested_changes.end();it++){
		if(minseqnum > (*it)->sequenceNumber){
			minseqnum = (*it)->sequenceNumber;
			cpoin = *it;
		}
	}
	return cpoin;
}

CacheChange_t* ReaderLocator::next_unsent_change() {
	std::vector<CacheChange_t*>::iterator it;
	SequenceNumber_t minseqnum = unsent_changes[0]->sequenceNumber;
	CacheChange_t* cpoin;
	for(it=unsent_changes.begin();it!=unsent_changes.end();it++){
		if(minseqnum > (*it)->sequenceNumber){
			minseqnum = (*it)->sequenceNumber;
			cpoin = *it;
		}
	}
	return cpoin;
}

void ReaderLocator::requested_changes_set(std::vector<SequenceNumber_t>seqs,HistoryCache* history) {
	std::vector<SequenceNumber_t>::iterator it;
	requested_changes.clear();
	for(it = seqs.begin();it!=seqs.end();it++)
	{
		CacheChange_t* cpoin;
		history->get_change(*it,cpoin);
		requested_changes.push_back(cpoin);
	}
}


} /* namespace rtps */
} /* namespace eprosima */


