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
 * @file Guid.h
 */

#ifndef _FASTDDS_RTPS_RTPS_GUID_H_
#define _FASTDDS_RTPS_RTPS_GUID_H_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>

#include <cstdint>
#include <cstring>
#include <sstream>

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct InstanceHandle_t;

//!@brief Structure GUID_t, entity identifier, unique in DDS-RTPS Domain.
//!@ingroup COMMON_MODULE
struct RTPS_DllAPI GUID_t
{
    //!Guid prefix
    GuidPrefix_t guidPrefix;
    //!Entity id
    EntityId_t entityId = ENTITYID_UNKNOWN;

    /*!
     * Default constructor. Contructs an unknown GUID.
     */
    GUID_t() noexcept
    {
    }

    /**
     * Construct
     * @param guid_prefix Guid prefix
     * @param id Entity id
     */
    GUID_t(
            const GuidPrefix_t& guid_prefix,
            EntityId_t id) noexcept
        : guidPrefix(guid_prefix)
        , entityId(id)
    {
    }

    /**
     * Checks whether this guid is for an entity on the same host as another guid.
     *
     * @param other_guid GUID_t to compare to.
     *
     * @return true when this guid is on the same host, false otherwise.
     */
    bool is_on_same_host_as(
            const GUID_t& other_guid) const
    {
        return memcmp(guidPrefix.value, other_guid.guidPrefix.value, 4) == 0;
    }

    /**
     * Checks whether this guid is for an entity on the same host and process as another guid.
     *
     * @param other_guid GUID_t to compare to.
     *
     * @return true when this guid is on the same host and process, false otherwise.
     */
    bool is_on_same_process_as(
            const GUID_t& other_guid) const
    {
        return memcmp(guidPrefix.value, other_guid.guidPrefix.value, 8) == 0;
    }

    /**
     * Checks whether this guid corresponds to a builtin entity.
     *
     * @return true when this guid corresponds to a builtin entity, false otherwise.
     */
    bool is_builtin()
    {
        return entityId & 0xC0 == 0xC0;
    }

    static GUID_t unknown() noexcept
    {
        return GUID_t();
    };

    // TODO Review this conversion once InstanceHandle_t is implemented as DDS standard defines
    explicit operator const InstanceHandle_t&() const
    {
        return *reinterpret_cast<const InstanceHandle_t*>(this);
    }
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * GUID comparison operator
 * @param g1 First GUID to compare
 * @param g2 Second GUID to compare
 * @return True if equal
 */
inline bool operator ==(
        const GUID_t& g1,
        const GUID_t& g2)
{
    return
        g1.guidPrefix == g2.guidPrefix &&
        g1.entityId == g2.entityId;
}

/**
 * GUID comparison operator
 * @param g1 First GUID to compare
 * @param g2 Second GUID to compare
 * @return True if not equal
 */
inline bool operator !=(
        const GUID_t& g1,
        const GUID_t& g2)
{
    return
        g1.guidPrefix != g2.guidPrefix ||
        g1.entityId != g2.entityId;
}

inline bool operator <(
        const GUID_t& g1,
        const GUID_t& g2)
{
    auto cmp = memcmp(g1.guidPrefix.value, g2.guidPrefix.value, GuidPrefix_t::size);
    if (cmp < 0)
    {
        return true;
    }
    else if (cmp > 0)
    {
        return false;
    }

    return g1.entityId < g2.entityId;
}
#endif

const GUID_t c_Guid_Unknown;

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Stream operator, prints a GUID.
 * @param output Output stream.
 * @param guid GUID_t to print.
 * @return Stream operator.
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const GUID_t& guid)
{
    if (guid !=c_Guid_Unknown)
    {
        output << guid.guidPrefix << "|" << std::hex << guid.entityId << std::dec;
    }
    else
    {
        output << "|GUID UNKNOWN|";
    }
    return output;
}

/**
 * Stream operator, retrieves a GUID.
 * @param input Input stream.
 * @param guid GUID_t to print.
 * @return Stream operator.
 */
inline std::istream& operator >>(
        std::istream& input,
        GUID_t& guid)
{
    std::istream::sentry s(input);

    if (s)
    {
        std::ios_base::iostate excp_mask = input.exceptions();

        try
        {
            input.exceptions(excp_mask | std::ios_base::failbit | std::ios_base::badbit);

            input >> guid.guidPrefix;
            input >> guid.entityId;
        }
        catch (std::ios_base::failure&)
        {
            // maybe is unknown or just invalid
            guid = c_Guid_Unknown;
        }

        input.exceptions(excp_mask);
    }

    return input;
}

#endif

}
}
}

#endif /* _FASTDDS_RTPS_RTPS_GUID_H_ */
