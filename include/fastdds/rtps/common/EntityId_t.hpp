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

#ifndef FASTDDS_RTPS_COMMON__ENTITYID_T_HPP
#define FASTDDS_RTPS_COMMON__ENTITYID_T_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Types.hpp>

#include <cstdint>
#include <cstring>
#include <sstream>

namespace eprosima {
namespace fastdds {
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

#ifdef FASTDDS_STATISTICS
#define ENTITYID_MONITOR_SERVICE_WRITER 0x004000D2
#endif // ifdef FASTDDS_STATISTICS

//!@brief Structure EntityId_t, entity id part of GUID_t.
//!@ingroup COMMON_MODULE
struct FASTDDS_EXPORTED_API EntityId_t
{
    static constexpr unsigned int size = 4;
    octet value[size];
    //! Default constructor. Unknown entity.
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

    /*!
     * @brief conversion to uint32_t
     * @return uint32_t representation
     */
    uint32_t to_uint32() const
    {
        uint32_t res = *reinterpret_cast<const uint32_t*>(value);

#if !FASTDDS_IS_BIG_ENDIAN_TARGET
        res = ( res >> 24 ) |
                (0x0000ff00 & ( res >> 8)) |
                (0x00ff0000 & ( res << 8)) |
                ( res << 24 );
#endif // if !FASTDDS_IS_BIG_ENDIAN_TARGET

        return res;
    }

    static EntityId_t unknown()
    {
        return EntityId_t();
    }

    bool is_reader() const
    {
        // RTPS Standard table 9.1
        return 0x4u & to_uint32();
    }

    bool is_writer() const
    {
        // RTPS Standard table 9.1
        return 0x2u & to_uint32() && !is_reader();
    }

    /**
     * Entity Id minor operator
     * @param other Second entity id to compare
     * @return True if \c other is higher than this
     */
    bool operator <(
            const EntityId_t& other) const
    {
        return std::memcmp(value, other.value, size) < 0;
    }

    /**
     * Entity Id compare static method.
     *
     * @param entity1 First entity id to compare
     * @param entity2 Second entity id to compare
     *
     * @return 0 if \c entity1 is equal to \c entity2 .
     * @return < 0 if \c entity1 is lower than \c entity2 .
     * @return > 0 if \c entity1 is higher than \c entity2 .
     */
    static int cmp(
            const EntityId_t& entity1,
            const EntityId_t& entity2)
    {
        return std::memcmp(entity1.value, entity2.value, size);
    }

};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Entity Id comparison operator
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
 * Entity Id comparison operator
 * @param id1 First EntityId to compare
 * @param id2 Second EntityId to compare
 * @return True if equal
 */
inline bool operator ==(
        const EntityId_t& id1,
        const EntityId_t& id2)
{
    return EntityId_t::cmp(id1, id2) == 0;
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
    // Use == operator as it is faster enough.
    // NOTE: this could be done comparing the entities backwards (starting in [3]) as it would probably be faster.
    return !(operator ==(id1, id2));
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

inline std::ostream& operator <<(
        std::ostream& output,
        const EntityId_t& enI)
{
    std::stringstream ss;
    ss << std::hex;
    ss << (int)enI.value[0] << "." << (int)enI.value[1] << "." << (int)enI.value[2] << "." << (int)enI.value[3];
    ss << std::dec;
    return output << ss.str();
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

const EntityId_t c_EntityId_spdp_reliable_participant_secure_reader =
        ENTITYID_SPDP_RELIABLE_BUILTIN_PARTICIPANT_SECURE_READER;
const EntityId_t c_EntityId_spdp_reliable_participant_secure_writer =
        ENTITYID_SPDP_RELIABLE_BUILTIN_PARTICIPANT_SECURE_WRITER;
#endif // if HAVE_SECURITY

const EntityId_t ds_server_virtual_writer = ENTITYID_DS_SERVER_VIRTUAL_WRITER;
const EntityId_t ds_server_virtual_reader = ENTITYID_DS_SERVER_VIRTUAL_READER;

#ifdef FASTDDS_STATISTICS
const EntityId_t monitor_service_status_writer = ENTITYID_MONITOR_SERVICE_WRITER;
#endif // if FASTDDS_STATISTICS

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

namespace std {
template <>
struct hash<eprosima::fastdds::rtps::EntityId_t>
{
    std::size_t operator ()(
            const eprosima::fastdds::rtps::EntityId_t& k) const
    {
        return (static_cast<size_t>(k.value[0]) << 16) |
               (static_cast<size_t>(k.value[1]) << 8) |
               static_cast<size_t>(k.value[2]);
    }

};

} // namespace std


#endif // FASTDDS_RTPS_COMMON__ENTITYID_T_HPP
