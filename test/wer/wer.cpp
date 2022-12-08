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
// Note: In order to worked with GTEST the following environment variable must
// be set at runtime: set GTEST_CATCH_EXCEPTIONS=0
// or the corresponding cli flag used --gtest_catch_exceptions=0

/**
 * @file wer.hpp
 *
 */

#pragma once

static_assert(_MSC_VER, "This header only applies for microsoft visual Studio compiler");
static_assert(WER_TIMEOUT_TIME > 1, "The timeout in seconds must be defined");

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#   define NOMINMAX
#endif // NOMINMAX
#include <windows.h>

// If the test is not using gtest ignore it
#if defined(__has_include) && __has_include(<gtest/gtest.h>)
#   include <gtest/gtest.h>
#endif

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
        // Check environment to see if gtest is properly set up
#       ifdef GOOGLETEST_INCLUDE_GTEST_GTEST_H_
            if (testing::GTEST_FLAG(catch_exceptions) == true)
            {
                const TCHAR* catch_exc = TEXT("GTEST_CATCH_EXCEPTIONS");
                TCHAR buffer[2];
                DWORD res = GetEnvironmentVariable(catch_exc, buffer, 2);
                if( !res || buffer[0] != '0')
                {
                    std::cerr << "The environment variable GTEST_CATCH_EXCEPTIONS must "
                              << "be 0 in order to activate WER. Otherwise use the gtest "
                              << "cli --gtest_catch_exceptions=0." << std::endl;
                }
            }
#       endif

        // avoid CTest preemption of crashes
        SetErrorMode(0);
        SetUnhandledExceptionFilter(&WerEnforcer::UnhandledExceptionFilter);

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
                                std::cerr << "Raising WER fail fast exception for timeout" << std::endl;
                                RaiseFailFastException(nullptr, nullptr, 0);
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

    // Play along CTest JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION setting
    static LONG UnhandledExceptionFilter(
            _EXCEPTION_POINTERS* ExceptionInfo)
    {
        PEXCEPTION_RECORD ExceptionRecord = nullptr;
        PCONTEXT ContextRecord = nullptr;

        if (ExceptionInfo)
        {
            ExceptionRecord = ExceptionInfo->ExceptionRecord;
            ContextRecord = ExceptionInfo->ContextRecord;
        }
        std::cerr << "Raising WER fail fast exception for exception: " <<
            ExceptionRecord->ExceptionCode << std::endl;
        RaiseFailFastException(ExceptionRecord, ContextRecord, FAIL_FAST_GENERATE_EXCEPTION_ADDRESS);

        // unreachable
        return -1;
    }

};

} // namespace eprosima

const eprosima::WerEnforcer wer_singleton;
