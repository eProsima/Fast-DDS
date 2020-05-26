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
 * @file PKIHandshakeHandle.h
 */
#ifndef _SECURITY_AUTHENTICATION_PKIHANDSHAKEHANDLE_H_
#define _SECURITY_AUTHENTICATION_PKIHANDSHAKEHANDLE_H_

#include <security/authentication/PKIIdentityHandle.h>
#include <fastdds/rtps/security/authentication/Handshake.h>
#include <fastdds/rtps/security/common/SharedSecretHandle.h>
#include <openssl/evp.h>
#include <string>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class PKIHandshake
{
    public:

        PKIHandshake() : dhkeys_(nullptr), peerkeys_(nullptr),
        local_identity_handle_(nullptr), remote_identity_handle_(nullptr),
        sharedsecret_(nullptr) {}

        ~PKIHandshake()
        {
            if(dhkeys_ != nullptr)
            {
                EVP_PKEY_free(dhkeys_);
            }

            if(peerkeys_ != nullptr)
            {
                EVP_PKEY_free(peerkeys_);
            }

            if(sharedsecret_ != nullptr)
            {
                delete sharedsecret_;
            }
        }


        static const char* const class_id_;

        std::string kagree_alg_;
        EVP_PKEY* dhkeys_;
        EVP_PKEY* peerkeys_;
        const PKIIdentityHandle* local_identity_handle_;
        PKIIdentityHandle* remote_identity_handle_;
        HandshakeMessageToken handshake_message_;
        SharedSecretHandle* sharedsecret_;
};

typedef HandleImpl<PKIHandshake> PKIHandshakeHandle;

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_PKIHANDSHAKEHANDLE_H_

