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

#include <fastdds/builtin/typelookupservice/TypeLookupReplyListener.hpp>
#include <fastdds/builtin/typelookupservice/TypeLookupManager.hpp>
#include <fastdds/builtin/typelookupservice/TypeLookupTypes.h>

#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastdds/dds/log/Log.hpp>

#include <rtps/participant/RTPSParticipantImpl.h>

using eprosima::fastrtps::rtps::RTPSReader;
using eprosima::fastrtps::rtps::CacheChange_t;
using eprosima::fastdds::dds::Log;

using eprosima::fastrtps::rtps::c_EntityId_TypeLookup_reply_writer;

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

TypeLookupReplyListener::TypeLookupReplyListener(
        TypeLookupManager* manager)
    : tlm_(manager)
{
}

TypeLookupReplyListener::~TypeLookupReplyListener()
{
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
    if (tlm_->reply_reception(*change, reply))
    {
        if (reply.header().requestId().writer_guid() != tlm_.builtin_request_writer_->getGuid())
        {
            // This message isn't for us.
            return;
        }

        switch (reply.return_value()._d())
        {
            case TypeLookup_getTypes_HashId:
            {
                for (auto pair : reply.return_value().getType().result().types())
                {
                    tlm_->participant_->getListener()->on_type_discovery(
                        tlm_->participant_->getUserRTPSParticipant(),
                        reply.header().requestId(),
                        "",
                        &pair.type_identifier(),
                        &pair.type_object(),
                        DynamicType_ptr(nullptr));
                }
                break;
            }
            case TypeLookup_getDependencies_HashId:
            {
                tlm_->get_RTPS_participant()->getListener()->on_type_dependencies_reply(
                    tlm_->builtin_protocols_->mp_participantImpl->getUserRTPSParticipant(),
                    reply.header().requestId(),
                    reply.return_value().getTypeDependencies().result().dependent_typeids);
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
