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
 * @file AESGCMGMAC.cpp
 */

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <security/cryptography/AESGCMGMAC.h>

using namespace eprosima::fastrtps::rtps::security;

AESGCMGMAC::AESGCMGMAC()
{
    m_cryptokeyexchange = new AESGCMGMAC_KeyExchange();
    m_cryptokeyfactory = new AESGCMGMAC_KeyFactory();
    m_cryptotransform = new AESGCMGMAC_Transform();

    // Seed prng
    RAND_load_file("/dev/urandom", 32);
}

AESGCMGMAC_KeyExchange* AESGCMGMAC::keyexchange(){

    return (AESGCMGMAC_KeyExchange*) m_cryptokeyexchange;
}

AESGCMGMAC_KeyFactory* AESGCMGMAC::keyfactory(){

    return (AESGCMGMAC_KeyFactory*) m_cryptokeyfactory;
}

AESGCMGMAC_Transform* AESGCMGMAC::cryptotransform(){

    return (AESGCMGMAC_Transform*) m_cryptotransform;
}

AESGCMGMAC::~AESGCMGMAC(){

    delete m_cryptokeyexchange;
    delete m_cryptokeyfactory;
    delete m_cryptotransform;

}

