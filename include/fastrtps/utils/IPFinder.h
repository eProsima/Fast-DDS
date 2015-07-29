/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
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

#include "../rtps/common/Locator.h"

namespace eprosima {
namespace fastrtps{
using namespace rtps;
/**
 * Class IPFinder, to determine the IP of the NICs.
 * @ingroup UTILITIES_MODULE
 */
class IPFinder {
public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
	/**
	 * Enum IPTYPE, to define the type of IP obtained from the NICs.
	 */
	enum IPTYPE
	{
		IP4,      //!< IP4
		IP6,      //!< IP6
		IP4_LOCAL,//!< IP4_LOCAL
		IP6_LOCAL //!< IP6_LOCAL
	};
	/**
	 * Structure info_IP with information about a specific IP obtained from a NIC.
	 */
	typedef struct info_IP
	{
		IPTYPE type;
		uint32_t scope_id;
		std::string name;
		Locator_t locator;
	}info_IP;
#endif
	IPFinder();
	virtual ~IPFinder();
#if defined(_WIN32)
	RTPS_DllAPI static bool getIPs(std::vector<info_IP>* vec_name);
#else
	static bool getIPs(std::vector<info_IP>* vec_name);
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
