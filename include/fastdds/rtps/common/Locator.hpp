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
 * @file Locator.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__LOCATOR_HPP
#define FASTDDS_RTPS_COMMON__LOCATOR_HPP

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <vector>

#include <fastdds/config.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/utils/IPLocator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/// Initialize locator with invalid values
#define LOCATOR_INVALID(loc)  {loc.kind = LOCATOR_KIND_INVALID; loc.port = LOCATOR_PORT_INVALID; \
                               LOCATOR_ADDRESS_INVALID(loc.address); \
}
/// Invalid locator kind
#define LOCATOR_KIND_INVALID -1

/// Set locator IP address to 0
#define LOCATOR_ADDRESS_INVALID(a) {std::memset(a, 0x00, 16 * sizeof(octet));}

/// Invalid locator port
#define LOCATOR_PORT_INVALID 0

/// Reserved locator kind
#define LOCATOR_KIND_RESERVED 0
/// UDP over IPv4 locator kind
#define LOCATOR_KIND_UDPv4 1
/// UDP over IPv6 locator kind
#define LOCATOR_KIND_UDPv6 2
/// TCP over IPv4 kind
#define LOCATOR_KIND_TCPv4 4
/// TCP over IPv6 locator kind
#define LOCATOR_KIND_TCPv6 8
/// Shared memory locator kind
#define LOCATOR_KIND_SHM 16 + FASTDDS_VERSION_MAJOR

/**
 * @brief Class Locator_t, uniquely identifies a communication channel for a particular transport.
 * For example, an address + port combination in the case of UDP.
 * @ingroup COMMON_MODULE
 */
class FASTDDS_EXPORTED_API Locator_t
{
public:

    /**
     * @brief Specifies the locator type. Valid values are:
     *
     * LOCATOR_KIND_UDPv4
     *
     * LOCATOR_KIND_UDPv6
     *
     * LOCATOR_KIND_TCPv4
     *
     * LOCATOR_KIND_TCPv6
     *
     * LOCATOR_KIND_SHM
     */
    int32_t kind;
    /// Network port
    uint32_t port;
    /// IP address
    octet address[16];

    /// Default constructor
    Locator_t()
        : kind(LOCATOR_KIND_UDPv4)
    {
        port = 0;
        LOCATOR_ADDRESS_INVALID(address);
    }

    /// Move constructor
    Locator_t(
            Locator_t&& loc)
        : kind(loc.kind)
    {
        port = loc.port;
        std::memcpy(address, loc.address, 16 * sizeof(octet));
    }

    /// Copy constructor
    Locator_t(
            const Locator_t& loc)
        : kind(loc.kind)
    {
        port = loc.port;
        std::memcpy(address, loc.address, 16 * sizeof(octet));
    }

    /// Port constructor
    Locator_t(
            uint32_t portin)
        : kind(LOCATOR_KIND_UDPv4)
    {
        port = portin;
        LOCATOR_ADDRESS_INVALID(address);
    }

    /// Kind and port constructor
    Locator_t(
            int32_t kindin,
            uint32_t portin)
        : kind(kindin)
    {
        port = portin;
        LOCATOR_ADDRESS_INVALID(address);
    }

    /// Copy assignment
    Locator_t& operator =(
            const Locator_t& loc)
    {
        kind = loc.kind;
        port = loc.port;
        std::memcpy(address, loc.address, 16 * sizeof(octet));
        return *this;
    }

    /**
     * @brief Set the locator IP address using another locator.
     *
     * @param other Locator which IP address is used to set this locator IP address.
     * @return always true.
     */
    bool set_address(
            const Locator_t& other)
    {
        memcpy(address, other.address, sizeof(octet) * 16);
        return true;
    }

    /**
     * @brief Getter for the locator IP address.
     *
     * @return IP address as octet pointer.
     */
    octet* get_address()
    {
        return address;
    }

    /**
     * @brief Getter for a specific field of the locator IP address.
     *
     * @param field IP address element to be accessed.
     * @return Octet value for the specific IP address element.
     */
    octet get_address(
            uint16_t field) const
    {
        return address[field];
    }

    /**
     * @brief Automatic setter for setting locator IP address to invalid address (0).
     */
    void set_Invalid_Address()
    {
        LOCATOR_ADDRESS_INVALID(address);
    }

};

