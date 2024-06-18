// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::dds;

class CustomLogConsumer : public LogConsumer
{

public:

    CustomLogConsumer(
            const char* regex_str)
        : regex_(regex_str)
    {
    }

    void set_regex(
            const char* regex_str)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        regex_ = std::regex(regex_str);
    }

    virtual void Consume(
            const Log::Entry& entry)
    {
        std::stringstream stream;
        print_timestamp(stream, entry, false);
        print_header(stream, entry, false);
        print_message(stream, entry, false);
        print_context(stream, entry, false);
        print_new_line(stream, false);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            EXPECT_TRUE(std::regex_match(stream.str(), regex_));
        }
    }

private:

    std::mutex mutex_;
    std::regex regex_;
};

TEST(ThreadSettingsQoS, thread_settings_qos_outputs_proper_logging_on_failure)
{
    std::string expected_log = "^(.+)(SYSTEM)(.+)(Problem to set priority of thread with id)(.+)(\n)$";
    CustomLogConsumer* custom_consumer = new CustomLogConsumer(expected_log.c_str());

    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(custom_consumer));
    Log::SetVerbosity(Log::Error);
    Log::SetCategoryFilter(std::regex("(SYSTEM)"));

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    eprosima::fastdds::rtps::ThreadSettings th_settings;
    th_settings.priority = -5;

    writer.set_events_thread_settings(th_settings)
            .init();

    Log::Flush();

#ifndef _WIN32
    custom_consumer->set_regex("^(.+)(SYSTEM)(.+)(Problem to set scheduler of thread with id)(.+)(\n)$");
    expected_log = "^(.+)(SYSTEM)(.+)(Problem to set scheduler of thread with id)(.+)(\n)$";
    writer.destroy();

    th_settings = eprosima::fastdds::rtps::ThreadSettings();
    th_settings.scheduling_policy = 1;

    writer.set_events_thread_settings(th_settings)
            .init();

    Log::Flush();
#endif // ifndef _WIN32
}
