/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Publisher.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/Publisher.h"
#include "eprosimartps/RTPSWriter.h"
#include "eprosimartps/ReaderLocator.h"
#include "eprosimartps/StatelessWriter.h"

namespace eprosima {
namespace dds {

Publisher::Publisher(){

}

Publisher::Publisher(RTPSWriter* Win) {
	// TODO Auto-generated constructor stub
	W = Win;
}

Publisher::~Publisher() {
	// TODO Auto-generated destructor stub
}

bool Publisher::write(void* Data) {
	//Convert data to serialized Payload
	RTPSLog::Info << "Writing New Data" << endl;RTPSLog::printInfo();
	SerializedPayload_t Payload;
	type.serialize(&Payload,Data);
	InstanceHandle_t handle;
	if(W->topicKind == WITH_KEY)
		type.getKey(Data,&handle);
	//create new change
	CacheChange_t change;
	W->new_change(ALIVE,NULL,handle,&change);
	CacheChange_t* ch_ptr;
	if(!W->writer_cache.add_change(&change,&ch_ptr))
		return false;
	if(W->stateType == STATELESS)
		((StatelessWriter*)W)->unsent_change_add(ch_ptr);
	else
	{}
	return true;

}

bool Publisher::dispose(void* Data) {
	//Convert data to serialized Payload
	RTPSLog::Info << "Disposing of Data" << endl;pI
	if(W->topicKind == WITH_KEY)
	{
		CacheChange_t change;
		InstanceHandle_t handle;
		type.getKey(Data,&handle);
		W->new_change(NOT_ALIVE_DISPOSED,NULL,handle,&change);
		CacheChange_t* ch_ptr;
		if(!W->writer_cache.add_change(&change,&ch_ptr))
			return false;
		if(W->stateType == STATELESS)
			((StatelessWriter*)W)->unsent_change_add(ch_ptr);
		else
		{}
		return true;
	}
	RTPSLog::Warning << "Not in NOKEY Topic" << endl;
	RTPSLog::printWarning();
	return false;
}


bool Publisher::unregister(void* Data) {
	//Convert data to serialized Payload
	RTPSLog::Info << "Unregistering of Data" << endl;
	RTPSLog::printInfo();
	if(W->topicKind == WITH_KEY)
	{
		CacheChange_t change;
		InstanceHandle_t handle;
		type.getKey(Data,&handle);
		W->new_change(NOT_ALIVE_UNREGISTERED,NULL,handle,&change);
		CacheChange_t* ch_ptr;
		if(!W->writer_cache.add_change(&change,&ch_ptr))
			return false;
		if(W->stateType == STATELESS)
			((StatelessWriter*)W)->unsent_change_add(ch_ptr);
		else
		{}
		return true;
	}
	RTPSLog::Warning << "Not in NOKEY Topic" << endl;
	RTPSLog::printWarning();
	return false;
}


bool Publisher::removeMinSeqChange()
{
	boost::lock_guard<HistoryCache> guard(W->writer_cache);

	if(!W->writer_cache.changes.empty())
	{
		SequenceNumber_t sn;
		GUID_t gui;
		W->writer_cache.get_seq_num_min(&sn,&gui);
		W->writer_cache.remove_change(sn,gui);

		return true;
	}

	RTPSLog::Warning<< B_RED << "No changes in History"<< DEF << endl;
	RTPSLog::printWarning();
	return false;
}

bool Publisher::removeAllChange()
{
	boost::lock_guard<HistoryCache> guard(W->writer_cache);
	return W->writer_cache.remove_all_changes();
}

int Publisher::getHistory_n()
{
	boost::lock_guard<HistoryCache> guard(W->writer_cache);
	return W->writer_cache.changes.size();
}

bool Publisher::addReaderLocator(Locator_t Loc,bool expectsInlineQos)
{
	ReaderLocator RL;
	RL.expectsInlineQos = expectsInlineQos;
	RL.locator = Loc;
	RTPSLog::Info << "Adding ReaderLocator at: "<< RL.locator.to_IP4_string()<<":"<<RL.locator.port<< endl;
	RTPSLog::printInfo();
	if(W->stateType==STATELESS)
		((StatelessWriter*)W)->reader_locator_add(RL);
	//TODOG add proxy
	return true;
}


} /* namespace dds */
} /* namespace eprosima */


