/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherListener.h	
 */

#ifndef PUBLISHERLISTENER_H_
#define PUBLISHERLISTENER_H_

#include "../rtps/common/Types.h"
#include "../rtps/common/MatchingInfo.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

class Publisher;

/**
 * Class PublisherListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup FASTRTPS_MODULE
 * @snippet fastrtps_example.cpp ex_PublisherListener
 */
class RTPS_DllAPI PublisherListener
{
public:
	PublisherListener(){};
	virtual ~PublisherListener(){};
	/**
	 * This method is called when the Publisher is matched (or unatched) against an endpoint.
	 * @param pub Pointer to the associated Publisher
	 * @param info Information regarding the matched subscriber
	 */
	virtual void onPublicationMatched(Publisher* pub,MatchingInfo& info){};
//	/**
//	 * This method is called when the History is full.
//	 * @param pub Pointer to the Publisher.
//	 */
//	virtual void onHistoryFull(Publisher* pub){};
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PUBLISHERLISTENER_H_ */
