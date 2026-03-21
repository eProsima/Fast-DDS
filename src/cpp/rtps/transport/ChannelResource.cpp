// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <rtps/transport/ChannelResource.h>

#include <utils/thread.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

ChannelResource::ChannelResource()
    : message_buffer_(RTPSMESSAGE_DEFAULT_SIZE)
    , alive_(true)
{
    EPROSIMA_LOG_INFO(RTPS_MSG_IN, "Created with CDRMessage of size: " << message_buffer_.max_size);
}

ChannelResource::ChannelResource(
        ChannelResource&& channelResource)
    : message_buffer_(std::move(channelResource.message_buffer_))
    , thread_(std::move(channelResource.thread_))
{
    bool b = channelResource.alive_;
    alive_.store(b);
    //EPROSIMA_LOG_INFO(RTPS_MSG_IN, "Created with CDRMessage of size: " << message_buffer_.max_size);
    //message_buffer_ = std::move(channelResource.message_buffer_);
}

ChannelResource::ChannelResource(
        uint32_t rec_buffer_size)
    : message_buffer_(rec_buffer_size)
    , alive_(true)
{
    memset(message_buffer_.buffer, 0, rec_buffer_size);
    EPROSIMA_LOG_INFO(RTPS_MSG_IN, "Created with CDRMessage of size: " << message_buffer_.max_size);
}

ChannelResource::~ChannelResource()
{
    clear();
}

void ChannelResource::clear()
{
    alive_.store(false);
    if (thread_.joinable())
    {
        if (!thread_.is_calling_thread())
        {
            // wait for it to finish
            thread_.join();
        }
        else
        {
            // killing my own thread
            thread_.detach();
        }
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
