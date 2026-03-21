// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
//
#ifndef LOG_MACROS_HPP
#define LOG_MACROS_HPP

#include <thread>

#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>

#include "../mock/MockConsumer.h"

using namespace eprosima::fastdds::dds;
using namespace std;

class LogMacrosTests : public ::testing::Test
{
public:

    LogMacrosTests()
    {
        Log::ClearConsumers();
        mockConsumer = new MockConsumer();
        Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
        Log::SetVerbosity(Log::Info);

        // Register consumer to see log in stdout
        std::unique_ptr<StdoutConsumer> stdout_consumer(new StdoutConsumer());
        Log::RegisterConsumer(std::move(stdout_consumer));
    }

    ~LogMacrosTests()
    {
        Log::Reset();
        Log::KillThread();
    }

    MockConsumer* mockConsumer;

    const uint32_t AsyncTries = 5;
    const uint32_t AsyncWaitMs = 25;

    std::vector<Log::Entry> HELPER_WaitForEntries(
            uint32_t amount)
    {
        for (uint32_t i = 0; i != AsyncTries; i++)
        {
            auto entries = mockConsumer->ConsumedEntries();
            if (entries.size() == amount)
            {
                return entries;
            }
            this_thread::sleep_for(chrono::milliseconds(AsyncWaitMs));
        }

        return mockConsumer->ConsumedEntries();
    }

    void non_valid_function(
            int& n)
    {
        n++;
    }

};

#endif // ifndef LOG_MACROS_HPP
