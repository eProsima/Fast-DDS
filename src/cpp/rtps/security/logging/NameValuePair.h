// Copyright 2020 Canonical ltd.
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
 * @file NameValuePair.h
 */
#ifndef _FASTDDS_RTPS_SECURITY_LOGGING_NAMEVALUEPAIR_H_
#define _FASTDDS_RTPS_SECURITY_LOGGING_NAMEVALUEPAIR_H_

#include <string>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

/**
 * @brief The NameValuePair struct
 *
 * @note Definition in DDS-Sec v1.1 9.6
 */
struct NameValuePair final
{
    std::string name;
    std::string value;
};

using NameValuePairSeq = std::vector<NameValuePair>;

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _FASTDDS_RTPS_SECURITY_LOGGING_NAMEVALUEPAIR_H_
