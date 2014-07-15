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



enum MatchingStatus{
	MATCHED_MATCHING,
	REMOVED_MATCHING

};


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
