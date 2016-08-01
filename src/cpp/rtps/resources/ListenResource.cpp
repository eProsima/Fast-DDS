// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file ListenResource.cpp
 *
 */

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/reader/RTPSReader.h>

#include <fastrtps/rtps/resources/ListenResource.h>
#include "ListenResourceImpl.h"
#include <fastrtps/rtps/messages/MessageReceiver.h>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {


ListenResource::ListenResource(RTPSParticipantImpl* partimpl,uint32_t ID,bool isDefault):
				mp_receiver(nullptr),
				mp_RTPSParticipantImpl(partimpl),
				m_ID(ID),
				m_isDefaultListenResource(isDefault)
{
	// TODO Auto-generated constructor stub
	mp_impl = new ListenResourceImpl(this);

}

ListenResource::~ListenResource() {
	// TODO Auto-generated destructor stub

	if(mp_impl!=nullptr)
		delete(mp_impl);
	if(mp_receiver !=nullptr)
		delete(mp_receiver);
}


bool ListenResource::removeAssociatedEndpoint(Endpoint* endp)
{
	boost::lock_guard<boost::mutex> guard(*this->getMutex());
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
			if((*rit)->getGuid().entityId == endp->getGuid().entityId)
			{
				m_assocReaders.erase(rit);
				return true;
			}
		}
	}
	return false;
}

bool ListenResource::addAssociatedEndpoint(Endpoint* endp)
{
	boost::lock_guard<boost::mutex> guard(*mp_impl->getMutex());
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
			logInfo(RTPS_MSG_IN,endp->getGuid().entityId << " added to: "<< mp_impl->getListenLocators());
			return true;
		}
	}
	else if(endp->getAttributes()->endpointKind == READER)
	{
		for(std::vector<RTPSReader*>::iterator rit = m_assocReaders.begin();rit!=m_assocReaders.end();++rit)
		{
			if((*rit)->getGuid().entityId == endp->getGuid().entityId)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			m_assocReaders.push_back((RTPSReader*)endp);
			logInfo(RTPS_MSG_IN,endp->getGuid().entityId << " added to: "
					<< mp_impl->getListenLocators());
			return true;
		}
	}
	return false;
}

void ListenResource::setMsgRecMsgLength(uint32_t length)
{
	mp_receiver->m_rec_msg.length = length;
}

bool ListenResource::init_thread(RTPSParticipantImpl* pimpl,Locator_t& loc,
		uint32_t listenSockSize,bool isMulti,bool isFixed)
{
	logInfo(RTPS_MSG_IN,"Creating ListenResource in: "<<loc << " with ID: "<< m_ID);
	if(!IsAddressDefined(loc) && isMulti)
	{
		logWarning(RTPS_MSG_IN,"MulticastAddresses need to have the IP defined, ignoring this address");
		return false;
	}
	mp_receiver = new MessageReceiver(listenSockSize);
	mp_receiver->mp_threadListen = this;
	return mp_impl->init_thread(pimpl,loc,listenSockSize,isMulti,isFixed);
}

bool ListenResource::isListeningTo(Locator_t&loc)
{
	return mp_impl->isListeningTo(loc);
}

const LocatorList_t& ListenResource::getListenLocators()
{
	return mp_impl->getListenLocators();
}

boost::mutex* ListenResource::getMutex(){return mp_impl->getMutex();}



}} /* namespace rtps */
} /* namespace eprosima */