/**
 * @brief Auxiliary method to check that IP address is not invalid (0).
 *
 * @param loc Locator which IP address is going to be checked.
 * @return true if IP address is defined (not 0).
 * @return false otherwise.
 */
inline bool IsAddressDefined(
        const Locator_t& loc)
{
    if (loc.kind == LOCATOR_KIND_UDPv4 || loc.kind == LOCATOR_KIND_TCPv4) // WAN addr in TCPv4 is optional, isn't?
    {
        for (uint8_t i = 12; i < 16; ++i)
        {
            if (loc.address[i] != 0)
            {
                return true;
            }
        }
    }
    else if (loc.kind == LOCATOR_KIND_UDPv6 || loc.kind == LOCATOR_KIND_TCPv6)
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (loc.address[i] != 0)
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Auxiliary method to check that locator kind is not LOCATOR_KIND_INVALID (-1).
 *
 * @param loc Locator to be checked.
 * @return true if the locator kind is not LOCATOR_KIND_INVALID.
 * @return false otherwise.
 */
inline bool IsLocatorValid(
        const Locator_t& loc)
{
    return (0 <= loc.kind);
}

/**
 * @brief Less than operator.
 *
 * @param loc1 Left hand side locator being compared.
 * @param loc2 Right hand side locator being compared.
 * @return true if \c loc1 is less than \c loc2.
 * @return false otherwise.
 */
inline bool operator <(
        const Locator_t& loc1,
        const Locator_t& loc2)
{
    return memcmp(&loc1, &loc2, sizeof(Locator_t)) < 0;
}

/**
 * @brief Equal to operator.
 *
 * @param loc1 Left hand side locator being compared.
 * @param loc2 Right hand side locator being compared.
 * @return true if \c loc1 is equal to  \c loc2.
 * @return false otherwise.
 */
inline bool operator ==(
        const Locator_t& loc1,
        const Locator_t& loc2)
{
    if (loc1.kind != loc2.kind)
    {
        return false;
    }
    if (loc1.port != loc2.port)
    {
        return false;
    }
    if (!std::equal(loc1.address, loc1.address + 16, loc2.address))
    {
        return false;
    }
    return true;
}

/**
 * @brief Not equal to operator.
 *
 * @param loc1 Left hand side locator being compared.
 * @param loc2 Right hand side locator being compared.
 * @return true if \c loc1 is not equal to \c loc2.
 * @return false otherwise.
 */
inline bool operator !=(
        const Locator_t& loc1,
        const Locator_t& loc2)
{
    return !(loc1 == loc2);
}

/**
 * @brief Insertion operator: serialize a locator
 *        The serialization format is kind:[address]:port
 *        \c kind must be one of the following:
 *            - UDPv4
 *            - UDPv6
 *            - TCPv4
 *            - TCPv6
 *            - SHM
 *        \c address IP address unless \c kind is SHM
 *        \c port number
 *
 * @param output Output stream where the serialized locator is appended.
 * @param loc Locator to be serialized/inserted.
 * @return \c std::ostream& Reference to the output stream with the serialized locator appended.
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const Locator_t& loc)
{
    // Stream Locator kind
    switch (loc.kind)
    {
        case LOCATOR_KIND_TCPv4:
        {
            output << "TCPv4:[";
            break;
        }
        case LOCATOR_KIND_UDPv4:
        {
            output << "UDPv4:[";
            break;
        }
        case LOCATOR_KIND_TCPv6:
        {
            output << "TCPv6:[";
            break;
        }
        case LOCATOR_KIND_UDPv6:
        {
            output << "UDPv6:[";
            break;
        }
        case LOCATOR_KIND_SHM:
        {
            output << "SHM:[";
            break;
        }
        default:
        {
            output << "Invalid_locator:[_]:0";
            return output;
        }
    }

    // Stream address
    if (loc.kind == LOCATOR_KIND_UDPv4 || loc.kind == LOCATOR_KIND_TCPv4)
    {
        output << IPLocator::toIPv4string(loc);
    }
    else if (loc.kind == LOCATOR_KIND_UDPv6 || loc.kind == LOCATOR_KIND_TCPv6)
    {
        output << IPLocator::toIPv6string(loc);
    }
    else if (loc.kind == LOCATOR_KIND_SHM)
    {
        if (loc.address[0] == 'M')
        {
            output << "M";
        }
        else
        {
            output << "_";
        }
    }

    // Stream port
    if (loc.kind == LOCATOR_KIND_TCPv4 || loc.kind == LOCATOR_KIND_TCPv6)
    {
        output << "]:" << std::to_string(IPLocator::getPhysicalPort(loc)) << "-" << std::to_string(IPLocator::getLogicalPort(
                    loc));
    }
    else
    {
        output << "]:" << loc.port;
    }

    return output;
}

/**
 * @brief Extraction operator: deserialize a locator
 *        The deserialization format is kind:[address]:port
 *        \c kind must be one of the following:
 *            - UDPv4
 *            - UDPv6
 *            - TCPv4
 *            - TCPv6
 *            - SHM
 *        \c address must be either a name which can be resolved by DNS or the IP address unless \c kind is SHM
 *        \c port number
 *
 * @param input Input stream where the locator to be deserialized is located.
 * @param loc Locator where the deserialized locator is saved.
 * @return \c std::istream& Reference to the input stream after extracting the locator.
 */
inline std::istream& operator >>(
        std::istream& input,
        Locator_t& loc)
{
    std::istream::sentry s(input);

    if (s)
    {
        std::ios_base::iostate excp_mask = input.exceptions();

        try
        {
            input.exceptions(excp_mask | std::ios_base::failbit | std::ios_base::badbit);

            // Locator info
            int32_t kind;
            uint32_t port;
            std::string address;

            // Deserialization variables
            std::stringbuf sb_kind;
            std::stringbuf sb_address;
            std::string str_kind;
            char punct;

            // Check the locator kind
            input.get(sb_kind, ':');
            str_kind = sb_kind.str();

            if (str_kind == "SHM")
            {
                kind = LOCATOR_KIND_SHM;
            }
            else if (str_kind == "TCPv4")
            {
                kind = LOCATOR_KIND_TCPv4;
            }
            else if (str_kind == "TCPv6")
            {
                kind = LOCATOR_KIND_TCPv6;
            }
            else if (str_kind == "UDPv4")
            {
                kind = LOCATOR_KIND_UDPv4;
            }
            else if (str_kind == "UDPv6")
            {
                kind = LOCATOR_KIND_UDPv6;
            }
            else
            {
                kind = LOCATOR_KIND_INVALID;
                loc.kind = LOCATOR_KIND_INVALID;
            }

            if (kind != LOCATOR_KIND_INVALID)
            {
                // Get chars :[
                input >> punct >> punct;

                // Get address in string
                input.get(sb_address, ']');
                address = sb_address.str();

                // check if this is a valid IPv4 or IPv6 and call DNS if not
                if ((kind == LOCATOR_KIND_UDPv4 || kind == LOCATOR_KIND_TCPv4) &&
                        !IPLocator::isIPv4(address))
                {
                    auto addresses = IPLocator::resolveNameDNS(address);
                    if (addresses.first.empty())
                    {
                        loc.kind = LOCATOR_KIND_INVALID;
                        EPROSIMA_LOG_WARNING(LOCATOR, "Error deserializing Locator");
                        return input;
                    }
                    address = *addresses.first.begin();
                }
                if ((kind == LOCATOR_KIND_UDPv6 || kind == LOCATOR_KIND_TCPv6) &&
                        !IPLocator::isIPv6(address))
                {
                    auto addresses = IPLocator::resolveNameDNS(address);
                    if (addresses.second.empty())
                    {
                        loc.kind = LOCATOR_KIND_INVALID;
                        EPROSIMA_LOG_WARNING(LOCATOR, "Error deserializing Locator");
                        return input;
                    }
                    address = *addresses.second.begin();
                }

                // Get char ]:
                input >> punct >> punct;

                // Get port
                input >> port;

                IPLocator::createLocator(kind, address, port, loc);
            }
        }
        catch (std::ios_base::failure& )
        {
            loc.kind = LOCATOR_KIND_INVALID;
            EPROSIMA_LOG_WARNING(LOCATOR, "Error deserializing Locator");
        }

        input.exceptions(excp_mask);
    }
    return input;
}

typedef std::vector<Locator_t>::iterator LocatorListIterator;
typedef std::vector<Locator_t>::const_iterator LocatorListConstIterator;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

namespace eprosima {
namespace fastdds {
namespace rtps {

using Locator = eprosima::fastdds::rtps::Locator_t;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__LOCATOR_HPP
