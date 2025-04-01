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

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <thread>
#include <utility>

#ifdef ANDROID
#include <boostconfig.hpp>
#include <unistd.h>
#endif // ifdef ANDROID

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/SenderResource.hpp>
#include <fastdds/rtps/transport/TransportInterface.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>

#include <rtps/messages/CDRMessage.hpp>
#include <rtps/messages/MessageReceiver.h>
#include <rtps/network/ReceiverResource.h>
#include <rtps/transport/shared_mem/SharedMemChannelResource.hpp>
#include <rtps/transport/shared_mem/SharedMemManager.hpp>
#include <rtps/transport/shared_mem/SharedMemSenderResource.hpp>
#include <rtps/transport/shared_mem/SharedMemTransport.h>
#include <rtps/transport/shared_mem/SHMLocator.hpp>
#include <statistics/rtps/messages/RTPSStatisticsMessages.hpp>

#define SHM_MANAGER_DOMAIN ("fastdds")

using namespace std;

namespace eprosima {
namespace fastdds {
namespace rtps {

// TODO(Adolfo): Calculate this value from UDP sockets buffers size.
static constexpr uint32_t shm_default_segment_size = SharedMemTransportDescriptor::shm_implicit_segment_size;

TransportInterface* SharedMemTransportDescriptor::create_transport() const
{
    return new SharedMemTransport(*this);
}

//*********************************************************
// SharedMemTransport
//*********************************************************

SharedMemTransport::SharedMemTransport(
        const SharedMemTransportDescriptor& descriptor)
    : TransportInterface(LOCATOR_KIND_SHM)
    , configuration_(descriptor)
{

}

SharedMemTransport::SharedMemTransport()
    : TransportInterface(LOCATOR_KIND_SHM)
{
}

SharedMemTransport::~SharedMemTransport()
{
    // Safely clean already opened resources
    clean_up();
}

bool SharedMemTransport::getDefaultMetatrafficMulticastLocators(
        LocatorList& locators,
        uint32_t metatraffic_multicast_port) const
{
    locators.push_back(SHMLocator::create_locator(metatraffic_multicast_port, SHMLocator::Type::MULTICAST));

    return true;
}

bool SharedMemTransport::getDefaultMetatrafficUnicastLocators(
        LocatorList& locators,
        uint32_t metatraffic_unicast_port) const
{
    locators.push_back(SHMLocator::create_locator(metatraffic_unicast_port, SHMLocator::Type::UNICAST));

    return true;
}

bool SharedMemTransport::getDefaultUnicastLocators(
        LocatorList& locators,
        uint32_t unicast_port) const
{
    auto locator = SHMLocator::create_locator(unicast_port, SHMLocator::Type::UNICAST);

    fillUnicastLocator(locator, unicast_port);
    locators.push_back(locator);

    return true;
}

void SharedMemTransport::AddDefaultOutputLocator(
        LocatorList& defaultList)
{
    (void)defaultList;
}

const SharedMemTransportDescriptor* SharedMemTransport::configuration() const
{
    return &configuration_;
}

bool SharedMemTransport::OpenInputChannel(
        const Locator& locator,
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
            auto channel_resource = CreateInputChannelResource(locator, maxMsgSize, receiver);
            input_channels_.push_back(channel_resource);
        }
        catch (std::exception& e)
        {
            (void)e;

            EPROSIMA_LOG_INFO(RTPS_MSG_OUT, std::string("CreateInputChannelResource failed for port ")
                    << locator.port << " msg: " << e.what());
            return false;
        }
    }

