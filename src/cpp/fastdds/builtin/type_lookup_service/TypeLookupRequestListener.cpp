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

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/RTPSDomainImpl.hpp>

using eprosima::fastrtps::rtps::RTPSReader;
using eprosima::fastrtps::rtps::CacheChange_t;
using eprosima::fastdds::dds::Log;

using eprosima::fastrtps::rtps::c_EntityId_TypeLookup_request_writer;

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

//! Constant that specifies the maximum number of dependent types to be included per reply.
//! This number is calculated considering the MTU.
const int32_t MAX_DEPENDENCIES_PER_REPLY = 75;

/**
 * @brief Calculates the opaque value of continuation point.
 * @param continuation_point[in] The continuation point.
 * @return The value of the continuation_point.
 */
inline size_t calculate_continuation_point(
        const std::vector<uint8_t>& continuation_point)
{
    size_t result = 0;
    for (size_t i = 0; i < continuation_point.size(); ++i)
    {
        result = (result << 8) | continuation_point[i];
    }
    return result;
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

    for (size_t value_i = 0; value_i < value; value_i++)
    {
        for (size_t i = continuation_point.size() - 1; i != SIZE_MAX; --i)
        {
            if (continuation_point[i] < 255)
            {
                ++continuation_point[i];
                // Break after successful increment
                break;
            }
            else
            {
                continuation_point[i] = 0;
            }
        }
    }
    return continuation_point;
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
                        typelookup_manager_->participant_->getAttributes().typelookup_service_thread,
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
    std::unique_lock<std::mutex> guard(request_processor_cv_mutex_);

    while (processing_)
    {
        // Wait until either processing is done or there are requests in the queue
        request_processor_cv_.wait(guard,
                [&]()
                {
                    return !processing_ || !requests_queue_.empty();
                });

        if (!requests_queue_.empty())
        {
            TypeLookup_Request& request = requests_queue_.front();
            {
                // Process the TypeLookup_Request based on its type
                switch (request.data()._d())
                {
                    case TypeLookup_getTypes_HashId:
                    {
                        check_get_types_request(request.header().requestId(), request.data().getTypes());
                        break;
                    }
                    case TypeLookup_getDependencies_HashId:
                    {
                        check_get_type_dependencies_request(request.header().requestId(),
                                request.data().getTypeDependencies());
                        break;
                    }
                    default:
                        // If the type of request is not known, log an error and answer with an exception
                        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                                "Received unknown request in type lookup service.");
                        answer_request(request.header().requestId(),
                                rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_OPERATION);
                        break;
                }
            }
            // Remove the requests from the queue
            requests_queue_.pop();
        }
    }
}

