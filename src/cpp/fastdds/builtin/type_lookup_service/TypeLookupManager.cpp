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
#include <iterator>
#include <unordered_set>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/BuiltinEndpoints.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/participant/PDP.h>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/resources/TimedEvent.h>
#include <rtps/domain/RTPSDomainImpl.hpp>
#include <rtps/writer/StatefulWriter.hpp>

namespace eprosima {

using namespace fastdds::rtps;

namespace fastdds {
namespace dds {
namespace builtin {


inline SequenceNumber_t sequence_number_rtps_2_dds(
        const fastdds::rtps::SequenceNumber_t& seq_number)
{
    return *reinterpret_cast<const SequenceNumber_t*>(&seq_number);
}

inline GUID_t guid_rtps_2_dds(
        const fastdds::rtps::GUID_t& rtps_guid)
{
    return *reinterpret_cast<const GUID_t*>(&rtps_guid);
}

inline fastdds::rtps::GUID_t guid_dds_2_rtps(
        const GUID_t& guid)
{
    return *reinterpret_cast<const fastdds::rtps::GUID_t*>(&guid);
}

TypeLookupManager::TypeLookupManager()
{
}

TypeLookupManager::~TypeLookupManager()
{
    // Delete the event before anything else, as it sends requests and accesses the pending collections
    delete request_timeout_event_;

    // Stop the processing threads
    if (nullptr != request_listener_)
    {
        request_listener_->stop_request_processor_thread();
    }
    if (nullptr != reply_listener_)
    {
        reply_listener_->stop_reply_processor_thread();
    }

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
    delete reply_listener_;
    reply_listener_ = nullptr;
    delete request_listener_;
    request_listener_ = nullptr;

    delete builtin_request_writer_history_;
    delete builtin_reply_writer_history_;
    delete builtin_request_reader_history_;
    delete builtin_reply_reader_history_;

    delete temp_reader_proxy_data_;
    delete temp_writer_proxy_data_;

    for (auto& writer_entry : async_get_type_writer_callbacks_)
    {
        // Delete the proxies and remove the entry
        for (auto& proxy_callback_pair : writer_entry.second)
        {
            delete proxy_callback_pair.first;
        }
    }

    for (auto& reader_entry : async_get_type_reader_callbacks_)
    {
        // Delete the proxies and remove the entry
        for (auto& proxy_callback_pair : reader_entry.second)
        {
            delete proxy_callback_pair.first;
        }
    }
}

bool TypeLookupManager::init(
        fastdds::rtps::BuiltinProtocols* protocols)
{
    participant_ = protocols->mp_participantImpl;
    builtin_protocols_ = protocols;
    const auto& locators_allocations = participant_->get_const_attributes().allocation.locators;

    local_instance_name_ = get_instance_name(participant_->getGuid());

    temp_reader_proxy_data_ = new fastdds::rtps::ReaderProxyData(
        locators_allocations.max_unicast_locators,
        locators_allocations.max_multicast_locators);

    temp_writer_proxy_data_ = new fastdds::rtps::WriterProxyData(
        locators_allocations.max_unicast_locators,
        locators_allocations.max_multicast_locators);

    type_propagation_ = participant_->type_propagation();

    // Time a request is kept waiting for an answer before it is failed. Zero disables the check
    uint64_t timeout_ms {default_typelookup_request_timeout_ms};
    const std::string* timeout_property = fastdds::rtps::PropertyPolicyHelper::find_property(
        participant_->get_const_attributes().properties, "fastdds.type_lookup_request_timeout_ms");
    if (nullptr != timeout_property)
    {
        try
        {
            timeout_ms = std::stoull(*timeout_property);
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE,
                    "Invalid value for fastdds.type_lookup_request_timeout_ms: " << e.what());
        }
    }
    typelookup_request_timeout_ = std::chrono::milliseconds(timeout_ms);

