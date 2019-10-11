// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file EntityId_t.hpp
 */

#ifndef _FASTDDS_RTPS_COMMON_ENTITYID_T_HPP_
#define _FASTDDS_RTPS_COMMON_ENTITYID_T_HPP_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/common/Types.h>

#include <cstdint>
#include <cstring>
#include <sstream>

namespace eprosima {
namespace fastrtps {
namespace rtps {

using EntityId_t = uint32_t;

#define ENTITYID_UNKNOWN 0x00000000
#define ENTITYID_RTPSParticipant  0x000001c1
#define ENTITYID_SEDP_BUILTIN_TOPIC_WRITER  0x000002c2
#define ENTITYID_SEDP_BUILTIN_TOPIC_READER 0x000002c7
#define ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER  0x000003c2
#define ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER  0x000003c7
#define ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER 0x000004c2
#define ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER  0x000004c7
#define ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER  0x000100c2
#define ENTITYID_SPDP_BUILTIN_RTPSParticipant_READER  0x000100c7
#define ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_WRITER  0x000200C2
#define ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_READER  0x000200C7
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER  0x000201C3
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_READER  0x000201C4

#if HAVE_SECURITY
#define ENTITYID_SEDP_BUILTIN_PUBLICATIONS_SECURE_WRITER  0xff0003c2
#define ENTITYID_SEDP_BUILTIN_PUBLICATIONS_SECURE_READER  0xff0003c7
#define ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_SECURE_WRITER 0xff0004c2
#define ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_SECURE_READER  0xff0004c7
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_SECURE_WRITER 0xff0200c2
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_SECURE_READER 0xff0200c7
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_VOLATILE_MESSAGE_SECURE_WRITER  0xff0202C3
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_VOLATILE_MESSAGE_SECURE_READER  0xff0202C4
#define ENTITYID_SPDP_RELIABLE_BUILTIN_PARTICIPANT_SECURE_WRITER 0xff0101c2
#define ENTITYID_SPDP_RELIABLE_BUILTIN_PARTICIPANT_SECURE_READER 0xff0101c7
#endif

const EntityId_t c_EntityId_Unknown = ENTITYID_UNKNOWN;
const EntityId_t c_EntityId_SPDPReader = ENTITYID_SPDP_BUILTIN_RTPSParticipant_READER;
const EntityId_t c_EntityId_SPDPWriter = ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER;

const EntityId_t c_EntityId_SEDPPubWriter = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
const EntityId_t c_EntityId_SEDPPubReader = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
const EntityId_t c_EntityId_SEDPSubWriter = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
const EntityId_t c_EntityId_SEDPSubReader = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;

const EntityId_t c_EntityId_RTPSParticipant = ENTITYID_RTPSParticipant;

const EntityId_t c_EntityId_WriterLiveliness = ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_WRITER;
const EntityId_t c_EntityId_ReaderLiveliness = ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_READER;

const EntityId_t participant_stateless_message_writer_entity_id = ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER;
const EntityId_t participant_stateless_message_reader_entity_id = ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_READER;

#if HAVE_SECURITY
const EntityId_t sedp_builtin_publications_secure_writer = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_SECURE_WRITER;
const EntityId_t sedp_builtin_publications_secure_reader = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_SECURE_READER;
const EntityId_t sedp_builtin_subscriptions_secure_writer = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_SECURE_WRITER;
const EntityId_t sedp_builtin_subscriptions_secure_reader = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_SECURE_READER;

const EntityId_t participant_volatile_message_secure_writer_entity_id =
        ENTITYID_P2P_BUILTIN_PARTICIPANT_VOLATILE_MESSAGE_SECURE_WRITER;
const EntityId_t participant_volatile_message_secure_reader_entity_id =
        ENTITYID_P2P_BUILTIN_PARTICIPANT_VOLATILE_MESSAGE_SECURE_READER;

const EntityId_t c_EntityId_WriterLivelinessSecure = ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_SECURE_WRITER;
const EntityId_t c_EntityId_ReaderLivelinessSecure = ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_SECURE_READER;
#endif

inline EntityId_t read_entity_id_from_buffer(
        const octet* buffer)
{
    return
        ((uint32_t)buffer[0]) << 24 |
        ((uint32_t)buffer[1]) << 16 |
        ((uint32_t)buffer[2]) << 8 |
        ((uint32_t)buffer[3]);
}

inline void write_entity_id_to_buffer(
        EntityId_t entity_id,
        octet buffer[4])
{
    buffer[0] = (entity_id >> 24) & 0xFF;
    buffer[1] = (entity_id >> 16) & 0xFF;
    buffer[2] = (entity_id >> 8) & 0xFF;
    buffer[3] = entity_id & 0xFF;
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif /* _FASTDDS_RTPS_COMMON_ENTITYID_T_HPP_ */
