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
 * @file InstanceHandle.h
 */

#ifndef _FASTDDS_RTPS_INSTANCEHANDLE_H_
#define _FASTDDS_RTPS_INSTANCEHANDLE_H_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/common/Guid.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * Struct InstanceHandle_t, used to contain the key for WITH_KEY topics.
 * @ingroup COMMON_MODULE
 */
struct RTPS_DllAPI InstanceHandle_t
{
    //!Value
    octet value[16];
    InstanceHandle_t()
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            value[i] = 0;
        }
    }

    InstanceHandle_t(
            const InstanceHandle_t& ihandle)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            value[i] = ihandle.value[i];
        }
    }

    InstanceHandle_t(
            const GUID_t& guid)
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (i < 12)
            {
                value[i] = guid.guidPrefix.value[i];
            }
            else
            {
                value[i] = guid.entityId.value[i - 12];
            }
        }
    }

    /**
     * Assignment operator
     * @param ihandle Instance handle to copy the data from
     */
    InstanceHandle_t& operator =(
            const InstanceHandle_t& ihandle)
    {

        for (uint8_t i = 0; i < 16; i++)
        {
            value[i] = ihandle.value[i];
        }
        return *this;
    }

    /**
     * Assignment operator
     * @param guid GUID to copy the data from
     */
    InstanceHandle_t& operator =(
            const GUID_t& guid)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            if (i < 12)
            {
                value[i] = guid.guidPrefix.value[i];
            }
            else
            {
                value[i] = guid.entityId.value[i - 12];
            }
        }
        return *this;
    }

    /**
     * Know if the instance handle is defined
     * @return True if the values are not zero.
     */
    bool isDefined() const
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (value[i] != 0)
            {
                return true;
            }
        }
        return false;
    }

    // TODO Review this conversion once InstanceHandle_t is implemented as DDS standard defines
    explicit operator const GUID_t&() const
    {
        return *reinterpret_cast<const GUID_t*>(this);
    }

};

const InstanceHandle_t c_InstanceHandle_Unknown;

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Comparison operator
 * @param ihandle1 First InstanceHandle_t to compare
 * @param ihandle2 Second InstanceHandle_t to compare
 * @return True if equal
 */
inline bool operator ==(
        const InstanceHandle_t& ihandle1,
        const InstanceHandle_t& ihandle2)
{
    for (uint8_t i = 0; i < 16; ++i)
    {
        if (ihandle1.value[i] != ihandle2.value[i])
        {
            return false;
        }
    }
    return true;
}

inline bool operator !=(
        const InstanceHandle_t& ihandle1,
        const InstanceHandle_t& ihandle2)
{
    return !(ihandle1 == ihandle2);
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Convert InstanceHandle_t to GUID
 * @param guid GUID to store the results
 * @param ihandle InstanceHandle_t to copy
 */
inline void iHandle2GUID(
        GUID_t& guid,
        const InstanceHandle_t& ihandle)
{
    for (uint8_t i = 0; i < 16; ++i)
    {
        if (i < 12)
        {
            guid.guidPrefix.value[i] = ihandle.value[i];
        }
        else
        {
            guid.entityId.value[i - 12] = ihandle.value[i];
        }
    }
    return;
}

/**
 * Convert GUID to InstanceHandle_t
 * @param ihandle InstanceHandle_t to store the results
 * @return GUID_t
 */
inline GUID_t iHandle2GUID(
        const InstanceHandle_t& ihandle)
{
    GUID_t guid;
    for (uint8_t i = 0; i < 16; ++i)
    {
        if (i < 12)
        {
            guid.guidPrefix.value[i] = ihandle.value[i];
        }
        else
        {
            guid.entityId.value[i - 12] = ihandle.value[i];
        }
    }
    return guid;
}

inline bool operator <(
        const InstanceHandle_t& h1,
        const InstanceHandle_t& h2)
{
    return memcmp(h1.value, h2.value, 16) < 0;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 *
 * @param output
 * @param iHandle
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const InstanceHandle_t& iHandle)
{
    output << std::hex;
    for (uint8_t i = 0; i < 15; ++i)
    {
        output << (int)iHandle.value[i] << ".";
    }
    output << (int)iHandle.value[15] << std::dec;
    return output;
}

/**
 *
 * @param input
 * @param iHandle
 */
inline std::istream& operator >>(
        std::istream& input,
        InstanceHandle_t& iHandle)
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

            iHandle.value[0] = static_cast<octet>(hex);

            for (int i = 1; i < 16; ++i)
            {
                input >> point >> hex;
                if ( point != '.' || hex > 255 )
                {
                    input.setstate(std::ios_base::failbit);
                }
                iHandle.value[i] = static_cast<octet>(hex);
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

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif /* _FASTDDS_RTPS_INSTANCEHANDLE_H_ */
