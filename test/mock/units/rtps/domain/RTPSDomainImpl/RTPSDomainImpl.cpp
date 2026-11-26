// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file RTPSDomainImpl.cpp
 *
 * Mock implementation of RTPSDomainImpl, compiled against the real header. No participant is created: the virtual
 * methods forward to the RTPSDomainImplMock companion of each instance.
 */

#include <rtps/domain/RTPSDomainImpl.hpp>
#include <mock/rtps/domain/RTPSDomainImplMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

namespace {

RTPSDomainImplMock& companion(
        const RTPSDomainImpl* domain)
{
    return mock::MockCompanion<RTPSDomainImpl>::get(domain);
}

} // namespace

// RTPSDomainImpl has a defaulted destructor, so the companion cannot be detached: it is kept on purpose.
RTPSDomainImpl::RTPSDomainImpl()
{
    mock::MockCompanion<RTPSDomainImpl>::attach(this);
    testing::Mock::AllowLeak(&companion(this));
}

std::shared_ptr<IDomainImpl> RTPSDomainImpl::get_instance()
{
    static std::shared_ptr<RTPSDomainImpl> instance = std::make_shared<RTPSDomainImpl>();
    return instance;
}

void RTPSDomainImpl::stop_all()
{
    companion(this).stop_all();
}

RTPSParticipant* RTPSDomainImpl::create_participant(
        uint32_t domain_id,
        bool enabled,
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* plisten)
{
    return companion(this).create_participant(domain_id, enabled, attrs, plisten);
}

RTPSParticipant* RTPSDomainImpl::create_client_server_participant(
        uint32_t domain_id,
        bool enabled,
        const RTPSParticipantAttributes& attrs,
        RTPSParticipantListener* plisten)
{
    return companion(this).create_client_server_participant(domain_id, enabled, attrs, plisten);
}

bool RTPSDomainImpl::remove_writer(
        RTPSWriter* writer)
{
    return companion(this).remove_writer(writer);
}

bool RTPSDomainImpl::remove_reader(
        RTPSReader* reader)
{
    return companion(this).remove_reader(reader);
}

bool RTPSDomainImpl::remove_participant(
        RTPSParticipant* p)
{
    return companion(this).remove_participant(p);
}

bool RTPSDomainImpl::client_server_environment_attributes_override(
        uint32_t,
        RTPSParticipantAttributes&)
{
    return false;
}

RTPSWriter* RTPSDomainImpl::create_writer(
        RTPSParticipant* p,
        const EntityId_t& entity_id,
        WriterAttributes& watt,
        WriterHistory* hist,
        WriterListener* listen)
{
    return companion(this).create_writer(p, entity_id, watt, hist, listen);
}

bool RTPSDomainImpl::create_participant_guid(
        int32_t&,
        GUID_t& guid)
{
    guid.guidPrefix.value[11] = 1;
    return true;
}

RTPSParticipantImpl* RTPSDomainImpl::find_participant(
        const GUID_t& guid)
{
    return companion(this).find_participant(guid);
}

void RTPSDomainImpl::find_reader(
        std::shared_ptr<LocalReaderPointer>& local_reader,
        const GUID_t& reader_guid)
{
    companion(this).find_reader(local_reader, reader_guid);
}

BaseWriter* RTPSDomainImpl::find_writer(
        const GUID_t& writer_guid)
{
    return companion(this).find_writer(writer_guid);
}

bool RTPSDomainImpl::should_intraprocess_between(
        const GUID_t&,
        const GUID_t&)
{
    return false;
}

void RTPSDomainImpl::file_watch_callback()
{
    companion(this).file_watch_callback();
}

void RTPSDomainImpl::set_filewatch_thread_config(
        const fastdds::rtps::ThreadSettings& watch_thread,
        const fastdds::rtps::ThreadSettings& callback_thread)
{
    companion(this).set_filewatch_thread_config(watch_thread, callback_thread);
}

bool RTPSDomainImpl::get_library_settings(
        fastdds::LibrarySettings&)
{
    return true;
}

bool RTPSDomainImpl::set_library_settings(
        const fastdds::LibrarySettings&)
{
    return true;
}

fastdds::dds::xtypes::ITypeObjectRegistry& RTPSDomainImpl::type_object_registry()
{
    return type_object_registry_;
}

fastdds::dds::xtypes::TypeObjectRegistry& RTPSDomainImpl::type_object_registry_observer()
{
    return type_object_registry_;
}

bool RTPSDomainImpl::run_easy_mode_discovery_server(
        uint32_t domain_id,
        const std::string& easy_mode_ip)
{
    return companion(this).run_easy_mode_discovery_server(domain_id, easy_mode_ip);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
