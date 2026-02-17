// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SendBuffersManager.cpp
 */

#include "SendBuffersManager.hpp"

#include "RTPSMessageGroup.hpp"
#include "../participant/RTPSParticipantImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

SendBuffersManager::SendBuffersManager(
        size_t reserved_size,
        bool allow_growing,
        ResourceLimitedContainerConfig network_buffers_config)
    : allow_growing_(allow_growing)
    , network_buffers_config_(network_buffers_config)
{
    pool_.reserve(reserved_size);
}

void SendBuffersManager::init(
        const RTPSParticipantImpl* participant)
{
    std::lock_guard<TimedMutex> guard(mutex_);

    if (n_created_ < pool_.capacity())
    {
        const GuidPrefix_t& guid_prefix = participant->getGuid().guidPrefix;

        // Single allocation for the data of all the buffers.
        // We align the payload size to the size of a pointer, so all buffers will
        // be aligned as if directly allocated.
        constexpr size_t align_size = sizeof(octet*) - 1;
        uint32_t payload_size = participant->getMaxMessageSize();
        assert(payload_size > 0u);
        payload_size = (payload_size + align_size) & ~align_size;
        size_t advance = payload_size;
#if HAVE_SECURITY
        bool secure = participant->is_secure();
        advance *= secure ? 3 : 2;
#else
        advance *= 2;
#endif // if HAVE_SECURITY
        size_t data_size = advance * (pool_.capacity() - n_created_);
        common_buffer_.assign(data_size, 0);

        octet* raw_buffer = common_buffer_.data();
        while (n_created_ < pool_.capacity())
        {
            pool_.emplace_back(new RTPSMessageGroup_t(
                        raw_buffer,
#if HAVE_SECURITY
                        secure,
#endif // if HAVE_SECURITY
                        payload_size, guid_prefix, network_buffers_config_
                        ));
            raw_buffer += advance;
            ++n_created_;
        }
    }
}

std::unique_ptr<RTPSMessageGroup_t> SendBuffersManager::get_buffer(
        const RTPSParticipantImpl* participant,
        const std::chrono::steady_clock::time_point& max_blocking_time)
{
#if HAVE_STRICT_REALTIME
    std::unique_lock<TimedMutex> lock(mutex_, std::defer_lock);
    if (!lock.try_lock_until(max_blocking_time))
    {
        throw RTPSMessageGroup::timeout();
    }
#else
    std::unique_lock<TimedMutex> lock(mutex_);
#endif // if HAVE_STRICT_REALTIME

    std::unique_ptr<RTPSMessageGroup_t> ret_val;

    while (pool_.empty())
    {
        if (allow_growing_ || n_created_ < pool_.capacity())
        {
            add_one_buffer(participant);
        }
        else
        {
            EPROSIMA_LOG_INFO(RTPS_PARTICIPANT, "Waiting for send buffer");
            if (std::cv_status::timeout == available_cv_.wait_until(lock, max_blocking_time))
            {
                throw RTPSMessageGroup::timeout();
            }
        }
    }

    ret_val = std::move(pool_.back());
    pool_.pop_back();

    return ret_val;
}

void SendBuffersManager::return_buffer(
        std::unique_ptr <RTPSMessageGroup_t>&& buffer)
{
    std::lock_guard<TimedMutex> guard(mutex_);
    pool_.push_back(std::move(buffer));
    available_cv_.notify_one();
}

void SendBuffersManager::add_one_buffer(
        const RTPSParticipantImpl* participant)
{
    RTPSMessageGroup_t* new_item = new RTPSMessageGroup_t(
#if HAVE_SECURITY
        participant->is_secure(),
#endif // if HAVE_SECURITY
        participant->getMaxMessageSize(), participant->getGuid().guidPrefix);
    pool_.emplace_back(new_item);
    ++n_created_;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
