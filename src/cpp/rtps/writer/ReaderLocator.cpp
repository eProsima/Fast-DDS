/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file ReaderLocator.cpp
 *
 */

#include <fastrtps/rtps/writer/ReaderLocator.h>
#include <fastrtps/rtps/common/CacheChange.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>


namespace eprosima {
namespace fastrtps{
namespace rtps {

ReaderLocator::ReaderLocator() {
	this->expectsInlineQos = false;
	n_used = 1;

}

ReaderLocator::ReaderLocator(Locator_t& a_locator, bool expectsQos ){
	locator = a_locator;
	expectsInlineQos = expectsQos;
	n_used = 1;
}

ReaderLocator::~ReaderLocator()
{
}

bool ReaderLocator::remove_requested_change(const CacheChange_t* cpoin){
	std::vector<const CacheChange_t*>::iterator it;
	for(it=requested_changes.begin();it!=requested_changes.end();++it)
	{
		if(cpoin == *it)
		{
			requested_changes.erase(it);
			return true;
		}
	}
	return false;

}

}
} /* namespace rtps */
} /* namespace eprosima */


