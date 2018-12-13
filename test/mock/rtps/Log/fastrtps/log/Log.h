
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
#ifndef _FASTRTPS_LOG_LOG_H_
#define _FASTRTPS_LOG_LOG_H_

#include <functional>
#include <memory>
#include <gmock/gmock.h>

/**
 * eProsima log mock.
 */

#define logInfo(cat,msg) do {} while(0)
#define logWarning(cat,msg) do {} while(0)
#define logError(cat,msg) do {} while(0)

namespace eprosima {
namespace fastrtps {

class LogConsumer
{
    public:

        virtual ~LogConsumer() {}
};

class Log
{
    public:

        static std::function<void(std::unique_ptr<LogConsumer>&)> RegisterConsumerFunc;
        static void RegisterConsumer(std::unique_ptr<LogConsumer>& c) { RegisterConsumerFunc(c); }

        static std::function<void()> ClearConsumersFunc;
        static void ClearConsumers() { ClearConsumersFunc(); }
};

class LogMock
{
    public:

        MOCK_METHOD1(RegisterConsumer, void(std::unique_ptr<LogConsumer>&));

        MOCK_METHOD0(ClearConsumers, void());
};

} // namespace fastrtps
} // namespace eprosima

#endif
