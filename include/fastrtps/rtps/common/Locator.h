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
 * @file Locator.h
 */

#ifndef RTPS_ELEM_LOCATOR_H_
#define RTPS_ELEM_LOCATOR_H_
#include "../../fastrtps_dll.h"

#include "Types.h"
#include <sstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <algorithm>

namespace eprosima {
namespace fastrtps {
namespace rtps {

#define LOCATOR_INVALID(loc)  {loc.kind=LOCATOR_KIND_INVALID;loc.port= LOCATOR_PORT_INVALID;LOCATOR_ADDRESS_INVALID(loc.address);}
#define LOCATOR_KIND_INVALID -1

#define LOCATOR_ADDRESS_INVALID(a) {std::memset(a,0x00,16*sizeof(octet));}
#define LOCATOR_PORT_INVALID 0
#define LOCATOR_KIND_RESERVED 0
#define LOCATOR_KIND_UDPv4 1
#define LOCATOR_KIND_UDPv6 2
#define LOCATOR_KIND_TCPv4 4
#define LOCATOR_KIND_TCPv6 8

//!@brief Class Locator_t, uniquely identifies a communication channel for a particular transport.
//For example, an address+port combination in the case of UDP.
//!@ingroup COMMON_MODULE
class RTPS_DllAPI Locator_t
{
public:

    /*!
        * @brief Specifies the locator type. Valid values are:
        * LOCATOR_KIND_UDPv4
        * LOCATOR_KIND_UDPv6
        * LOCATOR_KIND_TCPv4
        * LOCATOR_KIND_TCPv6
        */
    int32_t kind;
    uint32_t port;
    octet address[16];

    //!Default constructor
    Locator_t()
        : kind(LOCATOR_KIND_UDPv4)
    {
        port = 0;
        LOCATOR_ADDRESS_INVALID(address);
    }

    Locator_t(Locator_t&& loc)
        : kind(loc.kind)
    {
        port = loc.port;
        std::memcpy(address, loc.address, 16 * sizeof(octet));
    }

    Locator_t(const Locator_t& loc)
        : kind(loc.kind)
    {
        port = loc.port;
        std::memcpy(address, loc.address, 16 * sizeof(octet));
    }

    Locator_t(uint32_t portin)
        : kind(LOCATOR_KIND_UDPv4)
    {
        port = portin;
        LOCATOR_ADDRESS_INVALID(address);
    }

    Locator_t(int32_t kindin, uint32_t portin)
        : kind(kindin)
    {
        port = portin;
        LOCATOR_ADDRESS_INVALID(address);
    }

    Locator_t& operator=(const Locator_t& loc)
    {
        kind = loc.kind;
        port = loc.port;
        std::memcpy(address, loc.address, 16 * sizeof(octet));
        return *this;
    }

    bool set_address(const Locator_t &other)
    {
        memcpy(address, other.address, sizeof(octet) * 16);
        return true;
    }

    octet* get_address()
    {
        return address;
    }

    octet get_address(uint16_t field) const
    {
        return address[field];
    }

    void set_Invalid_Address()
    {
        LOCATOR_ADDRESS_INVALID(address);
    }
};


inline bool IsAddressDefined(const Locator_t& loc)
{
    if (loc.kind == LOCATOR_KIND_UDPv4 || loc.kind == LOCATOR_KIND_TCPv4) // WAN addr in TCPv4 is optional, isn't?
    {
        for (uint8_t i = 12; i < 16; ++i)
        {
            if (loc.address[i] != 0)
                return true;
        }
    }
    else if (loc.kind == LOCATOR_KIND_UDPv6 || loc.kind == LOCATOR_KIND_TCPv6)
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (loc.address[i] != 0)
                return true;
        }
    }
    return false;
}

inline bool IsLocatorValid(const Locator_t&loc)
{
    return (0 <= loc.kind);
}

inline bool operator<(const Locator_t &loc1, const Locator_t &loc2)
{
    return memcmp(&loc1, &loc2, sizeof(Locator_t)) < 0;
}

inline bool operator==(const Locator_t&loc1, const Locator_t& loc2)
{
    if (loc1.kind != loc2.kind)
        return false;
    if (loc1.port != loc2.port)
        return false;
    if (!std::equal(loc1.address, loc1.address + 16, loc2.address))
        return false;
    return true;
}

inline std::ostream& operator<<(std::ostream& output, const Locator_t& loc)
{
    if (loc.kind == LOCATOR_KIND_UDPv4 || loc.kind == LOCATOR_KIND_TCPv4)
    {
        output << (int)loc.address[12] << "." << (int)loc.address[13]
            << "." << (int)loc.address[14] << "." << (int)loc.address[15]
            << ":" << loc.port;
    }
    else if (loc.kind == LOCATOR_KIND_UDPv6 || loc.kind == LOCATOR_KIND_TCPv6)
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            output << (int)loc.address[i];
            if (i < 15)
                output << ".";
        }
        output << ":" << loc.port;
    }
    return output;
}



