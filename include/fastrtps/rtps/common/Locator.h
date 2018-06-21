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
namespace eprosima{
namespace fastrtps{
namespace rtps{

#define LOCATOR_INVALID(loc)  {loc.kind=LOCATOR_KIND_INVALID;loc.up_.port= LOCATOR_PORT_INVALID;LOCATOR_ADDRESS_INVALID(loc.ua_.address);}
#define LOCATOR_KIND_INVALID -1

#define LOCATOR_ADDRESS_INVALID(a) {std::memset(a,0x00,16*sizeof(octet));}
#define LOCATOR_PORT_INVALID 0
#define LOCATOR_KIND_RESERVED 0
#define LOCATOR_KIND_UDPv4 1
#define LOCATOR_KIND_UDPv6 2
#define LOCATOR_KIND_TCPv4 4
#define LOCATOR_KIND_TCPv6 8

// TODO Make this class a base class for each kind of locator?

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
    protected:
        union
        {
            uint32_t port;
            struct
            {
                uint16_t physical_port;
                uint16_t logical_port;
            } ports_;
        }up_;
        union
        {
            octet address[16];
            struct
            {
                octet unique_lan_id[8];
                octet wan_address[4];
                octet ip_address[4];
            } addresses_;
        }ua_;

    public:
        //!Default constructor
        Locator_t()
            : kind(1)
        {
            up_.port = 0;
            LOCATOR_ADDRESS_INVALID(ua_.address);
        }

        Locator_t(Locator_t&& loc)
            : kind(loc.kind)
        {
            up_.port = loc.up_.port;
            std::memcpy(ua_.address,loc.ua_.address,16*sizeof(octet));
        }

        Locator_t(const Locator_t& loc)
            : kind(loc.kind)
        {
            up_.port = loc.up_.port;
            std::memcpy(ua_.address,loc.ua_.address,16*sizeof(octet));
        }

        Locator_t(uint32_t portin)
            : kind(1)
        {
            up_.port = portin;
            LOCATOR_ADDRESS_INVALID(ua_.address);
        }

        Locator_t(int32_t kindin, uint32_t portin)
            : kind(kindin)
        {
            up_.port = portin;
            LOCATOR_ADDRESS_INVALID(ua_.address);
        }

        Locator_t(int32_t kindin, const std::string& address, uint32_t portin)
            : kind(kindin)
        {
            up_.port = portin;
            set_IP4_address(address);
        }

        Locator_t& operator=(const Locator_t& loc)
        {
            kind = loc.kind;
            up_.port =loc.up_.port;
            std::memcpy(ua_.address,loc.ua_.address,16*sizeof(octet));
            return *this;
        }

        bool set_port(uint16_t iPort, bool well_known = false)
        {
            if (kind == LOCATOR_KIND_TCPv4 || kind == LOCATOR_KIND_TCPv6)
            {
                if (well_known)
                {
                    this->up_.ports_.logical_port = iPort;
                }
                else
                {
                    this->up_.ports_.physical_port = iPort;
                }
                return iPort != 0;
            }
            else
            {
                this->up_.port =iPort;
            }
            return true;
        }

        bool set_logical_port(uint16_t rtps_port)
        {
            this->up_.ports_.logical_port = rtps_port;
            return (rtps_port != 0);
        }

        uint32_t get_port(bool fullPort = false) const
        {
            if (!fullPort && (kind == LOCATOR_KIND_TCPv4 || kind == LOCATOR_KIND_TCPv6))
            {
                return static_cast<uint32_t>(up_.ports_.physical_port);
            }
            else
            {
                return up_.port;
            }
        }

        inline uint16_t get_physical_port() const
        {
            return up_.ports_.physical_port;
        }

        uint16_t& get_physical_port_by_ref()
        {
            return up_.ports_.physical_port;
        }

        uint16_t& get_logical_port_by_ref()
        {
            return up_.ports_.logical_port;
        }

        uint32_t& get_port_by_ref()
        {
            return up_.port;
        }

        uint16_t get_logical_port() const
        {
            return up_.ports_.logical_port;
        }

