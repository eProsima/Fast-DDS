/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file IPFinder.h
 *
 */

#ifndef IPFINDER_H_
#define IPFINDER_H_

#if defined(_WIN32)
	#include <stdio.h>
	#include <winsock2.h>
	#include <iphlpapi.h>
	#include <ws2tcpip.h>
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

#include <vector>
#include <string>

#include "eprosimartps/common/types/Locator.h"

namespace eprosima {

using namespace rtps;
/**
 * Find IPs of the computer.
 * @ingroup UTILITIESMODULE
 */
class IPFinder {
public:
	IPFinder();
	virtual ~IPFinder();
#if defined(_WIN32)
	RTPS_DllAPI static bool getIP4s(std::vector<std::string>* vec_name);
#else
	static bool getIP4s(std::vector<std::string>* vec_name);
#endif
	//!Get the IPAdresses in all interfaces.
	static bool getIPAddress(LocatorList_t* locators);
};

} /* namespace eprosima */

#endif /* IPFINDER_H_ */
