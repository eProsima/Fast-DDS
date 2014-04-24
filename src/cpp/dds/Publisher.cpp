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

}

Publisher::~Publisher() {

	pDebugInfo("Publisher destructor"<<endl;);
}

bool Publisher::write(void* Data) {

	pInfo("Writing new data"<<endl)

	return mp_Writer->add_new_change(ALIVE,Data);
}

bool Publisher::dispose(void* Data) {

	pInfo("Disposing of Data"<<endl)

	return mp_Writer->add_new_change(NOT_ALIVE_DISPOSED,Data);
}


bool Publisher::unregister(void* Data) {
	//Convert data to serialized Payload
	pInfo("Unregistering of Data"<<endl)

	return mp_Writer->add_new_change(NOT_ALIVE_UNREGISTERED,Data);
}


bool Publisher::removeMinSeqChange()
{
	return mp_Writer->removeMinSeqCacheChange();
}

bool Publisher::removeAllChange(int32_t* removed)
{
	return mp_Writer->removeAllCacheChange(removed);
}

int Publisher::getHistory_n()
{
	return mp_Writer->getHistoryCacheSize();
}

bool Publisher::addReaderLocator(Locator_t& Loc,bool expectsInlineQos)
{
	if(mp_Writer->getStateType()==STATELESS)
	{
		ReaderLocator RL;
		RL.expectsInlineQos = expectsInlineQos;
		RL.locator = Loc;
		pDebugInfo("Adding ReaderLocator at: "<< RL.locator.to_IP4_string()<<":"<<RL.locator.port<< endl);
		((StatelessWriter*)mp_Writer)->reader_locator_add(RL);
	}
	else if(mp_Writer->getStateType()==STATEFUL)
	{
		pError("StatefulWriter expects Reader Proxies"<<endl);
		return false;
	}
	return true;
}


bool Publisher::addReaderProxy(Locator_t& loc,GUID_t& guid,bool expectsInline)
{
	if(mp_Writer->getStateType()==STATELESS)
	{
		pError("StatelessWriter expects reader locator"<<endl);
		return false;
	}
	else if(mp_Writer->getStateType()==STATEFUL)
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


