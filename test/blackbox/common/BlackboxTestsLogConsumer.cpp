// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "BlackboxTests.hpp"

#include <regex>

#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class CustomLogConsumer : public LogConsumer
{
public:

    virtual void Consume(
            const Log::Entry& entry)
    {
        std::stringstream stream;
        print_timestamp(stream, entry, false);
        print_header(stream, entry, false);
        print_message(stream, entry, false);
        print_context(stream, entry, false);
        print_new_line(stream, false);

        std::regex re("^(.+)(CUSTOM_LOG_CONSUMER_TEST)(.+)(Testing log consumer protected functions)(.+)(\n)$");
        EXPECT_TRUE(std::regex_match(stream.str(), re));
    }

};

TEST(LogConsumer, CheckLogConsumerPrintMemberFunctions)
{
    CustomLogConsumer* custom_consumer = new CustomLogConsumer();

    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(custom_consumer));
    Log::SetVerbosity(Log::Warning);
    Log::SetCategoryFilter(std::regex("(CUSTOM_LOG_CONSUMER_TEST)"));

    EPROSIMA_LOG_ERROR(CUSTOM_LOG_CONSUMER_TEST, "Testing log consumer protected functions");

    Log::Flush();
}



} // namespace dds
} // namespace fastdds
} // namespace eprosima
