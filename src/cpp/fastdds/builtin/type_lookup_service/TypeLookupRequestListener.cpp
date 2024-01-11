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

#include <future>
#include <unordered_set>
#include <utility>

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
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
const int MAX_DEPENDENCIES_PER_REPLY = 75;

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
        int value)
{
    std::vector<uint8_t> continuation_point(32, 0);

    for (int value_i = 0; value_i < value; value_i++)
    {
        for (int i = continuation_point.size() - 1; i >= 0; --i)
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
}

TypeLookupRequestListener::~TypeLookupRequestListener()
{
}

void TypeLookupRequestListener::check_get_types_request(
        SampleIdentity request_id,
        const TypeLookup_getTypes_In& request)
{
    TypeLookup_getTypes_Out out;
    // Iterate through requested type_ids
    for (const xtypes::TypeIdentifier& type_id : request.type_ids())
    {
        xtypes::TypeObject obj;
        if (RETCODE_OK == fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                        get_type_object(type_id, obj))
        {
            xtypes::TypeIdentifierTypeObjectPair pair;
            pair.type_identifier(type_id);
            pair.type_object(obj);
            // Add the pair to the result
            out.types().push_back(std::move(pair));
        }
    }
    // Create and send the reply
    TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(typelookup_manager_->reply_type_.createData());
    TypeLookup_getTypes_Result result;
    result.result(out);
    reply->return_value().getType(result);
    reply->header().relatedRequestId() = request_id;

    typelookup_manager_->send_reply(*reply);
    typelookup_manager_->reply_type_.deleteData(reply);
}

void TypeLookupRequestListener::check_get_type_dependencies_request(
        SampleIdentity request_id,
        const TypeLookup_getTypeDependencies_In& request)
{
    std::unordered_set<xtypes::TypeIdentfierWithSize> type_dependencies;
    ReturnCode_t type_dependencies_result;
    // Check if the received request has been done before and needed a continuation point
    {
        std::lock_guard<std::mutex> lock(requests_with_continuation_mutex_);
        auto requests_it = requests_with_continuation_.find(request.type_ids());
        if (requests_it != requests_with_continuation_.end())
        {
            // Get the dependencies without chechking the registry
            type_dependencies = requests_it->second;
            type_dependencies_result = RETCODE_OK;
        }
        else
        {
            // Get the dependencies from the registry
            type_dependencies_result = fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                            get_type_dependencies(request.type_ids(), type_dependencies);

            // If there are too many dependent types, store the type dependencies for future requests
            if (type_dependencies_result == RETCODE_OK && type_dependencies.size() > MAX_DEPENDENCIES_PER_REPLY)
            {
                requests_with_continuation_.emplace(request.type_ids(), type_dependencies);
            }
        }
    }

    if (RETCODE_OK == type_dependencies_result)
    {
        // Prepare and send the reply
        TypeLookup_getTypeDependencies_Out out = prepare_get_type_dependencies_response(
            request.type_ids(), type_dependencies, request.continuation_point());
        TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(typelookup_manager_->reply_type_.createData());
        TypeLookup_getTypeDependencies_Result result;
        result.result(out);
        reply->return_value().getTypeDependencies(result);
        reply->header().relatedRequestId() = request_id;

        typelookup_manager_->send_reply(*reply);
        typelookup_manager_->reply_type_.deleteData(reply);
    }
}

TypeLookup_getTypeDependencies_Out TypeLookupRequestListener::prepare_get_type_dependencies_response(
        const xtypes::TypeIdentifierSeq& id_seq,
        const std::unordered_set<xtypes::TypeIdentfierWithSize>& type_dependencies,
        const std::vector<uint8_t>& continuation_point)
{
    TypeLookup_getTypeDependencies_Out out;
    std::vector<xtypes::TypeIdentfierWithSize> dependent_types;

    // Check if all dependencies can be sent in a single response
    if (type_dependencies.size() < MAX_DEPENDENCIES_PER_REPLY)
    {
        std::copy(type_dependencies.begin(), type_dependencies.end(), std::back_inserter(dependent_types));
    }
    else
    {
        size_t start_index = 0;
        // Check if a continuation point is provided, and calculate starting point if there is
        if (!continuation_point.empty())
        {
            start_index = calculate_continuation_point(continuation_point) * MAX_DEPENDENCIES_PER_REPLY;
        }

        // Copy the dependencies within the specified range
        auto start_it = std::next(type_dependencies.begin(), start_index);
        auto end_it = std::next(start_it, std::min<size_t>(MAX_DEPENDENCIES_PER_REPLY,
                        type_dependencies.size() - start_index));
        std::copy(start_it, end_it, std::back_inserter(dependent_types));

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

    // Set the dependent types in the reply
    out.dependent_typeids(dependent_types);

    return out;
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
    }
    EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE_REQUEST_LISTENER, "Received new cache change");

    // Process the received TypeLookup Request and handle different types of requests
    TypeLookup_Request request;
    if (typelookup_manager_->receive_request(*change, request))
    {
        switch (request.data()._d())
        {
            case TypeLookup_getTypes_HashId:
            {
                std::async(std::launch::async,
                        &TypeLookupRequestListener::check_get_types_request, this,
                        request.header().requestId(), request.data().getTypes());
                break;
            }
            case TypeLookup_getDependencies_HashId:
            {
                std::async(std::launch::async,
                        &TypeLookupRequestListener::check_get_type_dependencies_request, this,
                        request.header().requestId(), request.data().getTypeDependencies());
                break;
            }
            default:
                break;
        }
    }

    // Remove the processed cache change from the history
    reader->getHistory()->remove_change(change);
}

} // namespace builtin

} // namespace dds
} // namespace fastdds
} // namespace eprosima
