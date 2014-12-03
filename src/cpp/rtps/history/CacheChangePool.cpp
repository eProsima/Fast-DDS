/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CacheChangePool.cpp
 *
 */

#include "fastrtps/rtps/history/CacheChangePool.h"
#include "fastrtps/rtps/common/CacheChange.h"
#include "fastrtps/utils/RTPSLog.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "CacheChangePool";

CacheChangePool::~CacheChangePool()
{
	const char* const METHOD_NAME = "~CacheChangePool";
	logInfo(RTPS_UTILS,"ChangePool destructor");
	for(std::vector<CacheChange_t*>::iterator it = m_allCaches.begin();
			it!=m_allCaches.end();++it)
	{
		delete(*it);
	}
}

CacheChangePool::CacheChangePool(uint16_t pool_size,uint32_t payload_size,int32_t max_pool_size)
{
	const char* const METHOD_NAME = "CacheChangePool";
	logInfo(RTPS_UTILS,"Creating CacheChangePool of size: "<<pool_size << " with payload of size: " << payload_size);
	m_payload_size = payload_size;
	m_pool_size = 0;
	m_max_pool_size = max_pool_size;
	allocateGroup(pool_size);
}

bool CacheChangePool::reserve_Cache(CacheChange_t** chan)
{
	if(m_freeCaches.empty())
	{
		if(!allocateGroup((uint16_t)(ceil((float)m_pool_size/10)+10)))
			return false;
	}
	*chan = m_freeCaches.front();
	m_freeCaches.erase(m_freeCaches.begin());
	return true;
}

void CacheChangePool::release_Cache(CacheChange_t* ch)
{
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
}

bool CacheChangePool::allocateGroup(uint16_t group_size)
{
	const char* const METHOD_NAME = "allocateGroup";
	logInfo(RTPS_UTILS,"Allocating group of cache changes of size: "<< group_size);
	bool added = false;
	for(uint16_t i = 0;i<group_size;i++)
	{
		if(m_max_pool_size < 0 || m_pool_size < m_max_pool_size)
		{
			CacheChange_t* ch = new CacheChange_t(m_payload_size);
			m_allCaches.push_back(ch);
			m_freeCaches.push_back(ch);
			++m_pool_size;
			added = true;
		}
		else
		{
			logWarning(RTPS_HISTORY,"Maximum number of allowed reserved caches reached");
			break;
		}
	}
	//logInfo(RTPS_UTILS,"Finish allocating CacheChange_t");
	return added;
}
}
} /* namespace rtps */
} /* namespace eprosima */
