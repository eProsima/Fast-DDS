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

// Link-seam implementation of TypeLookupManager: replaces src/cpp/fastdds/builtin/type_lookup_service/
// TypeLookupManager.cpp and forwards the methods tests need to the TypeLookupManagerMock companion of the instance.
// No builtin endpoint is created, and complete type information is propagated.

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>

#include <mock/fastdds/builtin/type_lookup_service/TypeLookupManagerMock.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

using Companion = mock::MockCompanion<TypeLookupManager>;

TypeLookupManager::TypeLookupManager()
{
    Companion::attach(this);
}

TypeLookupManager::~TypeLookupManager()
{
    Companion::detach(this);
}

void TypeLookupManager::remove_remote_endpoints(
        fastdds::rtps::ParticipantProxyData* pdata)
{
    Companion::get(this).remove_remote_endpoints(pdata);
}

SampleIdentity TypeLookupManager::get_type_dependencies(
        const xtypes::TypeIdentifierSeq& id_seq,
        const fastdds::rtps::GUID_t& type_server,
        const std::vector<uint8_t>& continuation_point) const
{
    return Companion::get(this).get_type_dependencies(id_seq, type_server, continuation_point);
}

SampleIdentity TypeLookupManager::get_types(
        const xtypes::TypeIdentifierSeq& id_seq,
        const fastdds::rtps::GUID_t& type_server) const
{
    return Companion::get(this).get_types(id_seq, type_server);
}

ReturnCode_t TypeLookupManager::async_get_type(
        eprosima::ProxyPool<eprosima::fastdds::rtps::WriterProxyData>::smart_ptr& temp_proxy_data,
        const fastdds::rtps::GUID_t& type_server,
        const AsyncGetTypeWriterCallback& callback)
{
    return Companion::get(this).async_get_type(temp_proxy_data, type_server, callback);
}

ReturnCode_t TypeLookupManager::async_get_type(
        eprosima::ProxyPool<eprosima::fastdds::rtps::ReaderProxyData>::smart_ptr& temp_proxy_data,
        const fastdds::rtps::GUID_t& type_server,
        const AsyncGetTypeReaderCallback& callback)
{
    return Companion::get(this).async_get_type(temp_proxy_data, type_server, callback);
}

TypeKind TypeLookupManager::get_type_kind_to_propagate() const
{
    return xtypes::EK_COMPLETE;
}

void TypeLookupManager::notify_callbacks(
        ReturnCode_t request_ret_status,
        const xtypes::TypeIdentfierWithSize& type_identifier_with_size)
{
    Companion::get(this).notify_callbacks(request_ret_status, type_identifier_with_size);
}

bool TypeLookupManager::add_async_get_type_request(
        const SampleIdentity& request,
        const xtypes::TypeIdentfierWithSize& type_identifier_with_size,
        const fastdds::rtps::GUID_t& type_server)
{
    return Companion::get(this).add_async_get_type_request(request, type_identifier_with_size, type_server);
}

bool TypeLookupManager::remove_async_get_type_request(
        SampleIdentity request)
{
    return Companion::get(this).remove_async_get_type_request(request);
}

bool TypeLookupManager::send(
        TypeLookup_Request& request) const
{
    return Companion::get(this).send(request);
}

bool TypeLookupManager::send(
        TypeLookup_Reply& reply) const
{
    return Companion::get(this).send(reply);
}

bool TypeLookupManager::receive(
        fastdds::rtps::CacheChange_t& change,
        TypeLookup_Request& request) const
{
    return Companion::get(this).receive(change, request);
}

bool TypeLookupManager::receive(
        fastdds::rtps::CacheChange_t& change,
        TypeLookup_Reply& reply) const
{
    return Companion::get(this).receive(change, reply);
}

void TypeLookupManager::remove_builtin_request_writer_history_change(
        fastdds::rtps::CacheChange_t* change)
{
    Companion::get(this).remove_builtin_request_writer_history_change(change);
}

void TypeLookupManager::remove_builtin_reply_writer_history_change(
        fastdds::rtps::CacheChange_t* change)
{
    Companion::get(this).remove_builtin_reply_writer_history_change(change);
}

} // namespace builtin
} // namespace dds
} // namespace fastdds
} // namespace eprosima
