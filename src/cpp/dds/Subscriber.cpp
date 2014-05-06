/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Subscriber.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/reader/StatefulReader.h"

namespace eprosima {
namespace dds {



Subscriber::Subscriber(RTPSReader* Rin):
		mp_Reader(Rin)
{

}


Subscriber::~Subscriber()
{
	pDebugInfo("Subscriber destructor"<<endl;);
}


void Subscriber::waitForUnreadMessage()
{
	if(!mp_Reader->isUnreadCacheChange())
		mp_Reader->m_semaphore.wait();
	mp_Reader->m_semaphore.reset();

}

void Subscriber::assignListener(SubscriberListener* p_listener)
{
	mp_Reader->mp_listener = p_listener;
}

bool Subscriber::isHistoryFull()
{
	return mp_Reader->m_reader_cache.isFull();
}


int Subscriber::getHistory_n()
{
	return mp_Reader->m_reader_cache.m_changes.size();
}


bool Subscriber::readNextData(void* data,SampleInfo_t* info)
{
	return this->mp_Reader->readNextCacheChange(data,info);
}

bool Subscriber::takeNextData(void* data,SampleInfo_t* info) {
	return this->mp_Reader->takeNextCacheChange(data,info);
}


bool Subscriber::addWriterProxy(Locator_t& loc, GUID_t& guid)
{
	if(mp_Reader->m_stateType==STATELESS)
	{
		pError("StatelessReader cannot have writerProxy"<<endl);
		return false;
	}
	else if(mp_Reader->m_stateType==STATEFUL)
	{
		WriterProxy_t WL;
		WL.unicastLocatorList.push_back(loc);
		WL.remoteWriterGuid = guid;
		pDebugInfo("Adding WriterProxy at: "<< loc.to_IP4_string()<<":"<< loc.port<< endl);
		((StatefulReader*)mp_Reader)->matched_writer_add(&WL);
		return true;
	}
	return false;
}

//bool Subscriber::updateParameters(const SubscriberAttributes& param)
//{
//
//	return true;
//}


} /* namespace dds */
} /* namespace eprosima */


