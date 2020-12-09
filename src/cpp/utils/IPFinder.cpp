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

#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/utils/IPLocator.h>

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
#endif // if defined(_WIN32)

#if defined(__FreeBSD__)
#include <netinet/in.h>
#endif // if defined(__FreeBSD__)

#include <cstddef>
#include <cstring>

using namespace eprosima::fastrtps::rtps;

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
        fprintf(stderr, "GetAdaptersAddresses() failed...");
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

                    if (info.type == IP4)
                    {
                        parseIP4(info);
                    }
                    else if (info.type == IP6)
                    {
                        parseIP6(info);
                    }

                    if (return_loopback || (info.type != IP6_LOCAL && info.type != IP4_LOCAL))
                    {
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

bool IPFinder::getIPs(
        std::vector<info_IP>* vec_name,
        bool return_loopback)
{
    struct ifaddrs* ifaddr, * ifa;
    int family, s;
    char host[NI_MAXHOST];

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

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                fprintf(stderr, "getnameinfo() failed: %s\n", gai_strerror(s));
                continue;
            }
            info_IP info;
            info.type = IP4;
            info.name = std::string(host);
            info.dev = std::string(ifa->ifa_name);
            parseIP4(info);

            if (return_loopback || info.type != IP4_LOCAL)
            {
                vec_name->push_back(info);
            }
        }
        else if (family == AF_INET6)
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                fprintf(stderr, "getnameinfo() failed: %s\n", gai_strerror(s));
                continue;
            }
            info_IP info;
            info.type = IP6;
            info.name = std::string(host);
            info.dev = std::string(ifa->ifa_name);
            if (parseIP6(info))
            {
                if (return_loopback || info.type != IP6_LOCAL)
                {
                    vec_name->push_back(info);
                }
            }
            //printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
        }
    }

    freeifaddrs(ifaddr);
    return true;
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
    info.locator.kind = 1;
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
