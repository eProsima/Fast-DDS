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
#include <fastdds/rtps/builtin/BuiltinProtocols.h>
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

    local_instance_name_ = get_instance_name(participant_->getGuid().guidPrefix);

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
        const fastrtps::rtps::GuidPrefix_t& type_server) const
{
    SampleIdentity id = INVALID_SAMPLE_IDENTITY;

    TypeLookup_getTypeDependencies_In in;
    in.type_ids() = id_seq;
    TypeLookup_RequestPubSubType type;
    TypeLookup_Request* request = static_cast<TypeLookup_Request*>(type.createData());
    request->data().getTypeDependencies(in);

    if (send_request(type_server, *request))
    {
        id = request->header().requestId();
    }
    type.deleteData(request);
    return id;
}

SampleIdentity TypeLookupManager::get_types(
        const xtypes::TypeIdentifierSeq& id_seq,
        const fastrtps::rtps::GuidPrefix_t& type_server) const
{
    SampleIdentity id = INVALID_SAMPLE_IDENTITY;

    TypeLookup_getTypes_In in;
    in.type_ids() = id_seq;
    TypeLookup_RequestPubSubType type;
    TypeLookup_Request* request = static_cast<TypeLookup_Request*>(type.createData());
    request->data().getTypes(in);

    if (send_request(type_server, *request))
    {
        id = request->header().requestId();
    }
    type.deleteData(request);
    return id;
}

ReturnCode_t TypeLookupManager::async_get_type(
        eprosima::ProxyPool<eprosima::fastrtps::rtps::WriterProxyData>::smart_ptr&& temp_writer_data,
        const AsyncGetTypeWriterCallback& callback)
{
    ReturnCode_t result = check_type_identifier_received(std::move(temp_writer_data), callback);
    if (RETCODE_OK == result)
    {
        // The type is already known, invoke the callback
        callback(std::move(temp_writer_data));
    }
    return result;
}

ReturnCode_t TypeLookupManager::async_get_type(
        eprosima::ProxyPool<eprosima::fastrtps::rtps::ReaderProxyData>::smart_ptr&&  temp_reader_data,
        const AsyncGetTypeReaderCallback& callback)
{
    ReturnCode_t result = check_type_identifier_received(std::move(temp_reader_data), callback);
    if (RETCODE_OK == result)
    {
        // The type is already known, invoke the callback
        callback(std::move(temp_reader_data));
    }
    return result;
}

ReturnCode_t TypeLookupManager::check_type_identifier_received(
        eprosima::ProxyPool<eprosima::fastrtps::rtps::WriterProxyData>::smart_ptr&& temp_writer_data,
        const AsyncGetTypeWriterCallback& callback)
{
    return check_type_identifier_received_impl<eprosima::fastrtps::rtps::WriterProxyData>(
        std::move(temp_writer_data), callback, async_get_type_writer_callbacks_);
}

ReturnCode_t TypeLookupManager::check_type_identifier_received(
        eprosima::ProxyPool<eprosima::fastrtps::rtps::ReaderProxyData>::smart_ptr&& temp_reader_data,
        const AsyncGetTypeReaderCallback& callback)
{
    return check_type_identifier_received_impl<eprosima::fastrtps::rtps::ReaderProxyData>(
        std::move(temp_reader_data), callback, async_get_type_reader_callbacks_);
}

