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

#ifndef UDP_CHANNEL_RESOURCE_INFO_
#define UDP_CHANNEL_RESOURCE_INFO_

#include <fastrtps/transport/ChannelResource.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class MessageReceiver;

#if defined(ASIO_HAS_MOVE)
    // Typedefs
	typedef asio::ip::udp::socket eProsimaUDPSocket;
    typedef eProsimaUDPSocket& eProsimaUDPSocketRef;

    // UDP
	inline eProsimaUDPSocket* getSocketPtr(eProsimaUDPSocket &socket)
    {
        return &socket;
    }
    inline eProsimaUDPSocket moveSocket(eProsimaUDPSocket &socket)
    {
        return std::move(socket);
    }
    inline eProsimaUDPSocket createUDPSocket(asio::io_service& io_service)
    {
        return std::move(asio::ip::udp::socket(io_service));
    }
    inline eProsimaUDPSocket& getRefFromPtr(eProsimaUDPSocket* socket)
    {
        return *socket;
    }
#else
    // Typedefs
	typedef std::shared_ptr<asio::ip::udp::socket> eProsimaUDPSocket;
    typedef eProsimaUDPSocket eProsimaUDPSocketRef;

    // UDP
    inline eProsimaUDPSocket getSocketPtr(eProsimaUDPSocket socket)
    {
        return socket;
    }
    inline eProsimaUDPSocket moveSocket(eProsimaUDPSocket socket)
    {
        return socket;
    }
    inline eProsimaUDPSocket createUDPSocket(asio::io_service& io_service)
    {
        return std::make_shared<asio::ip::udp::socket>(io_service);
    }
    inline eProsimaUDPSocket getRefFromPtr(eProsimaUDPSocket socket)
    {
        return socket;
    }
#endif

class UDPChannelResource : public ChannelResource
{
public:
    UDPChannelResource(eProsimaUDPSocket& socket);
    UDPChannelResource(eProsimaUDPSocket& socket, uint32_t maxMsgSize);
    UDPChannelResource(UDPChannelResource&& channelResource);
    virtual ~UDPChannelResource();

    UDPChannelResource& operator=(UDPChannelResource&& channelResource)
    {
        socket_ = moveSocket(channelResource.socket_);
        return *this;
    }

    void only_multicast_purpose(const bool value)
    {
        only_multicast_purpose_ = value;
    };

    bool& only_multicast_purpose()
    {
        return only_multicast_purpose_;
    }

    bool only_multicast_purpose() const
    {
        return only_multicast_purpose_;
    }

#if defined(ASIO_HAS_MOVE)
    inline eProsimaUDPSocket* getSocket()
#else
    inline eProsimaUDPSocket getSocket()
#endif
    {
        return getSocketPtr(socket_);
    }

    inline void SetMessageReceiver(MessageReceiver* receiver)
    {
        mMsgReceiver = receiver;
    }

    inline MessageReceiver* GetMessageReceiver()
    {
        return mMsgReceiver;
    }

private:

    MessageReceiver* mMsgReceiver; //Associated Readers/Writers inside of MessageReceiver
    eProsimaUDPSocket socket_;
    bool only_multicast_purpose_;
    UDPChannelResource(const UDPChannelResource&) = delete;
    UDPChannelResource& operator=(const UDPChannelResource&) = delete;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // UDP_CHANNEL_RESOURCE_INFO_