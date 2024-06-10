// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PermissionesTypes.h
 */
#ifndef __SECURITY_ACCESSCONTROL_PERMISSIONSTYPES_H__
#define __SECURITY_ACCESSCONTROL_PERMISSIONSTYPES_H__

#include <vector>
#include <string>
#include <cstdint>
#include <ctime>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

struct Domains
{
    std::vector<std::pair<uint32_t, uint32_t>> ranges;
};

struct Criteria
{
    std::vector<std::string> topics;
    std::vector<std::string> partitions;
};

struct Rule
{
    bool allow;
    Domains domains;
    std::vector<Criteria> publishes;
    std::vector<Criteria> subscribes;
    std::vector<Criteria> relays;
};

struct Validity
{
    std::time_t not_before;
    std::time_t not_after;
};

struct Grant
{
    std::string name;
    std::string subject_name;
    Validity validity;
    std::vector<Rule> rules;
    bool is_default_allow;
};

} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // __SECURITY_ACCESSCONTROL_PERMISSIONSTYPES_H__
