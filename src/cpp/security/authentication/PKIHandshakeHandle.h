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

#include <openssl/evp.h>
#include <rtps/security/authentication/Handshake.h>
#include <rtps/security/common/SharedSecretHandle.h>
#include <security/authentication/PKIIdentityHandle.h>
#include <string>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class PKIHandshake
{
public:

    PKIHandshake() = default;

    ~PKIHandshake()
    {
        if (dhkeys_ != nullptr)
        {
            EVP_PKEY_free(dhkeys_);
        }

        if (peerkeys_ != nullptr)
        {
            EVP_PKEY_free(peerkeys_);
        }
    }

    static const char* const class_id_;

    std::string kagree_alg_;
    EVP_PKEY* dhkeys_ = { nullptr };
    EVP_PKEY* peerkeys_ = { nullptr };
    const PKIIdentityHandle* local_identity_handle_ = { nullptr };
    PKIIdentityHandle* remote_identity_handle_ = { nullptr };
    HandshakeMessageToken handshake_message_;
    std::shared_ptr<SharedSecretHandle> sharedsecret_;
};

class PKIDH;

typedef HandleImpl<PKIHandshake, PKIDH> PKIHandshakeHandle;

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_PKIHANDSHAKEHANDLE_H_

