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
#include <limits.h>
#include <errno.h>

#if defined(__cplusplus_winrt)
#include <algorithm>
#include <regex>
#elif defined(_WIN32)
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

#if defined(__cplusplus_winrt)
void replace_all(std::string & subject, const std::string & search, const std::string & replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

bool StringMatching::matchString(const char* str1, const char* str2)
{
	std::string path(str1);
	std::string spec(str2);

	std::string base_path(path);
	std::string base_spec(spec);

	replace_all(base_spec, "*", ".*");
	replace_all(base_spec, "?", ".");

	std::regex base_spec_regex(base_spec);
	std::smatch base_spec_match;
	if (std::regex_match(path, base_spec_match, base_spec_regex))
	{
		return true;
	}

	replace_all(base_path, "*", ".*");
	replace_all(base_path, "?", ".");

	std::regex base_path_regex(base_path);
	std::smatch base_path_match;

	if (std::regex_match(spec, base_path_match, base_path_regex))
	{
		return true;
	}

	return false;
}

#elif defined(_WIN32)
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
	if(fnmatch(str1,str2,FNM_NOESCAPE)==0)
		return true;
	if(fnmatch(str2,str1,FNM_NOESCAPE)==0)
		return true;
	return false;
}

#endif

std::vector<std::string> StringMatching::split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

bool StringMatching::readUint32(uint32_t& n, std::string const& str, int base)
{
    char *endp;
    unsigned long value = strtoul(str.c_str(), &endp, base);
    if ( (endp == str.c_str()) || (value == ULONG_MAX && errno == ERANGE) ) 
    {
        return false;
    }

    n = value;
    return true;
}

}
} /* namespace rtps */
} /* namespace eprosima */
