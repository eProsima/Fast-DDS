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
 * @file EprosimaClientTest.cpp
 *
 */

#include "EprosimaClientTest.h"
#include "fastrtps/utils/TimeConversion.h"


EprosimaClientTest::EprosimaClientTest():m_overhead(0) {
    // TODO Auto-generated constructor stub

    m_client.init();



    m_clock.setTimeNow(&m_t1);
    for(int i=0;i<1000;i++)
        m_clock.setTimeNow(&m_t2);
    m_overhead = (TimeConv::Time_t2MicroSecondsDouble(m_t2)-TimeConv::Time_t2MicroSecondsDouble(m_t1))/1001;
    std::cout << "Overhead " << m_overhead << std::endl;
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
