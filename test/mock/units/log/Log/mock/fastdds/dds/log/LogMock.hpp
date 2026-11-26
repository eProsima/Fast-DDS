// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_TEST_MOCK_FASTDDS_DDS_LOG__LOGMOCK_HPP
#define FASTDDS_TEST_MOCK_FASTDDS_DDS_LOG__LOGMOCK_HPP

#include <memory>
#include <typeinfo>

#include <gmock/gmock.h>

#include <fastdds/dds/log/FileConsumer.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Expectations of the Log mock implementation.
 *
 * While an instance exists, Log::RegisterConsumer(), Log::ClearConsumers() and Log::SetThreadConfig() forward to it.
 * Otherwise they do nothing. Only one instance can exist at a time.
 */
class LogMock
{
public:

    LogMock()
    {
        instance() = this;
    }

    virtual ~LogMock()
    {
        instance() = nullptr;
    }

    // r-value support for mocked function.
    void RegisterConsumer(
            std::unique_ptr<LogConsumer>&& p)
    {
        RegisterConsumer(p);
    }

    MOCK_METHOD1(RegisterConsumer, void(std::unique_ptr<LogConsumer>&));

    MOCK_METHOD0(ClearConsumers, void());

    MOCK_METHOD(void, SetThreadConfig, ());

    /**
     * Get the instance the Log mock implementation forwards to.
     *
     * @return Reference to the pointer to the existing instance, nullptr if there is none.
     */
    static LogMock*& instance();
};

MATCHER(IsFileConsumer, "Argument is a FileConsumer object?")
{
    *result_listener << (typeid(*arg.get()) == typeid(FileConsumer));
    return typeid(*arg.get()) == typeid(FileConsumer);
}

MATCHER(IsStdoutErrConsumer, "Argument is a StdoutErrConsumer object?")
{
    *result_listener << (typeid(*arg.get()) == typeid(StdoutErrConsumer));
    return typeid(*arg.get()) == typeid(StdoutErrConsumer);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_FASTDDS_DDS_LOG__LOGMOCK_HPP
