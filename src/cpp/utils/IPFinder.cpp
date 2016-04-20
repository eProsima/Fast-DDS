/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file IPFinder.cpp
 *
 */

#include <fastrtps/utils/IPFinder.h>

#if defined(_WIN32)
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
#endif



namespace eprosima {
namespace fastrtps{

IPFinder::IPFinder() {


}

IPFinder::~IPFinder() {

}

#if defined(_WIN32)


bool IPFinder::getIPs(std::vector<info_IP>* vec_name)
{
	DWORD rv, size;
	PIP_ADAPTER_ADDRESSES adapter_addresses, aa;
	PIP_ADAPTER_UNICAST_ADDRESS ua;

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size);


	if (rv != ERROR_BUFFER_OVERFLOW) {
		fprintf(stderr, "GetAdaptersAddresses() failed...");
		return false;
	}
	adapter_addresses = (PIP_ADAPTER_ADDRESSES)malloc(size);

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &size);
	if (rv != ERROR_SUCCESS) {
		fprintf(stderr, "GetAdaptersAddresses() failed...");
		free(adapter_addresses);
		return false;
	}
	//cout << "IP INFORMATION " << endl;
	for (aa = adapter_addresses; aa != NULL; aa = aa->Next) {
		if (aa->OperStatus == 1) //is ENABLED
		{
			for (ua = aa->FirstUnicastAddress; ua != NULL; ua = ua->Next) {
				char buf[BUFSIZ];

				int family = ua->Address.lpSockaddr->sa_family;

				if (family == AF_INET || family == AF_INET6) //IP4
				{
					//printf("\t%s ",  family == AF_INET ? "IPv4":"IPv6");
					memset(buf, 0, BUFSIZ);
					getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
					info_IP info;
					info.type = family == AF_INET ? IP4 : IP6;
					info.name = std::string(buf);
					if (info.type == IP4 && !parseIP4(info.name, &info.locator))
						info.type = IP4_LOCAL;
					else if (info.type == IP6 && !parseIP6(info.name, &info.locator))
						info.type = IP6_LOCAL;
					if (info.type == IP6)
					{
						sockaddr_in6* so = (sockaddr_in6*)ua->Address.lpSockaddr;
						info.scope_id = so->sin6_scope_id;
					}
					vec_name->push_back(info);
					//printf("Buffer: %s\n", buf);
				}
			}
		}
	}

	free(adapter_addresses);
	return true;
}

#else

bool IPFinder::getIPs(std::vector<info_IP>* vec_name )
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{

		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;

		if (family == AF_INET)
		{
			s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
					host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
                freeifaddrs(ifaddr);
				exit(EXIT_FAILURE);
			}
			info_IP info;
			info.type = IP4;
			info.name = std::string(host);
			if(!parseIP4(info.name,&info.locator))
				info.type = IP4_LOCAL;
			vec_name->push_back(info);
			//printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);

		}
		else if(family == AF_INET6)
		{
			s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6),
					host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
                freeifaddrs(ifaddr);
				exit(EXIT_FAILURE);
			}
			struct sockaddr_in6 * so = (struct sockaddr_in6 *)ifa->ifa_addr;
			info_IP info;
			info.type = IP6;
			info.name = std::string(host);
			if(!parseIP6(info.name,&info.locator))
				info.type = IP6_LOCAL;
			info.scope_id = so->sin6_scope_id;
			vec_name->push_back(info);
			//printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
		}
	}

    freeifaddrs(ifaddr);
	return true;
}
#endif

bool IPFinder::getIP4Address(LocatorList_t* locators)
{
	std::vector<info_IP> ip_names;
	if(IPFinder::getIPs(&ip_names))
	{

		locators->clear();
		for(auto it=ip_names.begin();
				it!=ip_names.end();++it)
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

bool IPFinder::getAllIPAddress(LocatorList_t* locators)
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

bool IPFinder::getIP6Address(LocatorList_t* locators)
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

RTPS_DllAPI bool IPFinder::parseIP4(std::string& str,Locator_t*loc)
{
	std::stringstream ss(str);
	int a, b, c, d;
	char ch;
	ss >> a >> ch >> b >> ch >> c >> ch >> d;
    //TODO Property to activate or deactivate the loopback interface.
	//if (a == 127 && b == 0 && c == 0 && d == 1)
		//return false;
	//		if(a==169 && b==254)
	//			continue;
	loc->kind = 1;
	loc->port = 0;
	for (int8_t i = 0; i < 12; ++i)
		loc->address[i] = 0;
	loc->address[12] = (octet)a;
	loc->address[13] = (octet)b;
	loc->address[14] = (octet)c;
	loc->address[15] = (octet)d;
	return true;
}
RTPS_DllAPI bool IPFinder::parseIP6(std::string& str,Locator_t* loc)
{
	std::vector<std::string> hexdigits;

	size_t start = 0, end = 0;
	std::string auxstr;

    while(end != std::string::npos)
	{
		end = str.find(':',start);
		if (end - start > 1)
		{
			hexdigits.push_back(str.substr(start, end - start));
		}
		else
			hexdigits.push_back(std::string("EMPTY"));
		start = end + 1;
	}
	if (*hexdigits.begin() == std::string("EMPTY") && *(hexdigits.begin() + 1) == std::string("EMPTY"))
		return false;
	if ((hexdigits.end() - 1)->find('.') != std::string::npos) //FOUND a . in the last element (MAP TO IP4 address)
		return false;
	for (int8_t i = 0; i < 2; ++i)
		loc->address[i] = 0;
	loc->kind = LOCATOR_KIND_UDPv6;
	loc->port = 0;
	*(hexdigits.end() - 1) = (hexdigits.end() - 1)->substr(0, (hexdigits.end() - 1)->find('%'));

	int auxnumber = 0;
	uint8_t index= 15;
	for (auto it = hexdigits.rbegin(); it != hexdigits.rend(); ++it)
	{
		if (*it != std::string("EMPTY"))
		{
			if (it->length() <= 2)
			{
				loc->address[index - 1] = 0;
				std::stringstream ss;
				ss << std::hex << (*it);
				ss >> auxnumber;
				loc->address[index] = (octet)auxnumber;
			}
			else
			{
				std::stringstream ss;
				ss << std::hex << it->substr(it->length()-2);
				ss >> auxnumber;
				loc->address[index] = (octet)auxnumber;
				ss.str("");
				ss.clear();
				ss << std::hex << it->substr(0, it->length() - 2);
				ss >> auxnumber;
				loc->address[index - 1] = (octet)auxnumber;
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
				loc->address[index] = 0;
				std::stringstream ss;
				ss << std::hex << (*it);
				ss >> auxnumber;
				loc->address[index + 1]=(octet)auxnumber;
			}
			else
			{
				std::stringstream ss;
				ss << std::hex << it->substr(it->length() - 2);
				ss >> auxnumber;
				loc->address[index + 1] = (octet)auxnumber;
				ss.str("");
				ss.clear();
				ss << std::hex << it->substr(0, it->length() - 2);
				ss >> auxnumber;
				loc->address[index] =  (octet)auxnumber;
			}
			index += 2;
		}
		else
			break;
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

}
} /* namespace eprosima */
