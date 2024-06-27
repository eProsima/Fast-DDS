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
 * @file BuiltinWriter.hpp
 */

#ifndef RTPS_BUILTIN__BUILTINWRITER_HPP_
#define RTPS_BUILTIN__BUILTINWRITER_HPP_

#include <memory>

#include <fastdds/rtps/history/WriterHistory.hpp>

#include <rtps/history/ITopicPayloadPool.h>
#include <rtps/history/PoolConfig.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Keeps data of a builtin writer
 */
template<typename TWriter>
struct BuiltinWriter
{
    ~BuiltinWriter()
    {
        release();
    }

    void release()
    {
        if (history_)
        {
            auto cfg = fastdds::rtps::PoolConfig::from_history_attributes(history_->m_att);
            history_.reset();
            if (payload_pool_)
            {
                payload_pool_->release_history(cfg, false);
            }
        }
    }

    //! Payload pool for the topic
    std::shared_ptr<fastdds::rtps::ITopicPayloadPool> payload_pool_;
    //! History for the builtin writer
    std::unique_ptr<fastdds::rtps::WriterHistory> history_;
    //! Builtin RTPS writer
    TWriter* writer_ = nullptr;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_BUILTIN__BUILTINWRITER_HPP_