        std::string get_connection_id() const
        {
            std::string s = "";
            for (uint16_t i = 0; i < 16; ++i)
                s += std::to_string(ua_.address[i]);
            s += ":" + std::to_string(up_.port);
            return s;
        }

        bool set_IP4_address(octet o1,octet o2,octet o3,octet o4){
            LOCATOR_ADDRESS_INVALID(ua_.address);
            // address[12] = o1;
            // address[13] = o2;
            // address[14] = o3;
            // address[15] = o4;
            ua_.addresses_.ip_address[0] = o1;
            ua_.addresses_.ip_address[1] = o2;
            ua_.addresses_.ip_address[2] = o3;
            ua_.addresses_.ip_address[3] = o4;
            return true;
        }

        bool set_IP4_address(const Locator_t &other)
        {
            memcpy(ua_.addresses_.ip_address, other.ua_.addresses_.ip_address, sizeof(octet) * 4);
            return true;
        }

        bool set_IP6_address(const Locator_t &other)
        {
            memcpy(ua_.address, other.ua_.address, sizeof(octet) * 16);
            return true;
        }

		octet* get_IP4_address()
		{
			return ua_.addresses_.ip_address;
		}

		octet get_IP4_address(uint16_t field) const
		{
			return ua_.addresses_.ip_address[field];
		}

        octet* get_IP6_address()
        {
            return ua_.address;
        }

        bool set_IP4_WAN_address(octet o1,octet o2,octet o3,octet o4){
            if (kind == LOCATOR_KIND_UDPv4) return false;
            // address[8] = o1;
            // address[9] = o2;
            // address[10] = o3;
            // address[11] = o4;
            ua_.addresses_.wan_address[0] = o1;
            ua_.addresses_.wan_address[1] = o2;
            ua_.addresses_.wan_address[2] = o3;
            ua_.addresses_.wan_address[3] = o4;
            return true;
        }

		octet* get_IP4_WAN_address()
		{
			return ua_.addresses_.wan_address;
		}

		octet get_IP4_WAN_address(uint16_t field) const
		{
			return ua_.addresses_.wan_address[field];
		}

        bool has_IP4_WAN_address() const
        {
            return ua_.addresses_.wan_address[0] != 0 &&
                ua_.addresses_.wan_address[1] != 0 &&
                ua_.addresses_.wan_address[2] != 0 &&
                ua_.addresses_.wan_address[3] != 0;
        }

        bool set_IP4_address(const std::string& in_address)
        {
            std::stringstream ss(in_address);
            int a,b,c,d; //to store the 4 ints
            char ch; //to temporarily store the '.'
            ss >> a >> ch >> b >> ch >> c >> ch >> d;
            LOCATOR_ADDRESS_INVALID(ua_.address);
            // address[12] = (octet)a;
            // address[13] = (octet)b;
            // address[14] = (octet)c;
            // address[15] = (octet)d;
            ua_.addresses_.ip_address[0] = (octet)a;
            ua_.addresses_.ip_address[1] = (octet)b;
            ua_.addresses_.ip_address[2] = (octet)c;
            ua_.addresses_.ip_address[3] = (octet)d;
            return true;
        }