    return true;
}

bool SharedMemTransport::is_locator_allowed(
        const Locator& locator) const
{
    return IsLocatorSupported(locator);
}

bool SharedMemTransport::is_locator_reachable(
        const Locator_t& locator)
{
    bool is_reachable = SHMLocator::is_shm_and_from_this_host(locator);

    if (is_reachable)
    {
        try
        {
            is_reachable = (nullptr != find_port(locator.port));
        }
        catch (const std::exception& e)
        {
            (void)e;

            EPROSIMA_LOG_INFO(RTPS_MSG_OUT,
                    "Local SHM locator '" << locator <<
                    "' is not reachable; discarding. Reason: " << e.what());
            is_reachable = false;
        }
    }

    return is_reachable;
}

LocatorList SharedMemTransport::NormalizeLocator(
        const Locator& locator)
{
    LocatorList list;

    list.push_back(locator);

    return list;
}

bool SharedMemTransport::is_local_locator(
        const Locator& locator) const
{
    assert(locator.kind == LOCATOR_KIND_SHM);
    (void)locator;

    return true;
}

bool SharedMemTransport::is_localhost_allowed() const
{
    return true;
}

void SharedMemTransport::delete_input_channel(
        SharedMemChannelResource* channel)
{
    channel->disable();
    channel->release();
    channel->clear();
    delete channel;
}

bool SharedMemTransport::CloseInputChannel(
        const Locator& locator)
{
    std::lock_guard<std::recursive_mutex> lock(input_channels_mutex_);

    for (auto it = input_channels_.begin(); it != input_channels_.end(); it++)
    {
        if ((*it)->locator() == locator)
        {
            delete_input_channel((*it));
            input_channels_.erase(it);

            return true;
        }
    }

    return false;
}

void SharedMemTransport::clean_up()
{
    try
    {
        // Delete send ports
        {
            std::lock_guard<std::mutex> lock(opened_ports_mutex_);
            opened_ports_.clear();
        }

        // Delete input channels
        {
            std::lock_guard<std::recursive_mutex> lock(input_channels_mutex_);

            for (auto input_channel : input_channels_)
            {
                delete_input_channel(input_channel);
            }

            input_channels_.clear();
        }

        shared_mem_segment_.reset();
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_OUT, e.what());
    }
}

bool SharedMemTransport::DoInputLocatorsMatch(
        const Locator& left,
        const Locator& right) const
{
    return left.kind == right.kind && left.port == right.port;
}

bool SharedMemTransport::init(
        const fastdds::rtps::PropertyPolicy*,
        const uint32_t& max_msg_size_no_frag)
{
    (void) max_msg_size_no_frag;

    if (configuration_.segment_size() == 0)
    {
        configuration_.segment_size(shm_default_segment_size);
    }

    if (configuration_.segment_size() < configuration_.max_message_size())
    {
        EPROSIMA_LOG_ERROR(RTPS_MSG_OUT, "max_message_size cannot be greater than segment_size");
        return false;
    }

#ifdef ANDROID
    if (access(BOOST_INTERPROCESS_SHARED_DIR_PATH, W_OK) != F_OK)
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_OUT,
                "Unable to write on " << BOOST_INTERPROCESS_SHARED_DIR_PATH << ". SHM Transport not enabled");
        return false;
    }
#endif // ifdef ANDROID

    try
    {
        shared_mem_manager_ = SharedMemManager::create(SHM_MANAGER_DOMAIN);
        if (!shared_mem_manager_)
        {
            return false;
        }
        constexpr uint32_t mean_message_size =
                shm_default_segment_size / SharedMemTransportDescriptor::shm_default_port_queue_capacity;
        uint32_t max_allocations = configuration_.segment_size() / mean_message_size;
        if (configuration_.port_queue_capacity() > max_allocations)
        {
            max_allocations = configuration_.port_queue_capacity();
        }
        shared_mem_segment_ = shared_mem_manager_->create_segment(configuration_.segment_size(), max_allocations);

        // Memset the whole segment to zero in order to force physical map of the buffer
        auto buffer = shared_mem_segment_->alloc_buffer(configuration_.segment_size(),
                        (std::chrono::steady_clock::now() + std::chrono::milliseconds(100)));
        memset(buffer->data(), 0, configuration_.segment_size());
        buffer.reset();

        if (!configuration_.rtps_dump_file().empty())
        {
            auto packets_file_consumer = std::unique_ptr<SHMPacketFileConsumer>(
                new SHMPacketFileConsumer(configuration_.rtps_dump_file()));

            packet_logger_ = std::make_shared<PacketsLog<SHMPacketFileConsumer>>(0, configuration_.dump_thread());
            packet_logger_->RegisterConsumer(std::move(packets_file_consumer));
        }
    }
    catch (std::exception& e)
    {
        EPROSIMA_LOG_ERROR(RTPS_MSG_OUT, e.what());
        return false;
    }

    return true;
}

