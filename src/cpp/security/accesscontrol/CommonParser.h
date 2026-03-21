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

#ifndef __SECURITY_ACCESSCONTROL_COMMON_H__
#define __SECURITY_ACCESSCONTROL_COMMON_H__

#include <security/accesscontrol/PermissionsTypes.h>

#include <tinyxml2.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

bool parse_domain_id_set(
        tinyxml2::XMLElement* root,
        Domains& domains);

} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // __SECURITY_ACCESSCONTROL_COMMON_H__
