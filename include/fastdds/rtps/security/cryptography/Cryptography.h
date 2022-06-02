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
 * @file Cryptography.h
 */
#ifndef _FASTDDS_RTPS_SECURITY_CRYPTOGRAPHY_CRYPTOGRAPHY_H_
#define _FASTDDS_RTPS_SECURITY_CRYPTOGRAPHY_CRYPTOGRAPHY_H_

#include <fastdds/rtps/security/cryptography/CryptoKeyExchange.h>
#include <fastdds/rtps/security/cryptography/CryptoKeyFactory.h>
#include <fastdds/rtps/security/cryptography/CryptoTransform.h>
#include <fastdds/rtps/security/cryptography/CryptoTypes.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class Logging;

class Cryptography
{
public:

    Cryptography() = default;
    virtual ~Cryptography() = default;

    /* Specializations should add functions to access the different modules */
    virtual CryptoKeyExchange* cryptokeyexchange() = 0;
    virtual CryptoKeyFactory* cryptokeyfactory() = 0;
    virtual CryptoTransform* cryptotransform() = 0;

    bool set_logger(
            Logging* logger,
            SecurityException& /*exception*/)
    {
        m_logger = logger;
        return true;
    }

protected:

    const Logging* get_logger()
    {
        return m_logger;
    }

private:

    Logging* m_logger;
};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif //_FASTDDS_RTPS_SECURITY_CRYPTOGRAPHY_CRYPTOGRAPHY_H_
