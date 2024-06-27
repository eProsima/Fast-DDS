
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

/**
 * @file Log.hpp
 *
 */

#ifndef FASTDDS_DDS_LOG__LOG_HPP
#define FASTDDS_DDS_LOG__LOG_HPP

#include <functional>
#include <memory>

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>

/**
 * eProsima log mock.
 */

#define MOCK_EPROSIMA_LOG_COMMON(cat, msg)                                                              \
    {                                                                                                   \
        NulStreambuf null_buffer;                                                                       \
        std::ostream null_stream(&null_buffer);                                                         \
        null_stream << msg;                                                                             \
    }

#define EPROSIMA_LOG_INFO(cat, msg) MOCK_EPROSIMA_LOG_COMMON(cat, msg)
#define EPROSIMA_LOG_WARNING(cat, msg) MOCK_EPROSIMA_LOG_COMMON(cat, msg)
#define EPROSIMA_LOG_ERROR(cat, msg) MOCK_EPROSIMA_LOG_COMMON(cat, msg)

class NulStreambuf : public std::streambuf
{
protected:

    int overflow(
            int c)
    {
        return c;
    }

};

namespace eprosima {
namespace fastdds {
namespace dds {

class LogConsumer
{
public:

    virtual ~LogConsumer() = default;
};

class Log
{
public:

    enum Kind
    {
        Error,
        Warning,
        Info,
    };

    static std::function<void(std::unique_ptr<LogConsumer>&&)> RegisterConsumerFunc;
    static void RegisterConsumer(
            std::unique_ptr<LogConsumer>&& c)
    {
        RegisterConsumerFunc(std::move(c));
    }

    static std::function<void()> ClearConsumersFunc;
    static void ClearConsumers()
    {
        ClearConsumersFunc();
    }

    static std::function<void()> SetThreadConfigFunc;
    static void SetThreadConfig(
            rtps::ThreadSettings&)
    {
        SetThreadConfigFunc();
    }

};

using ::testing::_;
using ::testing::Invoke;
class LogMock
{
public:

    // r-value support for mocked function.
    void RegisterConsumer(
            std::unique_ptr<LogConsumer>&& p)
    {
        RegisterConsumer(p);
    }

    MOCK_METHOD1(RegisterConsumer, void(std::unique_ptr<LogConsumer>&));

    MOCK_METHOD0(ClearConsumers, void());

    MOCK_METHOD(void, SetThreadConfig, ());
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_LOG__LOG_HPP
