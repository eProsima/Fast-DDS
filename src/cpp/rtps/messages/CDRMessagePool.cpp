/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CDRMessagePool.cpp
 *
 */

#include <fastrtps/rtps/messages/CDRMessagePool.h>

#include <boost/thread/mutex.hpp>

namespace eprosima {
namespace fastrtps{
namespace rtps {


CDRMessagePool::CDRMessagePool(uint32_t defaultGroupsize):
m_group_size(defaultGroupsize), mutex_(nullptr)
{
	allocateGroup();
    mutex_ = new boost::mutex();
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

    if(mutex_ != nullptr)
        delete mutex_;
}


CDRMessage_t& CDRMessagePool::reserve_CDRMsg()
{
    boost::unique_lock<boost::mutex> lock(*mutex_);
	if(m_free_objects.empty())
		allocateGroup();
	CDRMessage_t* msg = m_free_objects.back();
	m_free_objects.erase(m_free_objects.end()-1);
	return *msg;
}

CDRMessage_t& CDRMessagePool::reserve_CDRMsg(uint16_t payload)
{
    boost::unique_lock<boost::mutex> lock(*mutex_);
	if(m_free_objects.empty())
		allocateGroup(payload);
	CDRMessage_t* msg = m_free_objects.back();
	if(msg->max_size-RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE<payload)
	{
		msg = new CDRMessage_t(payload+RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE);
		m_all_objects.push_back(msg);
	}
	else
	{
		m_free_objects.erase(m_free_objects.end()-1);
	}
	return *msg;
}


void CDRMessagePool::release_CDRMsg(CDRMessage_t& obj)
{
    boost::unique_lock<boost::mutex> lock(*mutex_);
	m_free_objects.push_back(&obj);
}
}
} /* namespace rtps */
} /* namespace eprosima */
