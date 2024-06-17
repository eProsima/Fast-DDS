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
 * @file Guid.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__GUID_HPP
#define FASTDDS_RTPS_COMMON__GUID_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>

#include <cstdint>
#include <cstring>
#include <sstream>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct InstanceHandle_t;

//!@brief Structure GUID_t, entity identifier, unique in DDS-RTPS Domain.
//!@ingroup COMMON_MODULE
struct FASTDDS_EXPORTED_API GUID_t
{
    //!Guid prefix
    GuidPrefix_t guidPrefix;
    //!Entity id
    EntityId_t entityId;

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
            uint32_t id) noexcept
        : guidPrefix(guid_prefix)
        , entityId(id)
    {
    }

    /**
     * @param guid_prefix Guid prefix
     * @param entity_id Entity id
     */
    GUID_t(
            const GuidPrefix_t& guid_prefix,
            const EntityId_t& entity_id) noexcept
        : guidPrefix(guid_prefix)
        , entityId(entity_id)
    {
    }

    /**
     * Checks whether this guid is from an entity on the same host as another guid.
     *
     * @note This method assumes the value of \c other_guid was originally assigned by Fast-DDS vendor.
     *
     * @param other_guid GUID_t to compare to.
     *
     * @return true when this guid is on the same host, false otherwise.
     */
    bool is_on_same_host_as(
            const GUID_t& other_guid) const
    {
        return guidPrefix.is_on_same_host_as(other_guid.guidPrefix);
    }

    /**
     * Checks whether this guid is from a (Fast-DDS) entity created on this host (from where this method is called).
     *
     * @return true when this guid is from a (Fast-DDS) entity created on this host, false otherwise.
     */
    bool is_from_this_host() const
    {
        return guidPrefix.is_from_this_host();
    }

    /**
     * Checks whether this guid is for an entity on the same host and process as another guid.
     *
     * @note This method assumes the value of \c other_guid was originally assigned by Fast-DDS vendor.
     *
     * @param other_guid GUID_t to compare to.
     *
     * @return true when this guid is on the same host and process, false otherwise.
     */
    bool is_on_same_process_as(
            const GUID_t& other_guid) const
    {
        return guidPrefix.is_on_same_process_as(other_guid.guidPrefix);
    }

    /**
     * Checks whether this guid is from a (Fast-DDS) entity created on this process (from where this method is called).
     *
     * @return true when this guid is from a (Fast-DDS) entity created on this process, false otherwise.
     */
    bool is_from_this_process() const
    {
        return guidPrefix.is_from_this_process();
    }

    /**
     * Checks whether this guid corresponds to a builtin entity.
     *
     * @return true when this guid corresponds to a builtin entity, false otherwise.
     */
    bool is_builtin() const
    {
        return entityId.value[3] >= 0xC0;
    }

    static GUID_t unknown() noexcept
    {
        return GUID_t();
    }

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
    if (g1.guidPrefix == g2.guidPrefix && g1.entityId == g2.entityId)
    {
        return true;
    }
    else
    {
        return false;
    }
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
    if (g1.guidPrefix != g2.guidPrefix || g1.entityId != g2.entityId)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline bool operator <(
        const GUID_t& g1,
        const GUID_t& g2)
{
    auto prefix_cmp = GuidPrefix_t::cmp(g1.guidPrefix, g2.guidPrefix);
    if (prefix_cmp < 0)
    {
        return true;
    }
    else if (prefix_cmp > 0)
    {
        return false;
    }
    else
    {
        return g1.entityId < g2.entityId;
    }
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

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
    if (guid != c_Guid_Unknown)
    {
        output << guid.guidPrefix << "|" << guid.entityId;
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

            char sep;
            input >> guid.guidPrefix >> sep >> guid.entityId;

            if (sep != '|')
            {
                input.setstate(std::ios_base::failbit);
            }
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

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__GUID_HPP
