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

#include <thread>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace clientserver;

EprosimaClientTest::EprosimaClientTest()
    : m_overhead(0)
{
    // TODO Auto-generated constructor stub

    m_client.init();

    m_t1 = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; i++)
    {
        m_t2 = std::chrono::steady_clock::now();
    }
    m_overhead = std::chrono::duration_cast<std::chrono::microseconds>(m_t2 - m_t1).count() / 1001.;
    std::cout << "Overhead " << m_overhead << std::endl;
}

EprosimaClientTest::~EprosimaClientTest()
{
    // TODO Auto-generated destructor stub
}

double EprosimaClientTest::run(
        int samples)
{
    while (!m_client.isReady())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    int32_t res;
    m_t1 = std::chrono::steady_clock::now();
    int isam = 0;
    for (isam = 0; isam < samples; ++isam)
    {
        if (m_client.calculate(Operation::ADDITION, 10, 20, &res) != Result::GOOD_RESULT)
        {
            break;
        }
    }
    m_t2 = std::chrono::steady_clock::now();
    if (isam == samples)
    {
        return (std::chrono::duration_cast<std::chrono::microseconds>(m_t2 - m_t1).count() - m_overhead) / samples;
    }
    return -1;
}
