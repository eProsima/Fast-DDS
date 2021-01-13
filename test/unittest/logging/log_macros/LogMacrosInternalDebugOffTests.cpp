
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
#define HAVE_LOG_NO_INFO 1

#ifdef HAVE_LOG_NO_WARNING
#undef HAVE_LOG_NO_WARNING
#endif // HAVE_LOG_NO_WARNING
#define HAVE_LOG_NO_WARNING 1

#ifdef HAVE_LOG_NO_ERROR
#undef HAVE_LOG_NO_ERROR
#endif // HAVE_LOG_NO_ERROR
#define HAVE_LOG_NO_ERROR 1

#ifdef __INTERNALDEBUG
#undef __INTERNALDEBUG
#endif // __INTERNALDEBUG
#define __INTERNALDEBUG 0

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include "../mock/MockConsumer.h"
#include <gtest/gtest.h>

using namespace eprosima::fastdds::dds;
using namespace std;

class LogMacrosInternalDebugOffTests : public ::testing::Test
{
public:

    LogMacrosInternalDebugOffTests()
    {
        Log::ClearConsumers();
        mockConsumer = new MockConsumer();
        Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
        Log::SetVerbosity(Log::Info);
        std::unique_ptr<StdoutConsumer> stdout_consumer(new StdoutConsumer());
        Log::RegisterConsumer(std::move(stdout_consumer));
        Log::SetVerbosity(Log::Info);
    }

    ~LogMacrosInternalDebugOffTests()
    {
        Log::Reset();
        Log::KillThread();
    }

    MockConsumer* mockConsumer;

    const uint32_t AsyncTries = 5;
    const uint32_t AsyncWaitMs = 25;

    std::vector<Log::Entry> HELPER_WaitForEntries(
            uint32_t amount);

    void internal_debug_ERROR_function(
            int& i);
};

std::vector<Log::Entry> LogMacrosInternalDebugOffTests::HELPER_WaitForEntries(
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

void LogMacrosInternalDebugOffTests::internal_debug_ERROR_function(
        int& i)
{
    i++;
}

/*
 * WARNING: If this test is not properly working, the expected behaviour would be a compilation failure
 * This test try to send in a macro a value (void) that is not convertible to string, so the compilation
 * must fail if INTERNAL_DEBUG is ON or any HAVE_LOG_NO_... is OFF
 */
TEST_F(LogMacrosInternalDebugOffTests, internal_debug_off)
{
    int n = 0;
    logError(SampleCategory, internal_debug_auxiliar_function(n));
    logWarning(SampleCategory, internal_debug_auxiliar_function(n));
    logInfo(SampleCategory, internal_debug_auxiliar_function(n));

    auto consumedEntries = HELPER_WaitForEntries(0);
    ASSERT_EQ(0u, consumedEntries.size());
    ASSERT_EQ(n, 0);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
