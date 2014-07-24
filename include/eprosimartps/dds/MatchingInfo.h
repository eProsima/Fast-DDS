/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MatchingInfo.h
 *
 */

#ifndef MATCHINGINFO_H_
#define MATCHINGINFO_H_

#include "eprosimartps/common/types/Guid.h"

namespace eprosima{

using namespace rtps;

namespace dds{


/**
 * Enumeration MatchingStatus, indicates whether the matched publication/subscription method of the PublisherListener or SubscriberListener has
 * been called for a matching or a removal of a remote endpoint.
 */
enum MatchingStatus{
	MATCHED_MATCHING,//!< MATCHED_MATCHING, new publisher/subscriber found
	REMOVED_MATCHING //!< REMOVED_MATCHING, publisher/subscriber removed

};

/**
 * Class MatchingInfo contains information about the matching between two endpoints.
 */
class MatchingInfo
{
public:
	MatchingInfo():status(MATCHED_MATCHING){};
	MatchingInfo(MatchingStatus stat,const GUID_t&guid):status(stat),remoteEndpointGuid(guid){};
	~MatchingInfo(){};
	MatchingStatus status;
	GUID_t remoteEndpointGuid;
};

}
}

#endif /* MATCHINGINFO_H_ */
