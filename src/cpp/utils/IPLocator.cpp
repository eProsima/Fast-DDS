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
 * @file IPLocator.cpp
 *
 */

#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/utils/IPFinder.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

IPLocator::IPLocator()
{
}

IPLocator::~IPLocator()
{
}

// Factory
void IPLocator::createLocator(
        int32_t kindin,
        const std::string& address,
        uint32_t portin,
        Locator_t& locator)
{
    locator.kind = kindin;
    locator.port = portin;
    LOCATOR_ADDRESS_INVALID(locator.address);

    switch (kindin)
    {
        case LOCATOR_KIND_TCPv4:
        case LOCATOR_KIND_UDPv4:
        {
            setIPv4(locator, address);
            break;
        }
        case LOCATOR_KIND_TCPv6:
        case LOCATOR_KIND_UDPv6:
        {
            setIPv6(locator, address);
            break;
        }
    }
}

// IPv4
bool IPLocator::setIPv4(
        Locator_t& locator,
        const unsigned char* addr)
{
    memcpy(&locator.address[12], addr, 4 * sizeof(char));
    return true;
}

bool IPLocator::setIPv4(
        Locator_t& locator,
        octet o1,
        octet o2,
        octet o3,
        octet o4)
{
    LOCATOR_ADDRESS_INVALID(locator.address);
    locator.address[12] = o1;
    locator.address[13] = o2;
    locator.address[14] = o3;
    locator.address[15] = o4;
    return true;
}

bool IPLocator::setIPv4(
        Locator_t& locator,
        const std::string& ipv4)
{
    //std::string _ipv4 = IPFinder::getIPv4Address(ipv4);
    std::stringstream ss(ipv4);
    int a, b, c, d; //to store the 4 ints
    char ch; //to temporarily store the '.'

    if (ss >> a >> ch >> b >> ch >> c >> ch >> d)
    {
        LOCATOR_ADDRESS_INVALID(locator.address);
        locator.address[12] = (octet)a;
        locator.address[13] = (octet)b;
        locator.address[14] = (octet)c;
        locator.address[15] = (octet)d;

        return true;
    }

    return false;
}

bool IPLocator::setIPv4(
        Locator_t& destlocator,
        const Locator_t& origlocator)
{
    return setIPv4(destlocator, getIPv4(origlocator));
}

const octet* IPLocator::getIPv4(
        const Locator_t& locator)
{
    return static_cast<const octet*>(&locator.address[12]);
}

bool IPLocator::hasIPv4(
        const Locator_t& locator)
{
    return locator.address[12] != 0 &&
           locator.address[13] != 0 &&
           locator.address[14] != 0 &&
           locator.address[15] != 0;
}

std::string IPLocator::toIPv4string(
        const Locator_t& locator)
{
    std::stringstream ss;
    ss << (int)locator.address[12] << "."
       << (int)locator.address[13] << "."
       << (int)locator.address[14] << "."
       << (int)locator.address[15];
    return ss.str();
}

bool IPLocator::copyIPv4(
        const Locator_t& locator,
        unsigned char* dest)
{
    memcpy(dest, &(locator.address[12]), 4 * sizeof(char));
    return true;
}

// IPv6
bool IPLocator::setIPv6(
        Locator_t& locator,
        const unsigned char* addr)
{
    memcpy(locator.address, addr, 16 * sizeof(char));
    return true;
}

bool IPLocator::setIPv6(
        Locator_t& locator,
        uint16_t group0,
        uint16_t group1,
        uint16_t group2,
        uint16_t group3,
        uint16_t group4,
        uint16_t group5,
        uint16_t group6,
        uint16_t group7)
{
    locator.address[0] = (octet)(group0 >> 8);
    locator.address[1] = (octet)group0;
    locator.address[2] = (octet)(group1 >> 8);
    locator.address[3] = (octet)group1;
    locator.address[4] = (octet)(group2 >> 8);
    locator.address[5] = (octet)group2;
    locator.address[6] = (octet)(group3 >> 8);
    locator.address[7] = (octet)group3;
    locator.address[8] = (octet)(group4 >> 8);
    locator.address[9] = (octet)group4;
    locator.address[10] = (octet)(group5 >> 8);
    locator.address[11] = (octet)group5;
    locator.address[12] = (octet)(group6 >> 8);
    locator.address[13] = (octet)group6;
    locator.address[14] = (octet)(group7 >> 8);
    locator.address[15] = (octet)group7;
    return true;
}

