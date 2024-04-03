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

/**
 * @file TypeLookupManager.cpp
 *
 */

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>

#include <algorithm>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/RTPSDomainImpl.hpp>

namespace eprosima {

using namespace fastrtps::rtps;
using eprosima::fastdds::dds::Log;

namespace fastdds {
namespace dds {
namespace builtin {


inline SequenceNumber_t sequence_number_rtps_2_dds(
        const fastrtps::rtps::SequenceNumber_t& seq_number)
{
    return *reinterpret_cast<const SequenceNumber_t*>(&seq_number);
}

inline GUID_t guid_rtps_2_dds(
        const fastrtps::rtps::GUID_t& rtps_guid)
{
    return *reinterpret_cast<const GUID_t*>(&rtps_guid);
}

inline fastrtps::rtps::GUID_t guid_dds_2_rtps(
        const GUID_t& guid)
{
    return *reinterpret_cast<const fastrtps::rtps::GUID_t*>(&guid);
}

TypeLookupManager::TypeLookupManager()
{
}

TypeLookupManager::~TypeLookupManager()
{
    if (nullptr != builtin_reply_reader_)
    {
        participant_->deleteUserEndpoint(builtin_reply_reader_->getGuid());
    }
    if (nullptr != builtin_reply_writer_)
    {
        participant_->deleteUserEndpoint(builtin_reply_writer_->getGuid());
    }
    if (nullptr != builtin_request_reader_)
    {
        participant_->deleteUserEndpoint(builtin_request_reader_->getGuid());
    }
    if (nullptr != builtin_request_writer_)
    {
        participant_->deleteUserEndpoint(builtin_request_writer_->getGuid());
    }
    delete builtin_request_writer_history_;
    delete builtin_reply_writer_history_;
    delete builtin_request_reader_history_;
    delete builtin_reply_reader_history_;

    delete reply_listener_;
    delete request_listener_;

    delete temp_reader_proxy_data_;
    delete temp_writer_proxy_data_;
}

bool TypeLookupManager::init(
        fastrtps::rtps::BuiltinProtocols* protocols)
{
    participant_ = protocols->mp_participantImpl;
    builtin_protocols_ = protocols;

    local_instance_name_ = get_instance_name(participant_->getGuid());

    temp_reader_proxy_data_ = new fastrtps::rtps::ReaderProxyData(
        protocols->mp_participantImpl->getRTPSParticipantAttributes().allocation.locators.max_unicast_locators,
        protocols->mp_participantImpl->getRTPSParticipantAttributes().allocation.locators.max_multicast_locators);
    temp_writer_proxy_data_ = new fastrtps::rtps::WriterProxyData(
        protocols->mp_participantImpl->getRTPSParticipantAttributes().allocation.locators.max_unicast_locators,
        protocols->mp_participantImpl->getRTPSParticipantAttributes().allocation.locators.max_multicast_locators);


    // Check if ReaderProxyData and WriterProxyData objects were created successfully
    if (temp_reader_proxy_data_ && temp_writer_proxy_data_)
    {
        return create_endpoints();
    }
    else
    {
        // Clean up on failure and return false
        delete temp_reader_proxy_data_;
        delete temp_writer_proxy_data_;
        return false;
    }
}

bool TypeLookupManager::assign_remote_endpoints(
        const ParticipantProxyData& pdata)
{
    const NetworkFactory& network = participant_->network_factory();
    uint32_t endp = pdata.m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;

    std::lock_guard<std::mutex> data_guard(temp_data_lock_);

    temp_writer_proxy_data_->guid().guidPrefix = pdata.m_guid.guidPrefix;
    temp_writer_proxy_data_->persistence_guid().guidPrefix = pdata.m_guid.guidPrefix;
    temp_writer_proxy_data_->set_remote_locators(pdata.metatraffic_locators, network, true);
    temp_writer_proxy_data_->topicKind(NO_KEY);
    temp_writer_proxy_data_->m_qos.m_durability.kind = fastrtps::VOLATILE_DURABILITY_QOS;
    temp_writer_proxy_data_->m_qos.m_reliability.kind = fastrtps::RELIABLE_RELIABILITY_QOS;

    temp_reader_proxy_data_->clear();
    temp_reader_proxy_data_->m_expectsInlineQos = false;
    temp_reader_proxy_data_->guid().guidPrefix = pdata.m_guid.guidPrefix;
    temp_reader_proxy_data_->set_remote_locators(pdata.metatraffic_locators, network, true);
    temp_reader_proxy_data_->topicKind(NO_KEY);
    temp_reader_proxy_data_->m_qos.m_durability.kind = fastrtps::VOLATILE_DURABILITY_QOS;
    temp_reader_proxy_data_->m_qos.m_reliability.kind = fastrtps::RELIABLE_RELIABILITY_QOS;

    EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "for RTPSParticipant: " << pdata.m_guid);

    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_WRITER;

