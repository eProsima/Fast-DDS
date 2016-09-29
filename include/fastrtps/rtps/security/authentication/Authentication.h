// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Authentication.h	
 */
#ifndef _RTPS_SECURITY_AUTHENTICATION_AUTHENTICATION_H_
#define _RTPS_SECURITY_AUTHENTICATION_AUTHENTICATION_H_

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

enum ValidationResult_t : uint32_t
{
    VALIDATION_OK = 0,
    VALIDATION_FAILED,
    VALIDATION_PENDING_RETRY,
    VALIDATION_PENDING_HANDSHAKE_REQUEST,
    VALIDATION_PENDING_HANDSHAKE_MESSAGE,
    VALIDATION_OK_FINAL_MESSAGE
};

class Authentication
{
    public:

        virtual ValidationResult_t validate_local_identity(const IdentityHandle& identity_handle) = 0;
};

} //namespace eprosima
} //namespace fastrtps
} //namespace rtps
} //namespace security

#endif //  _RTPS_SECURITY_AUTHENTICATION_AUTHENTICATION_H_
