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
#include <cstring>
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
static void replace_all(
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

static bool do_match_pattern(
        const char* pattern,
        const char* str)
{
    std::string path(pattern);
    std::string spec(str);

    replace_all(pattern, "*", ".*");
    replace_all(pattern, "?", ".");

    std::regex path_regex(path);
    std::smatch spec_match;
    return std::regex_match(spec, spec_match, path_regex);
}

#elif defined(_WIN32)
static bool do_match_pattern(
        const char* pattern,
        const char* str)
{
    // An empty pattern only matches an empty string
    if (strlen(pattern) == 0)
    {
        return strlen(str) == 0;
    }
    // An empty string also matches a pattern of "*"
    if (strlen(str) == 0)
    {
        return strcmp(pattern, "*") == 0;
    }
    // Leave rest of cases to PathMatchSpecA
    return PathMatchSpecA(str, pattern);
}

#else
static bool do_match_pattern(
        const char* pattern,
        const char* str)
{
    return fnmatch(pattern, str, FNM_NOESCAPE) == 0;
}

#endif // if defined(__cplusplus_winrt)

bool StringMatching::matchPattern(
        const char* pattern,
        const char* str)
{
    return do_match_pattern(pattern, str);
}

bool StringMatching::matchString(
        const char* str1,
        const char* str2)
{
    return do_match_pattern(str1, str2) || do_match_pattern(str2, str1);
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
