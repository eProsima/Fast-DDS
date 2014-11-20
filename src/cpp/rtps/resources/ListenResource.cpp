/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ListenResource.cpp
 *
 */

#include "eprosimartps/rtps/writer/RTPSWriter.h"

#include "eprosimartps/rtps/resources/ListenResource.h"
#include "eprosimartps/rtps/resources/ListenResourceImpl.h"
#include "eprosimartps/rtps/messages/MessageReceiver.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "ListenResource";

ListenResource::ListenResource():
		mp_receiver(nullptr)
{
	// TODO Auto-generated constructor stub
	mp_impl = new ListenResourceImpl(this);

}

ListenResource::~ListenResource() {
	// TODO Auto-generated destructor stub
}


bool ListenResource::removeAssociatedEndpoint(Endpoint* endp)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_impl->getMutex());
	if(endp->getAttributes()->endpointKind == WRITER)
	{
		for(auto wit = m_assocWriters.begin();
				wit!=m_assocWriters.end();++wit)
		{
			if((*wit)->getGuid().entityId == endp->getGuid().entityId)
			{
				m_assocWriters.erase(wit);
				return true;
			}
		}
	}
	else if(endp->getAttributes()->endpointKind == READER)
	{
		for(auto rit = m_assocReaders.begin();rit!=m_assocReaders.end();++rit)
		{
//			if((*rit)->getGuid().entityId == endp->getGuid().entityId)
//			{
//				m_assocReaders.erase(rit);
//				return true;
//			}
		}
	}
	return false;
}

bool ListenResource::addAssociatedEndpoint(Endpoint* endp)
{
	const char* const METHOD_NAME = "addAssociatedEndpoint";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_impl->getMutex());
	bool found = false;
	if(endp->getAttributes()->endpointKind == WRITER)
	{
		for(std::vector<RTPSWriter*>::iterator wit = m_assocWriters.begin();
				wit!=m_assocWriters.end();++wit)
		{
			if((*wit)->getGuid().entityId == endp->getGuid().entityId)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			m_assocWriters.push_back((RTPSWriter*)endp);
			logInfo(RTPS_MSG_IN,endp->getGuid().entityId << " added to: "<< mp_impl->getListenLoc(),C_BLUE);
			return true;
		}
	}
	else if(endp->getAttributes()->endpointKind == READER)
	{
		for(std::vector<RTPSReader*>::iterator rit = m_assocReaders.begin();rit!=m_assocReaders.end();++rit)
		{
//			if((*rit)->getGuid().entityId == endp->getGuid().entityId)
//			{
//				found = true;
//				break;
//			}
		}
		if(!found)
		{
			m_assocReaders.push_back((RTPSReader*)endp);
			logInfo(RTPS_MSG_IN,endp->getGuid().entityId << " added to: "<< mp_impl->getListenLoc(),C_BLUE);
			return true;
		}
	}
	return false;
}

void ListenResource::setMsgRecMsgLength(uint32_t length)
{
	mp_receiver->m_rec_msg.length = length;
}

Locator_t ListenResource::init_thread(ParticipantImpl* pimpl,Locator_t& loc,
			uint32_t listenSockSize,bool isMulti,bool isFixed)
{
	mp_receiver = new MessageReceiver(listenSockSize);
	return mp_impl->init_thread(pimpl,loc,listenSockSize,isMulti,isFixed);
}



} /* namespace rtps */
} /* namespace eprosima */
