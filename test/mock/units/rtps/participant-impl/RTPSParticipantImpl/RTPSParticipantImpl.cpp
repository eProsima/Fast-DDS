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

// Link-seam implementation of RTPSParticipantImpl: replaces src/cpp/rtps/participant/RTPSParticipantImpl.cpp, but the
// attribute accessors, provided by the RTPSParticipantImplAccessors mock. The participant has no transports, builtin
// protocols nor endpoints of its own, and only its event thread is started. The queries and the endpoint creation tests
// control are forwarded to the RTPSParticipantImplMock companion of the instance.

#include <rtps/participant/RTPSParticipantImpl.hpp>

#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <mock/rtps/participant/RTPSParticipantImplMock.hpp>
#include <rtps/messages/SendBuffersManager.hpp>
#include <statistics/rtps/monitor-service/MonitorService.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<RTPSParticipantImpl>;

RTPSParticipantImpl::RTPSParticipantImpl(
        uint32_t domain_id,
        const RTPSParticipantAttributes& PParam,
        const GuidPrefix_t& guidP,
        RTPSParticipant* par,
        RTPSParticipantListener* plisten)
    : RTPSParticipantImpl(domain_id, PParam, guidP, c_GuidPrefix_Unknown, par, plisten)
{
}

RTPSParticipantImpl::RTPSParticipantImpl(
        uint32_t domain_id,
        const RTPSParticipantAttributes& PParam,
        const GuidPrefix_t& guidP,
        const GuidPrefix_t&,
        RTPSParticipant* par,
        RTPSParticipantListener* plisten)
    : domain_id_(domain_id)
    , m_att(PParam)
    , m_const_att(PParam)
    , m_guid(guidP, c_EntityId_RTPSParticipant)
    , mp_builtinProtocols(nullptr)
    , IdCounter(0)
    , m_network_Factory(PParam)
    , type_check_fn_(nullptr)
    , client_override_(false)
    , internal_metatraffic_locators_(false)
    , internal_default_locators_(false)
#if HAVE_SECURITY
    , m_security_manager(this, PParam, *this)
#endif // if HAVE_SECURITY
    , mp_participantListener(plisten)
    , mp_userParticipant(par)
    , is_intraprocess_only_(false)
    , monitor_server_(nullptr)
    , conns_observer_(nullptr)
    , has_shm_transport_(false)
    , match_local_endpoints_(true)
{
    Companion::attach(this);
    mp_event_thr.init_thread();
}

RTPSParticipantImpl::~RTPSParticipantImpl()
{
    mp_event_thr.stop_thread();
    Companion::detach(this);
}

#if HAVE_SECURITY
security::Logging* RTPSParticipantImpl::create_builtin_logging_plugin()
{
    return nullptr;
}

#endif // if HAVE_SECURITY

bool RTPSParticipantImpl::register_in_writer(
        std::shared_ptr<fastdds::statistics::IListener>,
        GUID_t)
{
    return true;
}

bool RTPSParticipantImpl::register_in_reader(
        std::shared_ptr<fastdds::statistics::IListener>,
        GUID_t)
{
    return true;
}

bool RTPSParticipantImpl::unregister_in_writer(
        std::shared_ptr<fastdds::statistics::IListener>)
{
    return true;
}

bool RTPSParticipantImpl::unregister_in_reader(
        std::shared_ptr<fastdds::statistics::IListener>)
{
    return true;
}

void RTPSParticipantImpl::set_enabled_statistics_writers_mask(
        uint32_t)
{
}

bool RTPSParticipantImpl::get_entity_connections(
        const GUID_t&,
        fastdds::statistics::rtps::ConnectionList&)
{
    return true;
}

uint32_t RTPSParticipantImpl::getMaxDataSize()
{
    return Companion::get(this).getMaxDataSize();
}

template<EndpointKind_t kind, octet no_key, octet with_key>
bool RTPSParticipantImpl::preprocess_endpoint_attributes(
        const EntityId_t&,
        std::atomic<uint32_t>&,
        EndpointAttributes&,
        EntityId_t&)
{
    return true;
}

// The ones DataWriterImpl and DataReaderImpl use.
template bool RTPSParticipantImpl::preprocess_endpoint_attributes<WRITER, 0x03, 0x02>(
        const EntityId_t&,
        std::atomic<uint32_t>&,
        EndpointAttributes&,
        EntityId_t&);
template bool RTPSParticipantImpl::preprocess_endpoint_attributes<READER, 0x04, 0x07>(
        const EntityId_t&,
        std::atomic<uint32_t>&,
        EndpointAttributes&,
        EntityId_t&);

void RTPSParticipantImpl::get_sending_locators(
        rtps::LocatorList_t&) const
{
}

