/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CDRMessagePool.cpp
 *
 */

#include "eprosimartps/utils/CDRMessagePool.h"

namespace eprosima {
namespace rtps {


CDRMessagePool::CDRMessagePool(uint32_t defaultGroupsize):
			m_group_size(defaultGroupsize)
{
	allocateGroup();
}



void CDRMessagePool::allocateGroup()
{
	for(uint16_t i=0;i<m_group_size;++i)
	{
		CDRMessage_t* newObject = new CDRMessage_t();
		m_free_objects.push_back(newObject);
		m_all_objects.push_back(newObject);
	}
}
void CDRMessagePool::allocateGroup(uint16_t payload)
{
	for(uint16_t i=0;i<m_group_size;++i)
	{
		CDRMessage_t* newObject = new CDRMessage_t(payload+RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE);
		m_free_objects.push_back(newObject);
		m_all_objects.push_back(newObject);
	}
}


CDRMessagePool::~CDRMessagePool()
{

	for(std::vector<CDRMessage_t*>::iterator it=m_all_objects.begin();
			it!=m_all_objects.end();++it)
	{
		delete(*it);
	}
}


CDRMessage_t& CDRMessagePool::reserve_CDRMsg()
{
	if(m_free_objects.empty())
		allocateGroup();
	CDRMessage_t* msg = *m_free_objects.begin();
	m_free_objects.erase(m_free_objects.begin());
	return *msg;
}

CDRMessage_t& CDRMessagePool::reserve_CDRMsg(uint16_t payload)
{
	if(m_free_objects.empty())
		allocateGroup(payload);
	CDRMessage_t* msg = *m_free_objects.begin();
	if(msg->max_size-RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE<payload)
	{
		msg = new CDRMessage_t(payload+RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE);
		m_all_objects.push_back(msg);
	}
	else
	{
		m_free_objects.erase(m_free_objects.begin());
	}
	return *msg;
}


void CDRMessagePool::release_CDRMsg(CDRMessage_t& obj)
{
	m_free_objects.push_back(&obj);
}

} /* namespace rtps */
} /* namespace eprosima */
