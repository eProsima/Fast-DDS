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
//

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include "mock/MockConsumer.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <sstream>

using namespace eprosima::fastdds::dds;
using namespace std;

class LogTests : public ::testing::Test
{
public:

    LogTests()
    {
        mockConsumer = new MockConsumer();

        Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
        Log::SetVerbosity(Log::Info);
    }

    ~LogTests()
    {
        Log::Reset();
        Log::KillThread();
    }

    void Reset()
    {
        Log::ClearConsumers();
        mockConsumer = new MockConsumer();
        Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
        Log::SetVerbosity(Log::Info);
    }

    MockConsumer* mockConsumer;

    const uint32_t AsyncTries = 5;
    const uint32_t AsyncWaitMs = 25;

    std::vector<Log::Entry> HELPER_WaitForEntries(
            uint32_t amount);
};

TEST_F(LogTests, asynchronous_logging)
{
    logError(SampleCategory, "Sample error message");
    logWarning(SampleCategory, "Sample warning message");
    logWarning(DifferentCategory, "Sample warning message in another category");

    auto consumedEntries = HELPER_WaitForEntries(3);
    ASSERT_EQ(3u, consumedEntries.size());
}

TEST_F(LogTests, reporting_options)
{
    // moving away from the defaults
    Log::ReportFilenames(true);
    Log::ReportFunctions(false);

    logError(Reporting, "Error with different reporting options");
    auto consumedEntries = HELPER_WaitForEntries(1);
    ASSERT_EQ(1u, consumedEntries.size());

    auto entry = consumedEntries.back();
    ASSERT_NE(entry.context.filename, nullptr);
    ASSERT_EQ(entry.context.function, nullptr);
}

TEST_F(LogTests, multithreaded_logging)
{
    vector<unique_ptr<thread>> threads;
    for (int i = 0; i != 5; i++)
    {
        threads.emplace_back(new thread([i]
                {
                    logWarning(Multithread, "I'm thread " << i);
                }));
    }

    for (auto& thread: threads)
    {
        thread->join();
    }

    auto consumedEntries = HELPER_WaitForEntries(5);
    ASSERT_EQ(5u, consumedEntries.size());
}

TEST_F(LogTests, regex_category_filtering)
{
    Log::SetCategoryFilter(std::regex("(Good)"));
    logError(GoodCategory, "This should be logged because my regex filter allows for it");
    logError(BadCategory, "If you're seeing this, something went wrong");
    logWarning(EvenMoreGoodCategory, "This should be logged too!");
    auto consumedEntries = HELPER_WaitForEntries(3);
    ASSERT_EQ(2u, consumedEntries.size());
}

TEST_F(LogTests, multi_criteria_filtering_with_regex)
{
    Log::SetCategoryFilter(std::regex("(Good)"));
    Log::SetFilenameFilter(std::regex("(LogTests)", std::regex_constants::icase));
    Log::SetErrorStringFilter(std::regex("(Good)"));
    Log::ReportFilenames(true); // For clarity, not necessary.

    logError(GoodCategory, "This should be logged because it contains the word \"Good\" in the " \
            "error string and the category, and is in the right filename");
    logError(BadCategory,  "Despite the word \"Good\" being here, this shouldn't be logged");
    logError(GoodCategory, "And neither should this.");
    auto consumedEntries = HELPER_WaitForEntries(3);
    ASSERT_EQ(1u, consumedEntries.size());

    Log::SetFilenameFilter(std::regex("(we shouldn't find this ever)"));
    logError(GoodCategory,  "Despite the word \"Good\" being here, this shouldn't be logged because " \
            "the filename is all wrong");

    consumedEntries = HELPER_WaitForEntries(2);
    ASSERT_EQ(1u, consumedEntries.size());
}

TEST_F(LogTests, multiple_verbosity_levels)
{
    Log::SetVerbosity(Log::Warning);
    logError(VerbosityChecks, "This should be logged");
    logWarning(VerbosityChecks, "This should be logged too!");
    logInfo(VerbosityChecks, "If you're seeing this, something went wrong");
    auto consumedEntries = HELPER_WaitForEntries(3);
    ASSERT_EQ(2u, consumedEntries.size());

    Log::SetVerbosity(Log::Error);
    logError(VerbosityChecks, "This should be logged");
    logWarning(VerbosityChecks, "If you're seeing this, something went wrong");
    logInfo(VerbosityChecks, "If you're seeing this, something went wrong");

    consumedEntries = HELPER_WaitForEntries(5);
    ASSERT_EQ(3u, consumedEntries.size());
}

