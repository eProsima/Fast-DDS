/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CacheChangePool.h
 *
 *  Created on: Mar 26, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */


#include <vector>


#ifndef CACHECHANGEPOOL_H_
#define CACHECHANGEPOOL_H_
#include "eprosimartps/rtps_all.h"


namespace eprosima {
namespace rtps {

class CacheChangePool {
public:
	virtual ~CacheChangePool();
	CacheChangePool(uint16_t pool_size,uint32_t payload_size);
	CacheChange_t* reserve_Cache();
	void release_Cache(CacheChange_t*);
private:
	uint32_t payload_size;
	uint16_t pool_size;
	std::vector<CacheChange_t*> freeCaches;
	std::vector<CacheChange_t*> allCaches;
	void allocateGroup(uint16_t pool_size);
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* CACHECHANGEPOOL_H_ */
