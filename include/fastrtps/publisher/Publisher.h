/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Publisher.h
 *
 */

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include "../fastrtps_dll.h"
#include <cstdio>
#include "../rtps/common/Guid.h"
#include "../rtps/common/Time_t.h"

namespace eprosima {
namespace fastrtps {

namespace rtps
{
struct GUID_t;
class WriteParams;
}
using namespace rtps;


class PublisherImpl;

/**
 * Class Publisher, used to send data to associated subscribers.
 * @ingroup FASTRTPS_MODULE
 */
class RTPS_DllAPI Publisher {
	friend class PublisherImpl;
	virtual ~Publisher();
public:
	Publisher(PublisherImpl* pimpl);

	/**
	 * Write data to the topic.
	 * @param Data Pointer to the data
	 * @return True if correct
	 * @par Calling example:
	 * @snippet fastrtps_example.cpp ex_PublisherWrite
	 */
	bool write(void*Data);

	/**
	 * Write data with params to the topic.
	 * @param Data Pointer to the data
    * @param wparams Extra write parameters.
	 * @return True if correct
	 * @par Calling example:
	 * @snippet fastrtps_example.cpp ex_PublisherWrite
	 */
	bool write(void*Data, WriteParams &wparams);

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

	/**
	 * Remove all the Changes in the associated RTPSWriter.
	 * @param[out] removed Number of elements removed
	 * @return True if all elements were removed.
	 */
	bool removeAllChange(size_t* removed = nullptr);

    bool wait_for_all_acked(const Time_t& max_wait);

	/**
	 * Get the GUID_t of the associated RTPSWriter.
	 * @return GUID_t.
	 */
	const GUID_t& getGuid();

private:
	PublisherImpl* mp_impl;
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* PUBLISHER_H_ */
