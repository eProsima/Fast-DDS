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

#include <fastdds/rtps/transport/low-bandwidth/SourceTimestampTransport.h>
#include <fastrtps/rtps/common//Time_t.h>
#include <fastrtps/rtps/messages/CDRMessage.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

// ------------------------- Descriptor
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

SourceTimestampTransportDescriptor::SourceTimestampTransportDescriptor(
        std::shared_ptr<TransportDescriptorInterface> low_level)
    : ChainingTransportDescriptor(low_level)
    , callback(nullptr)
    , callback_parameter(nullptr)
{
}

SourceTimestampTransportDescriptor::SourceTimestampTransportDescriptor(
        const SourceTimestampTransportDescriptor& t)
    : ChainingTransportDescriptor(t)
    , callback(t.callback)
    , callback_parameter(t.callback_parameter)
{
}

TransportInterface* SourceTimestampTransportDescriptor::create_transport() const
{
    return new SourceTimestampTransport(*this);
}

// ------------------------- Descriptor
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

SourceTimestampTransport::SourceTimestampTransport(
        const SourceTimestampTransportDescriptor& descriptor)
    : ChainingTransport(descriptor)
    , configuration_(descriptor)
    , compress_buffer_(nullptr)
{
}

SourceTimestampTransport::~SourceTimestampTransport()
{
    if (compress_buffer_ != nullptr)
    {
        free(compress_buffer_);
        compress_buffer_ = nullptr;
    }
}

bool SourceTimestampTransport::init(
        const fastrtps::rtps::PropertyPolicy* properties)
{
    bool ret_val = ChainingTransport::init(properties);
    if (ret_val)
    {
        compress_buffer_ = (fastrtps::rtps::octet*) malloc(configuration_.max_message_size() );
        ret_val = compress_buffer_ != nullptr;
    }

    return ret_val;
}

bool SourceTimestampTransport::send(
        fastrtps::rtps::SenderResource* low_sender_resource,
        const fastrtps::rtps::octet* send_buffer,
        uint32_t send_buffer_size,
        fastrtps::rtps::LocatorsIterator* destination_locators_begin,
        fastrtps::rtps::LocatorsIterator* destination_locators_end,
        const std::chrono::steady_clock::time_point& timeout)
{
    std::unique_lock<std::recursive_mutex> scopedLock(compress_buffer_mutex_);
    fastrtps::rtps::Time_t send_time;
    fastrtps::rtps::Time_t::now(send_time);

    fastrtps::rtps::CDRMessage_t aux(0);
    aux.wraps = true;
    aux.msg_endian = fastrtps::rtps::LITTLEEND;
    aux.buffer = &compress_buffer_[1];
    aux.max_size = 4;
    aux.pos = aux.length = 0;
    fastrtps::rtps::CDRMessage::addInt32(&aux, send_time.seconds());

    compress_buffer_[0] = (fastrtps::rtps::octet) 'S';
    memcpy(&compress_buffer_[5], send_buffer, send_buffer_size);
    return low_sender_resource->send(compress_buffer_, send_buffer_size + 5,
                   destination_locators_begin, destination_locators_end, timeout);
}

void SourceTimestampTransport::receive(
        TransportReceiverInterface* next_receiver,
        const fastrtps::rtps::octet* receive_buffer,
        uint32_t receive_buffer_size,
        const fastrtps::rtps::Locator_t& local_locator,
        const fastrtps::rtps::Locator_t& remote_locator)
{

    bool ret_val = (receive_buffer_size > 5) && (receive_buffer[0] == (fastrtps::rtps::octet) 'S');

    if (ret_val)
    {
        if ( (configuration_.callback != nullptr) && (configuration_.callback_parameter != nullptr) )
        {
            int rec_sec = 0;
            fastrtps::rtps::CDRMessage_t aux(0);
            aux.wraps = true;
            aux.msg_endian = fastrtps::rtps::LITTLEEND;
            aux.buffer = (fastrtps::rtps::octet*)&receive_buffer[1];
            aux.max_size = aux.length = 4;
            aux.pos = 0;
            fastrtps::rtps::CDRMessage::readInt32(&aux, &rec_sec);

            fastrtps::rtps::Time_t curr_time;
            fastrtps::rtps::Time_t::now(curr_time);

            configuration_.callback(configuration_.callback_parameter, rec_sec, curr_time.seconds(),
                    receive_buffer_size);
        }

        receive_buffer_size -= 5;
        next_receiver->OnDataReceived(receive_buffer + 5, receive_buffer_size, local_locator, remote_locator);
    }
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