typedef std::vector<Locator_t>::iterator LocatorListIterator;
typedef std::vector<Locator_t>::const_iterator LocatorListConstIterator;


/**
    * Class LocatorList_t, a Locator_t vector that doesn't avoid duplicates.
    * @ingroup COMMON_MODULE
    */
class LocatorList_t
{
public:
    RTPS_DllAPI LocatorList_t() {};

    RTPS_DllAPI ~LocatorList_t() {};

    RTPS_DllAPI LocatorList_t(const LocatorList_t& list) : m_locators(list.m_locators) {}

    RTPS_DllAPI LocatorList_t(LocatorList_t&& list) : m_locators(std::move(list.m_locators)) {}

    RTPS_DllAPI LocatorList_t& operator=(const LocatorList_t& list)
    {
        m_locators = list.m_locators;
        return *this;
    }

    RTPS_DllAPI LocatorList_t& operator=(LocatorList_t&& list)
    {
        m_locators = std::move(list.m_locators);
        return *this;
    }

    RTPS_DllAPI bool operator==(const LocatorList_t& locator_list) const
    {
        if (locator_list.m_locators.size() == m_locators.size())
        {
            bool returnedValue = true;

            for (auto it = locator_list.m_locators.begin(); returnedValue &&
                it != locator_list.m_locators.end(); ++it)
            {
                returnedValue = false;

                for (auto it2 = m_locators.begin(); !returnedValue && it2 != m_locators.end(); ++it2)
                {
                    if (*it == *it2)
                        returnedValue = true;
                }
            }

            return returnedValue;
        }

        return false;
    }

    RTPS_DllAPI LocatorListIterator begin() {
        return m_locators.begin();
    }

    RTPS_DllAPI LocatorListIterator end() {
        return m_locators.end();
    }

    RTPS_DllAPI LocatorListConstIterator begin() const {
        return m_locators.begin();
    }

    RTPS_DllAPI LocatorListConstIterator end() const {
        return m_locators.end();
    }

    RTPS_DllAPI size_t size() const {
        return m_locators.size();
    }

    RTPS_DllAPI LocatorList_t& assign(const LocatorList_t& list)
    {
        if (!(*this == list))
        {
            m_locators = list.m_locators;
        }
        return *this;
    }

    RTPS_DllAPI void clear(){ return m_locators.clear(); }

    RTPS_DllAPI void reserve(size_t num) { return m_locators.reserve(num); }

    RTPS_DllAPI void resize(size_t num) { return m_locators.resize(num); }

    RTPS_DllAPI void push_back(const Locator_t& loc)
    {
        bool already = false;
        for (LocatorListIterator it = this->begin(); it != this->end(); ++it)
        {
            if (loc == *it)
            {
                already = true;
                break;
            }
        }
        if (!already)
            m_locators.push_back(loc);
    }

    RTPS_DllAPI void push_back(const LocatorList_t& locList)
    {
        for (auto it = locList.m_locators.begin(); it != locList.m_locators.end(); ++it)
        {
            this->push_back(*it);
        }
    }

    RTPS_DllAPI bool empty() const {
        return m_locators.empty();
    }

    RTPS_DllAPI void erase(const Locator_t& loc)
    {
        auto it = std::find(m_locators.begin(), m_locators.end(), loc);
        if (it != m_locators.end())
        {
            m_locators.erase(it);
        }
    }

    RTPS_DllAPI bool contains(const Locator_t& loc)
    {
        for (LocatorListIterator it = this->begin(); it != this->end(); ++it)
        {
            if (IsAddressDefined(*it))
            {
                if (loc == *it)
                    return true;
            }
            else
            {
                if (loc.kind == (*it).kind && loc.port == (*it).port)
                    return true;
            }
        }

        return false;
    }

    RTPS_DllAPI bool isValid() const
    {
        for (LocatorListConstIterator it = this->begin(); it != this->end(); ++it)
        {
            if (!IsLocatorValid(*it))
                return false;
        }
        return true;
    }


    RTPS_DllAPI void swap(LocatorList_t& locatorList)
    {
        this->m_locators.swap(locatorList.m_locators);
    }

    friend std::ostream& operator <<(std::ostream& output, const LocatorList_t& loc);

private:

    std::vector<Locator_t> m_locators;
};

inline std::ostream& operator<<(std::ostream& output, const LocatorList_t& locList)
{
    for (auto it = locList.m_locators.begin(); it != locList.m_locators.end(); ++it)
    {
        output << *it << ",";
    }
    return output;
}

}
}
}

#endif /* RTPS_ELEM_LOCATOR_H_ */
