/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ApacheClientTest.h
 *
 */

#ifndef APACHECLIENTTEST_H_
#define APACHECLIENTTEST_H_

#include "fastrtps/rtps_all.h"

class ApacheClientTest
{
public:
	ApacheClientTest():m_overhead(0){};
	~ApacheClientTest(){};
	double run(int samples);
	eClock m_clock;
	Time_t m_t1,m_t2;
	double m_overhead;
};



#endif /* APACHECLIENTTEST_H_ */
