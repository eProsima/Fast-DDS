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

#include "PKIIdentityHandle.h"
#include <fastrtps/rtps/security/authentication/Handshake.h>
#include <fastrtps/rtps/security/common/SharedSecretHandle.h>

#ifdef LIBDDSSEC_ENABLED
#include <dsec_hh.h>
#include <fastrtps/rtps/security/common/TEE.h>
#include <fastrtps/log/Log.h>
#else
#include <openssl/evp.h>
#endif

#include <string>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class PKIHandshake
{
    public:

#ifdef LIBDDSSEC_ENABLED

        PKIHandshake() : hh_id(-1), local_identity_handle_(nullptr),
        remote_identity_handle_(nullptr), sharedsecret_(nullptr)
        {
            int32_t libddssec_success;
            libddssec_success = dsec_hh_create(&hh_id, &(tee.instance));
            if (libddssec_success != 0) {
                logWarning(SECURITY_AUTHENTICATION, "Could not create handshake handle.");
            }
        }

        ~PKIHandshake()
        {
            dsec_hh_delete(&(tee.instance), hh_id);

            if(sharedsecret_ != nullptr)
            {
                delete sharedsecret_;
            }
        }

        int hh_id;

#else

        PKIHandshake() : dhkeys_(nullptr), peerkeys_(nullptr),
        local_identity_handle_(nullptr), remote_identity_handle_(nullptr),
        sharedsecret_(nullptr)
        {}

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

        EVP_PKEY* dhkeys_;
        EVP_PKEY* peerkeys_;

#endif

        static const char* const class_id_;

        std::string kagree_alg_;

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
