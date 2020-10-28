
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
#ifndef _FASTDDS_DDS_LOG_LOG_HPP_
#define _FASTDDS_DDS_LOG_LOG_HPP_

#include <functional>
#include <memory>
#include <gmock/gmock.h>

/**
 * eProsima log mock.
 */

#define logInfo(cat, msg)                                                                                \
    {                                                                                                   \
        NulStreambuf null_buffer;                                                                       \
        std::ostream null_stream(&null_buffer);                                                         \
        null_stream << msg;                                                                             \
    }

#define logWarning(cat, msg) logInfo(cat, msg)
#define logError(cat, msg) logInfo(cat, msg)

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
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_LOG_LOG_HPP_
