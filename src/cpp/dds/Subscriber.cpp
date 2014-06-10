/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Subscriber.cpp
 *
*/

#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/reader/WriterProxy.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/dds/DDSTopicDataType.h"

#include "eprosimartps/dds/SubscriberListener.h"


namespace eprosima {
namespace dds {



SubscriberImpl::SubscriberImpl(RTPSReader* Rin,DDSTopicDataType* ptype):
		mp_Reader(Rin),
		mp_type(ptype)
{

}


SubscriberImpl::~SubscriberImpl()
{
	pDebugInfo("Subscriber destructor"<<endl;);
}


void SubscriberImpl::waitForUnreadMessage()
{
	if(!mp_Reader->isUnreadCacheChange())
	{
		pDebugInfo("No Unread CacheChange, waiting..."<<endl);
		mp_Reader->m_semaphore.wait();
	}
//mp_Reader->m_semaphore.reset();

}

bool SubscriberImpl::assignListener(SubscriberListener* p_listener)
{
	mp_Reader->mp_listener = p_listener;
	return true;
}

bool SubscriberImpl::isHistoryFull()
{
	return mp_Reader->isHistoryFull();
}


int SubscriberImpl::getHistoryElementsNumber()
{
	return mp_Reader->getHistoryCacheSize();
}

size_t SubscriberImpl::getMatchedPublishers()
{return mp_Reader->getMatchedPublishers();};


bool SubscriberImpl::readNextData(void* data,SampleInfo_t* info)
{
	return this->mp_Reader->readNextCacheChange(data,info);
}

bool SubscriberImpl::takeNextData(void* data,SampleInfo_t* info) {
	return this->mp_Reader->takeNextCacheChange(data,info);
}


bool SubscriberImpl::addWriterProxy(Locator_t& loc, GUID_t& guid)
{
	if(mp_Reader->getStateType()==STATELESS)
	{
		pError("StatelessReader cannot have writerProxy"<<endl);
		return false;
	}
	else if(mp_Reader->getStateType()==STATEFUL)
	{
		WriterProxy_t WL;
		WL.unicastLocatorList.push_back(loc);
		WL.remoteWriterGuid = guid;
		pDebugInfo("Adding WriterProxy at: "<< loc.to_IP4_string()<<":"<< loc.port<< endl);
		((StatefulReader*)mp_Reader)->matched_writer_add(WL);
		return true;
	}
	return false;
}

const GUID_t& SubscriberImpl::getGuid(){
	return mp_Reader->getGuid();
}

//bool Subscriber::updateParameters(const SubscriberAttributes& param)
//{
//
//	return true;
//}


} /* namespace dds */
} /* namespace eprosima */


