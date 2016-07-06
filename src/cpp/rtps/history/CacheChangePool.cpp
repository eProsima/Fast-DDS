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
 * @file CacheChangePool.cpp
 *
 */

#include <fastrtps/rtps/history/CacheChangePool.h>
#include <fastrtps/rtps/common/CacheChange.h>
#include <fastrtps/utils/RTPSLog.h>

#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>


namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "CacheChangePool";

CacheChangePool::~CacheChangePool()
{
	const char* const METHOD_NAME = "~CacheChangePool";
	logInfo(RTPS_UTILS,"ChangePool destructor");
	//Deletion process does not depend on the memory management policy
	for(std::vector<CacheChange_t*>::iterator it = m_allCaches.begin();it!=m_allCaches.end();++it)
	{
		delete(*it);
	}
	delete(mp_mutex);
}

CacheChangePool::CacheChangePool(int32_t pool_size, uint32_t payload_size, int32_t max_pool_size, MemoryManagementPolicy_t policy) : mp_mutex(new boost::mutex()), memoryMode(policy)
{
	boost::lock_guard<boost::mutex> guard(*this->mp_mutex);
	const char* const METHOD_NAME = "CacheChangePool";
	logInfo(RTPS_UTILS,"Creating CacheChangePool of size: "<<pool_size << " with maximum payload size: " << payload_size);
	
	//Common for all modes: Set the payload size (maximum allowed), size and size limit
	m_payload_size = payload_size;
	m_pool_size = 0;
	if(max_pool_size > 0)
	{
		if (pool_size > max_pool_size)
		{
			m_max_pool_size = (uint32_t)abs(pool_size);
		}
		else
			m_max_pool_size = (uint32_t)abs(max_pool_size);
	}
	else
		m_max_pool_size = 0;

	switch(memoryMode)
	{
		case PREALLOCATED_MEMORY_MODE:
			logInfo(RTPS_UTILS,"Static Mode is active, preallocating memory for pool_size elements");
			allocateGroup(pool_size);
			break;
		case DYNAMIC_RESERVE_MEMORY_MODE:
			logInfo(RTPS_UTILS,"Dynamic Mode is active, CacheChanges are allocated on request");
			break;	
	}
}

bool CacheChangePool::reserve_Cache(CacheChange_t** chan)
{
	boost::lock_guard<boost::mutex> guard(*this->mp_mutex);
	switch(memoryMode)
	{
		case PREALLOCATED_MEMORY_MODE:
			if(m_freeCaches.empty())
			{
				if (!allocateGroup((uint16_t)(ceil((float)m_pool_size / 10) + 10)))
				{
					return false;
				}
			}
			*chan = m_freeCaches.back();
			m_freeCaches.erase(m_freeCaches.end()-1);
			break;
		case DYNAMIC_RESERVE_MEMORY_MODE:
			*chan = allocateSingle(); //Allocates a single, empty CacheChange. Allocated on Copy
			break;
	}
	return true;	
}

void CacheChangePool::release_Cache(CacheChange_t* ch)
{
	const char* const METHOD_NAME = "release_Cache";
	boost::lock_guard<boost::mutex> guard(*this->mp_mutex);

	switch(memoryMode)
	{
		case PREALLOCATED_MEMORY_MODE:
			ch->kind = ALIVE;
			ch->sequenceNumber.high = 0;
			ch->sequenceNumber.low = 0;
			ch->writerGUID = c_Guid_Unknown;
			ch->serializedPayload.length = 0;
			ch->serializedPayload.pos = 0;
			for(uint8_t i=0;i<16;++i)
				ch->instanceHandle.value[i] = 0;
			ch->isRead = 0;
			ch->sourceTimestamp.seconds = 0;
			ch->sourceTimestamp.fraction = 0;
			m_freeCaches.push_back(ch);
			break;
		case DYNAMIC_RESERVE_MEMORY_MODE:
			// Find pointer in CacheChange vector, remove element, then delete it
			std::vector<CacheChange_t*>::iterator target = m_allCaches.begin();	
			target = find(m_allCaches.begin(),m_allCaches.end(), ch);
			if(target != m_allCaches.end())
			{
				m_allCaches.erase(target);
			}else{
				logInfo(RTPS_UTILS,"Tried to release a CacheChange that is not logged in the Pool");
				break;
			}
			delete(ch);	
			--m_pool_size;
			break;

	}
}

bool CacheChangePool::allocateGroup(uint32_t group_size)
{
	const char* const METHOD_NAME = "allocateGroup";
	// This method should only called from within PREALLOCATED_MEMORY_MODE
	if(memoryMode != PREALLOCATED_MEMORY_MODE)
	{
		logInfo(RTPS_UTILS,"Illegal call to allocateGroup. CacheChangePool is not in PREALLOCATED_MEMORY_MODE");
		return false;
	}
	logInfo(RTPS_UTILS,"Allocating group of cache changes of size: "<< group_size);
	bool added = false;
	uint32_t reserved = 0;
	if (m_max_pool_size == 0)
		reserved = group_size;
	else
	{
		if (m_pool_size + group_size > m_max_pool_size)
		{
			reserved = m_max_pool_size - m_pool_size;
		}
		else
		{
			reserved = group_size;
		}
	}
	for(uint32_t i = 0;i<reserved;i++)
	{
			CacheChange_t* ch = new CacheChange_t(m_payload_size);
			ch->serializedPayload.memoryMode = memoryMode;
			m_allCaches.push_back(ch);
			m_freeCaches.push_back(ch);
			++m_pool_size;
			added = true;
	}
	if (!added)
		logWarning(RTPS_HISTORY, "Maximum number of allowed reserved caches reached");
	//logInfo(RTPS_UTILS,"Finish allocating CacheChange_t");
	return added;
}

CacheChange_t* CacheChangePool::allocateSingle()
{
	/*
	 *   In Dynamic Memory Mode CacheChanges are only allocated when they are needed.
	 *   This means when the buffer of the message receiver is copied into this struct, the size is allocated.
	 *   When the change is released and comes back to the pool, it is deallocated correspondingly.
	 *
	 *   In Preallocated mode, changes are allocated with a static maximum size and then they are dealt as
	 *   they are needed. In Dynamic mode, they are only allocated when they are needed. In Dynamic mode only
	 *   the m_allCaches vector is used, in order to keep track of all the changes that are dealt for destruction
	 *   purposes.
	 *
	 */
	const char * const METHOD_NAME = "allocateSingle";
	bool added = false;
	CacheChange_t*ch = nullptr;

	// This method should only be called from within DYNAMIC_RESERVE_MEMORY_MODE
	if(memoryMode != DYNAMIC_RESERVE_MEMORY_MODE)
	{
		logInfo(RTPS_UTILS, "Illegal call to allocateSingle. ChacheChangePool is not in DYNAMIC_RESERVE_MEMORY_MODE");
		return NULL;
	}
	if( (m_max_pool_size == 0) | (m_pool_size < m_max_pool_size) ){
		++m_pool_size;
		ch = new CacheChange_t(1);
		//This can be done freely since this is only executed in Dynamic Mode
		ch->serializedPayload.empty();
		ch->serializedPayload.memoryMode = memoryMode;
		ch->serializedPayload.max_size =m_payload_size; 
		m_allCaches.push_back(ch);
		added = true;
	}
	if(!added)
	{
		logWarning(RTPS_HISTORY, "Maximum number of allowed reserved caches reached");
		return NULL;
	}
		return ch;

}

}
} /* namespace rtps */
} /* namespace eprosima */
