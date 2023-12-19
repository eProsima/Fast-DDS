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
    TypeLookup_getTypeDependencies_Out out;

    std::unordered_set<xtypes::TypeIdentfierWithSize> type_dependencies;
    if (RETCODE_OK == fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                    get_type_dependencies(request.type_ids(), type_dependencies))
    {
        TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(typelookup_manager_->reply_type_.createData());
        TypeLookup_getTypeDependencies_Result result;
        result.result(out);
        reply->return_value().getTypeDependencies(result);
        reply->header().relatedRequestId() = request_id;

        typelookup_manager_->send_reply(*reply);
        typelookup_manager_->reply_type_.deleteData(reply);
    }
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