bool IPLocator::setIPv6(
        Locator_t& locator,
        const std::string& ipv6)
{
    //std::string _ipv6 = IPFinder::getIPv6Address(ipv6);
    std::vector<std::string> hexdigits;

    size_t start = 0, end = 0;
    std::string auxstr;

    while (end != std::string::npos)
    {
        end = ipv6.find(':', start);
        if (end - start > 1)
        {
            hexdigits.push_back(ipv6.substr(start, end - start));
        }
        else
        {
            hexdigits.push_back(std::string("EMPTY"));
        }
        start = end + 1;
    }

    //FOUND a . in the last element (MAP TO IP4 address)
    if ((hexdigits.end() - 1)->find('.') != std::string::npos)
    {
        return false;
    }

    *(hexdigits.end() - 1) = (hexdigits.end() - 1)->substr(0, (hexdigits.end() - 1)->find('%'));

    int auxnumber = 0;
    uint8_t index = 15;
    for (auto it = hexdigits.rbegin(); it != hexdigits.rend(); ++it)
    {
        if (*it != std::string("EMPTY"))
        {
            if (it->length() <= 2)
            {
                locator.address[index - 1] = 0;
                std::stringstream ss;
                ss << std::hex << (*it);
                ss >> auxnumber;
                locator.address[index] = (octet)auxnumber;
            }
            else
            {
                std::stringstream ss;
                ss << std::hex << it->substr(it->length() - 2);
                ss >> auxnumber;
                locator.address[index] = (octet)auxnumber;
                ss.str("");
                ss.clear();
                ss << std::hex << it->substr(0, it->length() - 2);
                ss >> auxnumber;
                locator.address[index - 1] = (octet)auxnumber;
            }
            index -= 2;
        }
        else
        {
            break;
        }
    }
    index = 0;
    for (auto it = hexdigits.begin(); it != hexdigits.end(); ++it)
    {
        if (*it != std::string("EMPTY"))
        {
            if (it->length() <= 2)
            {
                locator.address[index] = 0;
                std::stringstream ss;
                ss << std::hex << (*it);
                ss >> auxnumber;
                locator.address[index + 1] = (octet)auxnumber;
            }
            else
            {
                std::stringstream ss;
                ss << std::hex << it->substr(it->length() - 2);
                ss >> auxnumber;
                locator.address[index + 1] = (octet)auxnumber;
                ss.str("");
                ss.clear();
                ss << std::hex << it->substr(0, it->length() - 2);
                ss >> auxnumber;
                locator.address[index] = (octet)auxnumber;
            }
            index += 2;
        }
        else
        {
            break;
        }
    }

    return true;
}

bool IPLocator::setIPv6(
        Locator_t& destlocator,
        const Locator_t& origlocator)
{
    return setIPv6(destlocator, getIPv6(origlocator));
}

const octet* IPLocator::getIPv6(
        const Locator_t& locator)
{
    return locator.address;
}

bool IPLocator::hasIPv6(
        const Locator_t& locator)
{
    return locator.address[0] != 0 &&
           locator.address[1] != 0 &&
           locator.address[2] != 0 &&
           locator.address[3] != 0 &&
           locator.address[4] != 0 &&
           locator.address[5] != 0 &&
           locator.address[6] != 0 &&
           locator.address[7] != 0 &&
           locator.address[8] != 0 &&
           locator.address[9] != 0 &&
           locator.address[10] != 0 &&
           locator.address[11] != 0 &&
           locator.address[12] != 0 &&
           locator.address[13] != 0 &&
           locator.address[14] != 0 &&
           locator.address[15] != 0;
}