        bool set_address(const std::string& in_address)
        {
            if (kind == LOCATOR_KIND_UDPv4)
                return false;
            std::stringstream ss(in_address);
            int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p; //to store the 16 ints
            char ch; //to temporarily store the '.'
            ss >> a >> ch >> b >> ch >> c >> ch >> d >> ch >> e >> ch >> f >> ch >> g >> ch >> h
                >> ch >> i >> ch >> j >> ch >> k >> ch >> l >> ch >> m >> ch >> n >> ch >> o >> ch >> p;
            ua_.address[0] = (octet)a;
            ua_.address[1] = (octet)b;
            ua_.address[2] = (octet)c;
            ua_.address[3] = (octet)d;
            ua_.address[4] = (octet)e;
            ua_.address[5] = (octet)f;
            ua_.address[6] = (octet)g;
            ua_.address[7] = (octet)h;
            ua_.address[8] = (octet)i;
            ua_.address[9] = (octet)j;
            ua_.address[10] = (octet)k;
            ua_.address[11] = (octet)l;
            ua_.address[12] = (octet)m;
            ua_.address[13] = (octet)n;
            ua_.address[14] = (octet)o;
            ua_.address[15] = (octet)p;
            return true;
        }
        bool set_Unique_Lan_Id(const std::string& in_address)
        {
            if (kind == LOCATOR_KIND_UDPv4)
                return false;
            std::stringstream ss(in_address);
            int a, b, c, d, e, f, g, h; //to store the 8 ints
            char ch; //to temporarily store the '.'
            ss >> a >> ch >> b >> ch >> c >> ch >> d >> ch >> e >> ch >> f >> ch >> g >> ch >> h;
            // address[0] = (octet)a;
            // address[1] = (octet)b;
            // address[2] = (octet)c;
            // address[3] = (octet)d;
            // address[4] = (octet)e;
            // address[5] = (octet)f;
            // address[6] = (octet)g;
            // address[7] = (octet)h;
            ua_.addresses_.unique_lan_id[0] = (octet)a;
            ua_.addresses_.unique_lan_id[1] = (octet)b;
            ua_.addresses_.unique_lan_id[2] = (octet)c;
            ua_.addresses_.unique_lan_id[3] = (octet)d;
            ua_.addresses_.unique_lan_id[4] = (octet)e;
            ua_.addresses_.unique_lan_id[5] = (octet)f;
            ua_.addresses_.unique_lan_id[6] = (octet)g;
            ua_.addresses_.unique_lan_id[7] = (octet)h;
            return true;
        }

        bool set_IP4_address(const unsigned char* addr)
        {
            memcpy(ua_.addresses_.ip_address, addr, 4 * sizeof(char));
            return true;
        }

        bool set_IP4_WAN_address(const std::string& in_address)
        {
            if (kind == LOCATOR_KIND_UDPv4) return false;
            std::stringstream ss(in_address);
            int a,b,c,d; //to store the 4 ints
            char ch; //to temporarily store the '.'
            ss >> a >> ch >> b >> ch >> c >> ch >> d;
            // address[8] = (octet)a;
            // address[9] = (octet)b;
            // address[10] = (octet)c;
            // address[11] = (octet)d;
            ua_.addresses_.wan_address[0] = (octet)a;
            ua_.addresses_.wan_address[1] = (octet)b;
            ua_.addresses_.wan_address[2] = (octet)c;
            ua_.addresses_.wan_address[3] = (octet)d;
            return true;
        }

        std::string to_IP4_string() const {
            std::stringstream ss;
            // ss << (int)address[12] << "." << (int)address[13] << "." << (int)address[14]<< "." << (int)address[15];
            ss << (int)ua_.addresses_.ip_address[0] << "." << (int)ua_.addresses_.ip_address[1]
                << "." << (int)ua_.addresses_.ip_address[2]<< "." << (int)ua_.addresses_.ip_address[3];
            return ss.str();
        }

        std::string to_IP4_WAN_string() const {
            if (kind == LOCATOR_KIND_UDPv4) return "";
            std::stringstream ss;
            //ss << (int)address[11] << "." << (int)address[10] << "." << (int)address[9]<< "." << (int)address[8];
            ss << (int)ua_.addresses_.wan_address[0] << "." << (int)ua_.addresses_.wan_address[1]
                << "." << (int)ua_.addresses_.wan_address[2]<< "." << (int)ua_.addresses_.wan_address[3];
            return ss.str();
        }

        uint64_t to_LAN_ID() const {
            if (kind == LOCATOR_KIND_UDPv4) return 0;
            uint64_t lanId;
            octet* oLanId = (octet*)&lanId;
            std::memcpy(oLanId,ua_.address,8*sizeof(octet));
            return lanId;
        }

        octet* get_LAN_ID()
        {
            return ua_.addresses_.unique_lan_id;
        }

        octet* get_Address()
        {
            return ua_.address;
        }

        const octet* get_Address() const
        {
            return ua_.address;
        }

        bool set_LAN_ID(int64_t lanId)
        {
            octet* oLanId = (octet*)&lanId;
            std::memcpy(ua_.address, oLanId, 8*sizeof(octet));
            return true;
        }