// 'logless_flush_call' tests rif the Flush() operation may deadlock with an idle log
TEST_F(LogTests, logless_flush_call)
{
    // 1. without any entry log mechanism is deactivated. This shouldn block the Flush()
    Log::Flush();

    // 2. now we activate the log but removed all entries
    logWarning(flush_ckecks, "We must add, at least, an entry to activate the log mechanism.");

    HELPER_WaitForEntries(1); // retrieve the former one to make sure queue is cleaned

    Log::Flush();

    GTEST_SUCCESS_("If we are here there was no deadlock.");
}

/*
    'validate_single_flush_call' tests if the Flush() operation:
 + assures all log entries make to this point are consumed
 + do not deadlocks under heavy log load. Meaning that returns when there is still non consumed entries but that
      were filed after the current flush() call.
 */
TEST_F(LogTests, validate_single_flush_call)
{
    constexpr int threads_number = 5;
    constexpr int wait_milliseconds = 100;
    constexpr int thread_wait_milliseconds = 50;

    /* note that:
     + wait_milliseconds should be larger than thread_wait_milliseconds in order allow some logs to be done
          in each main thread loop.
     + thread_wait_milliseconds should share order of magnitude with the consumer retrieval delay. If not so,
          the back queue will grow so large while the front one is cleared, that the test will probably timeout.
     */

    bool done = false;
    int commited_before_flush = 0;
    // std::atomic<int> committed = 0; // only works on msvc and icc
    std::atomic<int> committed;
    committed = 0;

    // Populate the consumer from multiple threads
    vector<unique_ptr<thread>> threads;
    for (int i = 0; i < threads_number; i++)
    {
        threads.emplace_back(new thread([&done, &committed, &thread_wait_milliseconds]
                {
                    while (!done)
                    {
                        logWarning(flush_ckecks,
                        "I'm thread " << this_thread::get_id() << " logging sample " << committed);
                        // incremented after log
                        ++committed;
                        // wait before add a new log entry
                        this_thread::sleep_for(chrono::milliseconds(thread_wait_milliseconds));
                    }
                }));
    }

    // allow some logs to be done but not all
    while (commited_before_flush < threads_number)
    {
        this_thread::sleep_for(chrono::milliseconds(wait_milliseconds));

        // committed value before the Flush
        commited_before_flush = committed;
    }

    // Wait till the queues are empty
    Log::Flush();

    int consumed = static_cast<int>(mockConsumer->ConsumedEntries().size());
    // Flush doesn't wait for all log entries to finish but for the logged till its called
    // We must assert that at least commited_before_flush have been delivered
    ASSERT_GE(consumed, commited_before_flush);

    logWarning(flush_ckecks, "Flushing successful, consumed: "
            << consumed << " commited till flush " << commited_before_flush);

    done = true; // direct threads to shut-down

    for (auto& thread : threads)
    {
        thread->join();
    }
}

/*
    'validate_multithread_flush_calls' tests if the Flush() operation:
 + assures all log entries make to this point are consumed
 + do not deadlocks under heavy log load. Meaning that returns when there is still non consumed entries but that
      were filed after the current flush() call.
 + simultaneous Flush() calls from several threads do not interfere with each other operation
 */