std::string IPLocator::toIPv6string(
        const Locator_t& locator)
{
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i != 14; i += 2)
    {
        auto field = (locator.address[i] << 8) + locator.address[i + 1];
        ss << field << ":";
    }
    auto field = locator.address[14] + (locator.address[15] << 8);
    ss << field;
    return ss.str();
}

bool IPLocator::copyIPv6(
        const Locator_t& locator,
        unsigned char* dest)
{
    memcpy(dest, locator.address, 16 * sizeof(char));
    return true;
}

// Abstract from IPv4 and IPv6
bool IPLocator::ip(
        Locator_t& locator,
        const std::string& ip)
{
    if (locator.kind == LOCATOR_KIND_TCPv4 ||
            locator.kind == LOCATOR_KIND_UDPv4)
    {
        return setIPv4(locator, ip);
    }
    else if (locator.kind == LOCATOR_KIND_TCPv6 ||
            locator.kind == LOCATOR_KIND_UDPv6)
    {
        return setIPv6(locator, ip);
    }
    return false;
}

std::string IPLocator::ip_to_string(
        const Locator_t& locator)
{
    if (locator.kind == LOCATOR_KIND_TCPv4 ||
            locator.kind == LOCATOR_KIND_UDPv4)
    {
        return toIPv4string(locator);
    }
    else if (locator.kind == LOCATOR_KIND_TCPv6 ||
            locator.kind == LOCATOR_KIND_UDPv6)
    {
        return toIPv6string(locator);
    }
    return "";
}

// TCP
bool IPLocator::setLogicalPort(
        Locator_t& locator,
        uint16_t port)
{
    uint16_t* loc_logical = reinterpret_cast<uint16_t*>(&locator.port);
#if __BIG_ENDIAN__
    loc_logical[0] = port; // Logical port is stored at 2nd and 3rd bytes of port
#else
    loc_logical[1] = port; // Logical port is stored at 2nd and 3rd bytes of port
#endif // if __BIG_ENDIAN__
    return port != 0;
}

uint16_t IPLocator::getLogicalPort(
        const Locator_t& locator)
{
    const uint16_t* loc_logical = reinterpret_cast<const uint16_t*>(&locator.port);
#if __BIG_ENDIAN__
    return loc_logical[0];
#else
    return loc_logical[1];
#endif // if __BIG_ENDIAN__
}

bool IPLocator::setPhysicalPort(
        Locator_t& locator,
        uint16_t port)
{
    uint16_t* loc_physical = reinterpret_cast<uint16_t*>(&locator.port);
#if __BIG_ENDIAN__
    loc_physical[1] = port; // Physical port is stored at 0 and 1st bytes of port
#else
    loc_physical[0] = port; // Physical port is stored at 0 and 1st bytes of port
#endif // if __BIG_ENDIAN__
    return port != 0;
}

uint16_t IPLocator::getPhysicalPort(
        const Locator_t& locator)
{
    const uint16_t* loc_physical = reinterpret_cast<const uint16_t*>(&locator.port);
#if __BIG_ENDIAN__
    return loc_physical[1];
#else
    return loc_physical[0];
#endif // if __BIG_ENDIAN__
}

// TCPv4
bool IPLocator::setWan(
        Locator_t& locator,
        octet o1,
        octet o2,
        octet o3,
        octet o4)
{
    locator.address[8] = o1;
    locator.address[9] = o2;
    locator.address[10] = o3;
    locator.address[11] = o4;
    return true;
}

bool IPLocator::setWan(
        Locator_t& locator,
        const std::string& wan)
{
    std::stringstream ss(wan);
    int a, b, c, d; //to store the 4 ints
    char ch; //to temporarily store the '.'

    if ( ss >> a >> ch >> b >> ch >> c >> ch >> d)
    {
        locator.address[8]  = (octet)a;
        locator.address[9]  = (octet)b;
        locator.address[10] = (octet)c;
        locator.address[11] = (octet)d;
    }
    return true;
}