        uint32_t to_IP4_long() const
        {
            uint32_t addr;
            octet* oaddr = (octet*)&addr;
#if __BIG_ENDIAN__
            std::memcpy(oaddr,address+12,4*sizeof(octet));
#else
            // TODO (Santi) - Are we sure we want to flip this?
            // oaddr[0] = address[15];
            // oaddr[1] = address[14];
            // oaddr[2] = address[13];
            // oaddr[3] = address[12];
            oaddr[0] = ua_.addresses_.ip_address[3];
            oaddr[1] = ua_.addresses_.ip_address[2];
            oaddr[2] = ua_.addresses_.ip_address[1];
            oaddr[3] = ua_.addresses_.ip_address[0];
#endif

            return addr;
        }

        uint32_t to_IP4_WAN_long() const
        {
            if (kind == LOCATOR_KIND_UDPv4) return 0;
            uint32_t addr;
            octet* oaddr = (octet*)&addr;
#if __BIG_ENDIAN__
            std::memcpy(oaddr,address+8,4*sizeof(octet));
#else
            // TODO (Santi) - Are we sure we want to flip this?
            // oaddr[0] = address[11];
            // oaddr[1] = address[10];
            // oaddr[2] = address[9];
            // oaddr[3] = address[8];
            oaddr[0] = ua_.addresses_.wan_address[3];
            oaddr[1] = ua_.addresses_.wan_address[2];
            oaddr[2] = ua_.addresses_.wan_address[1];
            oaddr[3] = ua_.addresses_.wan_address[0];
#endif

            return addr;
        }

        bool set_IP6_address(uint16_t group0, uint16_t group1, uint16_t group2, uint16_t group3,
                uint16_t group4, uint16_t group5, uint16_t group6, uint16_t group7)
        {
            ua_.address[0]  = (octet) (group0 >> 8);
            ua_.address[1]  = (octet) group0;
            ua_.address[2]  = (octet) (group1 >> 8);
            ua_.address[3]  = (octet) group1;
            ua_.address[4]  = (octet) (group2 >> 8);
            ua_.address[5]  = (octet) group2;
            ua_.address[6]  = (octet) (group3 >> 8);
            ua_.address[7]  = (octet) group3;
            ua_.address[8]  = (octet) (group4 >> 8);
            ua_.address[9]  = (octet) group4;
            ua_.address[10] = (octet) (group5 >> 8);
            ua_.address[11] = (octet) group5;
            ua_.address[12] = (octet) (group6 >> 8);
            ua_.address[13] = (octet) group6;
            ua_.address[14] = (octet) (group7 >> 8);
            ua_.address[15] = (octet) group7;
            return true;
        }

        bool set_IP6_address(const unsigned char* addr)
        {
            memcpy(ua_.address, addr, 16 * sizeof(char));
            return true;
        }

        void copy_Address(unsigned char* dest) const
        {
            memcpy(dest, ua_.address, 16 * sizeof(char));
        }

        void copy_IP4_address(unsigned char* dest) const
        {
            memcpy(dest, ua_.addresses_.ip_address, 4 * sizeof(char));
        }

