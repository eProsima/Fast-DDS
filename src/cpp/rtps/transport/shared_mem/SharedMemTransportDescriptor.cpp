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

#include <cstdint>

#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>

#include <fastdds/rtps/transport/PortBasedTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

//*********************************************************
// SharedMemTransportDescriptor
//*********************************************************
SharedMemTransportDescriptor::SharedMemTransportDescriptor()
    : PortBasedTransportDescriptor(shm_default_segment_size, s_maximumInitialPeersRange)
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
           this->dump_thread_ == t.dump_thread() &&
           PortBasedTransportDescriptor::operator ==(t));
}

#ifdef FASTDDS_SHM_TRANSPORT_DISABLED
TransportInterface* SharedMemTransportDescriptor::create_transport() const
{
    return nullptr;
}

#endif // ifdef FASTDDS_SHM_TRANSPORT_DISABLED

} // rtps
} // fastdds
} // eprosima
