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

#include <fastdds/rtps/transport/TransportInterface.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransport.h>
#include <fastrtps/rtps/messages/CDRMessage.h>
#include <utility>
#include <cstring>
#include <algorithm>
#include <fastrtps/log/Log.h>
#include <fastrtps/utils/Semaphore.h>
#include <fastrtps/utils/IPLocator.h>
#include <fastdds/rtps/network/ReceiverResource.h>
#include <fastdds/rtps/network/SenderResource.h>
#include <fastrtps/rtps/messages/MessageReceiver.h>

using namespace std;
using namespace asio;

namespace eprosima{
namespace fastdds{
namespace rtps{

using IPFinder = fastrtps::rtps::IPFinder;
using IPLocator = fastrtps::rtps::IPLocator;
using Locator_t = fastrtps::rtps::Locator_t;
using LocatorList_t = fastrtps::rtps::LocatorList_t;
using Log = dds::Log;

std::vector<std::shared_ptr<Port>> eProsimaSharedMem::global_ports_;
std::mutex eProsimaSharedMem::global_ports_mutex_;

//*********************************************************
// SharedMemTransportDescriptor
//*********************************************************
SharedMemTransportDescriptor::SharedMemTransportDescriptor()
    : TransportDescriptorInterface(s_maximumMessageSize, s_maximumInitialPeersRange)
    , shared_memory_port(0)
{
}

SharedMemTransportDescriptor::SharedMemTransportDescriptor(
        const SharedMemTransportDescriptor& t)
    : TransportDescriptorInterface(t)
    , shared_memory_port(t.shared_memory_port)
{
}

TransportInterface* SharedMemTransportDescriptor::create_transport() const
{
    return new SharedMemTransport(*this);
}

//*********************************************************
// SharedMemTransport
//*********************************************************

SharedMemTransport::SharedMemTransport(
        const SharedMemTransportDescriptor& descriptor)
    : SharedMemTransportInterface()
    , configuration_(descriptor)
{
    
}

SharedMemTransport::SharedMemTransport()
    : SharedMemTransportInterface()
{
}

SharedMemTransport::~SharedMemTransport()
{
    clean();
}

bool SharedMemTransport::getDefaultMetatrafficMulticastLocators(
        LocatorList_t &locators,
        uint32_t metatraffic_multicast_port) const
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_SHMEM;
    locator.port = static_cast<uint16_t>(metatraffic_multicast_port);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locators.push_back(locator);
    return true;
}

bool SharedMemTransport::getDefaultMetatrafficUnicastLocators(
        LocatorList_t &locators,
        uint32_t metatraffic_unicast_port) const
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_SHMEM;
    locator.port = static_cast<uint16_t>(metatraffic_unicast_port);
    locator.set_Invalid_Address();
    locators.push_back(locator);

    return true;
}

bool SharedMemTransport::getDefaultUnicastLocators(
        LocatorList_t &locators,
        uint32_t unicast_port) const
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_SHMEM;
    locator.set_Invalid_Address();
    fillUnicastLocator(locator, unicast_port);
    locators.push_back(locator);

    return true;
}

void SharedMemTransport::AddDefaultOutputLocator(
        LocatorList_t &defaultList)
{
    Locator_t locator;
    locator.kind = LOCATOR_KIND_SHMEM;
    locator.set_Invalid_Address();
    locator.port = configuration_.shared_memory_port;
    defaultList.push_back(locator);
}

const SharedMemTransportDescriptor* SharedMemTransport::configuration() const
{
    return &configuration_;
}

bool SharedMemTransport::OpenInputChannel(
        const Locator_t& locator,
        TransportReceiverInterface* receiver,
        uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(input_channels_mutex_);

    if (!IsLocatorSupported(locator))
    {
        return false;
    }

	if (!IsInputChannelOpen(locator)) 
    {
		try
		{
			auto channel_resource = CreateInputChannelResource(locator, IPLocator::isMulticast(locator), maxMsgSize, receiver);
			input_channels_.push_back(channel_resource);
		}
		catch (std::exception& e)
		{
			logInfo(RTPS_MSG_OUT, "SharedMemTransport Error binding at port: (" << std::to_string(locator.port) << ")"
				<< " with msg: " << e.what());
			return false;
		}
	}

	return true;
}

bool SharedMemTransport::is_locator_allowed(
        const Locator_t& locator) const
{
    return IsLocatorSupported(locator);
}

LocatorList_t SharedMemTransport::NormalizeLocator(
        const Locator_t& locator)
{
    LocatorList_t list;

    list.push_back(locator);

    return list;
}

bool SharedMemTransport::is_local_locator(
        const Locator_t& locator) const
{
    assert(locator.kind == LOCATOR_KIND_SHMEM);

    return true;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