bool RTPSParticipantImpl::createWriter(
        RTPSWriter** Writer,
        WriterAttributes& param,
        WriterHistory* hist,
        WriterListener* listen,
        const EntityId_t& entityId,
        bool isBuiltin)
{
    RTPSParticipantImplMock& companion = Companion::get(this);
    bool ret = companion.createWriter(Writer, param, hist, listen, entityId, isBuiltin);
    if (nullptr != *Writer)
    {
        companion.endpoints.emplace((*Writer)->getGuid(), *Writer);
    }
    return ret;
}

bool RTPSParticipantImpl::create_writer(
        RTPSWriter** Writer,
        WriterAttributes& watt,
        WriterHistory* hist,
        WriterListener* listen,
        const EntityId_t& entityId,
        bool isBuiltin)
{
    return createWriter(Writer, watt, hist, listen, entityId, isBuiltin);
}

bool RTPSParticipantImpl::createReader(
        RTPSReader** Reader,
        ReaderAttributes& param,
        ReaderHistory* hist,
        ReaderListener* listen,
        const EntityId_t& entityId,
        bool isBuiltin,
        bool enable)
{
    RTPSParticipantImplMock& companion = Companion::get(this);
    bool ret = companion.createReader(Reader, param, hist, listen, entityId, isBuiltin, enable);
    if (nullptr != *Reader)
    {
        (*Reader)->set_listener(listen);
        companion.endpoints.emplace((*Reader)->getGuid(), *Reader);
    }
    return ret;
}

bool RTPSParticipantImpl::createReader(
        RTPSReader** Reader,
        ReaderAttributes& param,
        const std::shared_ptr<IPayloadPool>&,
        ReaderHistory* hist,
        ReaderListener* listen,
        const EntityId_t& entityId,
        bool isBuiltin,
        bool enable)
{
    return createReader(Reader, param, hist, listen, entityId, isBuiltin, enable);
}

bool RTPSParticipantImpl::deleteUserEndpoint(
        const GUID_t& endpoint)
{
    RTPSParticipantImplMock& companion = Companion::get(this);
    auto it = companion.endpoints.find(endpoint);
    if (it != companion.endpoints.end())
    {
        delete it->second;
        companion.endpoints.erase(it);
    }
    return true;
}

PDP* RTPSParticipantImpl::pdp()
{
    return Companion::get(this).pdp();
}

WLP* RTPSParticipantImpl::wlp()
{
    return Companion::get(this).wlp();
}

void RTPSParticipantImpl::setGuid(
        GUID_t& guid)
{
    Companion::get(this).setGuid(guid);
}

#if HAVE_SECURITY
bool RTPSParticipantImpl::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    return Companion::get(this).pairing_remote_reader_with_local_writer_after_security(local_writer,
                   remote_reader_data);
}

bool RTPSParticipantImpl::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    return Companion::get(this).pairing_remote_writer_with_local_reader_after_security(local_reader,
                   remote_writer_data);
}

#endif // if HAVE_SECURITY

uint32_t RTPSParticipantImpl::get_domain_id() const
{
    return Companion::get(this).get_domain_id();
}

bool RTPSParticipantImpl::should_send_optional_qos() const
{
    return Companion::get(this).should_send_optional_qos();
}

bool RTPSParticipantImpl::ignore_participant(
        const GuidPrefix_t& participant_guid)
{
    return Companion::get(this).ignore_participant(participant_guid);
}

void RTPSParticipantImpl::update_removed_participant(
        const LocatorList_t& remote_participant_locators)
{
    Companion::get(this).update_removed_participant(remote_participant_locators);
}

dds::utils::TypePropagation RTPSParticipantImpl::type_propagation() const
{
    return Companion::get(this).type_propagation();
}

void RTPSParticipantImpl::notify_reader_discovery(
        ReaderDiscoveryStatus reason,
        const SubscriptionBuiltinTopicData& info)
{
    notify_reader_discovery(reason, info, getListener());
}

void RTPSParticipantImpl::notify_reader_discovery(
        ReaderDiscoveryStatus reason,
        const SubscriptionBuiltinTopicData& info,
        RTPSParticipantListener* listener)
{
    Companion::get(this).notify_reader_discovery(reason, info, listener);
}

void RTPSParticipantImpl::notify_writer_discovery(
        WriterDiscoveryStatus reason,
        const PublicationBuiltinTopicData& info)
{
    notify_writer_discovery(reason, info, getListener());
}

void RTPSParticipantImpl::notify_writer_discovery(
        WriterDiscoveryStatus reason,
        const PublicationBuiltinTopicData& info,
        RTPSParticipantListener* listener)
{
    Companion::get(this).notify_writer_discovery(reason, info, listener);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
