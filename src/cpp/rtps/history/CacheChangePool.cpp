/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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
	for(std::vector<CacheChange_t*>::iterator it = m_allCaches.begin();
			it!=m_allCaches.end();++it)
	{
		delete(*it);
	}
	delete(mp_mutex);
}

CacheChangePool::CacheChangePool(int32_t pool_size, uint32_t payload_size, int32_t max_pool_size) : mp_mutex(new boost::mutex())
{
	boost::lock_guard<boost::mutex> guard(*this->mp_mutex);
	const char* const METHOD_NAME = "CacheChangePool";
	logInfo(RTPS_UTILS,"Creating CacheChangePool of size: "<<pool_size << " with payload of size: " << payload_size);
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
	//cout << "CREATING CACHECHANGEPOOL WIHT MAX: " << m_max_pool_size << " and pool size: " << pool_size << endl;
	allocateGroup(pool_size);
}

bool CacheChangePool::reserve_Cache(CacheChange_t** chan)
{
	boost::lock_guard<boost::mutex> guard(*this->mp_mutex);
	if(m_freeCaches.empty())
	{
		if (!allocateGroup((uint16_t)(ceil((float)m_pool_size / 10) + 10)))
		{
			return false;
		}
	}
	*chan = m_freeCaches.back();
	m_freeCaches.erase(m_freeCaches.end()-1);
	return true;
}

void CacheChangePool::release_Cache(CacheChange_t* ch)
{
	boost::lock_guard<boost::mutex> guard(*this->mp_mutex);
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

bool CacheChangePool::allocateGroup(uint32_t group_size)
{
	const char* const METHOD_NAME = "allocateGroup";
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
}
} /* namespace rtps */
} /* namespace eprosima */
