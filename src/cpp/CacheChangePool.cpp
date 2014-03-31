/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CacheChangePool.cpp
 *
 *  Created on: Mar 26, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/CacheChangePool.h"

namespace eprosima {
namespace rtps {


CacheChangePool::~CacheChangePool()
{
	pDebugInfo("ChangePool destructor"<<endl);
	for(std::vector<CacheChange_t*>::iterator it = allCaches.begin();
			it!=allCaches.end();++it)
	{
		delete(*it);
	}
}

CacheChangePool::CacheChangePool(uint16_t pool_size_in, uint32_t payload_size_in)
{
	pDebugInfo("Creating CacheChangePool of size: "<<pool_size_in << " with payload of size: " << payload_size_in << endl);
	payload_size = payload_size_in;
	pool_size = pool_size_in;
	allocateGroup(pool_size_in);
}

CacheChange_t* CacheChangePool::reserve_Cache()
{
	if(freeCaches.empty())
		allocateGroup(pool_size/10);
	CacheChange_t* ch = freeCaches.front();
	freeCaches.erase(freeCaches.begin());
	return ch;
}

void CacheChangePool::release_Cache(CacheChange_t* ch)
{
	freeCaches.push_back(ch);
}

void CacheChangePool::allocateGroup(uint16_t group_size)
{
pDebugInfo("Allocating memory"<<endl);
	for(uint16_t i = 0;i<group_size;i++)
	{
		CacheChange_t* ch = new CacheChange_t(payload_size);
		allCaches.push_back(ch);
		freeCaches.push_back(ch);
	}
	pDebugInfo("Finish allocating memory"<<endl);
}

} /* namespace rtps */
} /* namespace eprosima */
