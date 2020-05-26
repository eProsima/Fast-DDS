// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//G
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

#include <fastdds/rtps/security/cryptography/Cryptography.h>
#include <fastdds/rtps/attributes/PropertyPolicy.h>

#include <security/cryptography/AESGCMGMAC_KeyExchange.h>
#include <security/cryptography/AESGCMGMAC_KeyFactory.h>
#include <security/cryptography/AESGCMGMAC_Transform.h>


namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class AESGCMGMAC : public Cryptography
{
public:

    AESGCMGMAC();
    ~AESGCMGMAC();

    AESGCMGMAC_KeyExchange* keyexchange();
    AESGCMGMAC_KeyFactory* keyfactory();
    AESGCMGMAC_Transform* cryptotransform();

};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_AESGCMGMAC_H_
