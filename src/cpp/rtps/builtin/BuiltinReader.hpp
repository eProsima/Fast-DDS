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
 * @file BuiltinReader.hpp
 */

#ifndef RTPS_BUILTIN__BUILTINREADER_HPP_
#define RTPS_BUILTIN__BUILTINREADER_HPP_

#include <memory>

#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>

#include <rtps/history/ITopicPayloadPool.h>
#include <rtps/history/PoolConfig.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Keeps data of a builtin reader
 */
template<typename TReader>
struct BuiltinReader
{
    ~BuiltinReader()
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
                payload_pool_->release_history(cfg, true);
            }
        }

        listener_.reset();
    }

    void remove_from_history(
            const fastdds::rtps::InstanceHandle_t& key)
    {
        history_->getMutex()->lock();
        for (auto it = history_->changesBegin(); it != history_->changesEnd(); ++it)
        {
            if ((*it)->instanceHandle == key)
            {
                history_->remove_change(*it);
                break;
            }
        }
        history_->getMutex()->unlock();
    }

    //! Payload pool for the topic
    std::shared_ptr<fastdds::rtps::ITopicPayloadPool> payload_pool_;
    //! History for the builtin reader
    std::unique_ptr<fastdds::rtps::ReaderHistory> history_;
    //! Builtin RTPS reader
    TReader* reader_ = nullptr;
    //! Listener for the builtin RTPS reader
    std::unique_ptr<fastdds::rtps::ReaderListener> listener_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_BUILTIN__BUILTINREADER_HPP_