    // Check if ReaderProxyData and WriterProxyData objects were created successfully
    if (temp_reader_proxy_data_ && temp_writer_proxy_data_)
    {
        if (!create_endpoints())
        {
            return false;
        }

        if (std::chrono::milliseconds::zero() < typelookup_request_timeout_)
        {
            // Requests are failed between one and two timeouts after being sent
            request_timeout_event_ = new fastdds::rtps::TimedEvent(participant_->getEventResource(),
                            [this]() -> bool
                            {
                                check_pending_requests_timeout();
                                return true;
                            },
                            static_cast<double>(typelookup_request_timeout_.count()));
            request_timeout_event_->restart_timer();
        }

        return true;
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
    uint32_t endp = pdata.m_available_builtin_endpoints;
    uint32_t auxendp = endp;

    std::lock_guard<std::mutex> data_guard(temp_data_lock_);

    temp_writer_proxy_data_->guid.guidPrefix = pdata.guid.guidPrefix;
    temp_writer_proxy_data_->persistence_guid.guidPrefix = pdata.guid.guidPrefix;
    temp_writer_proxy_data_->set_remote_locators(pdata.metatraffic_locators, network, true, pdata.is_from_this_host());
    temp_writer_proxy_data_->topic_kind = NO_KEY;
    temp_writer_proxy_data_->durability.kind = fastdds::dds::VOLATILE_DURABILITY_QOS;
    temp_writer_proxy_data_->reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;

    temp_reader_proxy_data_->clear();
    temp_reader_proxy_data_->expects_inline_qos = false;
    temp_reader_proxy_data_->guid.guidPrefix = pdata.guid.guidPrefix;
    temp_reader_proxy_data_->set_remote_locators(pdata.metatraffic_locators, network, true, pdata.is_from_this_host());
    temp_reader_proxy_data_->topic_kind = NO_KEY;
    temp_reader_proxy_data_->durability.kind = fastdds::dds::VOLATILE_DURABILITY_QOS;
    temp_reader_proxy_data_->reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;

    EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "for RTPSParticipant: " << pdata.guid);

    auxendp &= rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_WRITER;

    if (auxendp != 0 && builtin_request_reader_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Adding remote writer to the local Builtin Request Reader");
        temp_writer_proxy_data_->guid.entityId = fastdds::rtps::c_EntityId_TypeLookup_request_writer;
        temp_writer_proxy_data_->persistence_guid.entityId = fastdds::rtps::c_EntityId_TypeLookup_request_writer;
        builtin_request_reader_->matched_writer_add_edp(*temp_writer_proxy_data_);
    }

    auxendp = endp;
    auxendp &= rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_WRITER;

    if (auxendp != 0 && builtin_reply_reader_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Adding remote writer to the local Builtin Reply Reader");
        temp_writer_proxy_data_->guid.entityId = fastdds::rtps::c_EntityId_TypeLookup_reply_writer;
        temp_writer_proxy_data_->persistence_guid.entityId = fastdds::rtps::c_EntityId_TypeLookup_reply_writer;
        builtin_reply_reader_->matched_writer_add_edp(*temp_writer_proxy_data_);
    }

    auxendp = endp;
    auxendp &= rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_READER;

    if (auxendp != 0 && builtin_request_writer_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Adding remote reader to the local Builtin Request Writer");
        temp_reader_proxy_data_->guid.entityId = fastdds::rtps::c_EntityId_TypeLookup_request_reader;
        builtin_request_writer_->matched_reader_add_edp(*temp_reader_proxy_data_);
    }

    auxendp = endp;
    auxendp &= rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_READER;

    if (auxendp != 0 && builtin_reply_writer_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Adding remote reader to the local Builtin Reply Writer");
        temp_reader_proxy_data_->guid.entityId = fastdds::rtps::c_EntityId_TypeLookup_reply_reader;
        builtin_reply_writer_->matched_reader_add_edp(*temp_reader_proxy_data_);
    }

    return true;
}

void TypeLookupManager::remove_remote_endpoints(
        fastdds::rtps::ParticipantProxyData* pdata)
{
    fastdds::rtps::GUID_t tmp_guid;
    tmp_guid.guidPrefix = pdata->guid.guidPrefix;

    EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "for RTPSParticipant: " << pdata->guid);
    uint32_t endp = pdata->m_available_builtin_endpoints;
    uint32_t partdet = endp;
    uint32_t auxendp = endp;
    partdet &= rtps::DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR; //Habria que quitar esta linea que comprueba si tiene PDP.
    auxendp &= rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_WRITER;

