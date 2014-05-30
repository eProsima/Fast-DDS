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
 *      		grcanosa@gmail.com
 */

#include "eprosimartps/writer/ReaderLocator.h"
#include "eprosimartps/common/CacheChange.h"
#include "eprosimartps/HistoryCache.h"

namespace eprosima {
namespace rtps {

ReaderLocator::ReaderLocator() {
	this->expectsInlineQos = false;

}

ReaderLocator::ReaderLocator(Locator_t& a_locator, bool expectsQos){
	locator = a_locator;
	expectsInlineQos = expectsQos;
}

ReaderLocator::~ReaderLocator() {

	//pDebugInfo("ReaderLocator destructor"<<endl;);
}

bool ReaderLocator::next_requested_change(CacheChange_t** cpoin) {
	if(!requested_changes.empty()){
		std::vector<CacheChange_t*>::iterator it;
		SequenceNumber_t minseqnum = requested_changes[0]->sequenceNumber;

		for(it=requested_changes.begin();it!=requested_changes.end();++it){
			if(minseqnum > (*it)->sequenceNumber){
				minseqnum = (*it)->sequenceNumber;
				*cpoin = *it;
			}
		}
		return true;
	}
	return false;
}

bool ReaderLocator::next_unsent_change(CacheChange_t** cpoin) {
	if(!unsent_changes.empty()){
		std::vector<CacheChange_t*>::iterator it;
		std::vector<CacheChange_t*>::iterator it2;
		SequenceNumber_t minseqnum = unsent_changes[0]->sequenceNumber;
		(*cpoin) = unsent_changes[0];
		it2 = it;
		for(it=unsent_changes.begin();it!=unsent_changes.end();++it)
		{
			if(minseqnum > (*it)->sequenceNumber)
			{
				minseqnum = (*it)->sequenceNumber;
				(*cpoin) = *it;
				it2 = it;
			}
		}
		return true;
	}
	return false;
}

void ReaderLocator::requested_changes_set(std::vector<SequenceNumber_t>& seqs,GUID_t& myGUID,HistoryCache* history) {
	std::vector<SequenceNumber_t>::iterator it;
	requested_changes.clear();
	for(it = seqs.begin();it!=seqs.end();++it)
	{
		CacheChange_t** cpoin = NULL;

		if(history->get_change(*it,myGUID,cpoin))
			requested_changes.push_back(*cpoin);
	}
}

bool ReaderLocator::remove_requested_change(CacheChange_t* cpoin){
	std::vector<CacheChange_t*>::iterator it;
	for(it=requested_changes.begin();it!=requested_changes.end();++it)
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
	for(it=unsent_changes.begin();it!=unsent_changes.end();++it)
	{
		if(cpoin->sequenceNumber.to64long() == (*it)->sequenceNumber.to64long())
		{
			unsent_changes.erase(it);
			return true;
		}
	}
	return false;
}

} /* namespace rtps */
} /* namespace eprosima */


