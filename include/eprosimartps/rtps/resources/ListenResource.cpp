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

#include "ListenResource.h"

#include "eprosimartps/ListenResourceImpl.h"

namespace eprosima {
namespace rtps {

ListenResource::ListenResource()
{
	// TODO Auto-generated constructor stub

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
			logInfo(RTPS_MSG_IN,endp->getGuid().entityId << " added to: "<< mp_impl->getListenLoc(),EPRO_BLUE);
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
			logInfo(RTPS_MSG_IN,endp->getGuid().entityId << " added to: "<< mp_impl->getListenLoc(),EPRO_BLUE);
			return true;
		}
	}
	return false;
}

} /* namespace rtps */
} /* namespace eprosima */