        bool set_IP6_address(const std::string &hex_address)
        {
            std::vector<std::string> hexdigits;

            size_t start = 0, end = 0;
            std::string auxstr;

            while(end != std::string::npos)
            {
                end = hex_address.find(':',start);
                if (end - start > 1)
                {
                    hexdigits.push_back(hex_address.substr(start, end - start));
                }
                else
                    hexdigits.push_back(std::string("EMPTY"));
                start = end + 1;
            }

            //FOUND a . in the last element (MAP TO IP4 address)
            if ((hexdigits.end() - 1)->find('.') != std::string::npos)
            {
                return false;
            }

            *(hexdigits.end() - 1) = (hexdigits.end() - 1)->substr(0, (hexdigits.end() - 1)->find('%'));

            int auxnumber = 0;
            uint8_t index= 15;
            for (auto it = hexdigits.rbegin(); it != hexdigits.rend(); ++it)
            {
                if (*it != std::string("EMPTY"))
                {
                    if (it->length() <= 2)
                    {
                        ua_.address[index - 1] = 0;
                        std::stringstream ss;
                        ss << std::hex << (*it);
                        ss >> auxnumber;
                        ua_.address[index] = (octet)auxnumber;
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << std::hex << it->substr(it->length()-2);
                        ss >> auxnumber;
                        ua_.address[index] = (octet)auxnumber;
                        ss.str("");
                        ss.clear();
                        ss << std::hex << it->substr(0, it->length() - 2);
                        ss >> auxnumber;
                        ua_.address[index - 1] = (octet)auxnumber;
                    }
                    index -= 2;
                }
                else
                    break;
            }
            index = 0;
            for (auto it = hexdigits.begin(); it != hexdigits.end(); ++it)
            {
                if (*it != std::string("EMPTY"))
                {
                    if (it->length() <= 2)
                    {
                        ua_.address[index] = 0;
                        std::stringstream ss;
                        ss << std::hex << (*it);
                        ss >> auxnumber;
                        ua_.address[index + 1]=(octet)auxnumber;
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << std::hex << it->substr(it->length() - 2);
                        ss >> auxnumber;
                        ua_.address[index + 1] = (octet)auxnumber;
                        ss.str("");
                        ss.clear();
                        ss << std::hex << it->substr(0, it->length() - 2);
                        ss >> auxnumber;
                        ua_.address[index] =  (octet)auxnumber;
                    }
                    index += 2;
                }
                else
                    break;
            }

            return true;
        }

        std::string to_IP6_string() const{
            std::stringstream ss;
            ss << std::hex;
            for (int i = 0; i != 14; i+= 2)
            {
                auto field = (ua_.address[i] << 8) + ua_.address[i+1];
                ss << field << ":";
            }
            auto field = ua_.address[14] + (ua_.address[15] << 8);
            ss << field;
            return ss.str();
        }

        bool is_Local_Address() const
        {
            if (kind == LOCATOR_KIND_UDPv4 || kind == LOCATOR_KIND_TCPv4)
            {
                return is_IP4_Local();
            }
            else
            {
                return is_IP6_Local();
            }
        }

        bool is_IP4_Local() const
        {
            return (ua_.addresses_.ip_address[0] == 127
                 && ua_.addresses_.ip_address[1] == 0
                 && ua_.addresses_.ip_address[2] == 0
                 && ua_.addresses_.ip_address[3] == 1);
        }

        bool is_IP6_Local() const
        {
            // Tal y como se calculaba antes... un poco optimista creo.
            // return (address[0] == 0 && address[1] == 0);
            return  ua_.address[0] == 0 && ua_.address[1] == 0 && ua_.address[2] == 0 && ua_.address[3] == 0 &&
                    ua_.address[4] == 0 && ua_.address[5] == 0 && ua_.address[6] == 0 && ua_.address[7] == 0 &&
                    ua_.address[8] == 0 && ua_.address[9] == 0 && ua_.address[10] == 0 && ua_.address[11] == 0 &&
                    ua_.address[12] == 0 && ua_.address[13] == 0 && ua_.address[14] == 0 && ua_.address[15] == 1;
        }

        void set_Invalid_Address()
        {
            LOCATOR_ADDRESS_INVALID(ua_.address);
        }

        bool is_Any() const
        {
            if (kind == LOCATOR_KIND_UDPv4 || kind == LOCATOR_KIND_TCPv4)
            {
                return ua_.addresses_.ip_address[0] == 0 &&
                    ua_.addresses_.ip_address[1] == 0 &&
                    ua_.addresses_.ip_address[2] == 0 &&
                    ua_.addresses_.ip_address[3] == 0;
            }
            else
            {
                return ua_.address[0] == 0 &&
                    ua_.address[1] == 0 &&
                    ua_.address[2] == 0 &&
                    ua_.address[3] == 0 &&
                    ua_.address[4] == 0 &&
                    ua_.address[5] == 0 &&
                    ua_.address[6] == 0 &&
                    ua_.address[7] == 0 &&
                    ua_.address[8] == 0 &&
                    ua_.address[9] == 0 &&
                    ua_.address[10] == 0 &&
                    ua_.address[11] == 0 &&
                    ua_.address[12] == 0 &&
                    ua_.address[13] == 0 &&
                    ua_.address[14] == 0 &&
                    ua_.address[15] == 0;
            }
        }