bool SharedMemTransport::IsInputChannelOpen(
        const Locator& locator) const
{
    std::lock_guard<std::recursive_mutex> lock(input_channels_mutex_);

    return IsLocatorSupported(locator) && (std::find_if(
               input_channels_.begin(), input_channels_.end(),
               [&](const SharedMemChannelResource* resource)
               {
                   return locator == resource->locator();
               }) != input_channels_.end());
}

bool SharedMemTransport::IsLocatorSupported(
        const Locator& locator) const
{
    return locator.kind == transport_kind_;
}

SharedMemChannelResource* SharedMemTransport::CreateInputChannelResource(
        const Locator& locator,
        uint32_t maxMsgSize,
        TransportReceiverInterface* receiver)
{
    (void) maxMsgSize;

    // Multicast locators implies ReadShared (Multiple readers) ports.
    auto open_mode = locator.address[0] == 'M' ? SharedMemGlobal::Port::OpenMode::ReadShared :
            SharedMemGlobal::Port::OpenMode::ReadExclusive;

    return new SharedMemChannelResource(
        shared_mem_manager_->open_port(
            locator.port,
            configuration_.port_queue_capacity(),
            configuration_.healthy_check_timeout_ms(),
            open_mode)->create_listener(),
        locator,
        receiver,
        configuration_.rtps_dump_file(),
        configuration_.dump_thread(),
        true,
        configuration_.get_thread_config_for_port(locator.port));
}

bool SharedMemTransport::OpenOutputChannel(
        SendResourceList& sender_resource_list,
        const Locator& locator)
{
    if (!IsLocatorSupported(locator))
    {
        return false;
    }

    // We try to find a SenderResource that can be reuse to this locator.
    // Note: This is done in this level because if we do in NetworkFactory level, we have to mantain what transport
    // already reuses a SenderResource.
    for (auto& sender_resource : sender_resource_list)
    {
        SharedMemSenderResource* sm_sender_resource = SharedMemSenderResource::cast(*this, sender_resource.get());

        if (sm_sender_resource)
        {
            return true;
        }
    }

    try
    {
        sender_resource_list.emplace_back(
            static_cast<SenderResource*>(new SharedMemSenderResource(*this)));
    }
    catch (std::exception& e)
    {
        EPROSIMA_LOG_ERROR(RTPS_MSG_OUT, "SharedMemTransport error opening port " << std::to_string(locator.port)
                                                                                  << " with msg: " << e.what());

        return false;
    }

    return true;
}

bool SharedMemTransport::OpenOutputChannels(
        SendResourceList& send_resource_list,
        const LocatorSelectorEntry& locator_selector_entry)
{
    bool success = false;
    for (size_t i = 0; i < locator_selector_entry.state.unicast.size(); ++i)
    {
        size_t index = locator_selector_entry.state.unicast[i];
        success |= OpenOutputChannel(send_resource_list, locator_selector_entry.unicast[index]);
    }
    return success;
}

Locator SharedMemTransport::RemoteToMainLocal(
        const Locator& remote) const
{
    if (!IsLocatorSupported(remote))
    {
        return false;
    }

    Locator mainLocal(remote);
    mainLocal.set_Invalid_Address();
    return mainLocal;
}

bool SharedMemTransport::transform_remote_locator(
        const Locator& remote_locator,
        Locator& result_locator,
        bool,
        bool) const
{
    if (IsLocatorSupported(remote_locator))
    {
        result_locator = remote_locator;

        return true;
    }

    return false;
}

