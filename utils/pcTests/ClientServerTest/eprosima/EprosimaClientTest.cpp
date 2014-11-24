/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EprosimaClientTest.cpp
 *
 */

#include "EprosimaClientTest.h"

EprosimaClientTest::EprosimaClientTest():m_overhead(0) {
	// TODO Auto-generated constructor stub

	m_client.init();



	m_clock.setTimeNow(&m_t1);
	for(int i=0;i<1000;i++)
		m_clock.setTimeNow(&m_t2);
	m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
	cout << "Overhead " << m_overhead << endl;
}

EprosimaClientTest::~EprosimaClientTest() {
	// TODO Auto-generated destructor stub
}


double EprosimaClientTest::run(int samples)
{
	while(!m_client.isReady())
	{
		eClock::my_sleep(100);
	}
	int32_t res;
	m_clock.setTimeNow(&m_t1);
	int isam = 0;
	for(isam = 0;isam<samples;++isam)
	{
		if(m_client.calculate(Operation::ADDITION,10,20,&res) != Result::GOOD_RESULT)
			break;
	}
	m_clock.setTimeNow(&m_t2);
	if(isam == samples)
	{
		return (TimeConv::Time_t2MicroSecondsDouble(m_t2)-
				TimeConv::Time_t2MicroSecondsDouble(m_t1)-
				m_overhead)/samples;
	}
	return -1;
}