    if ((auxendp != 0 || partdet != 0) && builtin_request_reader_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Removing remote writer from the local Builtin Request Reader");
        tmp_guid.entityId = fastdds::rtps::c_EntityId_TypeLookup_request_writer;
        builtin_request_reader_->matched_writer_remove(tmp_guid);
    }

    auxendp = endp;
    auxendp &= rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_WRITER;

    if ((auxendp != 0 || partdet != 0) && builtin_reply_reader_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Removing remote writer from the local Builtin Reply Reader");
        tmp_guid.entityId = fastdds::rtps::c_EntityId_TypeLookup_reply_writer;
        builtin_reply_reader_->matched_writer_remove(tmp_guid);
    }

    auxendp = endp;
    auxendp &= rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_READER;

    if ((auxendp != 0 || partdet != 0) && builtin_request_writer_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Removing remote reader from the local Builtin Request Writer");
        tmp_guid.entityId = fastdds::rtps::c_EntityId_TypeLookup_request_reader;
        builtin_request_writer_->matched_reader_remove(tmp_guid);
    }

    auxendp = endp;
    auxendp &= rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_READER;

    if ((auxendp != 0 || partdet != 0) && builtin_reply_writer_ != nullptr)
    {
        EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE, "Removing remote reader from the local Builtin Reply Writer");
        tmp_guid.entityId = fastdds::rtps::c_EntityId_TypeLookup_reply_reader;
        builtin_reply_writer_->matched_reader_remove(tmp_guid);
    }

    // The removed participant will not answer the requests sent to it, so they are failed here
    std::vector<SampleIdentity> failed_requests;
    {
        std::lock_guard<std::mutex> lock(async_get_types_mutex_);
        for (const auto& request : async_get_type_requests_)
        {
            if (request.second.type_server.guidPrefix == pdata->guid.guidPrefix)
            {
                failed_requests.push_back(request.first);
            }
        }
    }
    fail_pending_requests(failed_requests, false);
}

SampleIdentity TypeLookupManager::get_type_dependencies(
        const xtypes::TypeIdentifierSeq& id_seq,
        const fastdds::rtps::GUID_t& type_server,
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
    type.delete_data(request);
    return id;
}

SampleIdentity TypeLookupManager::get_types(
        const xtypes::TypeIdentifierSeq& id_seq,
        const fastdds::rtps::GUID_t& type_server) const
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
    type.delete_data(request);
    return id;
}

ReturnCode_t TypeLookupManager::async_get_type(
        eprosima::ProxyPool<eprosima::fastdds::rtps::WriterProxyData>::smart_ptr& temp_writer_data,
        const fastdds::rtps::GUID_t& type_server,
        const AsyncGetTypeWriterCallback& callback)
{
    return check_type_identifier_received<eprosima::fastdds::rtps::WriterProxyData>(
        temp_writer_data, type_server, callback, async_get_type_writer_callbacks_);
}

ReturnCode_t TypeLookupManager::async_get_type(
        eprosima::ProxyPool<eprosima::fastdds::rtps::ReaderProxyData>::smart_ptr&  temp_reader_data,
        const fastdds::rtps::GUID_t& type_server,
        const AsyncGetTypeReaderCallback& callback)
{
    return check_type_identifier_received<eprosima::fastdds::rtps::ReaderProxyData>(
        temp_reader_data, type_server, callback, async_get_type_reader_callbacks_);
}

TypeKind TypeLookupManager::get_type_kind_to_propagate() const
{
    switch (type_propagation_)
    {
        case utils::TypePropagation::TYPEPROPAGATION_DISABLED:
            return xtypes::TK_NONE;
        case utils::TypePropagation::TYPEPROPAGATION_ENABLED:
            return xtypes::EK_COMPLETE;
        case utils::TypePropagation::TYPEPROPAGATION_MINIMAL_BANDWIDTH:
            return xtypes::EK_MINIMAL;
        case utils::TypePropagation::TYPEPROPAGATION_REGISTRATION_ONLY:
            return xtypes::TK_NONE;
        default:
            return xtypes::EK_COMPLETE;
    }
}

