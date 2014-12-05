/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MatchingInfo.h
 *
 */

#ifndef MATCHINGINFO_H_
#define MATCHINGINFO_H_

#include "fastrtps/rtps/common/Guid.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
 * @enum MatchingStatus, indicates whether the matched publication/subscription method of the PublisherListener or SubscriberListener has
 * been called for a matching or a removal of a remote endpoint.
 * @ingroup COMMON_MODULE
 */
#if defined(_WIN32)
	enum RTPS_DllAPI MatchingStatus{
#else
		enum MatchingStatus{
#endif
	MATCHED_MATCHING,//!< MATCHED_MATCHING, new publisher/subscriber found
	REMOVED_MATCHING //!< REMOVED_MATCHING, publisher/subscriber removed

};

/**
 * Class MatchingInfo contains information about the matching between two endpoints.
 * @ingroup COMMON_MODULE
 */
class RTPS_DllAPI MatchingInfo
{
public:
	//!Default constructor
	MatchingInfo():status(MATCHED_MATCHING){};
	/**
	* @param stat Status
	* @param guid GUID
	*/
	MatchingInfo(MatchingStatus stat,const GUID_t&guid):status(stat),remoteEndpointGuid(guid){};
	~MatchingInfo(){};
	//!Status
	MatchingStatus status;
	//!Remote endpoint GUID
	GUID_t remoteEndpointGuid;
};
}
}
}

#endif /* MATCHINGINFO_H_ */
