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
 * @file TypeLookupReplyListener.cpp
 *
 */

#include <fastdds/builtin/type_lookup_service/TypeLookupReplyListener.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/RTPSDomainImpl.hpp>


using eprosima::fastrtps::rtps::RTPSReader;
using eprosima::fastrtps::rtps::CacheChange_t;
using eprosima::fastdds::dds::Log;
using eprosima::fastrtps::rtps::c_EntityId_TypeLookup_reply_writer;


namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

TypeLookupReplyListener::TypeLookupReplyListener(
        TypeLookupManager* manager)
    : typelookup_manager_(manager)
{
}

TypeLookupReplyListener::~TypeLookupReplyListener()
{
}

void TypeLookupReplyListener::check_get_types_reply(
        SampleIdentity request_id,
        const TypeLookup_getTypes_Out& reply)
{
    // Check if we were waiting that reply SampleIdentity
    auto requests_it = typelookup_manager_->async_get_type_requests_.find(request_id);
    if (requests_it != typelookup_manager_->async_get_type_requests_.end())
    {
        for (xtypes::TypeIdentifierTypeObjectPair pair : reply.types())
        {
            if (RETCODE_OK == fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                            register_type_object(pair.type_identifier(), pair.type_object()))
            {
                typelookup_manager_->notify_callbacks(requests_it->second);
            }
        }
        // Erase the processed SampleIdentity
        typelookup_manager_->remove_async_get_types_request(request_id);
    }
}

void TypeLookupReplyListener::check_get_type_dependencies_reply(
        SampleIdentity request_id,
        const TypeLookup_getTypeDependencies_Out& reply)
{
    // Check if we were waiting that reply SampleIdentity
    auto requests_it = typelookup_manager_->async_get_type_requests_.find(request_id);
    if (requests_it != typelookup_manager_->async_get_type_requests_.end())
    {
        bool are_dependencies_solved = true;
        for (xtypes::TypeIdentfierWithSize type_id : reply.dependent_typeids())
        {
            ReturnCode_t solve_ret = typelookup_manager_->check_type_identifier_received(type_id);
            if (RETCODE_OK != solve_ret)
            {
                are_dependencies_solved = false;
            }
        }

        // If all dependencies are known, ask for the parent type
        if (are_dependencies_solved)
        {
            xtypes::TypeIdentifierSeq uknown_type;
            uknown_type.push_back(requests_it->second.type_id());

            SampleIdentity get_types_request = typelookup_manager_->get_types(uknown_type);
            if (builtin::INVALID_SAMPLE_IDENTITY != get_types_request)
            {
                // Store the sent request and associated TypeIdentfierWithSize
                typelookup_manager_->add_async_get_type_request(get_types_request, requests_it->second);
            }
        }
        // Erase the processed SampleIdentity
        typelookup_manager_->remove_async_get_types_request(request_id);
    }
}

void TypeLookupReplyListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const changeIN)
{
    CacheChange_t* change = const_cast<CacheChange_t*>(changeIN);

    if (change->writerGUID.entityId != c_EntityId_TypeLookup_reply_writer)
    {
        EPROSIMA_LOG_WARNING(TL_REPLY_READER, "Received data from a bad endpoint.");
        reader->getHistory()->remove_change(change);
    }
    EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE_REPLY_LISTENER, "Received new cache change");

    TypeLookup_Reply reply;
    if (typelookup_manager_->receive_reply(*change, reply))
    {
        switch (reply.return_value()._d())
        {
            case TypeLookup_getTypes_HashId:
            {
                check_get_types_reply(reply.header().relatedRequestId(),
                        reply.return_value().getType().result());
                break;
            }
            case TypeLookup_getDependencies_HashId:
            {
                check_get_type_dependencies_reply(reply.header().relatedRequestId(),
                        reply.return_value().getTypeDependencies().result());
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
