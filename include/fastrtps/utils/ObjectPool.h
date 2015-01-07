/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ObjectPool.h
 *
 */

#ifndef OBJECTPOOL_H_
#define OBJECTPOOL_H_

#include <vector>
#include <cstdint>


namespace eprosima {
namespace fastrtps{
namespace rtps {


/**
 * ObjectPool class used to define an object pool of different types.
 * @ingroup UTILITIESMODULE
 */
template <typename T>
class ObjectPool {
public:
	/**
	* @param defaultGroupSize Default group size
	*/
	ObjectPool(uint16_t defaultGroupSize);
	virtual ~ObjectPool();
	T& reserve_Object();
	void release_Object(T& obj);
protected:
	std::vector<T*> m_free_objects;
	std::vector<T*> m_all_objects;
	uint16_t m_group_size;
	void allocateGroup();

};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* OBJECTPOOL_H_ */
