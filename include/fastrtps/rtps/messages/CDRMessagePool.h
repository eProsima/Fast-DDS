/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file CDRMessagePool.h
 *
 */

#ifndef CDRMESSAGEPOOL_H_
#define CDRMESSAGEPOOL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "fastrtps/rtps/common/CDRMessage_t.h"
#include <vector>

namespace eprosima {
namespace fastrtps{
namespace rtps {

/**
*@ingroup COMMON_MODULE
*/
class CDRMessagePool {
public:
	/**
	* @param
	*/
	CDRMessagePool(uint32_t defaultGroupSize);
	virtual ~CDRMessagePool();
	
	//!
	CDRMessage_t& reserve_CDRMsg();
	
	/**
	* @param payload
	*/
	CDRMessage_t& reserve_CDRMsg(uint16_t payload);
	
	/**
	* @param obj
	*/
	void release_CDRMsg(CDRMessage_t& obj);
protected:
	std::vector<CDRMessage_t*> m_free_objects;
	std::vector<CDRMessage_t*> m_all_objects;
	uint16_t m_group_size;
	void allocateGroup();
	void allocateGroup(uint16_t payload);
};





}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* CDRMESSAGEPOOL_H_ */
