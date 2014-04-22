/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ObjectPool.cpp
 *
 *  Created on: Mar 28, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/utils/ObjectPool.h"
#include "eprosimartps/rtps_all.h"

namespace eprosima {
namespace rtps {

template <typename T>
ObjectPool<T>::ObjectPool(uint16_t defaultGroupsize):
	m_group_size(defaultGroupsize)
{
	allocateGroup();
}


template <typename T>
void ObjectPool<T>::allocateGroup()
{
	for(uint16_t i=0;i<m_group_size;++i)
	{
		T* newObject = new T();
		m_free_objects.push_back(newObject);
		m_all_objects.push_back(newObject);
	}
}

template <typename T>
ObjectPool<T>::~ObjectPool()
{

	for(typename std::vector<T*>::iterator it=m_all_objects.begin();
			it!=m_all_objects.end();++it)
	{
		delete(*it);
	}
}


template<typename T>
inline T& ObjectPool<T>::reserve_Object()
{
	if(m_free_objects.empty())
		allocateGroup();
	T* obj = *m_free_objects.begin();
	m_free_objects.erase(m_free_objects.begin());
	return *obj;
}

template<typename T>
inline void ObjectPool<T>::release_Object(T& obj)
{
	m_free_objects.push_back(&obj);
}

template class ObjectPool<CDRMessage_t>;


} /* namespace rtps */
} /* namespace eprosima */
