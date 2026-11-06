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
 * @file TypeLookupRequestListener.cpp
 *
 */

#include <fastdds/builtin/type_lookup_service/TypeLookupRequestListener.hpp>

#include <algorithm>
#include <limits>
#include <vector>

#include <fastcdr/exceptions/Exception.h>

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/domain/RTPSDomainImpl.hpp>
#include <rtps/reader/StatefulReader.hpp>

using eprosima::fastdds::rtps::RTPSReader;
using eprosima::fastdds::rtps::CacheChange_t;
using eprosima::fastdds::dds::Log;

using eprosima::fastdds::rtps::c_EntityId_TypeLookup_request_writer;

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

//! Constant that specifies the maximum number of dependent types to be included per reply.
//! This number is calculated considering the MTU.
const size_t MAX_DEPENDENCIES_PER_REPLY = 75;

/**
 * @brief Calculates the opaque value of continuation point, which is the index of the requested page.
 * @param continuation_point[in] The continuation point.
 * @param value[out] The value of the continuation_point.
 * @return false if the value is not representable in size_t, true otherwise.
 */
inline bool calculate_continuation_point(
        const std::vector<uint8_t>& continuation_point,
        size_t& value)
{
    value = 0;
    for (size_t i = 0; i < continuation_point.size(); ++i)
    {
        // Shifting would discard the most significant bytes
        if (value > (std::numeric_limits<size_t>::max() >> 8))
        {
            return false;
        }
        value = (value << 8) | continuation_point[i];
    }
    return true;
}

/**
 * @brief Creates a continuation point with the given value.
 * @param value[in] The desired value.
 * @return The continuation_point.
 */
inline std::vector<uint8_t> create_continuation_point(
        size_t value)
{
    std::vector<uint8_t> continuation_point(32, 0);

    // Big endian encoding of the value in the last bytes
    for (size_t i = 0; i < sizeof(size_t); ++i)
    {
        continuation_point[continuation_point.size() - 1 - i] = static_cast<uint8_t>(value >> (8 * i));
    }
    return continuation_point;
}

/**
 * @brief Strict weak ordering of TypeIdentfierWithSize, so dependencies are paginated in a deterministic order.
 * @param lhs[in] Left hand side TypeIdentfierWithSize.
 * @param rhs[in] Right hand side TypeIdentfierWithSize.
 * @return true if lhs goes before rhs.
 */
inline bool type_identifier_with_size_less(
        const xtypes::TypeIdentfierWithSize& lhs,
        const xtypes::TypeIdentfierWithSize& rhs)
{
    const xtypes::TypeIdentifier& lhs_id {lhs.type_id()};
    const xtypes::TypeIdentifier& rhs_id {rhs.type_id()};
    if (lhs_id._d() != rhs_id._d())
    {
        return lhs_id._d() < rhs_id._d();
    }
    // Dependencies are direct hash TypeIdentifiers, so the EquivalenceHash makes the order total
    if ((xtypes::EK_COMPLETE == lhs_id._d() || xtypes::EK_MINIMAL == lhs_id._d()) &&
            lhs_id.equivalence_hash() != rhs_id.equivalence_hash())
    {
        return lhs_id.equivalence_hash() < rhs_id.equivalence_hash();
    }
    return lhs.typeobject_serialized_size() < rhs.typeobject_serialized_size();
}

TypeLookupRequestListener::TypeLookupRequestListener(
        TypeLookupManager* manager)
    : typelookup_manager_(manager)
{
    start_request_processor_thread();
}

TypeLookupRequestListener::~TypeLookupRequestListener()
{
    stop_request_processor_thread();
}

void TypeLookupRequestListener::start_request_processor_thread()
{
    std::unique_lock<std::mutex> guard(request_processor_cv_mutex_);
    // Check if is not already in progress and the thread is not joinable
    if (!processing_ && !request_processor_thread.joinable())
    {
        processing_ = true;
        // Lambda function to be executed by the thread
        auto thread_func = [this]()
                {
                    process_requests();
                };
        // Create and start the processing thread
        request_processor_thread = eprosima::create_thread(thread_func,
                        typelookup_manager_->participant_->get_const_attributes().typelookup_service_thread,
                        "dds.tls.requests.%u");
    }
}