    if (auxendp != 0 && builtin_request_reader_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Adding remote writer to the local Builtin Request Reader");
        temp_writer_proxy_data_->guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_request_writer;
        temp_writer_proxy_data_->persistence_guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_request_writer;
        builtin_request_reader_->matched_writer_add(*temp_writer_proxy_data_);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_WRITER;

    if (auxendp != 0 && builtin_reply_reader_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Adding remote writer to the local Builtin Reply Reader");
        temp_writer_proxy_data_->guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_reply_writer;
        temp_writer_proxy_data_->persistence_guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_reply_writer;
        builtin_reply_reader_->matched_writer_add(*temp_writer_proxy_data_);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_READER;

    if (auxendp != 0 && builtin_request_writer_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Adding remote reader to the local Builtin Request Writer");
        temp_reader_proxy_data_->guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_request_reader;
        builtin_request_writer_->matched_reader_add(*temp_reader_proxy_data_);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_READER;

    if (auxendp != 0 && builtin_reply_writer_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Adding remote reader to the local Builtin Reply Writer");
        temp_reader_proxy_data_->guid().entityId = fastrtps::rtps::c_EntityId_TypeLookup_reply_reader;
        builtin_reply_writer_->matched_reader_add(*temp_reader_proxy_data_);
    }

    return true;
}

void TypeLookupManager::remove_remote_endpoints(
        fastrtps::rtps::ParticipantProxyData* pdata)
{
    fastrtps::rtps::GUID_t tmp_guid;
    tmp_guid.guidPrefix = pdata->m_guid.guidPrefix;

    EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "for RTPSParticipant: " << pdata->m_guid);
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t partdet = endp;
    uint32_t auxendp = endp;
    partdet &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR; //Habria que quitar esta linea que comprueba si tiene PDP.
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_WRITER;

    if ((auxendp != 0 || partdet != 0) && builtin_request_reader_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Removing remote writer from the local Builtin Request Reader");
        tmp_guid.entityId = fastrtps::rtps::c_EntityId_TypeLookup_request_writer;
        builtin_request_reader_->matched_writer_remove(tmp_guid);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_WRITER;

    if ((auxendp != 0 || partdet != 0) && builtin_reply_reader_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Removing remote writer from the local Builtin Reply Reader");
        tmp_guid.entityId = fastrtps::rtps::c_EntityId_TypeLookup_reply_writer;
        builtin_reply_reader_->matched_writer_remove(tmp_guid);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_READER;

    if ((auxendp != 0 || partdet != 0) && builtin_request_writer_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Removing remote reader from the local Builtin Request Writer");
        tmp_guid.entityId = fastrtps::rtps::c_EntityId_TypeLookup_request_reader;
        builtin_request_writer_->matched_reader_remove(tmp_guid);
    }

    auxendp = endp;
    auxendp &= BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_READER;

    if ((auxendp != 0 || partdet != 0) && builtin_reply_writer_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Removing remote reader from the local Builtin Reply Writer");
        tmp_guid.entityId = fastrtps::rtps::c_EntityId_TypeLookup_reply_reader;
        builtin_reply_writer_->matched_reader_remove(tmp_guid);
    }
}

SampleIdentity TypeLookupManager::get_type_dependencies(
        const xtypes::TypeIdentifierSeq& id_seq,
        const fastrtps::rtps::GUID_t& type_server,
        const std::vector<uint8_t>& continuation_point) const
{
    TypeLookup_getTypeDependencies_In in;
    in.type_ids(id_seq);
    if (!continuation_point.empty())
    {
        in.continuation_point(continuation_point);
    }

    // Create a generic TypeLookup_Request
    TypeLookup_RequestPubSubType type;
    TypeLookup_Request* request = create_request(type_server, type);

    // Add the specific data to the request
    request->data().getTypeDependencies(in);

    SampleIdentity id = INVALID_SAMPLE_IDENTITY;
    if (send(*request))
    {
        id = request->header().requestId();
    }
    // Delete request data after sending
    type.deleteData(request);
    return id;
}

SampleIdentity TypeLookupManager::get_types(
        const xtypes::TypeIdentifierSeq& id_seq,
        const fastrtps::rtps::GUID_t& type_server) const
{
    TypeLookup_getTypes_In in;
    in.type_ids(id_seq);

    // Create a generic TypeLookup_Request
    TypeLookup_RequestPubSubType type;
    TypeLookup_Request* request = create_request(type_server, type);

    // Add the specific data to the request
    request->data().getTypes(in);

    SampleIdentity id = INVALID_SAMPLE_IDENTITY;
    if (send(*request))
    {
        id = request->header().requestId();
    }
    // Delete request data after sending
    type.deleteData(request);
    return id;
}

ReturnCode_t TypeLookupManager::async_get_type(
        eprosima::ProxyPool<eprosima::fastrtps::rtps::WriterProxyData>::smart_ptr& temp_writer_data,
        const AsyncGetTypeWriterCallback& callback)
{
    return check_type_identifier_received<eprosima::fastrtps::rtps::WriterProxyData>(
        temp_writer_data, callback, async_get_type_writer_callbacks_);
}

ReturnCode_t TypeLookupManager::async_get_type(
        eprosima::ProxyPool<eprosima::fastrtps::rtps::ReaderProxyData>::smart_ptr&  temp_reader_data,
        const AsyncGetTypeReaderCallback& callback)
{
    return check_type_identifier_received<eprosima::fastrtps::rtps::ReaderProxyData>(
        temp_reader_data, callback, async_get_type_reader_callbacks_);
}

template <typename ProxyType, typename AsyncCallback>
ReturnCode_t TypeLookupManager::check_type_identifier_received(
        typename eprosima::ProxyPool<ProxyType>::smart_ptr& temp_proxy_data,
        const AsyncCallback& callback,
        std::unordered_map<xtypes::TypeIdentfierWithSize,
        std::vector<std::pair<typename eprosima::ProxyPool<ProxyType>::smart_ptr,
        AsyncCallback>>>& async_get_type_callbacks)
{
    xtypes::TypeIdentfierWithSize type_identifier_with_size =
            temp_proxy_data->type_information().type_information.complete().typeid_with_size();
    fastrtps::rtps::GUID_t type_server = temp_proxy_data->guid();

    // Check if the type is known
    if (fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                    is_type_identifier_known(type_identifier_with_size))
    {
        // The type is already known, invoke the callback
        callback(temp_proxy_data);
        return RETCODE_OK;
    }

    {
        // Check if TypeIdentfierWithSize already exists in the map
        std::lock_guard<std::mutex> lock(async_get_types_mutex_);
        auto it = async_get_type_callbacks.find(type_identifier_with_size);
        if (it != async_get_type_callbacks.end())
        {
            // TypeIdentfierWithSize exists, add the callback
            it->second.push_back(std::make_pair(std::move(temp_proxy_data), callback));
            // Return without sending new request
            return RETCODE_NO_DATA;
        }
    }

    // TypeIdentfierWithSize doesn't exist, create a new entry
    SampleIdentity get_type_dependencies_request = get_type_dependencies(
        {type_identifier_with_size.type_id()}, type_server);
    if (INVALID_SAMPLE_IDENTITY != get_type_dependencies_request)
    {
        // Store the sent requests and callback
        add_async_get_type_request(get_type_dependencies_request, type_identifier_with_size);
        std::vector<std::pair<typename eprosima::ProxyPool<ProxyType>::smart_ptr, AsyncCallback>> types;
        types.push_back(std::make_pair(std::move(temp_proxy_data), callback));
        async_get_type_callbacks.emplace(type_identifier_with_size, std::move(types));

        return RETCODE_NO_DATA;
    }
    else
    {
        // Failed to send request, return error
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Failed to send get_type_dependencies request");
        return RETCODE_ERROR;
    }
}

void TypeLookupManager::notify_callbacks(
        xtypes::TypeIdentfierWithSize type_identifier_with_size)
{
    bool removed = false;
    // Check that type is pending to be resolved
    auto writer_callbacks_it = async_get_type_writer_callbacks_.find(type_identifier_with_size);
    if (writer_callbacks_it != async_get_type_writer_callbacks_.end())
    {
        for (auto& proxy_callback_pair : writer_callbacks_it->second)
        {
            proxy_callback_pair.second(proxy_callback_pair.first);
        }
        removed = true;
    }

    auto reader_callbacks_it = async_get_type_reader_callbacks_.find(type_identifier_with_size);
    if (reader_callbacks_it != async_get_type_reader_callbacks_.end())
    {
        for (auto& proxy_callback_pair : reader_callbacks_it->second)
        {
            proxy_callback_pair.second(proxy_callback_pair.first);
        }
        removed = true;
    }

    if (removed)
    {
        // Erase the solved TypeIdentfierWithSize
        remove_async_get_type_callback(type_identifier_with_size);
    }
}

bool TypeLookupManager::add_async_get_type_request(
        const SampleIdentity& request,
        const xtypes::TypeIdentfierWithSize& type_identifier_with_size)
{
    std::lock_guard<std::mutex> lock(async_get_types_mutex_);
    try
    {
        async_get_type_requests_.emplace(request, type_identifier_with_size);
        return true;
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE,
                "Error in TypeLookupManager::add_async_get_type_request: " << e.what());
        return false;
    }

}

bool TypeLookupManager::remove_async_get_type_callback(
        const xtypes::TypeIdentfierWithSize& type_identifier_with_size)
{
    std::lock_guard<std::mutex> lock(async_get_types_mutex_);
    try
    {
        bool removed = false;

        // Check if the key is in the writer map
        auto writer_it = async_get_type_writer_callbacks_.find(type_identifier_with_size);
        if (writer_it != async_get_type_writer_callbacks_.end())
        {
            async_get_type_writer_callbacks_.erase(writer_it);
            removed = true;
        }
        // Check if the key is in the reader map
        auto reader_it = async_get_type_reader_callbacks_.find(type_identifier_with_size);
        if (reader_it != async_get_type_reader_callbacks_.end())
        {
            async_get_type_reader_callbacks_.erase(reader_it);
            removed = true;
        }

        if (!removed)
        {
            // If not found in either map, log an error
            EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE,
                    "Error in TypeLookupManager::remove_async_get_type_callback: Key not found");
        }
        return removed;
    }
    catch (const std::exception& e)
    {
        // Log any exception that might occur during erasure
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE,
                "Error in TypeLookupManager::remove_async_get_type_callback: " << e.what());
        return false;
    }

}

bool TypeLookupManager::remove_async_get_type_request(
        SampleIdentity request)
{
    std::unique_lock<std::mutex> lock(async_get_types_mutex_);
    try
    {
        async_get_type_requests_.erase(request);
        return true;
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE,
                "Error in TypeLookupManager::remove_async_get_type_request: " << e.what());
        return false;
    }
}

bool TypeLookupManager::create_endpoints()
{
    bool ret = true;

    const RTPSParticipantAttributes& pattr = participant_->getRTPSParticipantAttributes();

    // Built-in history attributes.
    HistoryAttributes hatt;
    hatt.initialReservedCaches = 20;
    hatt.maximumReservedCaches = 1000;
    hatt.payloadMaxSize = TYPELOOKUP_DATA_MAX_SIZE;

    WriterAttributes watt;
    watt.endpoint.unicastLocatorList = builtin_protocols_->m_metatrafficUnicastLocatorList;
    watt.endpoint.multicastLocatorList = builtin_protocols_->m_metatrafficMulticastLocatorList;
    watt.endpoint.external_unicast_locators = builtin_protocols_->m_att.metatraffic_external_unicast_locators;
    watt.endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
    watt.endpoint.remoteLocatorList = builtin_protocols_->m_initialPeersList;
    watt.matched_readers_allocation = pattr.allocation.participants;
    watt.endpoint.topicKind = fastrtps::rtps::NO_KEY;
    watt.endpoint.reliabilityKind = fastrtps::rtps::RELIABLE;
    watt.endpoint.durabilityKind = fastrtps::rtps::VOLATILE;
    watt.mode = fastrtps::rtps::ASYNCHRONOUS_WRITER;

    ReaderAttributes ratt;
    ratt.endpoint.unicastLocatorList = builtin_protocols_->m_metatrafficUnicastLocatorList;
    ratt.endpoint.multicastLocatorList = builtin_protocols_->m_metatrafficMulticastLocatorList;
    ratt.endpoint.external_unicast_locators = builtin_protocols_->m_att.metatraffic_external_unicast_locators;
    ratt.endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
    ratt.endpoint.remoteLocatorList = builtin_protocols_->m_initialPeersList;
    ratt.matched_writers_allocation = pattr.allocation.participants;
    ratt.expectsInlineQos = true;
    ratt.endpoint.topicKind = fastrtps::rtps::NO_KEY;
    ratt.endpoint.reliabilityKind = fastrtps::rtps::RELIABLE;
    ratt.endpoint.durabilityKind = fastrtps::rtps::VOLATILE;

    // Built-in request writer
    request_listener_ = new TypeLookupRequestListener(this);
    builtin_request_writer_history_ = new WriterHistory(hatt);

    RTPSWriter* req_writer;
    if (participant_->createWriter(
                &req_writer,
                watt,
                builtin_request_writer_history_,
                request_listener_,
                fastrtps::rtps::c_EntityId_TypeLookup_request_writer,
                true))
    {
        builtin_request_writer_ = dynamic_cast<StatefulWriter*>(req_writer);
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Builtin Typelookup request writer created.");
    }
    else
    {
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Typelookup request writer creation failed.");
        ret = false;
    }

    // Built-in request reader
    builtin_request_reader_history_ = new ReaderHistory(hatt);

    RTPSReader* req_reader;
    if (participant_->createReader(
                &req_reader,
                ratt,
                builtin_request_reader_history_,
                request_listener_,
                fastrtps::rtps::c_EntityId_TypeLookup_request_reader,
                true))
    {
        builtin_request_reader_ = dynamic_cast<StatefulReader*>(req_reader);
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Builtin Typelookup request reader created.");
    }
    else
    {
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Typelookup request reader creation failed.");
        ret = false;
    }

    // Built-in reply writer
    reply_listener_ = new TypeLookupReplyListener(this);
    builtin_reply_writer_history_ = new WriterHistory(hatt);

    RTPSWriter* rep_writer;
    if (participant_->createWriter(
                &rep_writer,
                watt,
                builtin_reply_writer_history_,
                reply_listener_,
                fastrtps::rtps::c_EntityId_TypeLookup_reply_writer,
                true))
    {
        builtin_reply_writer_ = dynamic_cast<StatefulWriter*>(rep_writer);
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Builtin Typelookup reply writer created.");
    }
    else
    {
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Typelookup reply writer creation failed.");
        ret = false;
    }

    // Built-in reply reader
    builtin_reply_reader_history_ = new ReaderHistory(hatt);

    RTPSReader* rep_reader;
    if (participant_->createReader(
                &rep_reader,
                ratt,
                builtin_reply_reader_history_,
                reply_listener_,
                fastrtps::rtps::c_EntityId_TypeLookup_reply_reader,
                true))
    {
        builtin_reply_reader_ = dynamic_cast<StatefulReader*>(rep_reader);
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Builtin Typelookup reply reader created.");
    }
    else
    {
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Typelookup reply reader creation failed.");
        ret = false;
    }

    // Clean up if something failed.
    if (!ret)
    {
        if (nullptr != builtin_request_writer_history_)
        {
            delete builtin_request_writer_history_;
            builtin_request_writer_history_ = nullptr;
        }

        if (nullptr != builtin_reply_writer_history_)
        {
            delete builtin_reply_writer_history_;
            builtin_reply_writer_history_ = nullptr;
        }

        if (nullptr != builtin_request_reader_history_)
        {
            delete builtin_request_reader_history_;
            builtin_request_reader_history_ = nullptr;
        }

        if (nullptr != builtin_reply_reader_history_)
        {
            delete builtin_reply_reader_history_;
            builtin_reply_reader_history_ = nullptr;
        }

        if (nullptr != request_listener_)
        {
            delete request_listener_;
            request_listener_ = nullptr;
        }
        if (nullptr != reply_listener_)
        {
            delete reply_listener_;
            reply_listener_ = nullptr;
        }
    }

    return ret;
}

TypeLookup_Request* TypeLookupManager::create_request(
        const fastrtps::rtps::GUID_t& type_server,
        TypeLookup_RequestPubSubType& pupsubtype) const
{
    TypeLookup_Request* request = static_cast<TypeLookup_Request*>(pupsubtype.createData());
    request->header().instanceName() = get_instance_name(type_server);
    request->header().requestId().writer_guid(guid_rtps_2_dds(builtin_request_writer_->getGuid()));
    request->header().requestId().sequence_number(sequence_number_rtps_2_dds(request_seq_number_));
    request_seq_number_++;
    return request;
}

bool TypeLookupManager::send(
        TypeLookup_Request& request) const
{
    if (!send_impl(request, &request_type_, builtin_request_writer_, builtin_request_writer_history_))
    {
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE, "Error sending request.");
        return false;
    }
    return true;
}

bool TypeLookupManager::send(
        TypeLookup_Reply& reply) const
{
    if (!send_impl(reply, &reply_type_, builtin_reply_writer_, builtin_reply_writer_history_))
    {
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE, "Error sending reply.");
        return false;
    }
    return true;
}

template <typename Type, typename PubSubType>
bool TypeLookupManager::send_impl(
        Type& msg,
        PubSubType* pubsubtype,
        fastrtps::rtps::StatefulWriter* writer,
        fastrtps::rtps::WriterHistory* writer_history) const
{
    // Create a new CacheChange_t using the provided StatefulWriter
    CacheChange_t* change = writer->new_change(
        [&msg]()
        {
            // Calculate the serialized size of the message using a CdrSizeCalculator
            eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
            size_t current_alignment {0};
            return static_cast<uint32_t>(calculator.calculate_serialized_size(msg, current_alignment) + 4);
        },
        ALIVE);

    // Check if the creation of CacheChange_t was successful
    if (!change)
    {
        return false;
    }

    // Prepare the payload for sending the message
    SerializedPayload_t payload;
    payload.max_size = change->serializedPayload.max_size;
    payload.data = change->serializedPayload.data;

    // Serialize the message using the provided PubSubType
    bool result = pubsubtype->serialize(&msg, &payload, DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);
    // If serialization was successful, update the change and add it to the WriterHistory
    if (result)
    {
        change->serializedPayload.length += payload.length;
        change->serializedPayload.pos += payload.pos;
        result = writer_history->add_change(change);
    }
    // Release the payload data
    payload.data = nullptr;

    // If adding the change to WriterHistory failed, remove the change
    if (!result)
    {
        writer_history->remove_change(change);
    }

    return result;
}

bool TypeLookupManager::prepare_receive_payload(
        fastrtps::rtps::CacheChange_t& change,
        SerializedPayload_t& payload) const
{
    CDRMessage_t msg(change.serializedPayload);
    msg.pos += 1;

    payload.max_size = change.serializedPayload.max_size;
    payload.length = change.serializedPayload.length;
    payload.data = change.serializedPayload.data;
    return true;
}

bool TypeLookupManager::receive(
        fastrtps::rtps::CacheChange_t& change,
        TypeLookup_Request& request) const
{
    if (!receive_impl(change, request, &request_type_))
    {
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE, "Error receiving request.");
        return false;
    }