std::shared_ptr<SharedMemManager::Buffer> SharedMemTransport::copy_to_shared_buffer(
        const std::vector<NetworkBuffer>& buffers,
        uint32_t total_bytes,
        const std::chrono::steady_clock::time_point& max_blocking_time_point)
{
    using namespace eprosima::fastdds::statistics::rtps;

    assert(shared_mem_segment_);

    // Statistics submessage is always the last buffer to be added
    // If statistics message is present, skip last buffer
    auto it_end = remove_statistics_buffer(buffers.back(), total_bytes) ? std::prev(buffers.end()) : buffers.end();

    std::shared_ptr<SharedMemManager::Buffer> shared_buffer =
            shared_mem_segment_->alloc_buffer(total_bytes, max_blocking_time_point);
    uint8_t* pos = static_cast<uint8_t*>(shared_buffer->data());

    for (auto it = buffers.begin(); it != it_end; ++it)
    {
        // Direct copy from the const_buffer to the mutable shared_buffer
        memcpy(pos, (it->buffer), it->size);
        pos += it->size;
    }

    return shared_buffer;
}

bool SharedMemTransport::send(
        const std::vector<NetworkBuffer>& buffers,
        uint32_t total_bytes,
        fastdds::rtps::LocatorsIterator* destination_locators_begin,
        fastdds::rtps::LocatorsIterator* destination_locators_end,
        const std::chrono::steady_clock::time_point& max_blocking_time_point)
{
    using namespace eprosima::fastdds::statistics::rtps;

#if !defined(_WIN32)
    cleanup_output_ports();
#endif // if !defined(_WIN32)

    fastdds::rtps::LocatorsIterator& it = *destination_locators_begin;

    bool ret = true;

    std::shared_ptr<SharedMemManager::Buffer> shared_buffer;

    try
    {
        while (it != *destination_locators_end)
        {
            if (IsLocatorSupported(*it))
            {
                // Only copy the first time
                if (shared_buffer == nullptr)
                {
                    shared_buffer = copy_to_shared_buffer(buffers, total_bytes, max_blocking_time_point);
                }

                ret &= send(shared_buffer, *it);

                if (packet_logger_ && ret)
                {
                    packet_logger_->QueueLog({packet_logger_->now(), Locator(), *it, shared_buffer});
                }
            }

            ++it;
        }
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_INFO(RTPS_TRANSPORT_SHM, e.what());
        (void)e;

        // Segment overflow with discard policy doesn't return error.
        if (!shared_buffer)
        {
            ret = true;
        }
        else
        {
            ret = false;
        }
    }

    return ret;

}

void SharedMemTransport::cleanup_output_ports()
{
    std::lock_guard<std::mutex> lock(opened_ports_mutex_);
    auto it = opened_ports_.begin();
    while (it != opened_ports_.end())
    {
        if (it->second->has_listeners())
        {
            ++it;
        }
        else
        {
            it = opened_ports_.erase(it);
        }
    }
}

std::shared_ptr<SharedMemManager::Port> SharedMemTransport::find_port(
        uint32_t port_id)
{

    {
        std::lock_guard<std::mutex> lock(opened_ports_mutex_);

        auto ports_it = opened_ports_.find(port_id);

        // The port is already opened
        if (ports_it != opened_ports_.end())
        {
            return (*ports_it).second;
        }
    }

    // The port is not opened
    std::shared_ptr<SharedMemManager::Port> port = shared_mem_manager_->
                    open_port(port_id, configuration_.port_queue_capacity(), configuration_.healthy_check_timeout_ms(),
                    SharedMemGlobal::Port::OpenMode::Write);

    {
        std::lock_guard<std::mutex> lock(opened_ports_mutex_);
        opened_ports_[port_id] = port;
    }

    return port;
}