        bool is_Multicast() const
        {
            if (kind == LOCATOR_KIND_UDPv4 || kind == LOCATOR_KIND_TCPv4)
            {
                return ua_.addresses_.ip_address[0] >= 224 && ua_.addresses_.ip_address[0] <= 239;
            }
            else
            {
                return ua_.address[0] == 0xFF;
            }
        }

        bool compare_IP6_address(const Locator_t &other) const
        {
            return memcmp(ua_.address, other.ua_.address, 16) == 0;
        }

        bool compare_IP4_address(const Locator_t &other) const
        {
            // TODO Compare WAN?
            return memcmp(ua_.addresses_.ip_address, other.ua_.addresses_.ip_address, 4) == 0;
        }

        bool compare_IP4_address_and_port(const Locator_t& other) const
        {
            return compare_IP4_address(other) && up_.ports_.physical_port == other.up_.ports_.physical_port;
        }

        bool compare_IP6_address_and_port(const Locator_t& other) const
        {
            return compare_IP6_address(other) && up_.ports_.physical_port == other.up_.ports_.physical_port;
        }


        friend inline bool IsAddressDefined(const Locator_t& loc);
        friend inline bool operator==(const Locator_t&loc1,const Locator_t& loc2);
        friend inline bool equalsPhysicalLocator(const Locator_t &loc1, const Locator_t &loc2);
        friend inline std::ostream& operator<<(std::ostream& output,const Locator_t& loc);
};


inline bool IsAddressDefined(const Locator_t& loc)
{
    if(loc.kind == LOCATOR_KIND_UDPv4 || loc.kind == LOCATOR_KIND_TCPv4) // WAN addr in TCPv4 is optional, isn't?
    {
        for(uint8_t i = 0; i < 4; ++i)
        {
            if(loc.ua_.addresses_.ip_address[i] != 0)
                return true;
        }
    }
    else if (loc.kind == LOCATOR_KIND_UDPv6 || loc.kind == LOCATOR_KIND_TCPv6)
    {
        for(uint8_t i = 0; i < 16; ++i)
        {
            if(loc.ua_.address[i] != 0)
                return true;
        }
    }
    return false;
}

inline bool IsLocatorValid(const Locator_t&loc)
{
    if(loc.kind<0)
        return false;
    return true;
}

inline bool operator<(const Locator_t &loc1, const Locator_t &loc2)
{
    return memcmp(&loc1, &loc2, sizeof(Locator_t)) < 0;
}

inline bool operator==(const Locator_t&loc1,const Locator_t& loc2)
{
    if(loc1.kind!=loc2.kind)
        return false;
    if(loc1.up_.port !=loc2.up_.port)
        return false;
    if(!std::equal(loc1.ua_.address,loc1.ua_.address+16,loc2.ua_.address))
        return false;
    return true;
}

/**
 * Compares if both locators are equals, except for logical port
 * */
inline bool equalsPhysicalLocator(const Locator_t &loc1, const Locator_t &loc2)
{
    if(loc1.kind != loc2.kind)
        return false;
    if(loc1.get_logical_port() != loc2.get_logical_port())
        return false;
    if(!std::equal(loc1.ua_.address, loc1.ua_.address + 16, loc2.ua_.address))
        return false;
    return true;
}

inline std::ostream& operator<<(std::ostream& output,const Locator_t& loc)
{
    if(loc.kind == LOCATOR_KIND_UDPv4 || loc.kind == LOCATOR_KIND_TCPv4)
    {
        output<<(int)loc.ua_.addresses_.ip_address[0] << "." << (int)loc.ua_.addresses_.ip_address[1]
            << "." << (int)loc.ua_.addresses_.ip_address[2]<< "." << (int)loc.ua_.addresses_.ip_address[3]
            <<":"<<loc.up_.ports_.physical_port;
    }
    else if(loc.kind == LOCATOR_KIND_UDPv6 || loc.kind == LOCATOR_KIND_TCPv6)
    {
        for(uint8_t i =0;i<16;++i)
        {
            output<<(int)loc.ua_.address[i];
            if(i<15)
                output<<".";
        }
        output<<":"<<loc.up_.port;
    }
    if (loc.up_.ports_.logical_port != 0)
    {
        output <<":"<<loc.up_.ports_.logical_port;
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
        RTPS_DllAPI LocatorList_t(){};

        RTPS_DllAPI ~LocatorList_t(){};

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
            if(locator_list.m_locators.size() == m_locators.size())
            {
                 bool returnedValue = true;

                 for(auto it = locator_list.m_locators.begin(); returnedValue &&
                         it != locator_list.m_locators.end(); ++it)
                 {
                     returnedValue = false;

                     for(auto it2 = m_locators.begin();  !returnedValue && it2 != m_locators.end(); ++it2)
                     {
                         if(*it == *it2)
                             returnedValue = true;
                     }
                 }

                 return returnedValue;
            }

            return false;
        }

        RTPS_DllAPI LocatorListIterator begin(){
            return m_locators.begin();
        }

        RTPS_DllAPI LocatorListIterator end(){
            return m_locators.end();
        }

        RTPS_DllAPI LocatorListConstIterator begin() const {
            return m_locators.begin();
        }

        RTPS_DllAPI LocatorListConstIterator end() const {
            return m_locators.end();
        }

        RTPS_DllAPI size_t size(){
            return m_locators.size();
        }

        RTPS_DllAPI void clear(){ return m_locators.clear();}

        RTPS_DllAPI void reserve(size_t num){ return m_locators.reserve(num);}

        RTPS_DllAPI void resize(size_t num) { return m_locators.resize(num);}

        RTPS_DllAPI void push_back(const Locator_t& loc)
        {
            bool already = false;
            for(LocatorListIterator it=this->begin(); it!=this->end(); ++it)
            {
                if(loc == *it)
                {
                    already = true;
                    break;
                }
            }
            if(!already)
                m_locators.push_back(loc);
        }
/*
        template<typename... Args>
        RTPS_DllAPI void emplace_back(Args&&... args)
        {
            Locator_t loc(std::forward<Args>(args) ...);
            bool already = false;
            for(LocatorListIterator it=this->begin(); it!=this->end(); ++it)
            {
                if(loc == *it)
                {
                    already = true;
                    break;
                }
            }
            if(!already)
                m_locators.push_back(loc);
        }
*/
        RTPS_DllAPI void push_back(const LocatorList_t& locList)
        {
            for(auto it = locList.m_locators.begin(); it!=locList.m_locators.end(); ++it)
            {
                this->push_back(*it);
            }
        }

        RTPS_DllAPI bool empty(){
            return m_locators.empty();
        }

        RTPS_DllAPI void erase(const Locator_t& loc)
        {
            std::remove(m_locators.begin(), m_locators.end(), loc);
        }

        RTPS_DllAPI bool contains(const Locator_t& loc)
        {
            for(LocatorListIterator it=this->begin();it!=this->end();++it)
            {
                if(IsAddressDefined(*it))
                {
                    if(loc == *it)
                        return true;
                }
                else
                {
                    if(loc.kind == (*it).kind && loc.get_port() == (*it).get_port())
                        return true;
                }
            }

            return false;
        }

        RTPS_DllAPI bool isValid()
        {
            for(LocatorListIterator it=this->begin();it!=this->end();++it)
            {
                if(!IsLocatorValid(*it))
                    return false;
            }
            return true;
        }


        RTPS_DllAPI void swap(LocatorList_t& locatorList)
        {
            this->m_locators.swap(locatorList.m_locators);
        }

        friend std::ostream& operator <<(std::ostream& output,const LocatorList_t& loc);

    private:

        std::vector<Locator_t> m_locators;
};

inline std::ostream& operator<<(std::ostream& output,const LocatorList_t& locList)
{
    for(auto it = locList.m_locators.begin();it!=locList.m_locators.end();++it)
    {
        output << *it << ",";
    }
    return output;
}

}
}
}

#endif /* RTPS_ELEM_LOCATOR_H_ */
