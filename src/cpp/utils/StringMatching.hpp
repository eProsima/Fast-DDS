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

/*!
 * @file StringMatching.hpp
 *
 */

#ifndef _FASTDDS_STRINGMATCHING_H_
#define _FASTDDS_STRINGMATCHING_H_

#include <fastdds/fastdds_dll.hpp>

#include <cstdint>
#include <string>
#include <vector>

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
namespace eprosima {
namespace fastdds {
namespace rtps {
/**
 * Class StringMatching used to match different strings against each other as defined by the POSIX fnmatch API (1003.2-1992
   section B.6).
   @ingroup UTILITIES_MODULE
 */
class FASTDDS_EXPORTED_API StringMatching
{
public:

    StringMatching();

    virtual ~StringMatching();

    /** Static method to match two strings.
     * It checks if the input strings match. Any of the strings or both can be a pattern.
     */
    static bool matchString(
            const char* input1,
            const char* input2);

    /** Static method to match a string to a pattern.
     * It checks the string specified by the input argument to see if it matches the pattern specified by the pattern argument.
     */
    static bool matchPattern(
            const char* pattern,
            const char* input);
};

} /*/ namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // _FASTDDS_STRINGMATCHING_H_