const octet* IPLocator::getWan(
        const Locator_t& locator)
{
    return static_cast<const octet*>(&locator.address[8]);
}

bool IPLocator::hasWan(
        const Locator_t& locator)
{
    return locator.kind == LOCATOR_KIND_TCPv4 && // TCPv6 doesn't use WAN
           (locator.address[8] != 0 ||
           locator.address[9] != 0 ||
           locator.address[10] != 0 ||
           locator.address[11] != 0);
}

std::string IPLocator::toWanstring(
        const Locator_t& locator)
{
    std::stringstream ss;
    ss << (int)locator.address[8] << "."
       << (int)locator.address[9] << "."
       << (int)locator.address[10] << "."
       << (int)locator.address[11];
    return ss.str();
}

bool IPLocator::setLanID(
        Locator_t& locator,
        const std::string& lanId)
{
    if (locator.kind == LOCATOR_KIND_TCPv4)
    {
        std::stringstream ss(lanId);
        int a, b, c, d, e, f, g, h; //to store the 8 ints
        char ch; //to temporarily store the '.'

        if ( ss >> a >> ch >> b >> ch >> c >> ch >> d >> ch >> e >> ch >> f >> ch >> g >> ch >> h)
        {
            locator.address[0] = (octet)a;
            locator.address[1] = (octet)b;
            locator.address[2] = (octet)c;
            locator.address[3] = (octet)d;
            locator.address[4] = (octet)e;
            locator.address[5] = (octet)f;
            locator.address[6] = (octet)g;
            locator.address[7] = (octet)h;

            return true;
        }
    }

    return false;
}

const octet* IPLocator::getLanID(
        const Locator_t& locator)
{
    return static_cast<const octet*>(&locator.address[0]);
}

std::string IPLocator::toLanIDstring(
        const Locator_t& locator)
{
    if (locator.kind != LOCATOR_KIND_TCPv4)
    {
        return "";
    }

    std::stringstream ss;
    ss << (int)locator.address[0] << "."
       << (int)locator.address[1] << "."
       << (int)locator.address[2] << "."
       << (int)locator.address[3] << "."
       << (int)locator.address[4] << "."
       << (int)locator.address[5] << "."
       << (int)locator.address[6] << "."
       << (int)locator.address[7];

    return ss.str();
}

Locator_t IPLocator::toPhysicalLocator(
        const Locator_t& locator)
{
    Locator_t result = locator;
    setLogicalPort(result, 0);
    return result;
}

bool IPLocator::ip_equals_wan(
        const Locator_t& locator)
{
    return hasWan(locator) &&
           locator.address[8]  == locator.address[12] &&
           locator.address[9]  == locator.address[13] &&
           locator.address[10] == locator.address[14] &&
           locator.address[11] == locator.address[15];
}

// Common
bool IPLocator::setPortRTPS(
        Locator_t& locator,
        uint16_t port)
{
    if (locator.kind == LOCATOR_KIND_UDPv4 || locator.kind == LOCATOR_KIND_UDPv6)
    {
        return setPhysicalPort(locator, port);
    }
    else if (locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_TCPv6)
    {
        return setLogicalPort(locator, port);
    }
    return false;
}

uint16_t IPLocator::getPortRTPS(
        Locator_t& locator)
{
    if (locator.kind == LOCATOR_KIND_UDPv4 || locator.kind == LOCATOR_KIND_UDPv6)
    {
        return getPhysicalPort(locator);
    }
    else if (locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_TCPv6)
    {
        return getLogicalPort(locator);
    }
    return false;
}

