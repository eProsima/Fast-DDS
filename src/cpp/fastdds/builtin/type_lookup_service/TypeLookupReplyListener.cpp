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

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/RTPSDomainImpl.hpp>

using eprosima::fastdds::rtps::RTPSReader;
using eprosima::fastdds::rtps::CacheChange_t;
using eprosima::fastdds::rtps::c_EntityId_TypeLookup_reply_writer;

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

TypeLookupReplyListener::TypeLookupReplyListener(
        TypeLookupManager* manager)
    : typelookup_manager_(manager)
{
    start_reply_processor_thread();
}

TypeLookupReplyListener::~TypeLookupReplyListener()
{
    stop_reply_processor_thread();
}

void TypeLookupReplyListener::start_reply_processor_thread()
{
    std::unique_lock<std::mutex> guard(replies_processor_cv_mutex_);
    // Check if is not already in progress and the thread is not joinable
    if (!processing_ && !replies_processor_thread.joinable())
    {
        processing_ = true;
        // Lambda function to be executed by the thread
        auto thread_func = [this]()
                {
                    process_reply();
                };
        // Create and start the processing thread
        replies_processor_thread = eprosima::create_thread(thread_func,
                        typelookup_manager_->participant_->get_attributes().typelookup_service_thread,
                        "dds.tls.replies.%u");
    }
}

void TypeLookupReplyListener::stop_reply_processor_thread()
{
    {
        // Set processing_ to false to signal the processing thread to stop
        std::unique_lock<std::mutex> guard(replies_processor_cv_mutex_);
        processing_ = false;
    }

    if (replies_processor_thread.joinable())
    {
        // Notify the processing thread to wake up and check the exit condition
        replies_processor_cv_.notify_all();
        // Check if the calling thread is not the processing thread and join it
        if (!replies_processor_thread.is_calling_thread())
        {
            replies_processor_thread.join();
        }
    }
}

void TypeLookupReplyListener::process_reply()
{
    std::unique_lock<std::mutex> guard(replies_processor_cv_mutex_);

    while (processing_)
    {
        // Wait until either processing is done or there are replies in the queue
        replies_processor_cv_.wait(guard,
                [&]()
                {
                    return !processing_ || !replies_queue_.empty();
                });

        if (!replies_queue_.empty())
        {
            TypeLookup_Reply& reply = replies_queue_.front().reply;

            // Check if the received reply SampleIdentity corresponds to an outstanding request
            auto& request_id {reply.header().relatedRequestId()};
            auto request_it = typelookup_manager_->async_get_type_requests_.find(request_id);
            if (request_it != typelookup_manager_->async_get_type_requests_.end())
            {
                xtypes::TypeIdentfierWithSize type_id {request_it->second};
                // Process the TypeLookup_Reply based on its type
                switch (reply.return_value()._d())
                {
                    case TypeLookup_getTypes_HashId:
                    {
                        if (RETCODE_OK == reply.return_value().getType()._d())
                        {
                            check_get_types_reply(request_id, type_id,
                                    reply.return_value().getType().result(), reply.header().relatedRequestId());
                        }
                        else
                        {
                            typelookup_manager_->notify_callbacks(RETCODE_NO_DATA, type_id);
                            typelookup_manager_->remove_async_get_type_request(request_id);
                        }
                        break;
                    }
                    case TypeLookup_getDependencies_HashId:
                    {
                        if (RETCODE_OK == reply.return_value().getTypeDependencies()._d())
                        {
                            check_get_type_dependencies_reply(
                                request_id, type_id, replies_queue_.front().type_server,
                                reply.return_value().getTypeDependencies().result());
                        }
                        else
                        {
                            typelookup_manager_->notify_callbacks(RETCODE_NO_DATA, type_id);
                            typelookup_manager_->remove_async_get_type_request(request_id);
                        }
                        break;
                    }
                    default:
                        // If the type of request is not known, log an error
                        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REPLY_LISTENER,
                                "Received unknown reply operation type in type lookup service.");
                        break;
                }
            }
            // Remove the requests from the queue
            replies_queue_.pop();
        }
    }
}

void TypeLookupReplyListener::check_get_types_reply(
        const SampleIdentity& request_id,
        const xtypes::TypeIdentfierWithSize& type_id,
        const TypeLookup_getTypes_Out& reply,
        SampleIdentity related_request)
{
    ReturnCode_t register_result = RETCODE_OK;

    if (0 != reply.types().size())
    {
        for (xtypes::TypeIdentifierTypeObjectPair pair : reply.types())
        {
            xtypes::TypeIdentifierPair type_ids;
            type_ids.type_identifier1(pair.type_identifier());
            if (RETCODE_OK != fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                            register_type_object(pair.type_object(), type_ids, false))
            {
                // If any of the types is not registered, log error
                EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REPLY_LISTENER,
                        "Error registering remote type.");
                register_result = RETCODE_ERROR;
            }
        }

        if (RETCODE_OK == register_result)
        {
            // Check if the get_type_dependencies related to this reply required a continuation_point
            std::unique_lock<std::mutex> guard(replies_with_continuation_mutex_);
            auto it = std::find(replies_with_continuation_.begin(),
                            replies_with_continuation_.end(), related_request);
            if (it != replies_with_continuation_.end())
            {
                // If it did, remove it from the list and continue
                replies_with_continuation_.erase(it);
            }
            else
            {
                // If it did not, check that the type that originated the request is consistent
                // before notifying the callbacks associated with the request
                try
                {
                    xtypes::TypeObject type_object;
                    fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().get_type_object(
                        type_id.type_id(), type_object);
                    xtypes::TypeObjectUtils::type_object_consistency(type_object);
                    xtypes::TypeIdentifierPair type_ids;
                    if (RETCODE_OK !=
                            fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                                    register_type_object(type_object, type_ids, true))
                    {
                        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REPLY_LISTENER,
                                "Cannot register minimal of remote type");
                    }

                    typelookup_manager_->notify_callbacks(RETCODE_OK, type_id);
                }
                catch (const std::exception& exception)
                {
                    EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE_REPLY_LISTENER,
                            "Error registering remote type: " << exception.what());
                }
            }
        }
    }
    else
    {
        typelookup_manager_->notify_callbacks(RETCODE_NO_DATA, type_id);
        EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REPLY_LISTENER,
                "Received reply with no types.");
        register_result = RETCODE_ERROR;
    }

    // Remove the processed SampleIdentity from the outstanding requests
    typelookup_manager_->remove_async_get_type_request(request_id);
}

