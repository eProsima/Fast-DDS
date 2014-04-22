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

Publisher::Publisher(RTPSWriter* Win):
		mp_Writer(Win),
		mp_type(NULL)
{
	// TODO Auto-generated constructor stub
}

Publisher::~Publisher() {
	// TODO Auto-generated destructor stub
	pDebugInfo("Publisher destructor"<<endl;);
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
	if(kind != ALIVE && mp_Writer->topicKind == NO_KEY)
	{
		pWarning("NOT ALIVE change in NO KEY Topic "<<endl)
		return false;
	}

	CacheChange_t* change;
	mp_Writer->new_change(kind,Data,&change);
	pDebugInfo("New change created"<<endl);
	if(!mp_Writer->m_writer_cache.add_change(change))
	{
		pWarning("Change not added"<<endl);
		mp_Writer->m_writer_cache.release_Cache(change);
		return false;
	}

	//DO SOMETHING ONCE THE NEW HCANGE HAS BEEN ADDED.

	if(mp_Writer->m_stateType == STATELESS)
		((StatelessWriter*)mp_Writer)->unsent_change_add(change);
	else if(mp_Writer->m_stateType == STATEFUL)
	{
		((StatefulWriter*)mp_Writer)->unsent_change_add(change);
	}

	return true;
}


bool Publisher::removeMinSeqChange()
{
	boost::lock_guard<HistoryCache> guard(mp_Writer->m_writer_cache);

	if(!mp_Writer->m_writer_cache.m_changes.empty())
	{
		SequenceNumber_t sn;
		GUID_t gui;
		mp_Writer->m_writer_cache.get_seq_num_min(&sn,&gui);
		mp_Writer->m_writer_cache.remove_change(sn,gui);
		return true;
	}

	pWarning("No changes in History"<<endl)
	return false;
}

bool Publisher::removeAllChange()
{
	boost::lock_guard<HistoryCache> guard(mp_Writer->m_writer_cache);
	return mp_Writer->m_writer_cache.remove_all_changes();
}

int Publisher::getHistory_n()
{
	boost::lock_guard<HistoryCache> guard(mp_Writer->m_writer_cache);
	return mp_Writer->m_writer_cache.m_changes.size();
}

bool Publisher::addReaderLocator(Locator_t& Loc,bool expectsInlineQos)
{
	if(mp_Writer->m_stateType==STATELESS)
	{
		ReaderLocator RL;
		RL.expectsInlineQos = expectsInlineQos;
		RL.locator = Loc;
		pDebugInfo("Adding ReaderLocator at: "<< RL.locator.to_IP4_string()<<":"<<RL.locator.port<< endl);
		((StatelessWriter*)mp_Writer)->reader_locator_add(RL);
	}
	else if(mp_Writer->m_stateType==STATEFUL)
	{
		pError("StatefulWriter expects Reader Proxies"<<endl);
		return false;
	}
	return true;
}


bool Publisher::addReaderProxy(Locator_t& loc,GUID_t& guid,bool expectsInline)
{
	if(mp_Writer->m_stateType==STATELESS)
	{
		pError("StatelessWriter expects reader locator"<<endl);
		return false;
	}
	else if(mp_Writer->m_stateType==STATEFUL)
	{
		ReaderProxy_t RL;
		RL.expectsInlineQos = expectsInline;
		RL.remoteReaderGuid = guid;
		RL.unicastLocatorList.push_back(loc);
		pDebugInfo("Adding ReaderProxy at: "<< loc.to_IP4_string()<<":"<<loc.port<< endl);
		((StatefulWriter*)mp_Writer)->matched_reader_add(RL);
		return true;
	}
	return false;
}

const std::string& Publisher::getTopicName()
{
	return mp_Writer->getTopicName();
}

const std::string& Publisher::getTopicDataType()
{
	return mp_Writer->getTopicDataType();
}



} /* namespace dds */
} /* namespace eprosima */


