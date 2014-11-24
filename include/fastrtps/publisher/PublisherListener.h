/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherListener.h	
 */

#ifndef PUBLISHERLISTENER_H_
#define PUBLISHERLISTENER_H_

#include "fastrtps/rtps/common/Types.h"
#include "fastrtps/rtps/common/MatchingInfo.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

/**
 * Class PublisherListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup FASTRTPSMODULE
 * @snippet fastrtps_example.cpp ex_PublisherListener
 */
class RTPS_DllAPI PublisherListener
{
public:
	PublisherListener(){};
	virtual ~PublisherListener(){};
	/**
	 * This method is called when the Publisher is matched (or unatched) against an endpoint.
	 */
	virtual void onPublicationMatched(MatchingInfo info){};
	/**
	 * This method is called when the History is full.
	 */
	virtual void onHistoryFull(){};
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PUBLISHERLISTENER_H_ */
