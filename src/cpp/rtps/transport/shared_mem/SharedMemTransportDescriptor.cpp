// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

static constexpr uint32_t shm_default_segment_size = 0;
static constexpr uint32_t shm_default_port_queue_capacity = 512;
static constexpr uint32_t shm_default_healthy_check_timeout_ms = 1000;

} // rtps
} // fastdds
} // eprosima

//*********************************************************
// SharedMemTransportDescriptor
//*********************************************************
SharedMemTransportDescriptor::SharedMemTransportDescriptor()
    : TransportDescriptorInterface(shm_default_segment_size, s_maximumInitialPeersRange)
    , segment_size_(shm_default_segment_size)
    , port_queue_capacity_(shm_default_port_queue_capacity)
    , healthy_check_timeout_ms_(shm_default_healthy_check_timeout_ms)
    , rtps_dump_file_("")
{
    maxMessageSize = s_maximumMessageSize;
}

bool SharedMemTransportDescriptor::operator ==(
        const SharedMemTransportDescriptor& t) const
{
    return (this->segment_size_ == t.segment_size() &&
           this->port_queue_capacity_ == t.port_queue_capacity() &&
           this->healthy_check_timeout_ms_ == t.healthy_check_timeout_ms() &&
           this->rtps_dump_file_ == t.rtps_dump_file() &&
           TransportDescriptorInterface::operator ==(t));
}

#ifdef FASTDDS_SHM_TRANSPORT_DISABLED
TransportInterface* SharedMemTransportDescriptor::create_transport() const
{
    return nullptr;
}

#endif // ifdef FASTDDS_SHM_TRANSPORT_DISABLED
