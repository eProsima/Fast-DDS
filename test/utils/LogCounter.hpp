// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LogCounter.hpp
 */

#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace testing {

/**
 * This class holds all counting/state logic:
 * - Counts per Log::Kind (Warning, Error, Info, ...)
 * - Optional storage of full entries (disabled by default)
 * - "Needle" matcher for exact message occurrences
 * Intended to be used behind a LogCounterConsumer
 */
class LogCounterObserver
{
public:

    using Log  = eprosima::fastdds::dds::Log;
    using Kind = Log::Kind;

    explicit LogCounterObserver(
            bool store_logs = false)
        : store_(store_logs)
    {
        matched_.store(0, std::memory_order_relaxed);
    }

    // Set / reset the message substring to match and count
    void set_global_needle(
            std::string s)
    {
        std::lock_guard<std::mutex> lk(m_);
        needle_ = std::move(s);
        matched_.store(0, std::memory_order_relaxed);
    }

    // Number of messages that matched the current needle (substring match)
    size_t matched_global() const
    {
        return matched_.load(std::memory_order_relaxed);
    }

    // Count of logs for a specific kind (Warning, Error, ...)
    size_t count(
            Kind k) const
    {
        std::lock_guard<std::mutex> lk(m_);
        auto it = counts_.find(k);
        return (it == counts_.end()) ? 0 : it->second;
    }

    // Get stored entries (only if constructed with store_logs=true)
    const std::vector<Log::Entry>& entries() const
    {
        return entries_;
    }

    // Called by the consumer
    void on_log(
            const Log::Entry& e)
    {
        std::string local_needle;
        {
            std::lock_guard<std::mutex> lk(m_);
            ++counts_[e.kind];
            local_needle = needle_;
            if (store_)
            {
                entries_.push_back(e);
            }
        }
        if (!local_needle.empty() && e.message.find(local_needle) != std::string::npos)
        {
            matched_.fetch_add(1, std::memory_order_relaxed);
        }
    }

private:

    std::map<Kind, std::size_t> counts_;
    std::atomic<size_t> matched_{0};

    bool store_;
    mutable std::mutex m_;
    std::string needle_;
    std::vector<Log::Entry> entries_;
};

/**
 * Class holding a shared_ptr that ensures the observer
 * outlives asynchronous logging, so tests can safely read counters even if a
 * failure occurs mid-test.
 */
class LogCounterConsumer : public eprosima::fastdds::dds::LogConsumer
{
public:

    using Log  = eprosima::fastdds::dds::Log;

    explicit LogCounterConsumer(
            std::shared_ptr<LogCounterObserver> obs)
        : observer_(std::move(obs))
    {
    }

    void Consume(
            const Log::Entry& e) override
    {
        if (observer_)
        {
            observer_->on_log(e);
        }
    }

private:

    std::shared_ptr<LogCounterObserver> observer_;
};

} // namespace testing
} // namespace fastdds
} // namespace eprosima
