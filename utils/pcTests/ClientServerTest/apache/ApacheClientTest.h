// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
