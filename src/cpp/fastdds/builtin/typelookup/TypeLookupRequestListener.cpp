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

#include <fastdds/dds/builtin/typelookup/TypeLookupRequestListener.hpp>
#include <fastdds/dds/builtin/typelookup/TypeLookupManager.hpp>
#include <fastdds/dds/builtin/typelookup/common/TypeLookupTypes.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastdds/dds/log/Log.hpp>

#include <utility>

using eprosima::fastrtps::rtps::RTPSReader;
using eprosima::fastrtps::rtps::CacheChange_t;
using eprosima::fastrtps::types::TypeIdentifier;
using eprosima::fastrtps::types::TypeObject;
using eprosima::fastrtps::types::TypeIdentifierTypeObjectPair;
using eprosima::fastrtps::types::TypeIdentifierPair;
using eprosima::fastrtps::types::TypeObjectRegistry;
using eprosima::fastrtps::types::TypeIdentifierWithSize;
using eprosima::fastrtps::types::TypeIdentifierWithSizeSeq;
using eprosima::fastdds::dds::Log;

using eprosima::fastrtps::rtps::c_EntityId_TypeLookup_request_writer;

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

TypeLookupRequestListener::TypeLookupRequestListener(
        TypeLookupManager* manager)
    : tlm_(manager)
{
}

TypeLookupRequestListener::~TypeLookupRequestListener()
{
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
    if (tlm_->recv_request(*change, request))
    {
        // TODO checks?
        if (request.header.requestId.writer_guid() == tlm_->get_builtin_request_writer_guid())
        {
            // Message from our selves.
            return;
        }
        // request.header.instanceName

        switch (request.data._d())
        {
            case TypeLookup_getTypes_Hash:
            {
                const TypeLookup_getTypes_In in = request.data.getTypes();
                TypeLookup_getTypes_Out out;

                for (const TypeIdentifier& type_id : in.type_ids)
                {
                    TypeObject obj;
                    const TypeIdentifier* obj_ident;
                    ReturnCode_t ret_code = DomainParticipantFactory::get_instance()->type_object_registry()->get_type_object(type_id, obj);

                    if (obj_ident != nullptr && obj._d() != 0)
                    {
                        TypeIdentifierTypeObjectPair pair;
                        pair.type_identifier(type_id);
                        pair.type_object(obj);
                        out.types.push_back(std::move(pair));
                    }

                    if (obj_ident != nullptr && !(type_id == *obj_ident))
                    {
                        TypeIdentifierPair pair;
                        pair.type_identifier1(*obj_ident);
                        pair.type_identifier2(type_id);
                        out.complete_to_minimal.push_back(std::move(pair));
                    }
                }

                TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(tlm_->reply_type_.create_data());
                TypeLookup_getTypes_Result result;
                result.result(out);
                reply->return_value.getType(result);
                reply->header.requestId = request.header.requestId;

                tlm_->send_reply(*reply);
                tlm_->reply_type_.delete_data(reply);

                break;
            }
            case TypeLookup_getDependencies_Hash:
            {
                const TypeLookup_getTypeDependencies_In in = request.data.getTypeDependencies();
                TypeLookup_getTypeDependencies_Out out;

                out.dependent_typeids = get_registered_type_dependencies(in.type_ids, in.continuation_point, out.continuation_point);

                TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(tlm_->reply_type_.create_data());
                TypeLookup_getTypeDependencies_Result result;
                result.result(out);
                reply->return_value.getTypeDependencies(result);
                reply->header.requestId = request.header.requestId;

                tlm_->send_reply(*reply);
                tlm_->reply_type_.delete_data(reply);

                break;
            }
            default:
                break;
        }
    }
    reader->getHistory()->remove_change(change);
}

TypeIdentifierWithSizeSeq TypeLookupRequestListener::get_registered_type_dependencies(
    const TypeIdentifierSeq& identifiers,
    const OctetSeq& in_continuation_point,
    OctetSeq& out_continuation_point)
{
    const size_t max_size = 255;
    TypeIdentifierWithSizeSeq result;    
    size_t continuation_point = to_size_t(in_continuation_point);
    size_t start_index = max_size * continuation_point;

    {
        std::lock_guard<std::mutex> lock(dependencies_requests_cache_mutex);
        // Check if the identifier is already in the cache
        auto [dependencies_requests_cache_it, new_inserted] = dependencies_requests_cache.emplace(identifiers, std::unordered_set<TypeIdentifierWithSize>{});

        if (new_inserted)
        {
            // If not in cache, query the registry and handle errors
            std::unordered_set<TypeIdentfierWithSize> full_type_dependencies;
            ReturnCode_t ret_code = DomainParticipantFactory::get_instance()->type_object_registry()->get_type_dependencies(identifiers, full_type_dependencies);
            if(ret_code == RETCODE_OK)
            {
                // Create a new entry in the map
                dependencies_requests_cache[identifiers] = full_type_dependencies;
            }else{
                return result;
            }
        }

        // At this point, dependencies_requests_cache_it is guaranteed to be valid
        const auto& dependencies = dependencies_requests_cache_it->second;
        auto dependencies_it = dependencies.begin();
        
        // Advance the iterator to the starting point, respecting the set boundaries
        for (size_t i = 0; i < start_index && dependencies_it != dependencies.end(); ++i)
        {
            ++dependencies_it;
        }

        // Collect up to max_size dependencies
        for (size_t count = 0; dependencies_it != dependencies.end() && count < max_size; ++count)
        {
            result.push_back(*dependencies_it++);

            // If dependencies end reached, remove entry from dic
            if(dependencies_it == dependencies.end())
            {
                dependencies_requests_cache.erase(identifiers);
            }
            // If max_size reached, increment out_continuation_point
            if(count+1 == max_size)
            {
                ++out_continuation_point;
            }
        }
    }

    return result;
}

} // namespace builtin
} // namespace dds
} // namespace fastdds
} // namespace eprosima
