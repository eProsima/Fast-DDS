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

/**
 * @file wer.hpp
 *
 */

#pragma once

static_assert(_MSC_VER, "This header only applies for microsoft visual Studio compiler");
static_assert(WER_TIMEOUT_TIME > 0, "The timeout in seconds must be defined");

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>

#include <gtest/gtest.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace eprosima {

class WerEnforcer
{
    std::thread watchdog_;
    std::mutex mtx_;
    std::condition_variable cond_;
    bool done = false;

public:

    WerEnforcer()
    {
        // avoid gtest preemption of crashes
        testing::GTEST_FLAG(catch_exceptions) = false;

        // avoid CTest preemption of crashes
        SetErrorMode(0);

        // launch watchdog timer
        watchdog_ = std::thread([this]
                        {
                            std::unique_lock<std::mutex> lock(mtx_);
                            // Wait for test completion or timeout
                            cond_.wait_for(
                                lock,
                                std::chrono::seconds(WER_TIMEOUT_TIME),
                                [this]
                                {
                                    return done;
                                });

                            // on timeout force crash for WER sake
                            if (!done)
                            {
                                throw std::runtime_error("Test timeouts");
                            }
                        });
    }

    ~WerEnforcer()
    {
        // no timeout, close watchdog thread
        {
            std::unique_lock<std::mutex> lock(mtx_);
            done = true;
        }

        cond_.notify_one();
        watchdog_.join();
    }

};

} // namespace eprosima

#ifndef wer_EXPORTS
const extern __declspec(selectany) eprosima::WerEnforcer wer_singleton;
#endif // wer_EXPORTS
