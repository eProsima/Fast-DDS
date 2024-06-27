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
 * @file IPFinder.cpp
 *
 */

#include <fastdds/utils/IPFinder.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <fastdds/dds/log/Log.hpp>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif // if defined(__APPLE__)

#if defined(_WIN32)
#pragma comment(lib, "Iphlpapi.lib")
#include <stdio.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <assert.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#if defined(__APPLE__) && TARGET_OS_IPHONE
#include <net/ethernet.h>
#else
#include <net/if_arp.h>
#endif // if defined(__APPLE__) && TARGET_OS_IPHONE
#include <errno.h>
#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#endif // if defined(__APPLE__)
#if defined(__QNXNTO__)
#include <net/if_dl.h>
#endif // if defined(__QNXNTO__)
#endif // if defined(_WIN32)

#if defined(__FreeBSD__)
#include <netinet/in.h>
#endif // if defined(__FreeBSD__)

#include <bitset>
#include <cstddef>
#include <cstring>
#include <algorithm>

namespace eprosima {
namespace fastdds {
namespace rtps {

IPFinder::IPFinder()
{
}

IPFinder::~IPFinder()
{
}

#if defined(_WIN32)

#define DEFAULT_ADAPTER_ADDRESSES_SIZE 15360

bool IPFinder::getIPs(
        std::vector<info_IP>* vec_name,
        bool return_loopback)
{
    DWORD rv, size = DEFAULT_ADAPTER_ADDRESSES_SIZE;
    PIP_ADAPTER_ADDRESSES adapter_addresses, aa;
    PIP_ADAPTER_UNICAST_ADDRESS ua;

    adapter_addresses = (PIP_ADAPTER_ADDRESSES)malloc(DEFAULT_ADAPTER_ADDRESSES_SIZE);

    rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &size);

    if (rv != ERROR_SUCCESS)
    {
        adapter_addresses = (PIP_ADAPTER_ADDRESSES)realloc(adapter_addresses, size);

        rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &size);
    }

    if (rv != ERROR_SUCCESS)
    {
        EPROSIMA_LOG_WARNING(UTILS, "GetAdaptersAddresses() failed");
        free(adapter_addresses);
        return false;
    }

    for (aa = adapter_addresses; aa != NULL; aa = aa->Next)
    {
        if (aa->OperStatus == 1) //is ENABLED
        {
            for (ua = aa->FirstUnicastAddress; ua != NULL; ua = ua->Next)
            {
                char buf[BUFSIZ];

                int family = ua->Address.lpSockaddr->sa_family;

                if (family == AF_INET || family == AF_INET6) //IP4
                {
                    //printf("\t%s ",  family == AF_INET ? "IPv4":"IPv6");
                    memset(buf, 0, BUFSIZ);
                    getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, buf, sizeof(buf), NULL, 0,
                            NI_NUMERICHOST);
                    info_IP info;
                    info.type = family == AF_INET ? IP4 : IP6;
                    info.name = std::string(buf);
                    info.dev = std::string(aa->AdapterName);

                    // Currently not supported interfaces that not support multicast.
                    if (aa->Flags & 0x0010)
                    {
                        continue;
                    }

                    // Parse IP
                    bool success = false;
                    if (info.type == IP4)
                    {
                        success = parseIP4(info);
                    }
                    else if (info.type == IP6)
                    {
                        success = parseIP6(info);
                    }

                    // Abort if parsing failed
                    if (!success)
                    {
                        EPROSIMA_LOG_WARNING(UTILS, "Failed to parse IP address: " << info.name);
                        continue;
                    }

                    if (return_loopback || (info.type != IP6_LOCAL && info.type != IP4_LOCAL))
                    {
                        // Parse prefix length if available (introduced in Windows Vista)
                        if (ua->Length <= offsetof(IP_ADAPTER_UNICAST_ADDRESS_LH, OnLinkPrefixLength))
                        {
                            info.masked_locator.mask(0);
                        }
                        else
                        {
                            info.masked_locator.mask(ua->OnLinkPrefixLength);
                        }
                        info.masked_locator = info.locator; // NOTE: copy (kind and address) after parsing ip

                        vec_name->push_back(info);
                    }
                    //printf("Buffer: %s\n", buf);
                }
            }
        }
    }