TEST_F(LogTests, validate_multithread_flush_calls)
{
    constexpr int working_threads_number = 5;
    constexpr int flushing_threads_number = 3;
    constexpr int wait_milliseconds = 100;
    constexpr int thread_wait_milliseconds = 50;

    /* note that:
     + wait_milliseconds should be larger than thread_wait_milliseconds in order allow some logs to be done
          in each main thread loop.
     + thread_wait_milliseconds should share order of magnitude with the consumer retrieval delay. If not so,
          the back queue will grow so large while the front one is cleared, that the test will probably timeout.
     */

    bool done = false;
    // std::atomic<int> committed = 0; // only works on msvc and icc
    std::atomic<int> committed;
    committed = 0;

    // Populate the consumer from multiple threads
    vector<unique_ptr<thread>> threads;
    for (int i = 0; i < working_threads_number; i++)
    {
        threads.emplace_back(new thread([&done, &committed, &thread_wait_milliseconds]
                {
                    while (!done)
                    {
                        logWarning(flush_ckecks,
                        "I'm thread " << this_thread::get_id() << " logging sample " << committed);
                        // incremented after log
                        ++committed;
                        // wait before add a new log entry
                        this_thread::sleep_for(chrono::milliseconds(thread_wait_milliseconds));
                    }
                }));
    }

    {
        // allow some logs to be done but not all
        int currently_commited = 0;
        while (currently_commited < working_threads_number)
        {
            this_thread::sleep_for(chrono::milliseconds(wait_milliseconds));

            // committed value before the Flush
            currently_commited = committed;
        }
    }

    // Populate the consumer from multiple threads
    for (int i = 0; i < flushing_threads_number; i++)
    {
        threads.emplace_back(new thread([this, &committed]
                {
                    logWarning(flush_ckecks, "I'm thread " << this_thread::get_id() << " Flushing " << committed);

                    int commited_before_flush = ++committed;

                    // Wait till the queues are empty
                    Log::Flush();

                    int consumed = static_cast<int>(mockConsumer->ConsumedEntries().size());

                    // Flush doesn't wait for all log entries to finish but for the logged till its called
                    // We must assert that at least commited_before_flush have been delivered
                    ASSERT_GE(consumed, commited_before_flush);

                    logWarning(flush_ckecks,
                    "I'm thread " << this_thread::get_id() << " Flushing successful, consumed: "
                                  << consumed << " commited till flush " <<
                        commited_before_flush);

                }));
    }

    done = true; // direct threads to shut-down

    for (auto& thread : threads)
    {
        thread->join();
    }
}

/*
 * This test checks that the log messages go to the appropriate buffer (STDOUT) when using a StdoutConsumer.
 * 1. Set a StdoutConsumer as the only log consumer.
 * 2. Redirect std::cout and std::cerr to a stream buffer.
 * 3. Log a messages in every log level and wait until all logs are consumed.
 * 4. Reset std::cout to STDOUT and std::cerr to STDCERR.
 * 5. Check the number of messages in both stream buffers.
 * 6. Ensure that all the messages have been written to STDOUT and there is no message in STDERR.
 */
TEST_F(LogTests, stdout_consumer_stream)
{
    /*
     * Set a StdoutConsumer consumer
     *   1. Remove all previous consumers
     *   2. Create a StdoutConsumer
     *   3. Register the consumer
     */
    Log::ClearConsumers();
    StdoutConsumer* consumer = new StdoutConsumer;
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(consumer));

    // Redirect std::cout to a stream buffer
    std::stringstream out_stream_out;
    std::stringstream out_stream_err;
    streambuf* stream_buffer_out = std::cout.rdbuf(out_stream_out.rdbuf());
    streambuf* stream_buffer_err = std::cerr.rdbuf(out_stream_err.rdbuf());

    // Log messages on all levels and wait until they are all consumed
    logError(stdout_consumer_stream, "Error message");
    logWarning(stdout_consumer_stream, "Warning message");
    logInfo(stdout_consumer_stream, "Info message");
    Log::Flush();
    std::cout.flush();
    std::cerr.flush();

    // Wait for all lines to be consumed before reseting the buffers
    std::string out_string_out = out_stream_out.str();
    std::string::difference_type lines_out = std::count(out_string_out.begin(), out_string_out.end(), '\n');

#if !HAVE_LOG_NO_INFO
    while (lines_out < 3)
#else
    while (lines_out < 2)
#endif // !HAVE_LOG_NO_INFO
    {
        Log::Flush();
        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1));

        out_string_out = out_stream_out.str();
        lines_out = std::count(out_string_out.begin(), out_string_out.end(), '\n');
    }

    // Reset std::cout to STDOUT
    std::cout.rdbuf(stream_buffer_out);
    std::cerr.rdbuf(stream_buffer_err);

    // Count number of lines in the buffer. There is one line per each log message output to that buffer
    out_string_out = out_stream_out.str();
    std::string out_string_err = out_stream_err.str();
    lines_out = std::count(out_string_out.begin(), out_string_out.end(), '\n');
    std::string::difference_type lines_err = std::count(out_string_err.begin(), out_string_err.end(), '\n');

    // If CMAKE_BUILD_TYPE is Debug, the INTERNAL_DEBUG flag was set, and the logInfo messages were not deactivated,
    // then there should be 3 messages in the out buffer, one for the logError, one for the logWarning, and another one
    // for the logInfo.
    // Else, there should only be 2 messagea in the out buffer, corresponding to logError and logWarning.
#if !HAVE_LOG_NO_INFO
    ASSERT_EQ(3, lines_out);
#else
    ASSERT_EQ(2, lines_out);