template<typename ProxyType, typename AsyncCallback>
ReturnCode_t TypeLookupManager::check_type_identifier_received(
        typename eprosima::ProxyPool<ProxyType>::smart_ptr& temp_proxy_data,
        const fastdds::rtps::GUID_t& type_server,
        const AsyncCallback& callback,
        std::unordered_map<xtypes::TypeIdentfierWithSize,
        std::vector<std::pair<ProxyType*,
        AsyncCallback>>>& async_get_type_callbacks)
{
    xtypes::TypeIdentfierWithSize type_identifier_with_size =
            temp_proxy_data->type_information.type_information.complete().typeid_with_size().type_id()._d() !=
            TK_NONE ?
            temp_proxy_data->type_information.type_information.complete().typeid_with_size() :
            temp_proxy_data->type_information.type_information.minimal().typeid_with_size();

    // Check if the type is known
    if (fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                    is_type_identifier_known(type_identifier_with_size))
    {
        // The type is already known, invoke the callback
        callback(RETCODE_OK, temp_proxy_data.get());
        return RETCODE_OK;
    }

    // Hold the mutex across the lookup, the request and its bookkeeping, so the request cannot be duplicated and no
    // reply can be processed before both the callback and the request entries exist
    std::lock_guard<std::mutex> lock(async_get_types_mutex_);

    // Check if TypeIdentfierWithSize already exists in the map
    auto it = async_get_type_callbacks.find(type_identifier_with_size);
    if (it != async_get_type_callbacks.end())
    {
        // TypeIdentfierWithSize exists, add the callback
        // Make a copy of the proxy to free the EDP pool
        ProxyType* temp_proxy_data_copy(new ProxyType(*temp_proxy_data));
        it->second.push_back(std::make_pair(temp_proxy_data_copy, callback));
        // Return without sending new request
        return RETCODE_NO_DATA;
    }

    // TypeIdentfierWithSize doesn't exist, create a new entry
    SampleIdentity get_type_dependencies_request = get_type_dependencies(
        {type_identifier_with_size.type_id()}, type_server);
    if (INVALID_SAMPLE_IDENTITY == get_type_dependencies_request)
    {
        // Failed to send request, return error
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Failed to send get_type_dependencies request");
        return RETCODE_ERROR;
    }

    // Store the sent request and callback
    std::vector<std::pair<ProxyType*, AsyncCallback>> types;
    // Make a copy of the proxy to free the EDP pool
    ProxyType* temp_proxy_data_copy(new ProxyType(*temp_proxy_data));
    types.push_back(std::make_pair(temp_proxy_data_copy, callback));
    async_get_type_callbacks.emplace(type_identifier_with_size, std::move(types));
    add_async_get_type_request_nts(get_type_dependencies_request, type_identifier_with_size, type_server);

    return RETCODE_NO_DATA;
}

template<typename ProxyType, typename AsyncCallback>
void TypeLookupManager::take_pending_endpoints_nts(
        std::unordered_map<xtypes::TypeIdentfierWithSize,
        std::vector<std::pair<ProxyType*, AsyncCallback>>>& async_get_type_callbacks,
        const xtypes::TypeIdentfierWithSize& type_identifier_with_size,
        const fastdds::rtps::GuidPrefix_t& failed_server,
        bool notify,
        std::vector<std::pair<ProxyType*, AsyncCallback>>& failed,
        std::vector<std::pair<ProxyType*, AsyncCallback>>& pending)
{
    auto it = async_get_type_callbacks.find(type_identifier_with_size);
    if (it == async_get_type_callbacks.end())
    {
        return;
    }

    for (auto& proxy_callback_pair : it->second)
    {
        // The type server of an endpoint is the participant that announced it
        if (proxy_callback_pair.first->guid.guidPrefix != failed_server)
        {
            pending.push_back(std::move(proxy_callback_pair));
        }
        else if (notify)
        {
            failed.push_back(std::move(proxy_callback_pair));
        }
        else
        {
            // The type server is gone, so the endpoint cannot be paired any more
            delete proxy_callback_pair.first;
        }
    }
    async_get_type_callbacks.erase(it);
}