    free(adapter_addresses);
    return true;
}

#else

bool parseIfaddr(
        const ifaddrs* ifaddr,
        IPFinder::info_IP& info,
        bool return_loopback)
{
    int family, s;
    char host[NI_MAXHOST];

    IPFinder::IPTYPE ip_type;
    bool ipv4; // Binary discriminator to distinguish between IP4/IP4_LOCAL and IP6/IP6_LOCAL types
    family = ifaddr->ifa_addr->sa_family;
    if (family == AF_INET)
    {
        ip_type = IPFinder::IP4;
        ipv4 = true;
    }
    else if (family == AF_INET6)
    {
        ip_type = IPFinder::IP6;
        ipv4 = false;
    }
    else
    {
        // Not an IP address -> abort
        return false;
    }

    info.type = ip_type;
    info.dev = std::string(ifaddr->ifa_name);

    // Get interface name
    s = getnameinfo(ifaddr->ifa_addr, ipv4 ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
    if (s != 0)
    {
        EPROSIMA_LOG_WARNING(UTILS, "getnameinfo() failed: " << gai_strerror(s));
        return false;
    }
    info.name = std::string(host);

    // Get interface network mask
    s = getnameinfo(ifaddr->ifa_netmask, ipv4 ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
    if (s != 0)
    {
        EPROSIMA_LOG_WARNING(UTILS, "getnameinfo() failed: " << gai_strerror(s));
        return false;
    }
    auto netmask_str = std::string(host);

    IPFinder::IPTYPE local_ip_type = ipv4 ? IPFinder::IP4_LOCAL : IPFinder::IP6_LOCAL;
    if (ipv4 ? IPFinder::parseIP4(info) : IPFinder::parseIP6(info))
    {
        if (return_loopback || info.type != local_ip_type)
        {
            // Convert parsed netmask string to locator
            Locator_t netmask_locator;
            netmask_locator.kind = ipv4 ? LOCATOR_KIND_UDPv4 : LOCATOR_KIND_UDPv6;
            netmask_locator.port = 0;
            ipv4 ? IPLocator::setIPv4(netmask_locator, netmask_str) : IPLocator::setIPv6(netmask_locator, netmask_str);

            // Get netmask length from locator
            uint8_t netmask = 0;
            for (const auto& addr_octet: netmask_locator.address)
            {
                netmask += static_cast<uint8_t>(std::bitset<8>(addr_octet).count());
            }
            info.masked_locator.mask(netmask);
            info.masked_locator = info.locator; // NOTE: copy (kind and address) after parsing IP

            return true;
        }
    }
    else
    {
        EPROSIMA_LOG_WARNING(UTILS, "Failed to parse IP address: " << info.name);
    }
    return false;
}

bool IPFinder::getIPs(
        std::vector<info_IP>* vec_name,
        bool return_loopback)
{
    struct ifaddrs* ifaddr, * ifa;

    // TODO arm64 doesn't seem to support getifaddrs
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return false;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL || (ifa->ifa_flags & IFF_RUNNING) == 0)
        {
            continue;
        }

        info_IP info;
        if (parseIfaddr(ifa, info, return_loopback))
        {
            vec_name->push_back(info);
        }
    }

    freeifaddrs(ifaddr);
    return true;
}

#endif // if defined(_WIN32)

#if defined(_WIN32)

