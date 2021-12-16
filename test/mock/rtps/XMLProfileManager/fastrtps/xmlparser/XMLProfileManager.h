// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
//
#ifndef XML_PROFILE_MANAGER_H_
#define XML_PROFILE_MANAGER_H_

#include <fastrtps/xmlparser/XMLParser.h>

namespace eprosima {
namespace fastrtps {
namespace xmlparser {

class XMLProfileManager
{
public:

    static sp_transport_t getTransportById(
            const std::string&)
    {
        return sp_transport_t();
    }

};

} // namespace xmlparser
} // namespace fastrtps
} // namespace eprosima

#endif // XML_PROFILE_MANAGER_H_
