/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StringMatching.cpp
 *
 */

#include "eprosimartps/utils/StringMatching.h"

#if defined(_WIN32)
#include "Shlwapi.h"
#else
#include <fnmatch.h>
#endif

namespace eprosima {
namespace rtps {

StringMatching::StringMatching() {
	// TODO Auto-generated constructor stub

}

StringMatching::~StringMatching() {
	// TODO Auto-generated destructor stub
}

#if defined(_WIN32)
bool StringMatching::matchString(const char* str1, const char* str2)
{
	 if(PathMatchSpec(str1,str2))
		 return true;
	 if(PathMatchSpec(str2,str1))
	 	 return true;
	 return false;
}

#else
bool StringMatching::matchString(const char* str1, const char* str2)
{
	if(fnmatch(str1,str2,FNM_PATHNAME)==0)
		return true;
	if(fnmatch(str2,str1,FNM_PATHNAME)==0)
		return true;
	return false;
}

#endif


} /* namespace rtps */
} /* namespace eprosima */
