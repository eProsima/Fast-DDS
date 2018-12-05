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
#include <fastrtps/transport/UDPChannelResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

UDPChannelResource::UDPChannelResource(eProsimaUDPSocket& socket)
    : mMsgReceiver(nullptr)
    , socket_(moveSocket(socket))
    , only_multicast_purpose_(false)
{
}

UDPChannelResource::UDPChannelResource(eProsimaUDPSocket& socket, uint32_t maxMsgSize)
    : ChannelResource(maxMsgSize)
    , mMsgReceiver(nullptr)
    , socket_(moveSocket(socket))
    , only_multicast_purpose_(false)
{
}

UDPChannelResource::UDPChannelResource(UDPChannelResource&& channelResource)
    : mMsgReceiver(channelResource.mMsgReceiver)
    , socket_(moveSocket(channelResource.socket_))
    , only_multicast_purpose_(channelResource.only_multicast_purpose_)
{
    channelResource.mMsgReceiver = nullptr;
}

UDPChannelResource::~UDPChannelResource()
{
    mMsgReceiver = nullptr;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