bool SharedMemTransport::push_discard(
        const std::shared_ptr<SharedMemManager::Buffer>& buffer,
        const Locator& remote_locator)
{
    try
    {
        bool is_port_ok = false;
        const size_t num_retries = 2;
        for (size_t i = 0; i < num_retries && !is_port_ok; ++i)
        {
            if (!find_port(remote_locator.port)->try_push(buffer, is_port_ok))
            {
                if (is_port_ok)
                {
                    EPROSIMA_LOG_INFO(RTPS_MSG_OUT, "Port " << remote_locator.port << " full. Buffer dropped");
                }
                else
                {
                    std::lock_guard<std::mutex> lock(opened_ports_mutex_);
                    EPROSIMA_LOG_WARNING(RTPS_MSG_OUT, "Port " << remote_locator.port << " inconsistent. Port dropped");
                    opened_ports_.erase(remote_locator.port);
                }
            }
        }
    }
    catch (const std::exception& error)
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_OUT, error.what());
        return false;
    }

    return true;
}

bool SharedMemTransport::send(
        const std::shared_ptr<SharedMemManager::Buffer>& buffer,
        const Locator& remote_locator)
{
    if (!push_discard(buffer, remote_locator))
    {
        return false;
    }

    EPROSIMA_LOG_INFO(RTPS_MSG_OUT,
            "(ID:" << std::this_thread::get_id() << ") " << "SharedMemTransport: " << buffer->size() << " bytes to port " <<
            remote_locator.port);

    return true;
}

/**
 * Invalidate all selector entries containing certain multicast locator.
 *
 * This function will process all entries from 'index' onwards and, if any
 * of them has 'locator' on its multicast list, will invalidate them
 * (i.e. their 'transport_should_process' flag will be changed to false).
 *
 * If this function returns true, the locator received should be selected.
 *
 * @param entries   Selector entries collection to process
 * @param index     Starting index to process
 * @param locator   Locator to be searched
 *
 * @return true when at least one entry was invalidated, false otherwise
 */
void SharedMemTransport::select_locators(
        LocatorSelector& selector) const
{
    fastdds::ResourceLimitedVector<LocatorSelectorEntry*>& entries = selector.transport_starts();

    for (size_t i = 0; i < entries.size(); ++i)
    {
        LocatorSelectorEntry* entry = entries[i];
        if (entry->transport_should_process)
        {
            bool selected = false;

            // With shared-memory transport using multicast vs unicast is not an advantage
            // because no copies are saved. So no multicast locators are selected
            for (size_t j = 0; j < entry->unicast.size(); ++j)
            {
                if (IsLocatorSupported(entry->unicast[j]) && !selector.is_selected(entry->unicast[j]))
                {
                    entry->state.unicast.push_back(j);
                    selected = true;
                }
            }

            // Select this entry if necessary
            if (selected)
            {
                selector.select(i);
            }
        }
    }
}

bool SharedMemTransport::fillMetatrafficMulticastLocator(
        Locator& locator,
        uint32_t metatraffic_multicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = metatraffic_multicast_port;
    }

    return true;
}

bool SharedMemTransport::fillMetatrafficUnicastLocator(
        Locator& locator,
        uint32_t metatraffic_unicast_port) const
{
    if (locator.port == 0)
    {
        locator.port = metatraffic_unicast_port;
    }
    return true;
}

bool SharedMemTransport::configureInitialPeerLocator(
        Locator& locator,
        const PortParameters& port_params,
        uint32_t domainId,
        LocatorList& list) const
{
    if (locator.port == 0)
    {
        for (uint32_t i = 0; i < configuration()->maxInitialPeersRange; ++i)
        {
            Locator auxloc(locator);
            auxloc.port = port_params.getUnicastPort(domainId, i);

            list.push_back(auxloc);
        }
    }
    else
    {
        list.push_back(locator);
    }

    return true;
}

bool SharedMemTransport::fillUnicastLocator(
        Locator& locator,
        uint32_t well_known_port) const
{
    if (locator.port == 0)
    {
        locator.port = well_known_port;
    }

    return true;
}

}  // namsepace rtps
}  // namespace fastdds
}  // namespace eprosima
