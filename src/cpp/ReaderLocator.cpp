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

bool ReaderLocator::next_requested_change(CacheChange_t* cpoin) {
	if(!requested_changes.empty()){
		std::vector<CacheChange_t*>::iterator it;
		SequenceNumber_t minseqnum = requested_changes[0]->sequenceNumber;

		for(it=requested_changes.begin();it!=requested_changes.end();it++){
			if(minseqnum > (*it)->sequenceNumber){
				minseqnum = (*it)->sequenceNumber;
				cpoin = *it;
			}
		}
		return true;
	}
	return false;
}

bool ReaderLocator::next_unsent_change(CacheChange_t* cpoin) {
	if(!unsent_changes.empty()){
		std::vector<CacheChange_t*>::iterator it;
		SequenceNumber_t minseqnum = unsent_changes[0]->sequenceNumber;
		for(it=unsent_changes.begin();it!=unsent_changes.end();it++)
		{
			if(minseqnum > (*it)->sequenceNumber){
				minseqnum = (*it)->sequenceNumber;
				cpoin = *it;
			}
		}
		return true;
	}
	return false;
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

bool ReaderLocator::remove_requested_change(CacheChange_t* cpoin){
	std::vector<CacheChange_t*>::iterator it;
	for(it=requested_changes.begin();it!=requested_changes.end();it++)
	{
		if(cpoin == *it)
		{
			requested_changes.erase(it);
			return true;
		}
	}
	return false;

}

bool ReaderLocator::remove_unsent_change(CacheChange_t* cpoin){
	std::vector<CacheChange_t*>::iterator it;
	for(it=unsent_changes.begin();it!=unsent_changes.end();it++)
	{
		if(cpoin == *it)
		{
			unsent_changes.erase(it);
			return true;
		}
	}
	return false;
}

} /* namespace rtps */
} /* namespace eprosima */


