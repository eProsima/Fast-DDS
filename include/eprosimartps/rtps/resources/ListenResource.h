/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ListenResource.h
 *
 */

#ifndef LISTENRESOURCE_H_
#define LISTENRESOURCE_H_

#include <vector>


namespace eprosima {
namespace rtps {

class ListenResourceImpl;
class RTPSWriter;
class RTPSReader;

class ListenResource {
public:
	ListenResource();
	virtual ~ListenResource();
	/**
	 * Add an associated enpoint to the list.
	 * @param end Pointer to the endpoint.
	 * @return True if correct.
	 */
	bool addAssociatedEndpoint(Endpoint* end);
	/**
	 * Remove an endpoint from the associated endpoint list.
	 * @param end Pointer to the endpoint.
	 * @return True if correct.
	 */
	bool removeAssociatedEndpoint(Endpoint* end);
	ListenResourceImpl* mp_impl;
	MessageReceiver m_receiver;
	std::vector<RTPSWriter*> m_assocWriters;
	std::vector<RTPSReader*> m_assocReaders;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* LISTENRESOURCE_H_ */
