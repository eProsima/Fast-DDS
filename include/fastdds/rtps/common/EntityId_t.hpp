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

#define ENTITYID_TL_SVC_REQ_WRITER  0x000300C3
#define ENTITYID_TL_SVC_REQ_READER  0x000300C4
#define ENTITYID_TL_SVC_REPLY_WRITER  0x000301C3
#define ENTITYID_TL_SVC_REPLY_READER  0x000301C4

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
#endif // if HAVE_SECURITY

#define ENTITYID_DS_SERVER_VIRTUAL_WRITER 0x00030073
#define ENTITYID_DS_SERVER_VIRTUAL_READER 0x00030074

//!@brief Structure EntityId_t, entity id part of GUID_t.
//!@ingroup COMMON_MODULE
struct RTPS_DllAPI EntityId_t
{
    static constexpr unsigned int size = 4;
    octet value[size];
    //! Default constructor. Uknown entity.
    EntityId_t()
    {
        *this = ENTITYID_UNKNOWN;
    }

    /**
     * Main constructor.
     * @param id Entity id
     */
    EntityId_t(
            uint32_t id)
    {
        memcpy(value, &id, size);
#if !FASTDDS_IS_BIG_ENDIAN_TARGET
        reverse();
#endif // if !FASTDDS_IS_BIG_ENDIAN_TARGET
    }

    /*!
     * @brief Copy constructor
     */
    EntityId_t(
            const EntityId_t& id)
    {
        memcpy(value, id.value, size);
    }

    /*!
     * @brief Move constructor
     */
    EntityId_t(
            EntityId_t&& id)
    {
        memmove(value, id.value, size);
    }

    EntityId_t& operator =(
            const EntityId_t& id)
    {
        memcpy(value, id.value, size);
        return *this;
    }

    EntityId_t& operator =(
            EntityId_t&& id)
    {
        memmove(value, id.value, size);
        return *this;
    }

    /**
     * Assignment operator.
     * @param id Entity id to copy
     */
    EntityId_t& operator =(
            uint32_t id)
    {
        memcpy(value, &id, size);
#if !FASTDDS_IS_BIG_ENDIAN_TARGET
        reverse();
#endif // if !FASTDDS_IS_BIG_ENDIAN_TARGET
        return *this;
        //return id;
    }

#if !FASTDDS_IS_BIG_ENDIAN_TARGET
    //!
    void reverse()
    {
        octet oaux;
        oaux = value[3];
        value[3] = value[0];
        value[0] = oaux;
        oaux = value[2];
        value[2] = value[1];
        value[1] = oaux;
    }

#endif // if !FASTDDS_IS_BIG_ENDIAN_TARGET

    static EntityId_t unknown()
    {
        return EntityId_t();
    }

};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Guid prefix comparison operator
 * @param id1 EntityId to compare
 * @param id2 ID prefix to compare
 * @return True if equal
 */
inline bool operator ==(
        EntityId_t& id1,
        const uint32_t id2)
{
#if !FASTDDS_IS_BIG_ENDIAN_TARGET
    id1.reverse();
#endif // if !FASTDDS_IS_BIG_ENDIAN_TARGET
    const bool result = 0 == memcmp(id1.value, &id2, sizeof(id2));
#if !FASTDDS_IS_BIG_ENDIAN_TARGET
    id1.reverse();
#endif // if !FASTDDS_IS_BIG_ENDIAN_TARGET
    return result;
}

/**
 * Guid prefix comparison operator
 * @param id1 First EntityId to compare
 * @param id2 Second EntityId to compare
 * @return True if equal
 */
inline bool operator ==(
        const EntityId_t& id1,
        const EntityId_t& id2)
{
    for (uint8_t i = 0; i < 4; ++i)
    {
        if (id1.value[i] != id2.value[i])
        {
            return false;
        }
    }
    return true;
}

/**
 * Guid prefix comparison operator
 * @param id1 First EntityId to compare
 * @param id2 Second EntityId to compare
 * @return True if not equal
 */
inline bool operator !=(
        const EntityId_t& id1,
        const EntityId_t& id2)
{
    for (uint8_t i = 0; i < 4; ++i)
    {
        if (id1.value[i] != id2.value[i])
        {
            return true;
        }
    }
    return false;
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

inline std::ostream& operator <<(
        std::ostream& output,
        const EntityId_t& enI)
{
    output << std::hex;
    output << (int)enI.value[0] << "." << (int)enI.value[1] << "." << (int)enI.value[2] << "." << (int)enI.value[3];
    return output << std::dec;
}

inline std::istream& operator >>(
        std::istream& input,
        EntityId_t& enP)
{
    std::istream::sentry s(input);

    if (s)
    {
        char point;
        unsigned short hex;
        std::ios_base::iostate excp_mask = input.exceptions();

        try
        {
            input.exceptions(excp_mask | std::ios_base::failbit | std::ios_base::badbit);
            input >> std::hex >> hex;

            if (hex > 255)
            {
                input.setstate(std::ios_base::failbit);
            }

            enP.value[0] = static_cast<octet>(hex);

            for (int i = 1; i < 4; ++i)
            {
                input >> point >> hex;
                if ( point != '.' || hex > 255 )
                {
                    input.setstate(std::ios_base::failbit);
                }
                enP.value[i] = static_cast<octet>(hex);
            }

            input >> std::dec;
        }
        catch (std::ios_base::failure& )
        {
        }

        input.exceptions(excp_mask);
    }

    return input;
}

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

const EntityId_t c_EntityId_TypeLookup_request_writer = ENTITYID_TL_SVC_REQ_WRITER;
const EntityId_t c_EntityId_TypeLookup_request_reader = ENTITYID_TL_SVC_REQ_READER;
const EntityId_t c_EntityId_TypeLookup_reply_writer = ENTITYID_TL_SVC_REPLY_WRITER;
const EntityId_t c_EntityId_TypeLookup_reply_reader = ENTITYID_TL_SVC_REPLY_READER;

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
#endif // if HAVE_SECURITY

const EntityId_t ds_server_virtual_writer = ENTITYID_DS_SERVER_VIRTUAL_WRITER;
const EntityId_t ds_server_virtual_reader = ENTITYID_DS_SERVER_VIRTUAL_READER;

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

namespace std {
template <>
struct hash<eprosima::fastrtps::rtps::EntityId_t>
{
    std::size_t operator ()(
            const eprosima::fastrtps::rtps::EntityId_t& k) const
    {
        // recover the participant entity counter
        eprosima::fastrtps::rtps::octet value[4];

#if FASTDDS_IS_BIG_ENDIAN_TARGET
        value[3] = k.value[2];
        value[2] = k.value[1];
        value[1] = k.value[0];
        value[0] = 0;
#else
        value[3] = 0;
        value[2] = k.value[0];
        value[1] = k.value[1];
        value[0] = k.value[2];
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET
        return static_cast<std::size_t>(*reinterpret_cast<const uint32_t*>(&value));
    }

};

} // namespace std


#endif /* _FASTDDS_RTPS_COMMON_ENTITYID_T_HPP_ */