    //Compare only the "dds.builtin.TOS." + guid.guidPrefix
    if ((request.header().instanceName().to_string()).substr(0, 40) != local_instance_name_.substr(0, 40))
    {
        // Ignore request
        return false;
    }
    return true;
}

bool TypeLookupManager::receive(
        fastrtps::rtps::CacheChange_t& change,
        TypeLookup_Reply& reply) const
{
    if (!receive_impl(change, reply, &reply_type_))
    {
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE, "Error receiving reply.");
        return false;
    }

    if (guid_dds_2_rtps(reply.header().relatedRequestId().writer_guid()) != builtin_request_writer_->getGuid())
    {
        // Ignore reply
        return false;
    }
    return true;
}

template <typename Type, typename PubSubType>
bool TypeLookupManager::receive_impl(
        fastrtps::rtps::CacheChange_t& change,
        Type& msg,
        PubSubType* pubsubtype) const
{
    SerializedPayload_t payload;
    if (!prepare_receive_payload(change, payload))
    {
        return false;
    }

    bool result = pubsubtype->deserialize(&payload, &msg);
    payload.data = nullptr;

    return result;
}

std::string TypeLookupManager::get_instance_name(
        const fastrtps::rtps::GUID_t guid) const
{
    std::stringstream ss;
    ss << std::hex;
    for (const auto& elem : guid.guidPrefix.value)
    {
        ss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(elem);
    }
    for (const auto& elem : guid.entityId.value)
    {
        ss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(elem);
    }

    std::string str = ss.str();
    std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            });
    return "dds.builtin.TOS." + str;
}

void TypeLookupManager::remove_builtin_request_writer_history_change(
        fastrtps::rtps::CacheChange_t* change)
{
    builtin_request_writer_history_->remove_change(change);
}

void TypeLookupManager::remove_builtin_reply_writer_history_change(
        fastrtps::rtps::CacheChange_t* change)
{
    builtin_reply_writer_history_->remove_change(change);
}

} // namespace builtin

} // namespace dds
} // namespace fastdds
} // namespace eprosima
