// Copyright 2020 Canonical ltd.
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

/*!
 * @file Logging.h
 */
#ifndef _FASTDDS_RTPS_SECURITY_LOGGING_LOGTOPIC_H_
#define _FASTDDS_RTPS_SECURITY_LOGGING_LOGTOPIC_H_

#include <atomic>
#include <fstream>
#include <memory>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>

#include <rtps/security/exceptions/SecurityException.h>
#include <rtps/security/logging/Logging.h>
#include <rtps/security/logging/BuiltinLoggingType.h>
#include <utils/collections/concurrent_queue.h>
#include <utils/thread.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

/**
 * @brief LogTopic
 */
class LogTopic final : public Logging
{
    using BuiltinLoggingTypePtr = std::unique_ptr<BuiltinLoggingType>;

public:

    LogTopic(
            uint32_t thread_id = 0,
            const fastdds::rtps::ThreadSettings& thr_config = {});
    ~LogTopic();

private:

    /**
     * @brief log_impl
     * @param message
     * @param category
     * @param exception
     */
    void log_impl(
            const BuiltinLoggingType& message,
            SecurityException& exception) const override;

    bool enable_logging_impl(
            SecurityException& exception) override;

    void publish(
            BuiltinLoggingType& builtin_msg);

    void stop()
    {
        stop_ = true;
    }

    std::ofstream file_stream_;

    mutable ConcurrentQueue<BuiltinLoggingTypePtr> queue_;

    std::atomic_bool stop_;

    eprosima::thread thread_;
};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _FASTDDS_RTPS_SECURITY_LOGGING_LOGTOPIC_H_
