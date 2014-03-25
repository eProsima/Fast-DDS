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

#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/writer/ReaderLocator.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/writer/ReaderProxy.h"

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

	pInfo("Writing new data"<<endl)

	return add_new_change(ALIVE,Data);
}

bool Publisher::dispose(void* Data) {

	pInfo("Disposing of Data"<<endl)

	return add_new_change(NOT_ALIVE_DISPOSED,Data);
}


bool Publisher::unregister(void* Data) {
	//Convert data to serialized Payload
	pInfo("Unregistering of Data"<<endl)

	return add_new_change(NOT_ALIVE_UNREGISTERED,Data);
}


bool Publisher::add_new_change(ChangeKind_t kind,void*Data)
{
	if(kind != ALIVE && W->topicKind == NO_KEY)
	{
		pWarning("NOT ALIVE change in NO KEY Topic "<<endl)
		return false;
	}

	CacheChange_t* change;
	W->new_change(kind,Data,&change);

	if(!W->writer_cache.add_change(change))
	{
		pWarning("Change not added"<<endl);
		return false;
	}
	//DO SOMETHING ONCE THE NEW HCANGE HAS BEEN ADDED.

	if(W->stateType == STATELESS)
		((StatelessWriter*)W)->unsent_change_add(change);
	else if(W->stateType == STATEFUL)
	{
		((StatefulWriter*)W)->unsent_change_add(change);
	}

	return true;



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

	pWarning("No changes in History"<<endl)
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
	if(W->stateType==STATELESS)
	{
		ReaderLocator RL;
		RL.expectsInlineQos = expectsInlineQos;
		RL.locator = Loc;
		pDebugInfo("Adding ReaderLocator at: "<< RL.locator.to_IP4_string()<<":"<<RL.locator.port<< endl);
		((StatelessWriter*)W)->reader_locator_add(RL);
	}
	else if(W->stateType==STATEFUL)
	{
		ReaderProxy_t RL;
		RL.expectsInlineQos = expectsInlineQos;
		GUID_UNKNOWN(RL.remoteReaderGuid);
		RL.unicastLocatorList.push_back(Loc);

		pDebugInfo("Adding ReaderProxy at: "<< Loc.to_IP4_string()<<":"<< Loc.port<< endl);

		((StatefulWriter*)W)->matched_reader_add(RL);
	}
	//TODOG add proxy
	return true;
}

std::string Publisher::getTopicName()
	{
		return W->topicName;
	}

	std::string Publisher::getTopicDataType()
	{
		return W->topicDataType;
	}



} /* namespace dds */
} /* namespace eprosima */