void TypeLookupRequestListener::check_get_types_request(
        SampleIdentity request_id,
        const TypeLookup_getTypes_In& request)
{
    // Always Sends EK_COMPLETE
    // TODO: Add a property to the participant to configure this behavior. Allowing it to respond with EK_MINIMAL when possible.
    TypeLookup_getTypes_Out out;
    ReturnCode_t type_result = RETCODE_ERROR;
    xtypes::TypeObject obj;
    xtypes::TypeIdentifier complete_id;
    xtypes::TypeIdentifier minimal_id;
    // Iterate through requested type_ids
    for (const xtypes::TypeIdentifier& type_id : request.type_ids())
    {
        // If TypeIdentifier is EK_MINIMAL add complete_to_minimal to answer
        if (type_id._d() == xtypes::EK_MINIMAL)
        {
            minimal_id = type_id;
            // Get complete TypeIdentifier from registry
            complete_id = fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                            get_complementary_type_identifier(minimal_id);

            xtypes::TypeIdentifierPair id_pair;
            id_pair.type_identifier1(complete_id);
            id_pair.type_identifier2(minimal_id);
            // Add the id pair to the result
            out.complete_to_minimal().push_back(std::move(id_pair));
        }
        else
        {
            complete_id = type_id;
        }

        type_result = fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                        get_type_object(complete_id, obj);
        if (RETCODE_OK != type_result)
        {
            // If any object is unknown, abort and answer with exception
            break;
        }

        xtypes::TypeIdentifierTypeObjectPair id_obj_pair;
        id_obj_pair.type_identifier(complete_id);
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
        // Check if the received request has been done before and needed a continuation point
        std::lock_guard<std::mutex> lock(requests_with_continuation_mutex_);
        if (!request.continuation_point().empty())
        {
            auto requests_it = requests_with_continuation_.find(request.type_ids());
            if (requests_it != requests_with_continuation_.end())
            {
                // Get the dependencies without checking the registry
                type_dependencies = requests_it->second;
                type_dependencies_result = RETCODE_OK;
            }
            else
            {
                // If the the received request is not found, log error and answer with exception
                EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REQUEST_LISTENER,
                        "Error processing ongoing type dependencies request.");
                answer_request(request_id, rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_EXCEPTION);
            }
        }
        else
        {
            // Get the dependencies from the registry
            type_dependencies_result =
                    fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                            get_type_dependencies(request.type_ids(), type_dependencies);

            // If there are too many dependent types, store the type dependencies for future requests
            if (type_dependencies_result == RETCODE_OK && type_dependencies.size() > MAX_DEPENDENCIES_PER_REPLY)
            {
                requests_with_continuation_.emplace(request.type_ids(), type_dependencies);
            }
        }
    }

    // Handle the result based on the type_dependencies_result
    if (RETCODE_OK == type_dependencies_result)
    {
        // Prepare and send the reply for successful operation
        TypeLookup_getTypeDependencies_Out out = prepare_get_type_dependencies_response(
            request.type_ids(), type_dependencies, request.continuation_point());
        answer_request(request_id, rpc::RemoteExceptionCode_t::REMOTE_EX_OK, out);
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

TypeLookup_getTypeDependencies_Out TypeLookupRequestListener::prepare_get_type_dependencies_response(
        const xtypes::TypeIdentifierSeq& id_seq,
        const std::unordered_set<xtypes::TypeIdentfierWithSize>& type_dependencies,
        const std::vector<uint8_t>& continuation_point)
{
    TypeLookup_getTypeDependencies_Out out;

    // Check if all dependencies can be sent in a single response
    if (type_dependencies.size() < MAX_DEPENDENCIES_PER_REPLY)
    {
        for (const auto& type_identifier : type_dependencies)
        {
            out.dependent_typeids().emplace_back(type_identifier);
        }
    }
    else
    {
        size_t start_index = 0;
        // Check if a continuation point is provided, and calculate starting point if there is
        if (!continuation_point.empty())
        {
            start_index = calculate_continuation_point(continuation_point) * MAX_DEPENDENCIES_PER_REPLY;
        }

        // Copy the dependencies within the specified range directly to out
        auto start_it = std::next(type_dependencies.begin(), start_index);
        auto end_it = std::next(start_it, std::min<size_t>(MAX_DEPENDENCIES_PER_REPLY,
                        type_dependencies.size() - start_index));
        for (auto it = start_it; it != end_it; ++it)
        {
            out.dependent_typeids().emplace_back(*it);
        }

        if ((start_index + MAX_DEPENDENCIES_PER_REPLY) > type_dependencies.size())
        {
            // If all dependent types have been sent, remove from map
            std::lock_guard<std::mutex> lock(requests_with_continuation_mutex_);
            auto requests_it = requests_with_continuation_.find(id_seq);
            if (requests_it != requests_with_continuation_.end())
            {
                requests_with_continuation_.erase(requests_it);
            }
        }
        else
        {
            // Set the continuation point for the next request
            out.continuation_point(create_continuation_point(calculate_continuation_point(continuation_point) + 1));
        }
    }

    return out;
}

void TypeLookupRequestListener::answer_request(
        SampleIdentity request_id,
        rpc::RemoteExceptionCode_t exception_code,
        TypeLookup_getTypeDependencies_Out& out)
{
    TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(typelookup_manager_->reply_type_.createData());
    TypeLookup_getTypeDependencies_Result result;
    result.result(out);
    reply->return_value().getTypeDependencies(result);
    reply->header().relatedRequestId(request_id);
    reply->header().remoteEx(exception_code);

    typelookup_manager_->send(*reply);
    typelookup_manager_->reply_type_.deleteData(reply);
}

void TypeLookupRequestListener::answer_request(
        SampleIdentity request_id,
        rpc::RemoteExceptionCode_t exception_code,
        TypeLookup_getTypes_Out& out)
{
    TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(typelookup_manager_->reply_type_.createData());
    TypeLookup_getTypes_Result result;
    result.result(out);
    reply->return_value().getType(result);
    reply->header().relatedRequestId(request_id);
    reply->header().remoteEx(exception_code);

    typelookup_manager_->send(*reply);
    typelookup_manager_->reply_type_.deleteData(reply);
}

void TypeLookupRequestListener::answer_request(
        SampleIdentity request_id,
        rpc::RemoteExceptionCode_t exception_code)
{
    TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(typelookup_manager_->reply_type_.createData());
    reply->header().relatedRequestId(request_id);
    reply->header().remoteEx(exception_code);

    typelookup_manager_->send(*reply);
    typelookup_manager_->reply_type_.deleteData(reply);
}

void TypeLookupRequestListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const changeIN)
{
    CacheChange_t* change = const_cast<CacheChange_t*>(changeIN);

    // Check if the data is received from the expected TypeLookup Request writer
    if (change->writerGUID.entityId != c_EntityId_TypeLookup_request_writer)
    {
        // Log a warning and remove the change from the history
        EPROSIMA_LOG_WARNING(TL_REQUEST_READER, "Received data from a bad endpoint.");
        reader->getHistory()->remove_change(change);
        return;
    }

    // Process the received TypeLookup Request and handle different types of requests
    TypeLookup_Request request;
    if (typelookup_manager_->receive(*change, request))
    {
        // Add request to the processing queue
        requests_queue_.push(request);
        {
            // Notify processor
            std::unique_lock<std::mutex> guard(request_processor_cv_mutex_);
            request_processor_cv_.notify_all();
        }
    }

    // Remove the processed cache change from the history
    reader->getHistory()->remove_change(change);
}

void TypeLookupRequestListener::onWriterChangeReceivedByAll(
        fastrtps::rtps::RTPSWriter*,
        fastrtps::rtps::CacheChange_t* change)
{
    typelookup_manager_->remove_builtin_request_writer_history_change(change);
}

} // namespace builtin

} // namespace dds
} // namespace fastdds
} // namespace eprosima
