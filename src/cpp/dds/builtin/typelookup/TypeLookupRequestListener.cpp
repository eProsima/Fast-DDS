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

#include <fastrtps/types/TypeObjectFactory.h>

#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/log/Log.h>

#include <utility>

using eprosima::fastrtps::rtps::RTPSReader;
using eprosima::fastrtps::rtps::CacheChange_t;
using eprosima::fastrtps::types::TypeIdentifier;
using eprosima::fastrtps::types::TypeObject;
using eprosima::fastrtps::types::TypeIdentifierTypeObjectPair;
using eprosima::fastrtps::types::TypeIdentifierPair;
using eprosima::fastrtps::types::TypeObjectFactory;
using eprosima::fastrtps::types::TypeIdentifierWithSize;
using eprosima::fastrtps::types::TypeIdentifierWithSizeSeq;
using eprosima::fastrtps::Log;

using eprosima::fastrtps::rtps::c_EntityId_TypeLookup_request_writer;

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

TypeLookupRequestListener::TypeLookupRequestListener(
        TypeLookupManager* manager)
    : tlm_(manager)
    , factory_(TypeObjectFactory::get_instance())
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
        logWarning(TL_REQUEST_READER, "Received data from a bad endpoint.");
        reader->getHistory()->remove_change(change);
    }

    logInfo(TYPELOOKUP_SERVICE_REQUEST_LISTENER, "Received new cache change");
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
                    const TypeIdentifier* obj_ident = factory_->typelookup_get_type(type_id, obj);

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
                //for (size_t index = 0; index < in.type_ids.size(); ++index)
                {
                    out.dependent_typeids = factory_->typelookup_get_type_dependencies(
                        in.type_ids, in.continuation_point, out.continuation_point, 255); // TODO: Make configurable?
                }

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

} // namespace builtin
} // namespace dds
} // namespace fastdds
} // namespace eprosima