bool IPFinder::getAllMACAddress(
        std::vector<info_MAC>* macs)
{
    DWORD rv, size = DEFAULT_ADAPTER_ADDRESSES_SIZE;
    PIP_ADAPTER_ADDRESSES adapter_addresses, aa;

    adapter_addresses = (PIP_ADAPTER_ADDRESSES)malloc(DEFAULT_ADAPTER_ADDRESSES_SIZE);

    rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &size);

    if (rv != ERROR_SUCCESS)
    {
        adapter_addresses = (PIP_ADAPTER_ADDRESSES)realloc(adapter_addresses, size);

        rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &size);
    }

    if (rv != ERROR_SUCCESS)
    {
        EPROSIMA_LOG_WARNING(UTILS, "GetAdaptersAddresses() failed");
        free(adapter_addresses);
        return false;
    }

    for (aa = adapter_addresses; aa != NULL; aa = aa->Next)
    {
        if (aa->OperStatus == 1) //is ENABLED
        {
            info_MAC mac;
            memcpy(mac.address, aa->PhysicalAddress, (std::min)(6UL, aa->PhysicalAddressLength));

            if (std::find(macs->begin(), macs->end(), mac) == macs->end())
            {
                macs->push_back(mac);
            }
        }
    }

    free(adapter_addresses);
    return true;
}

#elif defined(__APPLE__)

bool IPFinder::getAllMACAddress(
        std::vector<info_MAC>* macs)
{
    int mib[6];
    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;

    std::vector<IPFinder::info_IP> ips;
    IPFinder::getIPs(&ips);
    for (auto& ip : ips)
    {
        if ((mib[5] = if_nametoindex(ip.dev.c_str())) == 0)
        {
            EPROSIMA_LOG_WARNING(UTILS, "Error on nametoindex: " << strerror(errno));
            return false;
        }

        size_t len;
        unsigned char* buf;
        if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0)
        {
            EPROSIMA_LOG_WARNING(UTILS, "Error on nametoindex: " << strerror(errno));
            return false;
        }

        if ((buf = (unsigned char*)malloc(len)) == NULL)
        {
            EPROSIMA_LOG_WARNING(UTILS, "Falure allocating " << len << " octets");
            return false;
        }

        if (sysctl(mib, 6, buf, &len, NULL, 0) < 0)
        {
            EPROSIMA_LOG_WARNING(UTILS, "Error on sysctl: " << strerror(errno));
            return false;
        }

        sockaddr_dl* sdl = (sockaddr_dl*)(buf + sizeof(if_msghdr));
        info_MAC mac;
        memcpy(mac.address, LLADDR(sdl), 6);

        if (std::find(macs->begin(), macs->end(), mac) == macs->end())
        {
            macs->push_back(mac);
        }

        free(buf);
    }
    return true;
}

#elif defined(__linux__)

bool IPFinder::getAllMACAddress(
        std::vector<info_MAC>* macs)
{
    std::vector<IPFinder::info_IP> ips;
    IPFinder::getIPs(&ips);
    for (auto& ip : ips)
    {
        struct ifreq ifr = {};
        strncpy(ifr.ifr_name, ip.dev.c_str(), sizeof(ifr.ifr_name) - 1);
        int fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (fd == -1)
        {
            EPROSIMA_LOG_WARNING(UTILS, "Error creating socket: " << strerror(errno));
            return false;
        }

        if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1)
        {
            EPROSIMA_LOG_WARNING(UTILS, "Error on ioctl: " << strerror(errno));
            close(fd);
            return false;
        }

        if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
        {
            continue;
        }

        info_MAC mac;
        memcpy(mac.address, ifr.ifr_hwaddr.sa_data, 6);

        if (std::find(macs->begin(), macs->end(), mac) == macs->end())
        {
            macs->push_back(mac);
        }
    }
    return true;
}

#elif defined(__QNXNTO__)

bool IPFinder::getAllMACAddress(
        std::vector<info_MAC>* macs)
{
    struct ifaddrs* ifaphead;
    struct ifaddrs* ifap;

    if (getifaddrs(&ifaphead) != 0)
    {
        EPROSIMA_LOG_WARNING(UTILS, "getifaddrs() failed: " << strerror(errno));
        return false;
    }

    struct sockaddr_dl* sdl = NULL;

    std::vector<IPFinder::info_IP> ips;
    IPFinder::getIPs(&ips);

    for (auto& ip : ips)
    {
        for (ifap = ifaphead; ifap; ifap = ifap->ifa_next)
        {
            if ((ifap->ifa_addr->sa_family == AF_LINK))
            {
                if (strcmp(ifap->ifa_name, ip.dev.c_str()) == 0)
                {
                    sdl = (struct sockaddr_dl*)ifap->ifa_addr;
                    if (sdl)
                    {
                        info_MAC mac;
                        memcpy(mac.address, LLADDR(sdl), sdl->sdl_alen);
                        if (std::find(macs->begin(), macs->end(), mac) == macs->end())
                        {
                            macs->push_back(mac);
                        }
                    }
                }
            }
        }
    }

    if (ifaphead)
    {
        freeifaddrs(ifaphead);
    }

    return true;
}

