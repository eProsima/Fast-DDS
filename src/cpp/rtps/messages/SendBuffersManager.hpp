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
 * @file SendBuffersManager.hpp
 */

#ifndef RTPS_MESSAGES_SENDBUFFERSMANAGER_HPP
#define RTPS_MESSAGES_SENDBUFFERSMANAGER_HPP
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "RTPSMessageGroup_t.hpp"
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/utils/TimedMutex.hpp>
#include <fastdds/utils/TimedConditionVariable.hpp>

#include <vector>              // std::vector
#include <memory>              // std::unique_ptr


namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;

/**
 * Manages a pool of send buffers.
 * @ingroup WRITER_MODULE
 */
class SendBuffersManager
{
public:

    /**
     * Construct a SendBuffersManager.
     * @param reserved_size Initial size for the pool.
     * @param allow_growing Whether we allow creation of more than reserved_size elements.
     * @param num_network_buffers Number of network buffers to allocate for each send buffer.
     * @param inc_network_buffers Number of network buffers to dynamically allocate when growing the vector.
     */
    SendBuffersManager(
            size_t reserved_size,
            bool allow_growing,
            ResourceLimitedContainerConfig network_buffers_config);

    ~SendBuffersManager()
    {
        assert(pool_.size() == n_created_);
    }

    /**
     * Initialization of pool.
     * Fills the pool to its reserved capacity.
     * @param participant Pointer to the participant creating the pool.
     */
    void init(
            const RTPSParticipantImpl* participant);

    /**
     * Get one buffer from the pool.
     * @param participant Pointer to the participant asking for a buffer.
     * @param max_blocking_time Maximum time the function can be blocked.
     * @return unique pointer to a send buffer.
     */
    std::unique_ptr<RTPSMessageGroup_t> get_buffer(
            const RTPSParticipantImpl* participant,
            const std::chrono::steady_clock::time_point& max_blocking_time);

    /**
     * Return one buffer to the pool.
     * @param buffer unique pointer to the buffer being returned.
     */
    void return_buffer(
            std::unique_ptr <RTPSMessageGroup_t>&& buffer);

private:

    void add_one_buffer(
            const RTPSParticipantImpl* participant);

    //!Protects all data
    TimedMutex mutex_;
    //!Send buffers pool
    std::vector<std::unique_ptr<RTPSMessageGroup_t>> pool_;
    //!Raw buffer shared by the buffers created inside init()
    std::vector<octet> common_buffer_;
    //!Creation counter
    std::size_t n_created_ = 0;
    //!Whether we allow n_created_ to grow beyond the pool_ capacity.
    bool allow_growing_ = true;
    //!To wait for a buffer to be returned to the pool.
    TimedConditionVariable available_cv_;
    //!Configuration for the network buffers.
    ResourceLimitedContainerConfig network_buffers_config_ = ResourceLimitedContainerConfig(16u,
                    std::numeric_limits<size_t>::max dummy_avoid_winmax (), 16u);

};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // RTPS_MESSAGES_SENDBUFFERSMANAGER_HPP
