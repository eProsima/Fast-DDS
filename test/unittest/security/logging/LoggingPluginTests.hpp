// Copyright 2020 Canonical, Ltd.
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

#ifndef _UNITTEST_SECURITY_LOGGING_LOGGINGPLUGINTESTS_HPP_
#define _UNITTEST_SECURITY_LOGGING_LOGGINGPLUGINTESTS_HPP_

#include <thread>

#include <gtest/gtest.h>

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>

#include <security/logging/LogTopic.h>

class LoggingPluginTest : public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        plugin = new eprosima::fastdds::rtps::security::LogTopic();

        ASSERT_NE(nullptr, plugin);
    }

    virtual void TearDown()
    {
        delete plugin;
    }

public:

    LoggingPluginTest() = default;
    ~LoggingPluginTest() = default;

    eprosima::fastdds::rtps::security::Logging* plugin = nullptr;

    const std::string security_log_filename = "security_logs.log";

    // number of LoggingLevel
    static constexpr long NUM_LOG_LEVELS = 8;
};

constexpr long LoggingPluginTest::NUM_LOG_LEVELS;

TEST_F(LoggingPluginTest, DefaultBehavior)
{
    eprosima::fastdds::rtps::security::SecurityException exception;

    // Options not set

    EXPECT_FALSE(plugin->options_set());

    eprosima::fastdds::rtps::security::LogOptions log_options;
    EXPECT_FALSE(plugin->get_log_options(log_options, exception)) << exception.what();

    EXPECT_FALSE(plugin->enable_logging(exception)) << exception.what();

    EXPECT_FALSE(plugin->enabled());

    EXPECT_EQ(nullptr, plugin->get_listener());

    // Options set

    EXPECT_TRUE(plugin->set_log_options(log_options, exception)) << exception.what();

    EXPECT_TRUE(plugin->options_set());

    EXPECT_TRUE(plugin->get_log_options(log_options, exception)) << exception.what();

    EXPECT_FALSE(plugin->enabled());

    // Logging enabled

    EXPECT_TRUE(plugin->enable_logging(exception)) << exception.what();

    EXPECT_TRUE(plugin->enabled());

    EXPECT_FALSE(plugin->set_log_options(log_options, exception)) << exception.what();
}

TEST_F(LoggingPluginTest, AsyncFileLogging)
{
    // First remove previous executions file
    std::remove(security_log_filename.c_str());

    eprosima::fastdds::rtps::security::LogOptions log_options;

    log_options.distribute = false;
    log_options.log_file = security_log_filename;
    log_options.log_level = eprosima::fastdds::rtps::security::LoggingLevel::DEBUG_LEVEL;

    eprosima::fastdds::rtps::security::SecurityException exception;

    //  plugin->set_domain_id();
    //  plugin->set_guid();

    EXPECT_TRUE(plugin->set_log_options(log_options, exception));
    EXPECT_TRUE(plugin->options_set());
    EXPECT_TRUE(plugin->enable_logging(exception)) << exception.what();
    EXPECT_TRUE(plugin->enabled());

    std::vector<std::unique_ptr<std::thread>> threads;
    for (long i = 0; i < NUM_LOG_LEVELS; ++i)
    {
        threads.emplace_back(new std::thread([this, i, &exception]
                {
                    plugin->log(
                        static_cast<eprosima::fastdds::rtps::security::LoggingLevel>(i),
                        std::string("Report from thread ") + std::to_string(i),
                        "Logging,fileloggingtest",
                        exception);
                }));
    }

    for (auto& thread: threads)
    {
        thread->join();
    }

    // give a chance to the logger to log all messages
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::ifstream ifs(security_log_filename);

    if (!ifs.is_open())
    {
        GTEST_FAIL();
    }


    std::string line;
    long count = 0;

    // log format is:
    // [<stamp>] [<severity>] <guid> <domain_id> <plugin_class::plugin_method> message
    while (std::getline(ifs, line))
    {
        // check stamp
        std::regex regex("\\[[0-9]+.[0-9]+\\]");
        EXPECT_TRUE(std::regex_search(line, regex)) << line;

        // check verbosity
        regex = std::regex("[EMERGENCY|ALERT|CRITICAL|ERROR|WARNING|NOTICE|INFORMATIONAL|DEBUG]");
        EXPECT_TRUE(std::regex_search(line, regex)) << line;

        //@TODO(artivis) check guid

        //@TODO(artivis) check domain_id

        // check call site
        regex = std::regex("Logging::fileloggingtest : ");
        EXPECT_TRUE(std::regex_search(line, regex)) << line;

        // check message
        regex = std::regex("Report from thread [0-9]{1}");
        EXPECT_TRUE(std::regex_search(line, regex)) << line;

        ++count;
    }

    EXPECT_EQ(NUM_LOG_LEVELS, count);
}

#endif // _UNITTEST_SECURITY_LOGGING_LOGGINGPLUGINTESTS_HPP_