void TypeLookupRequestListener::stop_request_processor_thread()
{
    {
        // Set processing_ to false to signal the processing thread to stop
        std::unique_lock<std::mutex> guard(request_processor_cv_mutex_);
        processing_ = false;
    }

    if (request_processor_thread.joinable())
    {
        // Notify the processing thread to wake up and check the exit condition
        request_processor_cv_.notify_all();
        // Check if the calling thread is not the processing thread and join it
        if (!request_processor_thread.is_calling_thread())
        {
            request_processor_thread.join();
        }
    }
}

void TypeLookupRequestListener::process_requests()
{
    while (true)
    {
        std::pair<TypeLookup_Request, rtps::VendorId_t> request;
        {
            std::unique_lock<std::mutex> guard(request_processor_cv_mutex_);

            // Wait until either processing is done or there are requests in the queue
            request_processor_cv_.wait(guard,
                    [&]()
                    {
                        return !processing_ || !requests_queue_.empty();
                    });

            if (!processing_)
            {
                return;
            }

            // Take the request out of the queue, as it is processed with the mutex released
            request = std::move(requests_queue_.front());
            requests_queue_.pop();
        }

        // Answering takes the writer mutex, while a request is delivered holding that same mutex and taking
        // this one. Holding it here would invert that order and deadlock both threads
        try
        {
            // Process the TypeLookup_Request based on its type
            switch (request.first.data()._d())
            {
                case TypeLookup_getTypes_HashId:
                {
                    check_get_types_request(request.first.header().requestId(),
                            request.first.data().getTypes(), request.second);
                    break;
                }
                case TypeLookup_getDependencies_HashId:
                {
                    check_get_type_dependencies_request(request.first.header().requestId(),
                            request.first.data().getTypeDependencies());
                    break;
                }
                default:
                    // If the type of request is not known, log an error and answer with an exception
                    EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                            "Received unknown request in type lookup service.");
                    answer_request(request.first.header().requestId(),
                            rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_OPERATION);
                    break;
            }
        }
        catch (const std::exception& exception)
        {
            EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                    "Error processing request in type lookup service: " << exception.what());
        }
        catch (const eprosima::fastcdr::exception::Exception& exception)
        {
            EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                    "Error processing request in type lookup service: " << exception.what());
        }
        catch (...)
        {
            EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                    "Unknown error processing request in type lookup service.");
        }
    }
}