void TypeLookupManager::check_pending_requests_timeout()
{
    std::vector<SampleIdentity> timed_out_requests;
    {
        const auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(async_get_types_mutex_);
        for (const auto& request : async_get_type_requests_)
        {
            if (typelookup_request_timeout_ <= now - request.second.sent_time)
            {
                timed_out_requests.push_back(request.first);
            }
        }
    }

    if (!timed_out_requests.empty())
    {
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE,
                "Failing " << timed_out_requests.size() << " TypeLookup requests with no answer.");
        fail_pending_requests(timed_out_requests, true);
    }
}

void TypeLookupManager::fail_pending_requests(
        const std::vector<SampleIdentity>& failed_requests,
        bool notify)
{
    // Endpoints whose callbacks are notified of the failure, so they are paired by type name
    std::vector<std::pair<fastdds::rtps::WriterProxyData*, AsyncGetTypeWriterCallback>> failed_writers;
    std::vector<std::pair<fastdds::rtps::ReaderProxyData*, AsyncGetTypeReaderCallback>> failed_readers;
    std::vector<SampleIdentity> removed_requests;

    {
        // A type being resolved may have more than one request outstanding, as pagination sends a new one per
        // page, so its endpoints are only taken and requested again once
        std::unordered_set<xtypes::TypeIdentfierWithSize> handled_types;

        std::lock_guard<std::mutex> lock(async_get_types_mutex_);
        for (const SampleIdentity& failed_request : failed_requests)
        {
            auto request_it = async_get_type_requests_.find(failed_request);
            if (request_it == async_get_type_requests_.end())
            {
                continue;
            }
            const xtypes::TypeIdentfierWithSize type_identifier_with_size = request_it->second.type_id;
            const fastdds::rtps::GuidPrefix_t failed_server = request_it->second.type_server.guidPrefix;
            async_get_type_requests_.erase(request_it);
            removed_requests.push_back(failed_request);

            if (!handled_types.insert(type_identifier_with_size).second)
            {
                continue;
            }

            std::vector<std::pair<fastdds::rtps::WriterProxyData*, AsyncGetTypeWriterCallback>> pending_writers;
            std::vector<std::pair<fastdds::rtps::ReaderProxyData*, AsyncGetTypeReaderCallback>> pending_readers;
            take_pending_endpoints_nts(async_get_type_writer_callbacks_, type_identifier_with_size, failed_server,
                    notify, failed_writers, pending_writers);
            take_pending_endpoints_nts(async_get_type_reader_callbacks_, type_identifier_with_size, failed_server,
                    notify, failed_readers, pending_readers);

            if (pending_writers.empty() && pending_readers.empty())
            {
                continue;
            }

            // Any of the participants that announced the type can serve it, request it again to one of them
            fastdds::rtps::GUID_t type_server(pending_writers.empty() ?
                    pending_readers.front().first->guid.guidPrefix :
                    pending_writers.front().first->guid.guidPrefix,
                    fastdds::rtps::c_EntityId_Unknown);
            SampleIdentity new_request = get_type_dependencies(
                {type_identifier_with_size.type_id()}, type_server);
            if (INVALID_SAMPLE_IDENTITY != new_request)
            {
                if (!pending_writers.empty())
                {
                    async_get_type_writer_callbacks_.emplace(type_identifier_with_size, std::move(pending_writers));
                }
                if (!pending_readers.empty())
                {
                    async_get_type_reader_callbacks_.emplace(type_identifier_with_size, std::move(pending_readers));
                }
                add_async_get_type_request_nts(new_request, type_identifier_with_size, type_server);
            }
            else
            {
                // The request could not be sent, so these endpoints cannot be resolved either. Their own type
                // server is still there, so they are failed instead of dropped
                EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Failed to resend get_type_dependencies request");
                failed_writers.insert(failed_writers.end(),
                        std::make_move_iterator(pending_writers.begin()),
                        std::make_move_iterator(pending_writers.end()));
                failed_readers.insert(failed_readers.end(),
                        std::make_move_iterator(pending_readers.begin()),
                        std::make_move_iterator(pending_readers.end()));
            }
        }
    }

    if (nullptr != reply_listener_)
    {
        // A request that is not answered would keep its entry in the reply listener forever
        reply_listener_->remove_replies_with_continuation(removed_requests);
    }

    // Notify outside the mutex, as the callbacks re-enter discovery. The type could not be resolved, so they are
    // failed exactly as any other failed resolution, and the endpoints end up paired by type name
    for (auto& proxy_callback_pair : failed_writers)
    {
        proxy_callback_pair.second(RETCODE_NO_DATA, proxy_callback_pair.first);
        delete proxy_callback_pair.first;
    }
    for (auto& proxy_callback_pair : failed_readers)
    {
        proxy_callback_pair.second(RETCODE_NO_DATA, proxy_callback_pair.first);
        delete proxy_callback_pair.first;
    }
}

