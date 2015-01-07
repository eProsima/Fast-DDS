/*************************************************************************
 * Copyright (c) 2015 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MetaTestSubscriber.h
 *
 */

#ifndef METATESTSUBSCRIBER_H_
#define METATESTSUBSCRIBER_H_

#include "fastrtps/fastrtps_fwd.h"
using namespace eprosima::fastrtps;

namespace eprosima {

class MetaTestSubscriber {
public:
	MetaTestSubscriber();
	virtual ~MetaTestSubscriber();
	bool init();
	void run();

private:
	Publisher* mp_pub;
	Subscriber* mp_sub;
};

} /* namespace eprosima */

#endif /* METATESTSUBSCRIBER_H_ */
