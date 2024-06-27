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
 * @file AESGCMGMAC.h
 */

#ifndef _SECURITY_AUTHENTICATION_AESGCMGMAC_H_
#define _SECURITY_AUTHENTICATION_AESGCMGMAC_H_


#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <rtps/security/cryptography/Cryptography.h>
#include <security/cryptography/AESGCMGMAC_KeyExchange.h>
#include <security/cryptography/AESGCMGMAC_KeyFactory.h>
#include <security/cryptography/AESGCMGMAC_Transform.h>

#include <memory>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class AESGCMGMAC : public Cryptography
{
    CryptoKeyExchange* m_cryptokeyexchange;
    std::shared_ptr<AESGCMGMAC_KeyFactory> m_cryptokeyfactory;
    CryptoTransform* m_cryptotransform;

public:

    AESGCMGMAC();
    ~AESGCMGMAC();

    CryptoKeyExchange* cryptokeyexchange() override
    {
        return keyexchange();
    }

    CryptoKeyFactory* cryptokeyfactory() override
    {
        return keyfactory().get();
    }

    CryptoTransform* cryptotransform() override
    {
        return transform();
    }

    AESGCMGMAC_KeyExchange* keyexchange();
    std::shared_ptr<AESGCMGMAC_KeyFactory> keyfactory();
    AESGCMGMAC_Transform* transform();
};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_AESGCMGMAC_H_
