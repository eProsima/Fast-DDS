// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file StringMatching.cpp
 *
 */

#include <fastrtps/utils/StringMatching.h>

#if defined(_WIN32)
#include "Shlwapi.h"
#else
#include <fnmatch.h>
#endif

namespace eprosima {
namespace fastrtps{
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

}
} /* namespace rtps */
} /* namespace eprosima */
