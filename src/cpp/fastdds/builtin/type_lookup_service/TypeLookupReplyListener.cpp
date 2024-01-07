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
        const SampleIdentity& request_id,
        const TypeLookup_getTypes_Out& reply)
{
    // Check if the received reply SampleIdentity corresponds to an outstanding request
    auto requests_it = typelookup_manager_->async_get_type_requests_.find(request_id);
    if (requests_it != typelookup_manager_->async_get_type_requests_.end())
    {
        for (xtypes::TypeIdentifierTypeObjectPair pair : reply.types())
        {
            if (RETCODE_OK == fastrtps::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                            register_type_object(pair.type_identifier(), pair.type_object()))
            {
                // Notify the callbacks associated with the request
                typelookup_manager_->notify_callbacks(requests_it->second);
            }
        }
        // Remove the processed SampleIdentity from the outstanding requests
        typelookup_manager_->remove_async_get_types_request(request_id);
    }
}

void TypeLookupReplyListener::check_get_type_dependencies_reply(
        const SampleIdentity& request_id,
        const fastrtps::rtps::GuidPrefix_t type_server,
        const TypeLookup_getTypeDependencies_Out& reply)
{
    // Check if the received reply SampleIdentity corresponds to an outstanding request
    auto requests_it = typelookup_manager_->async_get_type_requests_.find(request_id);
    if (requests_it == typelookup_manager_->async_get_type_requests_.end())
    {
        // The reply is not associated with any outstanding request, ignore it
        return;
    }

    // Check each dependent TypeIdentifierWithSize to ensure all dependencies are known
    bool all_dependencies_known = std::all_of(
        reply.dependent_typeids().begin(), reply.dependent_typeids().end(),
        [&](const xtypes::TypeIdentfierWithSize& type_id)
        {
            // Check if the dependent TypeIdentifierWithSize is known; creating a new request if it is not
            return RETCODE_OK == typelookup_manager_->check_type_identifier_received(type_id, type_server);
        });

    // If the received reply has continuation point, send next request
    if (!reply.continuation_point().empty())
    {
        // Make a new request with the continuation point
        xtypes::TypeIdentifierSeq unknown_type{requests_it->second.type_id()};
        SampleIdentity next_request_id = typelookup_manager_->
                        get_type_dependencies(unknown_type, type_server, reply.continuation_point());
        if (INVALID_SAMPLE_IDENTITY != next_request_id)
        {
            // Store the sent requests and associated TypeIdentfierWithSize
            typelookup_manager_->add_async_get_type_request(next_request_id, requests_it->second);
        }
        else
        {
            // Failed to send request
            EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Failed to send get_type_dependencies request");
        }
    }
    // If all dependencies are known and there is no continuation point, request the parent type
    else if (all_dependencies_known)
    {
        xtypes::TypeIdentifierSeq unknown_type{requests_it->second.type_id()};

        // Initiate a type request to obtain the parent type
        SampleIdentity get_types_request = typelookup_manager_->get_types(unknown_type, type_server);

        if (INVALID_SAMPLE_IDENTITY != get_types_request)
        {
            // Store the type request
            typelookup_manager_->add_async_get_type_request(get_types_request, requests_it->second);
        }
        else
        {
            // Failed to send request
            EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE, "Failed to send get_types request");
        }
    }

    // Remove the processed SampleIdentity from the outstanding requests
    typelookup_manager_->remove_async_get_types_request(request_id);
}

void TypeLookupReplyListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = const_cast<CacheChange_t*>(change_in);

    // Check if the data is received from the expected TypeLookup Reply writer
    if (change->writerGUID.entityId != c_EntityId_TypeLookup_reply_writer)
    {
        // Log a warning and remove the change from the history
        EPROSIMA_LOG_WARNING(TL_REPLY_READER, "Received data from a bad endpoint.");
        reader->getHistory()->remove_change(change);
    }
    EPROSIMA_LOG_INFO(TYPELOOKUP_SERVICE_REPLY_LISTENER, "Received new cache change");

    // Process the received TypeLookup Reply and handle different types of replies
    TypeLookup_Reply reply;
    if (typelookup_manager_->receive_reply(*change, reply))
    {
        switch (reply.return_value()._d())
        {
            case TypeLookup_getTypes_HashId:
            {
                check_get_types_reply(reply.header().relatedRequestId(), reply.return_value().getType().result());
                break;
            }
            case TypeLookup_getDependencies_HashId:
            {
                check_get_type_dependencies_reply(reply.header().relatedRequestId(), change->writerGUID.guidPrefix,
                        reply.return_value().getTypeDependencies().result());
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
