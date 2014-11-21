/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Publisher.h
 *
 */

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

namespace eprosima {
namespace pubsub {

class PublisherImpl;

class Publisher {
public:
	Publisher(PublisherImpl* pimpl);
	virtual ~Publisher();
	/**
	 * Write data to the topic.
	 * @param Data Pointer to the data
	 * @return True if correct
	 * @par Calling example:
	 * @snippet pubsub_example.cpp ex_PublisherWrite
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

private:
	PublisherImpl* mp_impl;
};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* PUBLISHER_H_ */
