/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EprosimaClientTest.h
 *
 */

#ifndef EPROSIMACLIENTTEST_H_
#define EPROSIMACLIENTTEST_H_

#include "EprosimaClient.h"

class EprosimaClientTest {
public:
	EprosimaClientTest();
	virtual ~EprosimaClientTest();
	EprosimaClient m_client;
	double run(int samples);
	eClock m_clock;
	Time_t m_t1,m_t2;
	double m_overhead;
};

#endif /* EPROSIMACLIENTTEST_H_ */
