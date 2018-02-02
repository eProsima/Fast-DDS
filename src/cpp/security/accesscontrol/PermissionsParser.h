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

#include "CommonParser.h"

#include <tinyxml2.h>
#include <string>
#include <time.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

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
    Validity()
    {
        memset(&not_before, 0, sizeof(struct tm));
        memset(&not_after, 0, sizeof(struct tm));
    }

    struct tm not_before;
    struct tm not_after;
};

struct Grant
{
    std::string name;
    std::string subject_name;
    Validity validity;
    std::vector<Rule> rules;
};

struct PermissionsData
{
    std::vector<Grant> grants;
};

class PermissionsParser
{
    public:

        bool parse_stream(const char* stream, size_t stream_length);

        void swap(PermissionsData& permissions);

    private:

        bool parse_permissions(tinyxml2::XMLElement* root);

        bool parse_grant(tinyxml2::XMLElement* root, Grant& grant);

        bool parse_validity(tinyxml2::XMLElement* root, Validity& validity);

        bool parse_rule(tinyxml2::XMLElement* root, Rule& rule);

        bool parse_criteria(tinyxml2::XMLElement* root, Criteria& criteria);

        bool parse_topic(tinyxml2::XMLElement* root, std::string& topic);

        PermissionsData permissions_;
};

}
}
}
}

#endif // __SECURITY_ACCESSCONTROL_PERMISSIONSPARSER_H__