void TypeLookupRequestListener::check_get_types_request(
        SampleIdentity request_id,
        const TypeLookup_getTypes_In& request,
        const rtps::VendorId_t& vendor_id)
{
    xtypes::TypeKind type_to_propagate = typelookup_manager_->get_type_kind_to_propagate();

    // Early return in case type propagation is disabled
    if (xtypes::TK_NONE == type_to_propagate)
    {
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                "getTypes request received for a participant with type propagation disabled.");
        answer_request(request_id, rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_EXCEPTION);
        return;
    }

    TypeLookup_getTypes_Out out;
    ReturnCode_t type_result = RETCODE_ERROR;
    xtypes::TypeObject obj;
    xtypes::TypeIdentifier reply_typeid;
    xtypes::TypeIdentifier complete_id;
    xtypes::TypeIdentifier minimal_id;

    if (0 == request.type_ids().size())
    {
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                "Received request with no type identifiers.");
    }

    // Iterate through requested type_ids
    for (const xtypes::TypeIdentifier& type_id : request.type_ids())
    {
        // If TypeIdentifier is EK_MINIMAL add complete_to_minimal to answer
        if (type_id._d() == xtypes::EK_MINIMAL && rtps::c_VendorId_opendds != vendor_id)
        {
            minimal_id = type_id;
            reply_typeid = minimal_id;

            // Get complete TypeIdentifier from registry
            complete_id = fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                            get_complementary_type_identifier(minimal_id);

            if (xtypes::EK_COMPLETE == type_to_propagate)
            {
                reply_typeid = complete_id;

                xtypes::TypeIdentifierPair id_pair;
                id_pair.type_identifier1(complete_id);
                id_pair.type_identifier2(minimal_id);

                // Add the id pair to the result
                out.complete_to_minimal().push_back(std::move(id_pair));
            }

        }
        else
        {
            reply_typeid = type_id;
        }

        type_result = fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                        get_type_object(reply_typeid, obj);

        if (RETCODE_OK != type_result)
        {
            // If any object is unknown, abort and answer with exception
            break;
        }

        xtypes::TypeIdentifierTypeObjectPair id_obj_pair;
        id_obj_pair.type_identifier(reply_typeid);
        id_obj_pair.type_object(obj);
        // Add the id/obj pair to the result
        out.types().push_back(std::move(id_obj_pair));
    }

    // Handle the result based on the type_result
    if (RETCODE_OK == type_result)
    {
        // Prepare and send the reply for successful operation
        answer_request(request_id, rpc::RemoteExceptionCode_t::REMOTE_EX_OK, out);
    }
    else if (RETCODE_NO_DATA == type_result)
    {
        // Log error for type not found and reply with appropriate exception
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                "Requested TypeIdentifier is not found in the registry.");
        answer_request(request_id, rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_EXCEPTION);
    }
    else if (RETCODE_PRECONDITION_NOT_MET == type_result)
    {
        // Log error for invalid argument and reply with appropriate exception
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                "Requested TypeIdentifier is not a direct hash.");
        answer_request(request_id, rpc::RemoteExceptionCode_t::REMOTE_EX_INVALID_ARGUMENT);
    }
}

void TypeLookupRequestListener::check_get_type_dependencies_request(
        SampleIdentity request_id,
        const TypeLookup_getTypeDependencies_In& request)
{
    std::unordered_set<xtypes::TypeIdentfierWithSize> type_dependencies;
    ReturnCode_t type_dependencies_result = RETCODE_ERROR;
    if (!request.type_ids().empty())
    {
        // Dependencies are computed on each request, so no state is kept between paginated requests
        type_dependencies_result =
                fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                        get_type_dependencies(request.type_ids(), type_dependencies);
    }
    else
    {
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER, "Type dependencies request is empty.");
    }

    // Handle the result based on the type_dependencies_result
    if (RETCODE_OK == type_dependencies_result)
    {
        TypeLookup_getTypeDependencies_Out out;
        if (prepare_get_type_dependencies_response(type_dependencies, request.continuation_point(), out))
        {
            // Prepare and send the reply for successful operation
            answer_request(request_id, rpc::RemoteExceptionCode_t::REMOTE_EX_OK, out);
        }
        else
        {
            // Log error for invalid continuation point and reply with appropriate exception
            EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                    "Invalid continuation point in type dependencies request.");
            answer_request(request_id, rpc::RemoteExceptionCode_t::REMOTE_EX_INVALID_ARGUMENT);
        }
    }
    else if (RETCODE_NO_DATA == type_dependencies_result)
    {
        // Log error for type not found and reply with appropriate exception
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                "Requested TypeIdentifier is not found in the registry.");
        answer_request(request_id, rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_EXCEPTION);
    }
    else if (RETCODE_BAD_PARAMETER == type_dependencies_result)
    {
        // Log error for invalid argument and reply with appropriate exception
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                "Requested TypeIdentifier is not a direct hash.");
        answer_request(request_id, rpc::RemoteExceptionCode_t::REMOTE_EX_INVALID_ARGUMENT);
    }
}

