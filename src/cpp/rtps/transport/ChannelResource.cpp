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

#include <chrono>
#include <thread>

#include <asio.hpp>

#include <rtps/transport/ChannelResource.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

const uint64_t ChannelResource::MAX_WAIT_SECONDS_ON_CLOSURE_ = 5;

ChannelResource::ChannelResource()
    : message_buffer_(RTPSMESSAGE_DEFAULT_SIZE)
    , alive_(true)
{
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << message_buffer_.max_size);
}

ChannelResource::ChannelResource(
        ChannelResource&& channelResource)
    : message_buffer_(std::move(channelResource.message_buffer_))
    , thread_(std::move(channelResource.thread_))
{
    bool b = channelResource.alive_;
    alive_.store(b);
    //logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << message_buffer_.max_size);
    //message_buffer_ = std::move(channelResource.message_buffer_);
}

ChannelResource::ChannelResource(
        uint32_t rec_buffer_size)
    : message_buffer_(rec_buffer_size)
    , alive_(true)
{
    memset(message_buffer_.buffer, 0, rec_buffer_size);
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << message_buffer_.max_size);
}

ChannelResource::~ChannelResource()
{
    clear();
}

void ChannelResource::waiting_join_channel_()
{
    thread_.join();
}

void ChannelResource::clear()
{
    alive_.store(false);
    if (thread_.joinable())
    {
        if (thread_.get_id() != std::this_thread::get_id())
        {
            // Thanks to: https://stackoverflow.com/users/6255513/smeeheey
            // https://stackoverflow.com/a/40551227/17049427

            std::mutex m;
            std::condition_variable cv;

            // Wait for it to finish
            std::thread t([&cv, this]()
            {
                waiting_join_channel_();
                cv.notify_one();
            });

            // Detach the waiting thread
            t.detach();

            {
                std::unique_lock<std::mutex> l(m);

                // Wait in condition variable.
                // If wait for is awake by timeoout, it s consider to be stucked. Thus detach it and close.
                if(cv.wait_for(l, std::chrono::seconds(ChannelResource::MAX_WAIT_SECONDS_ON_CLOSURE_)) ==
                        std::cv_status::timeout)
                {
                    logWarning(RTPS, "Channel closed before thread join");
                    thread_.detach();
                }
            }
        }
        else
        {
            // killing my own thread
            thread_.detach();
        }
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