#endif // if !HAVE_LOG_NO_INFO
    ASSERT_EQ(0, lines_err);
    std::cout << "Number of messages in the out buffer is correct: " << lines_out << std::endl;
    std::cout << "No messages in the err buffer: " << lines_err << std::endl;

    // Reset the log module to the test default
    Reset();
}

/*
 * This test checks that the log messages go to the appropriate buffer (STDOUT or STDERR) when using a StdoutErrConsumer.
 * 1. Set a StdoutErrConsumer as the only log consumer, setting the STDERR threshold level to Error.
 * 2. Redirect std::cout and std::cerr to two stream buffers.
 * 3. Log a messages in every log level and wait until all logs are consumed.
 * 4. Reset std::cout and std::cerr to STDOUT and STDERR respectively.
 * 5. Check the number of messages in each of the stream buffers.
 */
TEST_F(LogTests, stdouterr_consumer_stream)
{
    /*
     * Set a StdoutErrConsumer consumer
     *   1. Remove all previous consumers
     *   2. Create a StdoutErrConsumer
     *   3. Set STDERR threshold to Error. This way Error and more severe event will be output to STDERR, whilst less
     *      severe will be output to STDOUT
     *   4. Register the consumer
     */
    Log::ClearConsumers();
    StdoutErrConsumer* consumer = new StdoutErrConsumer;
    consumer->stderr_threshold(Log::Kind::Error);
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(consumer));

    // Redirect std::cout and std::cerr to stream buffers
    std::stringstream out_stream_err;
    std::stringstream out_stream_out;
    streambuf* stream_buffer_err = std::cerr.rdbuf(out_stream_err.rdbuf());
    streambuf* stream_buffer_out = std::cout.rdbuf(out_stream_out.rdbuf());

    // Log messages on all levels and wait until they are all consumed
    logError(stdouterr_consumer_stream, "Error message");
    logWarning(stdouterr_consumer_stream, "Warning message");
    logInfo(stdouterr_consumer_stream, "Info message");
    Log::Flush();
    std::cout.flush();
    std::cerr.flush();

    // Wait for all lines to be consumed before reseting the buffers
    std::string out_string_err = out_stream_err.str();
    std::string out_string_out = out_stream_out.str();
    std::string::difference_type lines_err = std::count(out_string_err.begin(), out_string_err.end(), '\n');
    std::string::difference_type lines_out = std::count(out_string_out.begin(), out_string_out.end(), '\n');

#if !HAVE_LOG_NO_INFO
    while (lines_err < 1 || lines_out < 2)
#else
    while (lines_err < 1 || lines_out < 1)
#endif // if !HAVE_LOG_NO_INFO
    {
        Log::Flush();
        std::cout.flush();
        std::cerr.flush();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1));

        out_string_out = out_stream_out.str();
        out_string_err = out_stream_err.str();
        lines_out = std::count(out_string_out.begin(), out_string_out.end(), '\n');
        lines_err = std::count(out_string_err.begin(), out_string_err.end(), '\n');
    }

    // Reset std::cout and std::cerr to STDOUT and STDERR respectively
    std::cout.rdbuf(stream_buffer_out);
    std::cerr.rdbuf(stream_buffer_err);

    // Count number of lines in each of the buffers. There is one line per each log message output to that buffer
    out_string_err = out_stream_err.str();
    out_string_out = out_stream_out.str();
    lines_err = std::count(out_string_err.begin(), out_string_err.end(), '\n');
    lines_out = std::count(out_string_out.begin(), out_string_out.end(), '\n');

    // Only the logError message should be in the error buffer, since stderr_threshold was set to Log::Kind::Error.
    ASSERT_EQ(1, lines_err);
    std::cout << "Number of messages in the error buffer is correct: " << lines_err << std::endl;

    // If CMAKE_BUILD_TYPE is Debug, the INTERNAL_DEBUG flag was set, and the logInfo messages were not deactivated,
    // then there should be 2 messages in the out buffer, one for the logWarning, and another one for the logInfo.
    // Else, there should only be 1 message in the out buffer, corresponding to the logWarning.
#if !HAVE_LOG_NO_INFO
    ASSERT_EQ(2, lines_out);
#else
    ASSERT_EQ(1, lines_out);
#endif // if !HAVE_LOG_NO_INFO
    std::cout << "Number of messages in the out buffer is correct: " << lines_out << std::endl;
    // Reset the log module to the test default
    Reset();
}

std::vector<Log::Entry> LogTests::HELPER_WaitForEntries(
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

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
