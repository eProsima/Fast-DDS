/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StringMatching.h
 *
 */

#ifndef STRINGMATCHING_H_
#define STRINGMATCHING_H_

namespace eprosima {
namespace rtps {

class StringMatching {
public:
	StringMatching();
	virtual ~StringMatching();
	static bool matchString(const char* pattern,const char* input);
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STRINGMATCHING_H_ */
