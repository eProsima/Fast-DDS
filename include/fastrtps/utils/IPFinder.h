/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file IPFinder.h
 *
 */

#ifndef IPFINDER_H_
#define IPFINDER_H_



#include <vector>
#include <string>

#include "fastrtps/rtps/common/Locator.h"

namespace eprosima {
namespace fastrtps{
using namespace rtps;
/**
 * Find IPs of the computer.
 * @ingroup UTILITIES_MODULE
 */
class IPFinder {
public:
	enum IPTYPE
	{
		IP4,
		IP6
	};
	typedef std::pair<IPTYPE, std::string> pair_IP;
	IPFinder();
	virtual ~IPFinder();
#if defined(_WIN32)
	RTPS_DllAPI static bool getIPs(std::vector<pair_IP>* vec_name);
#else
	static bool getIPs(std::vector<pair_IP>* vec_name);
#endif
	//!Get the IPAdresses in all interfaces.
	RTPS_DllAPI static bool getIP4Address(LocatorList_t* locators);
	RTPS_DllAPI static bool getIP6Address(LocatorList_t* locators);
	RTPS_DllAPI static bool getAllIPAddress(LocatorList_t* locators);
	RTPS_DllAPI static bool parseIP4(std::string& str,Locator_t* loc);
	RTPS_DllAPI static bool parseIP6(std::string& str,Locator_t* loc);
};
}
} /* namespace eprosima */

#endif /* IPFINDER_H_ */
