/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ApacheClient.h
 *
 */

#ifndef APACHECLIENT_H_
#define APACHECLIENT_H_

#include <string>

using namespace std;

class ApacheClientTest
{
public:
	ApacheClientTest():m_overhead(0){};
	~ApacheClientTest(){};
	double run(string ip, int samples, int bytes);
	double m_overhead;
};



#endif /* APACHECLIENT_H_ */
