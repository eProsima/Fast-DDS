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

#include <fastrtps/rtps/security/common/Handle.h>
#include <fastrtps/rtps/common/Guid.h>
#include <openssl/x509.h>
#include <string>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class PKIIdentity
{
    public:

        PKIIdentity() : store_(nullptr),
        cert_(nullptr), cert_content_(nullptr),
        kagree_alg_("DH+MODP-2048-256")
        {}

        ~PKIIdentity()
        {
            if(store_ != nullptr)
                X509_STORE_free(store_);

            if(cert_ != nullptr)
                X509_free(cert_);

            if(cert_content_ != nullptr)
                BUF_MEM_free(cert_content_);
        }


        static const char* const class_id_;

        X509_STORE* store_;
        X509* cert_;
        GUID_t participant_key_;
        BUF_MEM* cert_content_;
        std::string sign_alg_;
        std::string kagree_alg_;
};

typedef HandleImpl<PKIIdentity> PKIIdentityHandle;

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_PKIIDENTITYHANDLE_H_
