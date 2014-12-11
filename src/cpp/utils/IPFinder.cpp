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

namespace eprosima {
namespace fastrtps{

IPFinder::IPFinder() {


}

IPFinder::~IPFinder() {

}

#if defined(_WIN32)

bool IPFinder::getIP4s(std::vector<std::string>* vec_name)
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

	for (aa = adapter_addresses; aa != NULL; aa = aa->Next) {
		
		for (ua = aa->FirstUnicastAddress; ua != NULL; ua = ua->Next) {
			char buf[BUFSIZ];

			int family = ua->Address.lpSockaddr->sa_family;
			if(family == AF_INET) //IP4
			{
			//printf("\t%s ",  family == AF_INET ? "IPv4":"IPv6");

			memset(buf, 0, BUFSIZ);
			getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, buf, sizeof(buf), NULL, 0,NI_NUMERICHOST);
			vec_name->push_back(std::string(buf));
			//printf("Buffer: %s\n", buf);
			}
		}
	}

	free(adapter_addresses);
	return true;
}

#else

bool IPFinder::getIP4s(std::vector<std::string>* vec_name )
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

		if (family == AF_INET) {
			s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
					host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				exit(EXIT_FAILURE);
			}
			vec_name->push_back(host);
			//printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
		}
	}
return true;
}
#endif

bool IPFinder::getIPAddress(LocatorList_t* locators)
{
	std::vector<std::string> ip_names;
	if(IPFinder::getIP4s(&ip_names))
	{

		locators->clear();
		for(std::vector<std::string>::iterator it=ip_names.begin();
				it!=ip_names.end();++it)
		{
			std::stringstream ss(*it);
			int a,b,c,d;
			char ch;
			ss >> a >>ch >>b >> ch >> c >>ch >>d;
			if(a== 127 && b== 0 && c== 0 && d == 1)
				continue;
	//		if(a==169 && b==254)
	//			continue;
			Locator_t loc;
			loc.kind = 1;
			loc.port = 0;
			loc.address[12] = (octet)a;
			loc.address[13] = (octet)b;
			loc.address[14] = (octet)c;
			loc.address[15] = (octet)d;
			locators->push_back(loc);
		}
		return true;
	}

	return false;
}
}
} /* namespace eprosima */
