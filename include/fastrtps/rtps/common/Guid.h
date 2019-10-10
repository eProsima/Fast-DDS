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

#ifndef RTPS_GUID_H_
#define RTPS_GUID_H_

#include "../../fastrtps_dll.h"
#include "Types.h"
#include <fastrtps/rtps/common/GuidPrefix_t.hpp>

#include <cstdint>
#include <cstring>
#include <sstream>

namespace eprosima{
namespace fastrtps{
namespace rtps{

//!@brief Structure GUID_t, entity identifier, unique in DDS-RTPS Domain.
//!@ingroup COMMON_MODULE
struct RTPS_DllAPI GUID_t{
    //!Guid prefix
    GuidPrefix_t guidPrefix;
    //!Entity id
    EntityId_t entityId;


    /*!
     * DDefault constructor. Contructs an unknown GUID.
     */
    GUID_t(){};

    /*!
     * Copy constructor.
     */
    GUID_t(const GUID_t& g)
        : guidPrefix(g.guidPrefix)
        , entityId(g.entityId)
    {
    }

    /*!
     * Move constructor.
     */
    GUID_t(GUID_t&& g)
        : guidPrefix(std::move(g.guidPrefix))
        , entityId(std::move(g.entityId))
    {
    }

    /**
     * Assignment operator
     * @param guid GUID to copy the data from.
     */
    GUID_t& operator=(const GUID_t &guid)
    {
        guidPrefix = guid.guidPrefix;
        entityId = guid.entityId;
        return *this;
    }

    /**
     * Assignment operator
     * @param guid GUID to copy the data from.
     */
    GUID_t& operator=(GUID_t &&guid)
    {
        guidPrefix = std::move(guid.guidPrefix);
        entityId = std::move(guid.entityId);
        return *this;
    }

    /**
     * @param guidP Guid prefix
     * @param id Entity id
     */
    GUID_t(
            const GuidPrefix_t& guidP,
            uint32_t id)
        : guidPrefix(guidP)
        , entityId(id)
    {
    }

    /**
     * @param guidP Guid prefix
     * @param entId Entity id
     */
    GUID_t(
            const GuidPrefix_t& guidP,
            const EntityId_t& entId)
        : guidPrefix(guidP)
        , entityId(entId)
    {
    }

    static GUID_t unknown()
    {
        return GUID_t();
    };
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * GUID comparison operator
 * @param g1 First GUID to compare
 * @param g2 Second GUID to compare
 * @return True if equal
 */
inline bool operator==(
        const GUID_t& g1,
        const GUID_t& g2)
{
    if(g1.guidPrefix == g2.guidPrefix && g1.entityId==g2.entityId)
        return true;
    else
        return false;
}

/**
 * GUID comparison operator
 * @param g1 First GUID to compare
 * @param g2 Second GUID to compare
 * @return True if not equal
 */
inline bool operator!=(
        const GUID_t& g1,
        const GUID_t& g2)
{
    if(g1.guidPrefix != g2.guidPrefix || g1.entityId!=g2.entityId)
        return true;
    else
        return false;
}

inline bool operator<(
        const GUID_t& g1,
        const GUID_t& g2)
{
    for (uint8_t i = 0; i < 12; ++i)
    {
        if(g1.guidPrefix.value[i] < g2.guidPrefix.value[i])
            return true;
        else if(g1.guidPrefix.value[i] > g2.guidPrefix.value[i])
            return false;
    }
    for (uint8_t i = 0; i < 4; ++i)
    {
        if(g1.entityId.value[i] < g2.entityId.value[i])
            return true;
        else if(g1.entityId.value[i] > g2.entityId.value[i])
            return false;
    }
    return false;
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
inline std::ostream& operator<<(
        std::ostream& output,
        const GUID_t& guid)
{
    if(guid !=c_Guid_Unknown)
        output<<guid.guidPrefix<<"|"<<guid.entityId;
    else
        output << "|GUID UNKNOWN|";
    return output;
}

/**
 * Stream operator, retrieves a GUID.
 * @param input Input stream.
 * @param guid GUID_t to print.
 * @return Stream operator.
 */
inline std::istream& operator>>(
        std::istream& input,
        GUID_t& guid)
{
    std::istream::sentry s(input);

    if(s)
    {
        std::ios_base::iostate excp_mask = input.exceptions();

        try
        {
            input.exceptions(excp_mask | std::ios_base::failbit | std::ios_base::badbit);

            input >> guid.guidPrefix;
            input >> guid.entityId;
        }
        catch(std::ios_base::failure &)
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

#endif /* RTPS_GUID_H_ */
