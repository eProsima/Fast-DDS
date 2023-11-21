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

#include <fastdds/builtin/typelookupservice/TypeLookupRequestListener.hpp>
#include <fastdds/builtin/typelookupservice/TypeLookupManager.hpp>
#include <fastdds/builtin/typelookupservice/TypeLookupTypes.h>

#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastdds/dds/log/Log.hpp>

#include <utility>

using eprosima::fastrtps::rtps::RTPSReader;
using eprosima::fastrtps::rtps::CacheChange_t;
using eprosima::fastdds::dds::Log;

using eprosima::fastrtps::rtps::c_EntityId_TypeLookup_request_writer;

using namespace eprosima::fastrtps::types;
using eprosima::fastdds::dds::xtypes1_3::TypeIdentifier;
using eprosima::fastdds::dds::xtypes1_3::TypeObject;
using eprosima::fastdds::dds::xtypes1_3::TypeIdentifierTypeObjectPair;
using eprosima::fastdds::dds::xtypes1_3::TypeIdentifierPair;
using eprosima::fastdds::dds::xtypes1_3::TypeObjectRegistry;
using eprosima::fastdds::dds::xtypes1_3::TypeIdentfierWithSize;
using eprosima::fastdds::dds::xtypes1_3::TypeIdentfierWithSizeSeq;


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
    EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE_REQUEST_LISTENER, "Received new cache change.");

    TypeLookup_Request request;
    if (tlm_->request_reception(*change, request))
    {
        if (request.header().requestId().writer_guid() == tlm_->builtin_request_writer_->getGuid())
        {
            // Message from ourselves.
            return;
        }

        switch (request.data()._d())
        {
            case TypeLookup_getTypes_HashId:
            {
                const TypeLookup_getTypes_In in = request.data().getTypes();
                TypeLookup_getTypes_Out out;

                if(ReturnCode_t::RETCODE_OK != tlm_->get_registered_type_object(in, out))
                {
                    //RETCODE_NO_DATA if the given TypeIdentifier is not found in the registry.
                    break;
                }

                TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(tlm_->reply_type_.createData());
                TypeLookup_getTypes_Result result;
                result.result(out);
                reply->return_value().getType(result);
                reply->header().relatedRequestId() = request.header().requestId();

                tlm_->send_reply(*reply);
                tlm_->reply_type_.deleteData(reply);

                break;
            }
            case TypeLookup_getDependencies_HashId:
            {
                const TypeLookup_getTypeDependencies_In in = request.data().getTypeDependencies();
                TypeLookup_getTypeDependencies_Out out;

                if(ReturnCode_t::RETCODE_OK != tlm_->get_registered_type_dependencies(in, out))
                {
                    //RETCODE_NO_DATA if any given TypeIdentifier is unknown to the registry.
                    //RETCODE_BAD_PARAMETER if any given TypeIdentifier is not a direct hash.
                    break;
                }

                TypeLookup_Reply* reply = static_cast<TypeLookup_Reply*>(tlm_->reply_type_.createData());
                TypeLookup_getTypeDependencies_Result result;
                result.result(out);
                reply->return_value().getTypeDependencies(result);
                reply->header().relatedRequestId() = request.header().requestId();

                tlm_->send_reply(*reply);
                tlm_->reply_type_.deleteData(reply);

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
