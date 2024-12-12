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
 * @file PKIIdentityHandle.h
 */
#ifndef _SECURITY_AUTHENTICATION_PKIIDENTITYHANDLE_H_
#define _SECURITY_AUTHENTICATION_PKIIDENTITYHANDLE_H_

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/Token.hpp>
#include <rtps/security/common/Handle.h>

#include <openssl/x509.h>
#include <string>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

static const char* const RSA_SHA256 = "RSASSA-PSS-SHA256";
static const char* const ECDSA_SHA256 = "ECDSA-SHA256";

static const char* const RSA_SHA256_FOR_TOKENS = "RSA-2048";
static const char* const ECDSA_SHA256_FOR_TOKENS = "EC-prime256v1";

static const char* const DH_2048_256 = "DH+MODP-2048-256";
static const char* const ECDH_prime256v1 = "ECDH+prime256v1-CEUM";

class PKIIdentity
{
public:

    PKIIdentity()
        : store_(nullptr)
        , cert_(nullptr)
        , pkey_(nullptr)
        , cert_content_(nullptr)
        , kagree_alg_(DH_2048_256)
        , there_are_crls_(false)
    {
    }

    ~PKIIdentity()
    {
        if (store_ != nullptr)
        {
            X509_STORE_free(store_);
        }

        if (cert_ != nullptr)
        {
            X509_free(cert_);
        }

        if (pkey_ != nullptr)
        {
            EVP_PKEY_free(pkey_);
        }

        if (cert_content_ != nullptr)
        {
            BUF_MEM_free(cert_content_);
        }
    }

    static const char* const class_id_;

    X509_STORE* store_;
    X509* cert_;
    EVP_PKEY* pkey_;
    GUID_t participant_key_;
    BUF_MEM* cert_content_;
    std::string sn;
    std::string algo;
    std::string sign_alg_;
    std::string kagree_alg_;
    std::string cert_sn_;
    std::string cert_sn_rfc2253_;
    bool there_are_crls_;
    IdentityToken identity_token_;
    PermissionsCredentialToken permissions_credential_token_;
};

class PKIDH;

typedef HandleImpl<PKIIdentity, PKIDH> PKIIdentityHandle;

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_PKIIDENTITYHANDLE_H_
