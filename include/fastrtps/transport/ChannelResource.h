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

#ifndef CHANNEL_RESOURCE_INFO_
#define CHANNEL_RESOURCE_INFO_

#include <memory>
#include <map>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/common/CDRMessage_t.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class ChannelResource
{
public:
    ChannelResource();
    ChannelResource(ChannelResource&& channelResource);
    ChannelResource(uint32_t rec_buffer_size);
    virtual ~ChannelResource();

    virtual void clear();

    inline void thread(std::thread* pThread, bool thread_joinable = true)
    {
        if(thread_)
        {
            thread_->join();
            delete thread_;
            thread_ = nullptr;
        }

        thread_ = pThread;
        thread_joinable_ = thread_joinable;
    }

    void make_thread_joinable()
    {
        thread_joinable_ = true;
    }

    inline bool alive() const
    {
        return alive_;
    }

    inline virtual bool disable()
    {
        alive_ = false;
        return true;
    }

    inline CDRMessage_t& message_buffer()
    {
        return message_buffer_;
    }

protected:
    //!Received message
    CDRMessage_t message_buffer_;

    std::atomic<bool> alive_;
    std::thread* thread_;
    bool thread_joinable_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // CHANNEL_RESOURCE_INFO_