bool TypeLookupRequestListener::prepare_get_type_dependencies_response(
        const std::unordered_set<xtypes::TypeIdentfierWithSize>& type_dependencies,
        const std::vector<uint8_t>& continuation_point,
        TypeLookup_getTypeDependencies_Out& out) const
{
    // The continuation point is the index of the requested page
    size_t page {0};
    if (!calculate_continuation_point(continuation_point, page))
    {
        return false;
    }

    // Checked against the number of pages, so the page offset cannot overflow
    const size_t pages {(type_dependencies.size() + MAX_DEPENDENCIES_PER_REPLY - 1) / MAX_DEPENDENCIES_PER_REPLY};
    if (page >= std::max<size_t>(pages, 1))
    {
        return false;
    }

    // Deterministically sorted, so the dependencies of each page are the same when recomputed on the following requests
    std::vector<xtypes::TypeIdentfierWithSize> sorted_dependencies(type_dependencies.begin(), type_dependencies.end());
    std::sort(sorted_dependencies.begin(), sorted_dependencies.end(), type_identifier_with_size_less);

    const size_t start_index {page * MAX_DEPENDENCIES_PER_REPLY};
    const size_t end_index {std::min(start_index + MAX_DEPENDENCIES_PER_REPLY, sorted_dependencies.size())};
    out.dependent_typeids().assign(sorted_dependencies.begin() + start_index, sorted_dependencies.begin() + end_index);

    if (end_index < sorted_dependencies.size())
    {
        // Set the continuation point for the next request
        out.continuation_point(create_continuation_point(page + 1));
    }

    return true;
}

void TypeLookupRequestListener::answer_request(
        SampleIdentity request_id,
        rpc::RemoteExceptionCode_t exception_code,
        TypeLookup_getTypeDependencies_Out& out)
{
    TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(typelookup_manager_->reply_type_.create_data());
    TypeLookup_getTypeDependencies_Result result;
    result.result(out);
    reply->return_value().getTypeDependencies(result);
    reply->header().relatedRequestId(request_id);
    reply->header().remoteEx(exception_code);

    typelookup_manager_->send(*reply);
    typelookup_manager_->reply_type_.delete_data(reply);
}

void TypeLookupRequestListener::answer_request(
        SampleIdentity request_id,
        rpc::RemoteExceptionCode_t exception_code,
        TypeLookup_getTypes_Out& out)
{
    TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(typelookup_manager_->reply_type_.create_data());
    TypeLookup_getTypes_Result result;
    result.result(out);
    reply->return_value().getType(result);
    reply->header().relatedRequestId(request_id);
    reply->header().remoteEx(exception_code);

    typelookup_manager_->send(*reply);
    typelookup_manager_->reply_type_.delete_data(reply);
}

void TypeLookupRequestListener::answer_request(
        SampleIdentity request_id,
        rpc::RemoteExceptionCode_t exception_code)
{
    TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(typelookup_manager_->reply_type_.create_data());
    reply->header().relatedRequestId(request_id);
    reply->header().remoteEx(exception_code);

    typelookup_manager_->send(*reply);
    typelookup_manager_->reply_type_.delete_data(reply);
}

void TypeLookupRequestListener::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const changeIN)
{
    CacheChange_t* change = const_cast<CacheChange_t*>(changeIN);

    // Check if the data is received from the expected TypeLookup Request writer
    if (change->writerGUID.entityId != c_EntityId_TypeLookup_request_writer)
    {
        // Log a warning and remove the change from the history
        EPROSIMA_LOG_WARNING(TL_REQUEST_READER, "Received data from a bad endpoint.");
        reader->get_history()->remove_change(change);
        return;
    }

    // Process the received TypeLookup Request and handle different types of requests
    TypeLookup_Request request;
    if (typelookup_manager_->receive(*change, request))
    {
        {
            std::unique_lock<std::mutex> guard(request_processor_cv_mutex_);
            // Add request to the processing queue
            requests_queue_.push({request, change->vendor_id});
            // Notify processor
            request_processor_cv_.notify_all();
        }
    }

    // Remove the processed cache change from the history
    reader->get_history()->remove_change(change);
}

void TypeLookupRequestListener::on_writer_change_received_by_all(
        fastdds::rtps::RTPSWriter*,
        fastdds::rtps::CacheChange_t* change)
{
    typelookup_manager_->remove_builtin_request_writer_history_change(change);
}

} // namespace builtin

} // namespace dds
} // namespace fastdds
} // namespace eprosima
