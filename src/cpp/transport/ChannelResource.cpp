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

#include <asio.hpp>
#include <fastrtps/transport/ChannelResource.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

ChannelResource::ChannelResource()
    : message_buffer_()
    , alive_(true)
    , thread_(nullptr)
{
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << message_buffer_.max_size);
}

ChannelResource::ChannelResource(ChannelResource&& channelResource)
    : message_buffer_(std::move(channelResource.message_buffer_))
    , thread_(channelResource.thread_)
{
    bool b = channelResource.alive_;
    alive_ = b;
    channelResource.thread_ = nullptr;
    //logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << message_buffer_.max_size);
    //message_buffer_ = std::move(channelResource.message_buffer_);
}

ChannelResource::ChannelResource(uint32_t rec_buffer_size)
    : message_buffer_(rec_buffer_size)
    , alive_(true)
    , thread_(nullptr)
{
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << message_buffer_.max_size);
}

ChannelResource::~ChannelResource()
{
    clear();
}

void ChannelResource::clear()
{
    alive_ = false;
    if (thread_ != nullptr)
    {
        thread_->join();
        delete thread_;
        thread_ = nullptr;
    }
}

std::thread* ChannelResource::release_thread()
{
    std::thread* outThread = thread_;
    thread_ = nullptr;
    return outThread;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
