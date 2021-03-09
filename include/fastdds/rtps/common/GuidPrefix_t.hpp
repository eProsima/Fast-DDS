// Copyright 2016-2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file GuidPrefix_t.hpp
 */

#ifndef _FASTDDS_RTPS_COMMON_GUIDPREFIX_T_HPP_
#define _FASTDDS_RTPS_COMMON_GUIDPREFIX_T_HPP_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/common/Types.h>

#include <cstdint>
#include <cstring>
#include <sstream>
#include <iomanip>

namespace eprosima {
namespace fastrtps {
namespace rtps {

//!@brief Structure GuidPrefix_t, Guid Prefix of GUID_t.
//!@ingroup COMMON_MODULE
struct RTPS_DllAPI GuidPrefix_t
{
    static constexpr unsigned int size = 12;
    octet value[size];

    //!Default constructor. Set the Guid prefix to 0.
    GuidPrefix_t()
    {
        memset(value, 0, size);
    }

    static GuidPrefix_t unknown()
    {
        return GuidPrefix_t();
    }

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

    /**
     * Guid prefix comparison operator
     * @param prefix guid prefix to compare
     * @return True if the guid prefixes are equal
     */
    bool operator ==(
            const GuidPrefix_t& prefix) const
    {
        return (memcmp(value, prefix.value, size) == 0);
    }

    /**
     * Guid prefix comparison operator
     * @param prefix Second guid prefix to compare
     * @return True if the guid prefixes are not equal
     */
    bool operator !=(
            const GuidPrefix_t& prefix) const
    {
        return (memcmp(value, prefix.value, size) != 0);
    }

    /**
     * Guid prefix minor operator
     * @param prefix Second guid prefix to compare
     * @return True if prefix is higher
     */
    bool operator <(
            const GuidPrefix_t& prefix) const
    {
        return std::memcmp(value, prefix.value, size) < 0;
    }

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
};

const GuidPrefix_t c_GuidPrefix_Unknown;

inline std::ostream& operator <<(
        std::ostream& output,
        const GuidPrefix_t& guiP)
{
    output << std::hex;
    char old_fill = output.fill('0');
    for (uint8_t i = 0; i < 11; ++i)
    {
        output << std::setw(2) << (int)guiP.value[i] << ".";
    }
    output << std::setw(2) << (int)guiP.value[11];
    output.fill(old_fill);
    return output << std::dec;
}

inline std::istream& operator >>(
        std::istream& input,
        GuidPrefix_t& guiP)
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

            guiP.value[0] = static_cast<octet>(hex);

            for (int i = 1; i < 12; ++i)
            {
                input >> point >> hex;
                if ( point != '.' || hex > 255 )
                {
                    input.setstate(std::ios_base::failbit);
                }
                guiP.value[i] = static_cast<octet>(hex);
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

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif /* _FASTDDS_RTPS_COMMON_GUIDPREFIX_T_HPP_ */
