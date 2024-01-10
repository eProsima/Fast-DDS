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

const int MAX_DEPENDENCIES_PER_REQUEST = 100;

/**
 * @brief Calculates the opaque value of continuation point.
 * @param continuation_point[in] The continuation point.
 * @return The value of the continuation_point.
 */
size_t calculate_continuation_point(
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
std::vector<uint8_t> create_continuation_point(
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
    for (const xtypes::TypeIdentifier& type_id : request.type_ids())
    {
        xtypes::TypeObject obj;
        if (RETCODE_OK == fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                        get_type_object(type_id, obj))
        {
            xtypes::TypeIdentifierTypeObjectPair pair;
            pair.type_identifier(type_id);
            pair.type_object(obj);
            out.types().push_back(std::move(pair));
        }
    }
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
    ReturnCode_t result;
    // Check if the received request has been done before and needed a continuation point
    {
        std::lock_guard<std::mutex> lock(requests_with_continuation_mutex_);
        auto requests_it = requests_with_continuation_.find(request.type_ids());
        if (requests_it != requests_with_continuation_.end())
        {
            // Get the dependencies without chechking the registry
            type_dependencies = requests_it->second;
            result = RETCODE_OK;
        }
        else
        {
            // Get the dependencies from the registry
            result = fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                            get_type_dependencies(request.type_ids(), type_dependencies);

            // If there are too many dependent types, store the result for future requests
            if (result == RETCODE_OK && type_dependencies.size() > MAX_DEPENDENCIES_PER_REQUEST)
            {
                requests_with_continuation_.emplace(request.type_ids(), type_dependencies);
            }
        }
    }

    if (RETCODE_OK == result)
    {
        TypeLookup_getTypeDependencies_Out out =
                prepare_dependent_types(request.type_ids(), type_dependencies, request.continuation_point());

        TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(typelookup_manager_->reply_type_.createData());
        TypeLookup_getTypeDependencies_Result result;
        result.result(out);
        reply->return_value().getTypeDependencies(result);
        reply->header().relatedRequestId() = request_id;

        typelookup_manager_->send_reply(*reply);
        typelookup_manager_->reply_type_.deleteData(reply);
    }
}

TypeLookup_getTypeDependencies_Out TypeLookupRequestListener::prepare_dependent_types(
        const xtypes::TypeIdentifierSeq& id_seq,
        const std::unordered_set<xtypes::TypeIdentfierWithSize>& type_dependencies,
        const std::vector<uint8_t>& continuation_point)
{
    TypeLookup_getTypeDependencies_Out out;

    std::vector<xtypes::TypeIdentfierWithSize> dependent_types;
    if (type_dependencies.size() < MAX_DEPENDENCIES_PER_REQUEST)
    {
        std::copy(type_dependencies.begin(), type_dependencies.end(), std::back_inserter(dependent_types));
    }
    else
    {
        size_t start_index = 0;
        if (!continuation_point.empty())
        {
            start_index = calculate_continuation_point(continuation_point) * MAX_DEPENDENCIES_PER_REQUEST;
        }

        auto start_it = std::next(type_dependencies.begin(), start_index);
        auto end_it = std::next(start_it, std::min<size_t>(MAX_DEPENDENCIES_PER_REQUEST,
                        type_dependencies.size() - start_index));
        std::copy(start_it, end_it, std::back_inserter(dependent_types));

        if ((start_index + MAX_DEPENDENCIES_PER_REQUEST) > type_dependencies.size())
        {
            // Is all dependent types have been sent, remove from map
            std::lock_guard<std::mutex> lock(requests_with_continuation_mutex_);
            auto requests_it = requests_with_continuation_.find(id_seq);
            if (requests_it != requests_with_continuation_.end())
            {
                requests_with_continuation_.erase(requests_it);
            }
        }
        else
        {
            out.continuation_point(create_continuation_point(calculate_continuation_point(continuation_point) + 1));
        }
    }

    out.dependent_typeids(dependent_types);

    return out;
}

void TypeLookupRequestListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const changeIN)
{
    CacheChange_t* change = const_cast<CacheChange_t*>(changeIN);

    if (change->writerGUID.entityId != c_EntityId_TypeLookup_request_writer)
    {
        EPROSIMA_LOG_WARNING(TL_REQUEST_READER, "Received data from a bad endpoint.");
        reader->getHistory()->remove_change(change);
    }

    EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE_REQUEST_LISTENER, "Received new cache change");
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
    reader->getHistory()->remove_change(change);
}

} // namespace builtin

} // namespace dds
} // namespace fastdds
} // namespace eprosima
