/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Publisher.h
 *
 */

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include "fastrtps/config/fastrtps_dll.h"
#include <cstdio>
#include "fastrtps/rtps/common/Guid.h"

namespace eprosima {
namespace fastrtps {

namespace rtps
{
class GUID_t;
}
using namespace rtps;


class PublisherImpl;

class RTPS_DllAPI Publisher {
public:
	Publisher(PublisherImpl* pimpl);
	virtual ~Publisher();
	/**
	 * Write data to the topic.
	 * @param Data Pointer to the data
	 * @return True if correct
	 * @par Calling example:
	 * @snippet fastrtps_example.cpp ex_PublisherWrite
	 */
	bool write(void*Data);

	/**
	 * Dispose of a previously written data.
	 * @param Data Pointer to the data.
	 * @return True if correct.
	 */
	bool dispose(void*Data);
	/**
	 * Unregister a previously written data.
	 * @param Data Pointer to the data.
	 * @return True if correct.
	 */
	bool unregister(void*Data);
	/**
	 * Dispose and unregister a previously written data.
	 * @param Data Pointer to the data.
	 * @return True if correct.
	 */
	bool dispose_and_unregister(void*Data);

	bool removeAllChange(size_t* removed = nullptr);

	const GUID_t& getGuid();

private:
	PublisherImpl* mp_impl;
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* PUBLISHER_H_ */