#else

bool IPFinder::getAllMACAddress(
        std::vector<info_MAC>* macs)
{
    return false;
}

#endif // if defined(_WIN32)


bool IPFinder::getIP4Address(
        LocatorList_t* locators)
{
    std::vector<info_IP> ip_names;
    if (IPFinder::getIPs(&ip_names))
    {

        locators->clear();
        for (auto it = ip_names.begin();
                it != ip_names.end(); ++it)
        {
            if (it->type == IP4)
            {
                locators->push_back(it->locator);
            }
        }
        return true;
    }
    return false;
}

bool IPFinder::getAllIPAddress(
        LocatorList_t* locators)
{
    std::vector<info_IP> ip_names;
    if (IPFinder::getIPs(&ip_names))
    {
        locators->clear();
        for (auto it = ip_names.begin();
                it != ip_names.end(); ++it)
        {
            if (it->type == IP6)
            {
                locators->push_back(it->locator);
            }
            else if (it->type == IP4)
            {
                locators->push_back(it->locator);
            }
        }
        return true;
    }
    return false;
}

bool IPFinder::getIP6Address(
        LocatorList_t* locators)
{
    std::vector<info_IP> ip_names;
    if (IPFinder::getIPs(&ip_names))
    {

        locators->clear();
        for (auto it = ip_names.begin();
                it != ip_names.end(); ++it)
        {
            if (it->type == IP6)
            {
                locators->push_back(it->locator);
            }
        }
        return true;
    }
    return false;
}

bool IPFinder::parseIP4(
        info_IP& info)
{
    info.locator.kind = LOCATOR_KIND_UDPv4;
    info.locator.port = 0;
    IPLocator::setIPv4(info.locator, info.name);
    if (IPLocator::isLocal(info.locator))
    {
        info.type = IP4_LOCAL;
    }
    return true;
}

bool IPFinder::parseIP6(
        info_IP& info)
{
    info.locator.kind = LOCATOR_KIND_UDPv6;
    info.locator.port = 0;
    IPLocator::setIPv6(info.locator, info.name);
    if (IPLocator::isLocal(info.locator))
    {
        info.type = IP6_LOCAL;
    }
    /*
       cout << "IPSTRING: ";
       for (auto it : hexdigits)
       cout << it << " ";
       cout << endl;
       cout << "LOCATOR: " << *loc << endl;
     */
    return true;
}

std::string IPFinder::getIPv4Address(
        const std::string& name)
{
    addrinfo hints;
    addrinfo* result;
    char str[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_addr = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = nullptr;
    hints.ai_next = nullptr;

    int s = getaddrinfo(name.c_str(), nullptr, &hints, &result);

    if ((s == 0) && (inet_ntop(AF_INET, result[0].ai_addr, str, INET_ADDRSTRLEN) != nullptr))
    {
        freeaddrinfo(result);
        return str;
    }

    freeaddrinfo(result);
    return "";
}

std::string IPFinder::getIPv6Address(
        const std::string& name)
{
    addrinfo hints;
    addrinfo* result;
    char str[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_addr = nullptr;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = 0;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = nullptr;
    hints.ai_next = nullptr;

    int s = getaddrinfo(name.c_str(), nullptr, &hints, &result);

    if ((s == 0) && (inet_ntop(AF_INET6, result[0].ai_addr, str, INET6_ADDRSTRLEN) != nullptr))
    {
        freeaddrinfo(result);
        return str;
    }

    freeaddrinfo(result);
    return "";
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