template <typename ProxyType, typename AsyncCallback>
ReturnCode_t TypeLookupManager::check_type_identifier_received_impl(
        typename eprosima::ProxyPool<ProxyType>::smart_ptr&& temp_proxy_data,
        const AsyncCallback& callback,
        std::unordered_map<xtypes::TypeIdentfierWithSize,
        std::vector<std::pair<typename eprosima::ProxyPool<ProxyType>::smart_ptr,
        AsyncCallback>>>& async_get_type_callbacks)
{
    xtypes::TypeIdentfierWithSize type_identifier_with_size =
            temp_proxy_data->type_information().type_information.complete().typeid_with_size();
    fastrtps::rtps::GuidPrefix_t type_server = temp_proxy_data->guid().guidPrefix;

    // Check if the type is known
    if (fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                    is_type_identifier_known(type_identifier_with_size))
    {
        return RETCODE_OK;
    }

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
    else
    {
        // TypeIdentfierWithSize doesn't exist, create a new entry
        xtypes::TypeIdentifierSeq unknown_type{type_identifier_with_size.type_id()};
        SampleIdentity get_type_dependencies_request = get_type_dependencies(unknown_type, type_server);
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
}

void TypeLookupManager::notify_callbacks(
        xtypes::TypeIdentfierWithSize type_identifier_with_size)
{
    // Check that type is not solved
    auto callbacks_it = async_get_type_callbacks_.find(type_identifier_with_size);
    if (callbacks_it != async_get_type_callbacks_.end())
    {
        for (AsyncGetTypeCallback& callback : callbacks_it->second)
        {
            callback();
        }
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

bool TypeLookupManager::remove_async_get_types_request(
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
                "Error in TypeLookupManager::remove_async_get_types_request: " << e.what());
        return false;
    }
}

bool TypeLookupManager::create_endpoints()
{
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

    // Built-in request writer
    builtin_request_writer_history_ = new WriterHistory(hatt);

    RTPSWriter* req_writer;
    if (participant_->createWriter(
                &req_writer,
                watt,
                builtin_request_writer_history_,
                nullptr,
                fastrtps::rtps::c_EntityId_TypeLookup_request_writer,
                true))
    {
        builtin_request_writer_ = dynamic_cast<StatefulWriter*>(req_writer);
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Builtin Typelookup request writer created.");
    }
    else
    {
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Typelookup request writer creation failed.");
        delete builtin_request_writer_history_;
        builtin_request_writer_history_ = nullptr;
        return false;
    }

    // Built-in reply writer
    builtin_reply_writer_history_ = new WriterHistory(hatt);

    RTPSWriter* rep_writer;
    if (participant_->createWriter(
                &rep_writer,
                watt,
                builtin_reply_writer_history_,
                nullptr,
                fastrtps::rtps::c_EntityId_TypeLookup_reply_writer,
                true))
    {
        builtin_reply_writer_ = dynamic_cast<StatefulWriter*>(rep_writer);
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Builtin Typelookup reply writer created.");
    }
    else
    {
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Typelookup reply writer creation failed.");
        delete builtin_reply_writer_history_;
        builtin_reply_writer_history_ = nullptr;
        return false;
    }

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

    // Built-in request reader
    request_listener_ = new TypeLookupRequestListener(this);
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
        delete builtin_request_reader_history_;
        builtin_request_reader_history_ = nullptr;
        delete request_listener_;
        request_listener_ = nullptr;
        return false;
    }

    // Built-in reply reader
    reply_listener_ = new TypeLookupReplyListener(this);
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
        delete builtin_reply_reader_history_;
        builtin_reply_reader_history_ = nullptr;
        delete reply_listener_;
        reply_listener_ = nullptr;
        return false;
    }

    return true;
}

bool TypeLookupManager::send_request(
        const fastrtps::rtps::GuidPrefix_t& type_server,
        TypeLookup_Request& request) const
{
    request.header().instanceName() = get_instance_name(type_server);
    request.header().requestId().writer_guid(guid_rtps_2_dds(builtin_request_writer_->getGuid()));
    request.header().requestId().sequence_number(sequence_number_rtps_2_dds(request_seq_number_));
    request_seq_number_++;

    CacheChange_t* change = builtin_request_writer_->new_change(
        [&request]()
        {
            eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv1);
            size_t current_alignment {0};
            return static_cast<uint32_t>(calculator.calculate_serialized_size(request, current_alignment) + 4);
        },
        ALIVE);

    if (change != nullptr)
    {
        CDRMessage_t msg(change->serializedPayload);

        bool valid = CDRMessage::addOctet(&msg, 0);
        change->serializedPayload.encapsulation = static_cast<uint16_t>(PL_DEFAULT_ENCAPSULATION);
        msg.msg_endian = DEFAULT_ENDIAN;
        valid &= CDRMessage::addOctet(&msg, PL_DEFAULT_ENCAPSULATION);
        valid &= CDRMessage::addUInt16(&msg, 0);

        change->serializedPayload.pos = msg.pos;
        change->serializedPayload.length = msg.length;

        SerializedPayload_t payload;
        payload.max_size = change->serializedPayload.max_size - 4;
        payload.data = change->serializedPayload.data + 4;
        if (valid && request_type_.serialize(&request, &payload, DataRepresentationId_t::XCDR2_DATA_REPRESENTATION))
        {
            change->serializedPayload.length += payload.length;
            change->serializedPayload.pos += payload.pos;
            payload.data = nullptr;
            return builtin_request_writer_history_->add_change(change);
        }
    }
    builtin_request_writer_history_->remove_change(change);
    return false;
}

