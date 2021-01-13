
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

#ifdef HAVE_LOG_NO_INFO
#undef HAVE_LOG_NO_INFO
#endif // HAVE_LOG_NO_INFO
#define HAVE_LOG_NO_INFO 0

#ifdef HAVE_LOG_NO_WARNING
#undef HAVE_LOG_NO_WARNING
#endif // HAVE_LOG_NO_WARNING
#define HAVE_LOG_NO_WARNING 0

#ifdef HAVE_LOG_NO_ERROR
#undef HAVE_LOG_NO_ERROR
#endif // HAVE_LOG_NO_ERROR
#define HAVE_LOG_NO_ERROR 1

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include "../mock/MockConsumer.h"
#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;
using namespace std;

class LogMacrosNoErrorTests : public ::testing::Test
{
public:

    LogMacrosNoErrorTests()
    {
        Log::ClearConsumers();
        mockConsumer = new MockConsumer();
        Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
        Log::SetVerbosity(Log::Info);
        std::unique_ptr<StdoutConsumer> stdout_consumer(new StdoutConsumer());
        Log::RegisterConsumer(std::move(stdout_consumer));
        Log::SetVerbosity(Log::Info);
    }

    ~LogMacrosNoErrorTests()
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

std::vector<Log::Entry> LogMacrosNoErrorTests::HELPER_WaitForEntries(
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

/* Check error is not consumed */
TEST_F(LogMacrosNoErrorTests, no_error)
{
    logError(SampleCategory, "Sample error message");
    logWarning(SampleCategory, "Sample warning message");
    logInfo(SampleCategory, "Sample info message");

    auto consumedEntries = HELPER_WaitForEntries(2);
    ASSERT_EQ(2u, consumedEntries.size());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
