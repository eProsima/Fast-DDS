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

#include <utils/StringMatching.hpp>
#include <limits.h>
#include <errno.h>

#if defined(__cplusplus_winrt)
#include <algorithm>
#include <regex>
#elif defined(_WIN32)
#include "Shlwapi.h"
#else
#include <fnmatch.h>
#endif // if defined(__cplusplus_winrt)

namespace eprosima {
namespace fastdds {
namespace rtps {

StringMatching::StringMatching()
{
    // TODO Auto-generated constructor stub

}

StringMatching::~StringMatching()
{
    // TODO Auto-generated destructor stub
}

#if defined(__cplusplus_winrt)
void replace_all(
        std::string& subject,
        const std::string& search,
        const std::string& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

bool StringMatching::matchPattern(
        const char* pattern,
        const char* str)
{
    std::string path(pattern);
    std::string spec(str);

    replace_all(pattern, "*", ".*");
    replace_all(pattern, "?", ".");

    std::regex path_regex(path);
    std::smatch spec_match;
    if (std::regex_match(spec, spec_match, path_regex))
    {
        return true;
    }

    return false;
}

bool StringMatching::matchString(
        const char* str1,
        const char* str2)
{
    if (StringMatching::matchPattern(str1, str2))
    {
        return true;
    }

    if (StringMatching::matchPattern(str2, str1))
    {
        return true;
    }

    return false;
}

#elif defined(_WIN32)
bool StringMatching::matchPattern(
        const char* pattern,
        const char* str)
{
    if (PathMatchSpecA(str, pattern))
    {
        return true;
    }
    return false;
}

bool StringMatching::matchString(
        const char* str1,
        const char* str2)
{
    if (PathMatchSpecA(str1, str2))
    {
        return true;
    }
    if (PathMatchSpecA(str2, str1))
    {
        return true;
    }
    return false;
}

#else
bool StringMatching::matchPattern(
        const char* pattern,
        const char* str)
{
    if (fnmatch(pattern, str, FNM_NOESCAPE) == 0)
    {
        return true;
    }
    return false;
}

bool StringMatching::matchString(
        const char* str1,
        const char* str2)
{
    if (fnmatch(str1, str2, FNM_NOESCAPE) == 0)
    {
        return true;
    }
    if (fnmatch(str2, str1, FNM_NOESCAPE) == 0)
    {
        return true;
    }
    return false;
}

#endif // if defined(__cplusplus_winrt)

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
