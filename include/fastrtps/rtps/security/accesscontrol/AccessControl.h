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
 * @file AccessControl.h
 */
#ifndef __RTPS_SECURITY_ACCESSCONTROL_ACCESSCONTROL_H__
#define __RTPS_SECURITY_ACCESSCONTROL_ACCESSCONTROL_H__

#include "../common/Handle.h"

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipantAttributes;

namespace security {

class Authentication;
class SecurityException;

class AccessControl
{
    public:

        virtual ~AccessControl() = default;

        virtual PermissionsHandle* validate_local_permissions(Authentication& auth_plugin,
                const IdentityHandle& identity,
                const uint32_t domain_id,
                const RTPSParticipantAttributes& participant_attr,
                SecurityException& exception) = 0;
};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // __RTPS_SECURITY_ACCESSCONTROL_ACCESSCONTROL_H__
