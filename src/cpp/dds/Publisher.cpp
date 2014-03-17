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
#include "eprosimartps/StatefulWriter.h"
#include "eprosimartps/ReaderProxy.h"

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
	return add_new_change(ALIVE,Data);
}

bool Publisher::dispose(void* Data) {
	//Convert data to serialized Payload
	RTPSLog::Info << "Disposing of Data" << endl;pI
	return add_new_change(NOT_ALIVE_DISPOSED,Data);
}


bool Publisher::unregister(void* Data) {
	//Convert data to serialized Payload
	RTPSLog::Info << "Unregistering of Data" << endl;
	RTPSLog::printInfo();
	return add_new_change(NOT_ALIVE_UNREGISTERED,Data);
}


bool Publisher::add_new_change(ChangeKind_t kind,void*Data)
{
	if(kind != ALIVE && W->topicKind == NO_KEY)
	{
		RTPSLog::Warning << "NOT ALIVE change in NO KEY Topic " << endl;pW
		return false;
	}

	CacheChange_t change;
	InstanceHandle_t handle;
	SerializedPayload_t Payload;
	if(W->topicKind == WITH_KEY)
	{
		type.getKey(Data,&handle);
	}
	if(kind == ALIVE)
	{
		type.serialize(&Payload,Data);
		W->new_change(kind,&Payload,handle,&change);
	}
	else
		W->new_change(kind,NULL,handle,&change);

	CacheChange_t* ch_ptr;
	if(!W->writer_cache.add_change(&change,&ch_ptr))
		return false;
	//DO SOMETHING ONCE THE NEW HCANGE HAS BEEN ADDED.
	if(W->stateType == STATELESS)
		((StatelessWriter*)W)->unsent_change_add(ch_ptr);
	else if(W->stateType == STATEFUL)
	{
		((StatefulWriter*)W)->unsent_change_add(ch_ptr);
	}
	return true;


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