bool TypeLookupManager::send_reply(
        TypeLookup_Reply& reply) const
{
    CacheChange_t* change = builtin_reply_writer_->new_change(
        [&reply]()
        {
            eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv1);
            size_t current_alignment {0};
            return static_cast<uint32_t>(calculator.calculate_serialized_size(reply, current_alignment) + 4);
        },
        ALIVE);

    if (change != nullptr)
    {
        CDRMessage_t msg(change->serializedPayload);

        bool valid = CDRMessage::addOctet(&msg, 0);
        change->serializedPayload.encapsulation = static_cast<uint16_t>(PL_DEFAULT_ENCAPSULATION);
        msg.msg_endian = DEFAULT_ENDIAN;
        valid &= CDRMessage::addOctet(&msg, PL_DEFAULT_ENCAPSULATION);
        valid &= CDRMessage::addUInt16(&msg, 0);

        change->serializedPayload.pos = msg.pos;
        change->serializedPayload.length = msg.length;

        SerializedPayload_t payload;
        payload.max_size = change->serializedPayload.max_size - 4;
        payload.data = change->serializedPayload.data + 4;
        if (valid && reply_type_.serialize(&reply, &payload, DataRepresentationId_t::XCDR2_DATA_REPRESENTATION))
        {
            change->serializedPayload.length += payload.length;
            change->serializedPayload.pos += payload.pos;
            payload.data = nullptr;
            return builtin_reply_writer_history_->add_change(change);
        }
    }
    builtin_request_writer_history_->remove_change(change);
    return false;
}

bool TypeLookupManager::receive_request(
        fastrtps::rtps::CacheChange_t& change,
        TypeLookup_Request& request) const
{
    CDRMessage_t msg(change.serializedPayload);
    msg.pos += 1;
    octet encapsulation = 0;
    CDRMessage::readOctet(&msg, &encapsulation);
    if (encapsulation == PL_CDR_BE)
    {
        msg.msg_endian = BIGEND;
    }
    else if (encapsulation == PL_CDR_LE)
    {
        msg.msg_endian = LITTLEEND;
    }
    else
    {
        return false;
    }
    change.serializedPayload.encapsulation = static_cast<uint16_t>(encapsulation);
    msg.pos += 2; // Skip encapsulation options.

    SerializedPayload_t payload;
    payload.max_size = change.serializedPayload.max_size - 4;
    payload.length = change.serializedPayload.length - 4;
    payload.data = change.serializedPayload.data + 4;
    bool result = request_type_.deserialize(&payload, &request);
    payload.data = nullptr;
    if (result && request.header().instanceName() != local_instance_name_)
    {
        // Ignore request
        result = false;
    }
    return result;
}

bool TypeLookupManager::receive_reply(
        fastrtps::rtps::CacheChange_t& change,
        TypeLookup_Reply& reply) const
{
    CDRMessage_t msg(change.serializedPayload);
    msg.pos += 1;
    octet encapsulation = 0;
    CDRMessage::readOctet(&msg, &encapsulation);
    if (encapsulation == PL_CDR_BE)
    {
        msg.msg_endian = BIGEND;
    }
    else if (encapsulation == PL_CDR_LE)
    {
        msg.msg_endian = LITTLEEND;
    }
    else
    {
        return false;
    }
    change.serializedPayload.encapsulation = static_cast<uint16_t>(encapsulation);
    msg.pos += 2;     // Skip encapsulation options.

    SerializedPayload_t payload;
    payload.max_size = change.serializedPayload.max_size - 4;
    payload.length = change.serializedPayload.length - 4;
    payload.data = change.serializedPayload.data + 4;
    bool result = reply_type_.deserialize(&payload, &reply);
    payload.data = nullptr;
    if (result &&
            guid_dds_2_rtps(reply.header().relatedRequestId().writer_guid()) != builtin_request_writer_->getGuid())
    {
        // Ignore reply
        result = false;
    }
    return result;
}

std::string TypeLookupManager::get_instance_name(
        const fastrtps::rtps::GuidPrefix_t guid) const
{
    std::stringstream ss;
    ss << guid;
    std::string str = ss.str();
    std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c)
            {
                return static_cast<unsigned char>(std::tolower(c));
            });
    str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
    return "dds.builtin.TOS." + str;
}

} // namespace builtin

} // namespace dds
} // namespace fastdds
} // namespace eprosima
