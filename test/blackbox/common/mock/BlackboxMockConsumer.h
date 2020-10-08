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
#ifndef MOCK_BLACKBOX_LOG_CONSUMER_H
#define MOCK_BLACKBOX_LOG_CONSUMER_H

#include <fastdds/dds/log/Log.hpp>
#include <thread>
#include <mutex>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace dds {

class BlackboxMockConsumer : public LogConsumer
{
public:

    virtual void Consume(
            const Log::Entry& entry)
    {
        std::unique_lock<std::mutex> guard(mMutex);
        mEntriesConsumed.push_back(entry);
        cv_.notify_one();
    }

    const std::vector<Log::Entry> ConsumedEntries() const
    {
        std::unique_lock<std::mutex> guard(mMutex);
        return mEntriesConsumed;
    }

    std::condition_variable& cv()
    {
        return cv_;
    }

private:

    std::vector<Log::Entry> mEntriesConsumed;
    mutable std::mutex mMutex;
    std::condition_variable cv_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // ifndef MOCK_BLACKBOX_LOG_CONSUMER_H

