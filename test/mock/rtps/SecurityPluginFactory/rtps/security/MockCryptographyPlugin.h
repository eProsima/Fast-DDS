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
 * @file MockCryptographyPlugin.h
 */

#ifndef _RTPS_SECURITY_MOCKCRYPTOGRAPHYPLUGIN_H_
#define _RTPS_SECURITY_MOCKCRYPTOGRAPHYPLUGIN_H_

#include <fastrtps/rtps/security/cryptography/Cryptography.h>

#include <rtps/security/MockCryptoKeyFactory.h>
#include <rtps/security/MockCryptoKeyExchange.h>
#include <rtps/security/MockCryptoTransform.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class MockCryptographyPlugin : public Cryptography
{
public:

    CryptoKeyExchange* cryptokeyexchange() override
    {
        return &cryptokeyexchange_;
    }

    CryptoKeyFactory* cryptokeyfactory() override
    {
        return &cryptokeyfactory_;
    }

    CryptoTransform* cryptotransform() override
    {
        return &cryptotransform_;
    }

    MockCryptoKeyFactory cryptokeyfactory_;
    MockCryptoKeyExchange cryptokeyexchange_;
    MockCryptoTransform cryptotransform_;
};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _RTPS_SECURITY_MOCKCRYPTOGRAPHYPLUGIN_H_
