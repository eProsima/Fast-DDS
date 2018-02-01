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

#include "CommonParser.h"

#include <fastrtps/log/Log.h>

#include <cassert>

static const char* DomainId_str = "id";
static const char* DomainIdRange_str = "id_range";

using namespace eprosima::fastrtps;
using namespace ::rtps::security;

bool eprosima::fastrtps::rtps::security::parse_domain_id_set(tinyxml2::XMLElement* root, Domains& domains)
{
    assert(root);

    bool returned_value = false;
    tinyxml2::XMLElement* node = root->FirstChildElement();

    if(node != nullptr)
    {
        returned_value = true;

        do
        {
            if(strcmp(node->Name(), DomainId_str) == 0)
            {
                uint32_t domain_id = 0;

                if(tinyxml2::XMLError::XML_SUCCESS == node->QueryUnsignedText(&domain_id))
                {
                    domains.ids.push_back(domain_id);
                }
                else
                {
                    logError(XMLPARSER, "Invalid value of " << DomainId_str <<
                            " tag. Line " << node->GetLineNum());
                    returned_value = false;
                }
            }
            else if(strcmp(node->Name() ,DomainIdRange_str) == 0)
            {
            }
            else
            {
                logError(XMLPARSER, "Not valid tag. Expected " << DomainId_str << " or " << DomainIdRange_str <<
                        " tag. Line " << node->GetLineNum());
                returned_value = false;
            }
        }
        while(returned_value && (node = node->NextSiblingElement()) != nullptr);
    }
    else
    {
        logError(XMLPARSER, "Minimum one " << DomainId_str << " or " << DomainIdRange_str << " tag. Line " <<
                root->GetLineNum() + 1);
    }

    return returned_value;
}

