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
        print_timestamp(stream_, entry, false);
        print_header(stream_, entry, false);
        print_message(stream_, entry, false);
        print_context(stream_, entry, false);
        print_new_line(stream_, false);

        std::regex re("^(.+)CUSTOM_LOG_CONSUMER_TEST(.+)Testing log consumer protected functions -> (.+)$");
        EXPECT_TRUE(std::regex_match(stream_.str(), re));

        stream_.str("");
    }

private:

    std::stringstream stream_;
};

TEST(LogConsumer, CheckLogConsumerPrintMemberFunctions)
{
    CustomLogConsumer* custom_consumer = new CustomLogConsumer();

    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(custom_consumer));
    Log::SetVerbosity(Log::Warning);
    Log::SetCategoryFilter(std::regex("(CUSTOM_LOG_CONSUMER_TEST)"));

    logError(CUSTOM_LOG_CONSUMER_TEST, "Testing log consumer protected functions")
}



} // namespace dds
} // namespace fastdds
} // namespace eprosima
