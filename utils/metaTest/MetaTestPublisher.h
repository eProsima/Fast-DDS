/*************************************************************************
 * Copyright (c) 2015 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MetaTestPublisher.h
 *
 */

#ifndef METATESTPUBLISHER_H_
#define METATESTPUBLISHER_H_

#include "fastrtps/fastrtps_fwd.h"
using namespace eprosima::fastrtps;

namespace eprosima {

class MetaTestPublisher {
public:
	MetaTestPublisher();
	virtual ~MetaTestPublisher();
	bool init();
	void run();

private:
	Publisher* mp_pub;
	Subscriber* mp_sub;
};

} /* namespace eprosima */

#endif /* METATESTPUBLISHER_H_ */
