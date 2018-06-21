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
    : m_rec_msg()
    , mAlive(true)
    , mThread(nullptr)
{
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
}

ChannelResource::ChannelResource(ChannelResource&& channelResource)
    : m_rec_msg(std::move(channelResource.m_rec_msg))
    , mThread(channelResource.mThread)
{
    bool b = channelResource.mAlive;
    mAlive = b;
    channelResource.mThread = nullptr;
    //logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
    //m_rec_msg = std::move(channelResource.m_rec_msg);
}

ChannelResource::ChannelResource(uint32_t rec_buffer_size)
    : m_rec_msg(rec_buffer_size)
    , mAlive(true)
    , mThread(nullptr)
{
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << m_rec_msg.max_size);
}

ChannelResource::~ChannelResource()
{
    Clear();
}

void ChannelResource::Clear()
{
    mAlive = false;
    if (mThread != nullptr)
    {
        mThread->join();
        delete mThread;
        mThread = nullptr;
    }
}

std::thread* ChannelResource::ReleaseThread()
{
    std::thread* outThread = mThread;
    mThread = nullptr;
    return outThread;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
