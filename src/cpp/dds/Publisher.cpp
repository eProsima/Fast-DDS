/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Publisher.cpp
 *
 */

#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/dds/DDSTopicDataType.h"
#include "eprosimartps/dds/PublisherListener.h"
#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/writer/ReaderLocator.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/writer/ReaderProxy.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace dds {

PublisherImpl::PublisherImpl(RTPSWriter* Win,DDSTopicDataType*p):
		mp_Writer(Win),
		mp_type(p)
{

}

PublisherImpl::~PublisherImpl() {

	pDebugInfo("Publisher destructor"<<endl;);
}

bool PublisherImpl::write(void* Data) {

	pInfo("Writing new data"<<endl)
	return mp_Writer->add_new_change(ALIVE,Data);
}

bool PublisherImpl::dispose(void* Data) {

	pInfo("Disposing of Data"<<endl)
	return mp_Writer->add_new_change(NOT_ALIVE_DISPOSED,Data);
}


bool PublisherImpl::unregister(void* Data) {
	//Convert data to serialized Payload
	pInfo("Unregistering of Data"<<endl)
	return mp_Writer->add_new_change(NOT_ALIVE_UNREGISTERED,Data);
}

bool PublisherImpl::dispose_and_unregister(void* Data) {
	//Convert data to serialized Payload
	pInfo("Disposing and Unregistering Data"<<endl)
	return mp_Writer->add_new_change(NOT_ALIVE_DISPOSED_UNREGISTERED,Data);
}


bool PublisherImpl::removeMinSeqChange()
{
	return mp_Writer->removeMinSeqCacheChange();
}

bool PublisherImpl::removeAllChange(size_t* removed)
{
	return mp_Writer->removeAllCacheChange(removed);
}

size_t PublisherImpl::getHistoryElementsNumber()
{
	return mp_Writer->getHistoryCacheSize();
}

size_t PublisherImpl::getMatchedSubscribers()
{
	return mp_Writer->getMatchedSubscribers();
}


bool PublisherImpl::addReaderLocator(Locator_t& Loc,bool expectsInlineQos)
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


bool PublisherImpl::addReaderProxy(Locator_t& loc,GUID_t& guid,bool expectsInline)
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


bool PublisherImpl::assignListener(PublisherListener* listen_in)
{
	mp_Writer->setListener(listen_in);
	return true;
}

const GUID_t& PublisherImpl::getGuid()
	{
		return mp_Writer->getGuid();
	}

} /* namespace dds */
} /* namespace eprosima */


