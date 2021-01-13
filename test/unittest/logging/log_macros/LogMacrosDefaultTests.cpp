
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

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include "../mock/MockConsumer.h"
#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;
using namespace std;

class LogMacrosDefaultTests : public ::testing::Test
{
public:

    LogMacrosDefaultTests()
    {
        Log::ClearConsumers();
        mockConsumer = new MockConsumer();
        Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
        Log::SetVerbosity(Log::Info);
        std::unique_ptr<StdoutConsumer> stdout_consumer(new StdoutConsumer());
        Log::RegisterConsumer(std::move(stdout_consumer));
        Log::SetVerbosity(Log::Info);
    }

    ~LogMacrosDefaultTests()
    {
        Log::Reset();
        Log::KillThread();
    }

    MockConsumer* mockConsumer;

    const uint32_t AsyncTries = 5;
    const uint32_t AsyncWaitMs = 25;

    std::vector<Log::Entry> HELPER_WaitForEntries(
            uint32_t amount);
};

std::vector<Log::Entry> LogMacrosDefaultTests::HELPER_WaitForEntries(
        uint32_t amount)
{
    size_t entries = 0;
    for (uint32_t i = 0; i != AsyncTries; i++)
    {
        entries = mockConsumer->ConsumedEntries().size();
        if (entries == amount)
        {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(AsyncWaitMs));
    }

    return mockConsumer->ConsumedEntries();
}

/* Check all log levels are active in debug mode, or INFO is not active in Release mode */
TEST_F(LogMacrosDefaultTests, default_macros_test)
{
    logError(SampleCategory, "Sample error message");
    logWarning(SampleCategory, "Sample warning message");
    logInfo(SampleCategory, "Sample info message");

    unsigned int expected_result = 3;

#if !(__DEBUG || _DEBUG)
    --expected_result;
#endif // CMAKE_BUILD_TYPE == DEBUG_TYPE

    auto consumedEntries = HELPER_WaitForEntries(expected_result);
    ASSERT_EQ(expected_result, consumedEntries.size());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
