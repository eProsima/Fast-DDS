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

#ifndef FASTDDS_RTPS_COMMON__GUIDPREFIX_T_HPP
#define FASTDDS_RTPS_COMMON__GUIDPREFIX_T_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Types.hpp>

#include <cstdint>
#include <cstring>
#include <sstream>
#include <iomanip>

namespace eprosima {
namespace fastdds {
namespace rtps {

//!@brief Structure GuidPrefix_t, Guid Prefix of GUID_t.
//!@ingroup COMMON_MODULE
struct FASTDDS_EXPORTED_API GuidPrefix_t
{
    static constexpr unsigned int size = 12;
    octet value[size];

    //!Default constructor. Set the Guid prefix to 0.
    GuidPrefix_t()
    {
        memset(value, 0, size);
    }

    /**
     * Checks whether this guid prefix is from an entity on the same host as another guid prefix.
     *
     * @note This method assumes the value of \c other_guid_prefix was originally assigned by Fast-DDS vendor.
     *
     * @param other_guid_prefix GuidPrefix_t to compare to.
     *
     * @return true when this guid prefix is on the same host, false otherwise.
     */
    bool is_on_same_host_as(
            const GuidPrefix_t& other_guid_prefix) const;

    /**
     * Checks whether this guid prefix is from a (Fast-DDS) entity created on this host (from where this method is called).
     *
     * @return true when this guid prefix is from a (Fast-DDS) entity created on this host, false otherwise.
     */
    bool is_from_this_host() const;

    /**
     * Checks whether this guid prefix is for an entity on the same host and process as another guid prefix.
     *
     * @note This method assumes the value of \c other_guid_prefix was originally assigned by Fast-DDS vendor.
     *
     * @param other_guid_prefix GuidPrefix_t to compare to.
     *
     * @return true when this guid prefix is on the same host and process, false otherwise.
     */
    bool is_on_same_process_as(
            const GuidPrefix_t& other_guid_prefix) const;

    /**
     * Checks whether this guid prefix is from a (Fast-DDS) entity created on this host and process (from where this method is called).
     *
     * @return true when this guid prefix is from a (Fast-DDS) entity created on this host and process, false otherwise.
     */
    bool is_from_this_process() const;

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
     * @return True if prefix is higher than this
     */
    bool operator <(
            const GuidPrefix_t& prefix) const
    {
        return std::memcmp(value, prefix.value, size) < 0;
    }

    /**
     * Guid Prefix compare static method.
     *
     * @param prefix1 First guid prefix to compare
     * @param prefix2 Second guid prefix to compare
     *
     * @return 0 if \c prefix1 is equal to \c prefix2 .
     * @return < 0 if \c prefix1 is lower than \c prefix2 .
     * @return > 0 if \c prefix1 is higher than \c prefix2 .
     */
    static int cmp(
            const GuidPrefix_t& prefix1,
            const GuidPrefix_t& prefix2)
    {
        return std::memcmp(prefix1.value, prefix2.value, size);
    }

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
};

const GuidPrefix_t c_GuidPrefix_Unknown;

inline std::ostream& operator <<(
        std::ostream& output,
        const GuidPrefix_t& guiP)
{
    std::stringstream ss;
    ss << std::hex;
    char old_fill = ss.fill('0');
    for (uint8_t i = 0; i < 11; ++i)
    {
        ss << std::setw(2) << (int)guiP.value[i] << ".";
    }
    ss << std::setw(2) << (int)guiP.value[11];
    ss.fill(old_fill);
    ss << std::dec;
    return output << ss.str();
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
            guiP = GuidPrefix_t::unknown();
        }

        input.exceptions(excp_mask);
    }

    return input;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__GUIDPREFIX_T_HPP
