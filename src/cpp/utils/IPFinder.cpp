/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file IPFinder.cpp
 *
 */

#include "fastrtps/utils/IPFinder.h"

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


bool IPFinder::getIPs(std::vector<pair_IP>* vec_name)
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

		for (ua = aa->FirstUnicastAddress; ua != NULL; ua = ua->Next) {
			char buf[BUFSIZ];

			int family = ua->Address.lpSockaddr->sa_family;
			if(family == AF_INET || family == AF_INET6) //IP4
			{
				//printf("\t%s ",  family == AF_INET ? "IPv4":"IPv6");
				memset(buf, 0, BUFSIZ);
				getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, buf, sizeof(buf), NULL, 0,NI_NUMERICHOST);
				pair_IP pai;
				pai.first = family == AF_INET ? IP4 : IP6;
				pai.second = std::string(buf);
				vec_name->push_back(pai);
				//printf("Buffer: %s\n", buf);
			}
		}
	}

	free(adapter_addresses);
	return true;
}

#else

bool IPFinder::getIPs(std::vector<pair_IP>* vec_name )
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
		family = ifa->ifa_addr->sa_family;

		if (family == AF_INET)
		{
			s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
					host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				exit(EXIT_FAILURE);
			}
			pair_IP pai;
			pai.first = IP4;
			pai.second = std::string(host);
			vec_name->push_back(pai);
			printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
		}
		else if(family == AF_INET6)
		{
			s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6),
					host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				exit(EXIT_FAILURE);
			}
			pair_IP pai;
			pai.first = IP6;
			pai.second = std::string(host);
			vec_name->push_back(pai);
			printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
		}
	}
	return true;
}
#endif

bool IPFinder::getIP4Address(LocatorList_t* locators)
{
	std::vector<pair_IP> ip_names;
	if(IPFinder::getIPs(&ip_names))
	{

		locators->clear();
		for(auto it=ip_names.begin();
				it!=ip_names.end();++it)
		{
			if (it->first == IP4)
			{
				Locator_t loc;
				if (parseIP4(it->second,&loc))
					locators->push_back(loc);
			}
		}
		return true;
	}
	return false;
}

bool IPFinder::getAllIPAddress(LocatorList_t* locators)
{
	std::vector<pair_IP> ip_names;
	if (IPFinder::getIPs(&ip_names))
	{
		locators->clear();
		for (auto it = ip_names.begin();
				it != ip_names.end(); ++it)
		{
			if (it->first == IP6)
			{
				Locator_t loc;
				if (parseIP6(it->second, &loc))
					locators->push_back(loc);
			}
			else if (it->first == IP4)
			{
				Locator_t loc;
				if (parseIP4(it->second, &loc))
					locators->push_back(loc);
			}
		}
		return true;
	}
	return false;
}

bool IPFinder::getIP6Address(LocatorList_t* locators)
{
	std::vector<pair_IP> ip_names;
	if (IPFinder::getIPs(&ip_names))
	{

		locators->clear();
		for (auto it = ip_names.begin();
				it != ip_names.end(); ++it)
		{
			if (it->first == IP6)
			{
				Locator_t loc;
				if (parseIP6(it->second, &loc))
					locators->push_back(loc);

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
	if (a == 127 && b == 0 && c == 0 && d == 1)
		return false;
	//		if(a==169 && b==254)
	//			continue;
	loc->kind = 1;
	loc->port = 0;
	for (int8_t i = 0; i < 2; ++i)
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

	size_t start = 0;
	size_t end;
	std::string auxstr;
	while (1)
	{
		end = str.find(':',start);
		if (end - start > 1)
		{
			hexdigits.push_back(str.substr(start, end - start));
		}
		else
			hexdigits.push_back(std::string("EMPTY"));
		start = end + 1;
		if (end == std::string::npos)
			break;
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