bool IPLocator::isLocal(
        const Locator_t& locator)
{
    if (locator.kind == LOCATOR_KIND_UDPv4
            || locator.kind == LOCATOR_KIND_TCPv4)
    {
        return locator.address[12] == 127
               && locator.address[13] == 0
               && locator.address[14] == 0
               && locator.address[15] == 1;
    }
    else
    {
        return locator.address[0] == 0
               && locator.address[1] == 0
               && locator.address[2] == 0
               && locator.address[3] == 0
               && locator.address[4] == 0
               && locator.address[5] == 0
               && locator.address[6] == 0
               && locator.address[7] == 0
               && locator.address[8] == 0
               && locator.address[9] == 0
               && locator.address[10] == 0
               && locator.address[11] == 0
               && locator.address[12] == 0
               && locator.address[13] == 0
               && locator.address[14] == 0
               && locator.address[15] == 1;
    }
}

bool IPLocator::isAny(
        const Locator_t& locator)
{
    if (locator.kind == LOCATOR_KIND_UDPv4
            || locator.kind == LOCATOR_KIND_TCPv4)
    {
        return locator.address[12] == 0 &&
               locator.address[13] == 0 &&
               locator.address[14] == 0 &&
               locator.address[15] == 0;
    }
    else
    {
        return locator.address[0] == 0 &&
               locator.address[1] == 0 &&
               locator.address[2] == 0 &&
               locator.address[3] == 0 &&
               locator.address[4] == 0 &&
               locator.address[5] == 0 &&
               locator.address[6] == 0 &&
               locator.address[7] == 0 &&
               locator.address[8] == 0 &&
               locator.address[9] == 0 &&
               locator.address[10] == 0 &&
               locator.address[11] == 0 &&
               locator.address[12] == 0 &&
               locator.address[13] == 0 &&
               locator.address[14] == 0 &&
               locator.address[15] == 0;
    }
}

bool IPLocator::compareAddress(
        const Locator_t& loc1,
        const Locator_t& loc2,
        bool fullAddress)
{
    if (loc1.kind != loc2.kind)
    {
        return false;
    }

    if (!fullAddress && (loc1.kind == LOCATOR_KIND_UDPv4 || loc1.kind == LOCATOR_KIND_TCPv4) )
    {
        return memcmp(&loc1.address[12], &loc2.address[12], 4) == 0;
    }
    else
    {
        return memcmp(loc1.address, loc2.address, 16) == 0;
    }
}

bool IPLocator::compareAddressAndPhysicalPort(
        const Locator_t& loc1,
        const Locator_t& loc2)
{
    return compareAddress(loc1, loc2, true) && getPhysicalPort(loc1) == getPhysicalPort(loc2);
}

std::string IPLocator::to_string(
        const Locator_t& loc)
{
    std::stringstream ss;
    if (loc.kind == LOCATOR_KIND_UDPv4 || loc.kind == LOCATOR_KIND_TCPv4)
    {
        ss << (int)loc.address[8] << "."
           << (int)loc.address[9] << "."
           << (int)loc.address[10] << "."
           << (int)loc.address[11] << "@";
        ss << (int)loc.address[12] << "." << (int)loc.address[13]
           << "." << (int)loc.address[14] << "." << (int)loc.address[15]
           << ":" << loc.port;
    }
    else if (loc.kind == LOCATOR_KIND_UDPv6 || loc.kind == LOCATOR_KIND_TCPv6)
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            ss << (int)loc.address[i];
            if (i < 15)
            {
                ss << ".";
            }
        }
        ss << ":" << loc.port;
    }
    return ss.str();
}

// UDP
bool IPLocator::isMulticast(
        const Locator_t& locator)
{
    if (locator.kind == LOCATOR_KIND_TCPv4
            || locator.kind == LOCATOR_KIND_TCPv6)
    {
        return false;
    }

    if (locator.kind == LOCATOR_KIND_UDPv4)
    {
        return locator.address[12] >= 224 &&
               locator.address[12] <= 239;
    }
    else
    {
        return locator.address[0] == 0xFF;
    }
}

}
}
} /* namespace eprosima */