void TypeLookupManager::notify_callbacks(
        ReturnCode_t request_ret_status,
        const xtypes::TypeIdentfierWithSize& type_identifier_with_size)
{
    std::vector<std::pair<fastdds::rtps::WriterProxyData*, AsyncGetTypeWriterCallback>> writer_callbacks;
    std::vector<std::pair<fastdds::rtps::ReaderProxyData*, AsyncGetTypeReaderCallback>> reader_callbacks;

    {
        // Detach the callbacks under the mutex, so they are not invoked while another thread appends to the vectors
        std::lock_guard<std::mutex> lock(async_get_types_mutex_);
        auto writer_it = async_get_type_writer_callbacks_.find(type_identifier_with_size);
        if (writer_it != async_get_type_writer_callbacks_.end())
        {
            writer_callbacks = std::move(writer_it->second);
            async_get_type_writer_callbacks_.erase(writer_it);
        }
        auto reader_it = async_get_type_reader_callbacks_.find(type_identifier_with_size);
        if (reader_it != async_get_type_reader_callbacks_.end())
        {
            reader_callbacks = std::move(reader_it->second);
            async_get_type_reader_callbacks_.erase(reader_it);
        }
    }

    // Invoke outside the mutex, so a callback that re-enters the TypeLookup service does not deadlock
    for (auto& proxy_callback_pair : writer_callbacks)
    {
        proxy_callback_pair.second(request_ret_status, proxy_callback_pair.first);
        delete proxy_callback_pair.first;
    }
    for (auto& proxy_callback_pair : reader_callbacks)
    {
        proxy_callback_pair.second(request_ret_status, proxy_callback_pair.first);
        delete proxy_callback_pair.first;
    }
}

bool TypeLookupManager::add_async_get_type_request(
        const SampleIdentity& request,
        const xtypes::TypeIdentfierWithSize& type_identifier_with_size,
        const fastdds::rtps::GUID_t& type_server)
{
    std::lock_guard<std::mutex> lock(async_get_types_mutex_);
    return add_async_get_type_request_nts(request, type_identifier_with_size, type_server);
}

