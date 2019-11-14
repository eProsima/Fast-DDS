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

#ifndef _FASTDDS_UDP_CHANNEL_RESOURCE_INFO_
#define _FASTDDS_UDP_CHANNEL_RESOURCE_INFO_

#include <fastdds/rtps/transport/ChannelResource.h>
#include <fastdds/rtps/common/Locator.h>
#include <asio.hpp>

namespace eprosima{
namespace fastdds{
namespace rtps{

class TransportReceiverInterface;
class UDPTransportInterface;

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
        return asio::ip::udp::socket(io_service);
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

    UDPChannelResource(
        UDPTransportInterface* transport,
        eProsimaUDPSocket& socket,
        uint32_t maxMsgSize,
        const fastrtps::rtps::Locator_t& locator,
        const std::string& sInterface,
        TransportReceiverInterface* receiver);

    virtual ~UDPChannelResource() override;

    UDPChannelResource& operator=(UDPChannelResource&& channelResource)
    {
        socket_ = moveSocket(channelResource.socket_);
        return *this;
    }

    void only_multicast_purpose(const bool value)
    {
        only_multicast_purpose_ = value;
    }

    bool& only_multicast_purpose()
    {
        return only_multicast_purpose_;
    }

    bool only_multicast_purpose() const
    {
        return only_multicast_purpose_;
    }

#if defined(ASIO_HAS_MOVE)
    inline eProsimaUDPSocket* socket()
#else
    inline eProsimaUDPSocket socket()
#endif
    {
        return getSocketPtr(socket_);
    }

    inline void interface(const std::string& interface)
    {
        interface_ = interface;
    }

    inline const std::string& interface() const
    {
        return interface_;
    }

    inline void message_receiver(TransportReceiverInterface* receiver)
    {
        message_receiver_ = receiver;
    }

    inline TransportReceiverInterface* message_receiver()
    {
        return message_receiver_;
    }

    inline virtual void disable() override
    {
        ChannelResource::disable();
    }

    void release();

protected:
    /**
     * Function to be called from a new thread, which takes cares of performing a blocking receive
     * operation on the ReceiveResource
     * @param input_locator - Locator that triggered the creation of the resource
    */
    void perform_listen_operation(
            fastrtps::rtps::Locator_t input_locator);

    /**
    * Blocking Receive from the specified channel.
    * @param receive_buffer vector with enough capacity (not size) to accomodate a full receive buffer. That
    * capacity must not be less than the receive_buffer_size supplied to this class during construction.
    * @param receive_buffer_capacity Maximum size of the receive_buffer.
    * @param[out] receive_buffer_size Size of the received buffer.
    * @param[out] remote_locator Locator describing the remote restination we received a packet from.
    */
    bool Receive(
            fastrtps::rtps::octet* receive_buffer,
            uint32_t receive_buffer_capacity,
            uint32_t& receive_buffer_size,
            fastrtps::rtps::Locator_t& remote_locator);

private:

    TransportReceiverInterface* message_receiver_; //Associated Readers/Writers inside of MessageReceiver
    eProsimaUDPSocket socket_;
    bool only_multicast_purpose_;
    std::string interface_;
    UDPTransportInterface* transport_;

    UDPChannelResource(const UDPChannelResource&) = delete;
    UDPChannelResource& operator=(const UDPChannelResource&) = delete;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_UDP_CHANNEL_RESOURCE_INFO_
