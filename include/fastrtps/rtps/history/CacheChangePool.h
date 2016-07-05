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
 * @file CacheChangePool.h
 *
 */



#ifndef CACHECHANGEPOOL_H_
#define CACHECHANGEPOOL_H_

#include <fastrtps/rtps/resources/ResourceManagement.h>

#include <vector>
#include <cstdint>
#include <cstddef>

namespace boost
{
	class mutex;
}

namespace eprosima {
namespace fastrtps{
namespace rtps {

struct CacheChange_t;

/**
 * Class CacheChangePool, used by the HistoryCache to pre-reserve a number of CacheChange_t to avoid dynamically reserving memory in the middle of execution loops.
 * @ingroup COMMON_MODULE
 */
class CacheChangePool {
public:
	virtual ~CacheChangePool();
	/**
	 * Constructor.
	* @param pool_size The initial pool size
	* @param payload_size The payload size associated with the pool.
	* @param max_pool_size Maximum payload size. If set to 0 the pool will keep reserving until something breaks.
	*/
	CacheChangePool(int32_t pool_size, uint32_t payload_size, int32_t max_pool_size, MemoryManagementPolicy_t policy);
	//!Reserve a Cache from the pool.
	bool reserve_Cache(CacheChange_t** chan);
	//!Release a Cache back to the pool.
	void release_Cache(CacheChange_t*);
	//!Get the size of the cache vector; all of them (reserved and not reserved).
	size_t get_allCachesSize(){return m_allCaches.size();}
	//!Get the number of frre caches.
	size_t get_freeCachesSize(){return m_freeCaches.size();}
	//!Get the payload size associated with the Pool.
	inline uint32_t getPayloadSize(){return m_payload_size;};
private:
	uint32_t m_payload_size;
	uint32_t m_pool_size;
	uint32_t m_max_pool_size;
	std::vector<CacheChange_t*> m_freeCaches;
	std::vector<CacheChange_t*> m_allCaches;
	bool allocateGroup(uint32_t pool_size);
	CacheChange_t* allocateSingle();
	boost::mutex* mp_mutex;
	MemoryManagementPolicy_t memoryMode;
};
}
} /* namespace rtps */
} /* namespace eprosima */


#endif /* CACHECHANGEPOOL_H_ */
