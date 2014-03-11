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
 *  Created on: Mar 10, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

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

#ifndef IPFINDER_H_
#define IPFINDER_H_

namespace eprosima {


/**
 * Find IPs of the computer.
 * @ingroup UTILITIESMODULE
 */
class IPFinder {
public:
	IPFinder();
	virtual ~IPFinder();
#if defined(_WIN32)
	static bool getIP_win(std::vector<std::string>* vec_name);
#else
	static bool getIP_unix(std::vector<std::string>* vec_name);
#endif
};

} /* namespace eprosima */

#endif /* IPFINDER_H_ */
