// Copyright 2088 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Permissions.h
 */

#ifndef _SECURITY_ACCESSCONTROL_PERMISSIONS_H_
#define _SECURITY_ACCESSCONTROL_PERMISSIONS_H_

#include <fastrtps/rtps/security/accesscontrol/AccessControl.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class Permissions : public AccessControl
{
    public:

        virtual ~Permissions() = default;

        PermissionsHandle* validate_local_permissions(Authentication& auth_plugin,
                const IdentityHandle& identity,
                const uint32_t domain_id,
                const RTPSParticipantAttributes& participant_attr,
                SecurityException& exception);
};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _SECURITY_ACCESSCONTROL_PERMISSIONS_H_