bool TypeLookupManager::add_async_get_type_request_nts(
        const SampleIdentity& request,
        const xtypes::TypeIdentfierWithSize& type_identifier_with_size,
        const fastdds::rtps::GUID_t& type_server)
{
    try
    {
        async_get_type_requests_.emplace(request,
                AsyncGetTypeRequest{type_identifier_with_size, type_server, std::chrono::steady_clock::now()});
        return true;
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE,
                "Error in TypeLookupManager::add_async_get_type_request: " << e.what());
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

    // Built-in history attributes.
    HistoryAttributes hatt;
    hatt.initialReservedCaches = 20;
    hatt.maximumReservedCaches = 1000;
    hatt.payloadMaxSize = TypeLookupManager::typelookup_data_max_size;

    WriterAttributes watt = participant_->pdp()->create_builtin_writer_attributes();
    watt.endpoint.remoteLocatorList = builtin_protocols_->m_initialPeersList;
    watt.endpoint.topicKind = fastdds::rtps::NO_KEY;
    watt.endpoint.durabilityKind = fastdds::rtps::VOLATILE;
    watt.mode = fastdds::rtps::ASYNCHRONOUS_WRITER;

    ReaderAttributes ratt = participant_->pdp()->create_builtin_reader_attributes();
    ratt.endpoint.remoteLocatorList = builtin_protocols_->m_initialPeersList;
    ratt.expects_inline_qos = true;
    ratt.endpoint.topicKind = fastdds::rtps::NO_KEY;
    ratt.endpoint.durabilityKind = fastdds::rtps::VOLATILE;

    // Built-in request writer
    request_listener_ = new TypeLookupRequestListener(this);
    builtin_request_writer_history_ = new WriterHistory(hatt);

    RTPSWriter* req_writer;
    if (participant_->createWriter(
                &req_writer,
                watt,
                builtin_request_writer_history_,
                request_listener_,
                fastdds::rtps::c_EntityId_TypeLookup_request_writer,
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
                fastdds::rtps::c_EntityId_TypeLookup_request_reader,
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
                fastdds::rtps::c_EntityId_TypeLookup_reply_writer,
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
                fastdds::rtps::c_EntityId_TypeLookup_reply_reader,
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
        const fastdds::rtps::GUID_t& type_server,
        TypeLookup_RequestPubSubType& pupsubtype) const
{
    TypeLookup_Request* request = static_cast<TypeLookup_Request*>(pupsubtype.create_data());
    request->header().instanceName() = get_instance_name(type_server);
    request->header().requestId().writer_guid(guid_rtps_2_dds(builtin_request_writer_->getGuid()));
    request->header().requestId().sequence_number(
        sequence_number_rtps_2_dds(fastdds::rtps::SequenceNumber_t(request_seq_number_++)));
    return request;
}

bool TypeLookupManager::send(
        TypeLookup_Request& request) const
{
    if (!send_impl(request, &request_type_, builtin_request_writer_history_))
    {
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE, "Error sending request.");
        return false;
    }
    return true;
}

bool TypeLookupManager::send(
        TypeLookup_Reply& reply) const
{
    if (!send_impl(reply, &reply_type_, builtin_reply_writer_history_))
    {
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE, "Error sending reply.");
        return false;
    }
    return true;
}

template<typename Type, typename PubSubType>
bool TypeLookupManager::send_impl(
        Type& msg,
        PubSubType* pubsubtype,
        fastdds::rtps::WriterHistory* writer_history) const
{
    // Calculate the serialized size of the message using a CdrSizeCalculator
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment {0};
    uint32_t payload_size = static_cast<uint32_t>(calculator.calculate_serialized_size(msg, current_alignment) + 4);

    // Create a new CacheChange_t using the provided StatefulWriter
    CacheChange_t* change = writer_history->create_change(payload_size, ALIVE);

    // Check if the creation of CacheChange_t was successful
    if (!change)
    {
        return false;
    }

    // Serialize the message using the provided PubSubType
    bool result = pubsubtype->serialize(&msg, change->serializedPayload,
                    DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);
    // If serialization was successful, update the change and add it to the WriterHistory
    if (result)
    {
        result = writer_history->add_change(change);
    }
    // If adding the change to WriterHistory failed, remove the change
    if (!result)
    {
        writer_history->remove_change(change);
    }

    return result;
}

bool TypeLookupManager::prepare_receive_payload(
        fastdds::rtps::CacheChange_t& change,
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
        fastdds::rtps::CacheChange_t& change,
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
        fastdds::rtps::CacheChange_t& change,
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

template<typename Type, typename PubSubType>
bool TypeLookupManager::receive_impl(
        fastdds::rtps::CacheChange_t& change,
        Type& msg,
        PubSubType* pubsubtype) const
{
    SerializedPayload_t payload;
    if (!prepare_receive_payload(change, payload))
    {
        return false;
    }

    bool result = pubsubtype->deserialize(payload, &msg);
    payload.data = nullptr;

    return result;
}

std::string TypeLookupManager::get_instance_name(
        const fastdds::rtps::GUID_t guid) const
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
        fastdds::rtps::CacheChange_t* change)
{
    builtin_request_writer_history_->remove_change(change);
}

void TypeLookupManager::remove_builtin_reply_writer_history_change(
        fastdds::rtps::CacheChange_t* change)
{
    builtin_reply_writer_history_->remove_change(change);
}

} // namespace builtin

} // namespace dds
} // namespace fastdds
} // namespace eprosima