void TypeLookupReplyListener::check_get_type_dependencies_reply(
        const SampleIdentity& request_id,
        const xtypes::TypeIdentfierWithSize& type_id,
        const fastdds::rtps::GUID_t type_server,
        const TypeLookup_getTypeDependencies_Out& reply)
{
    // Add the dependent types to the list for the get_type request
    xtypes::TypeIdentifierSeq needed_types;
    std::unordered_set<xtypes::TypeIdentifier> unique_types;

    for (xtypes::TypeIdentfierWithSize type : reply.dependent_typeids())
    {
        // Check if the type is known
        if (!fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                        is_type_identifier_known(type))
        {
            // Insert the type into the unordered_set and check if the insertion was successful
            if (unique_types.insert(type.type_id()).second)
            {
                // If the insertion was successful, it means the type was not already in the set
                needed_types.push_back(type.type_id());
            }
            // If the insertion was not successful, the type is a duplicate and can be ignored
        }
    }

    // If there is no continuation point, add the parent type
    if (reply.continuation_point().empty())
    {
        needed_types.push_back(type_id.type_id());
    }
    // Make a new request with the continuation point
    else
    {
        SampleIdentity next_request_id = typelookup_manager_->
                        get_type_dependencies({type_id.type_id()}, type_server,
                        reply.continuation_point());
        if (INVALID_SAMPLE_IDENTITY != next_request_id)
        {
            // Store the sent requests and associated TypeIdentfierWithSize
            typelookup_manager_->add_async_get_type_request(next_request_id, type_id);
        }
        else
        {
            // Failed to send request
            EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE_REPLY_LISTENER, "Failed to send get_type_dependencies request");
        }
    }

    // Send the type request
    SampleIdentity get_types_request = typelookup_manager_->get_types(needed_types, type_server);

    if (INVALID_SAMPLE_IDENTITY != get_types_request)
    {
        // Store the type request
        typelookup_manager_->add_async_get_type_request(get_types_request, type_id);

        // If this get_types request has a continuation_point, store it in the list
        if (!reply.continuation_point().empty())
        {
            std::unique_lock<std::mutex> guard(replies_with_continuation_mutex_);
            replies_with_continuation_.push_back(get_types_request);
        }
    }
    else
    {
        // Failed to send request
        EPROSIMA_LOG_ERROR(TYPELOOKUP_SERVICE_REPLY_LISTENER, "Failed to send get_types request");
    }

    // Remove the processed SampleIdentity from the outstanding requests
    typelookup_manager_->remove_async_get_type_request(request_id);
}

void TypeLookupReplyListener::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = const_cast<CacheChange_t*>(change_in);

    // Check if the data is received from the expected TypeLookup Reply writer
    if (change->writerGUID.entityId != c_EntityId_TypeLookup_reply_writer)
    {
        // Log a warning and remove the change from the history
        EPROSIMA_LOG_WARNING(TL_REPLY_READER, "Received data from a bad endpoint.");
        reader->get_history()->remove_change(change);
        return;
    }

    // Process the received TypeLookup Reply and handle different types of replies
    TypeLookup_Reply reply;
    if (typelookup_manager_->receive(*change, reply))
    {
        // Check if the reply has any exceptions
        if (reply.header().remoteEx() != rpc::RemoteExceptionCode_t::REMOTE_EX_OK)
        {
            // TODO: Implement specific handling for each exception
            EPROSIMA_LOG_WARNING(TYPELOOKUP_SERVICE_REPLY_LISTENER,
                    "Received reply with exception code: " << static_cast<int>(reply.header().remoteEx()));
            // If the reply was not ok, ignore it
            return;
        }

        {
            std::unique_lock<std::mutex> guard(replies_processor_cv_mutex_);
            // Add reply to the processing queue
            replies_queue_.push(ReplyWithServerGUID{reply, change->writerGUID});
            // Notify processor
            replies_processor_cv_.notify_all();
        }
    }

    // Remove the processed cache change from the history
    reader->get_history()->remove_change(change);
}

void TypeLookupReplyListener::on_writer_change_received_by_all(
        fastdds::rtps::RTPSWriter*,
        fastdds::rtps::CacheChange_t* change)
{
    typelookup_manager_->remove_builtin_reply_writer_history_change(change);
}

} // namespace builtin

} // namespace dds
} // namespace fastdds
} // namespace eprosima
