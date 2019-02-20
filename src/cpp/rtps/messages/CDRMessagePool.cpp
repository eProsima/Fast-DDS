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
 * @file CDRMessagePool.cpp
 *
 */

#include <fastrtps/rtps/messages/CDRMessagePool.h>

#include <mutex>

namespace eprosima {
namespace fastrtps{
namespace rtps {


CDRMessagePool::CDRMessagePool(uint32_t defaultGroupsize):
    m_group_size((uint16_t)defaultGroupsize)
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
    std::unique_lock<std::mutex> lock(mutex_);
    if(m_free_objects.empty())
        allocateGroup();
    CDRMessage_t* msg = m_free_objects.back();
    m_free_objects.erase(m_free_objects.end()-1);
    return *msg;
}

CDRMessage_t& CDRMessagePool::reserve_CDRMsg(uint16_t payload)
{
    std::unique_lock<std::mutex> lock(mutex_);
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
    std::unique_lock<std::mutex> lock(mutex_);
    m_free_objects.push_back(&obj);
}
}
} /* namespace rtps */
} /* namespace eprosima */
