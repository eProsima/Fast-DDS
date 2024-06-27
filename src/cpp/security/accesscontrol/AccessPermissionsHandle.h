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
 * @file AccessPermissionsHandle.h
 */
#ifndef __SECURITY_ACCESSCONTROL_ACCESSPERMISSIONSHANDLE_H__
#define __SECURITY_ACCESSCONTROL_ACCESSPERMISSIONSHANDLE_H__

#include <fastdds/rtps/common/Token.hpp>
#include <fastdds/rtps/attributes/EndpointSecurityAttributes.hpp>
#include <rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#include <rtps/security/common/Handle.h>
#include <security/accesscontrol/PermissionsTypes.h>

#include <openssl/x509.h>
#include <string>
#include <map>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class AccessPermissions
{
public:

    AccessPermissions()
        : store_(nullptr)
        , there_are_crls_(false)
    {
    }

    ~AccessPermissions()
    {
        if (store_ != nullptr)
        {
            X509_STORE_free(store_);
        }
    }

    static const char* const class_id_;

    X509_STORE* store_;
    std::string sn;
    std::string algo;
    bool there_are_crls_;
    PermissionsToken permissions_token_;
    PermissionsCredentialToken permissions_credential_token_;
    ParticipantSecurityAttributes governance_rule_;
    std::vector<std::pair<std::string, EndpointSecurityAttributes>> governance_topic_rules_;
    Grant grant;
};

class Permissions;

typedef HandleImpl<AccessPermissions, Permissions> AccessPermissionsHandle;

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // __SECURITY_ACCESSCONTROL_ACCESSPERMISSIONSHANDLE_H__
