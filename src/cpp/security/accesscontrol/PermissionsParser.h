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

#ifndef __SECURITY_ACCESSCONTROL_PERMISSIONSPARSER_H__
#define __SECURITY_ACCESSCONTROL_PERMISSIONSPARSER_H__

#include <security/accesscontrol/CommonParser.h>

#include <tinyxml2.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

struct PermissionsData
{
    std::vector<Grant> grants;
};

class PermissionsParser
{
public:

    bool parse_stream(
            const char* stream,
            size_t stream_length);

    void swap(
            PermissionsData& permissions);

private:

    bool parse_permissions(
            tinyxml2::XMLElement* root);

    bool parse_grant(
            tinyxml2::XMLElement* root,
            Grant& grant);

    bool parse_validity(
            tinyxml2::XMLElement* root,
            Validity& validity);

    bool parse_rule(
            tinyxml2::XMLElement* root,
            Rule& rule);

    bool parse_criteria(
            tinyxml2::XMLElement* root,
            Criteria& criteria);

    bool parse_topic(
            tinyxml2::XMLElement* root,
            std::vector<std::string>& topics);

    bool parse_partition(
            tinyxml2::XMLElement* root,
            std::vector<std::string>& partitions);

    PermissionsData permissions_;
};

} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // __SECURITY_ACCESSCONTROL_PERMISSIONSPARSER_H__
