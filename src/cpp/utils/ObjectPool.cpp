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
 * @file ObjectPool.cpp
 *
 */

#include <fastrtps/utils/ObjectPool.h>
#include <fastrtps/common/types/CDRMessage_t.h>


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
