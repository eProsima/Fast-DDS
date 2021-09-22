// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file Pkcs11Provider.cpp
 */

#include <security/artifact_providers/Pkcs11Provider.hpp>

#include <iostream>

#include <libp11.h>

#include <utils/SystemInfo.hpp>

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION " (" __FILE__ ":" S2(__LINE__) ")"
#define _SecurityException_(str) SecurityException(std::string(str) + LOCATION)


namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {
namespace detail {

constexpr const char* FASTDDS_PKCS11_PIN = "FASTDDS_PKCS11_PIN";
constexpr const char* PKCS11_ENGINE_ID = "pkcs11";

Pkcs11Provider::Pkcs11Provider()
{
    SSL_load_error_strings();                /* readable error messages */
    SSL_library_init();                      /* initialize library */

    ENGINE_load_builtin_engines();
    pkcs11_ = ENGINE_by_id(PKCS11_ENGINE_ID);
    if (!pkcs11_)
    {
        has_initialization_error_ = true;
        initialization_exception_ = _SecurityException_(std::string("Error retrieving 'pkcs11' engine"));
    }

    const char* pin;
    if (ReturnCode_t::RETCODE_OK == SystemInfo::get_env(FASTDDS_PKCS11_PIN, &pin))
    {
        if (!ENGINE_ctrl_cmd_string( pkcs11_, "PIN", pin, 0))
        {
            has_initialization_error_ = true;
            initialization_exception_ = _SecurityException_(std::string("Error setting the PIN in the 'pkcs11' engine"));
            ENGINE_free(pkcs11_);
        }
    }

    if(!ENGINE_init(pkcs11_))
    {
        has_initialization_error_ = true;
        initialization_exception_ = _SecurityException_(std::string("Error initializeing the HSM provider library"));
        ENGINE_free(pkcs11_);
    }
}

Pkcs11Provider::~Pkcs11Provider()
{
    ENGINE_finish(pkcs11_);
    ENGINE_free(pkcs11_);
}

EVP_PKEY* Pkcs11Provider::load_private_key(
        X509* certificate,
        const std::string& pkey,
        const std::string& password,
        SecurityException& exception)
{
    return Pkcs11Provider::instance().load_private_key_impl(certificate, pkey, password, exception);
}

EVP_PKEY* Pkcs11Provider::load_private_key_impl(
        X509* certificate,
        const std::string& pkey,
        const std::string& /*password*/,
        SecurityException& exception)
{
    std::cerr << "We have PKCS11 key: " << pkey << std::endl;

    if (has_initialization_error_)
    {
        exception = initialization_exception_;
        return nullptr;
    }

    EVP_PKEY* returnedValue = ENGINE_load_private_key(pkcs11_, pkey.c_str(), NULL, NULL);
    if (!returnedValue)
    {
        exception = _SecurityException_(std::string("Error opening the private key ") + pkey.substr(7));
        return returnedValue;
    }

    // Verify private key.
    if (!X509_check_private_key(certificate, returnedValue))
    {
        exception = _SecurityException_(std::string("Error verifying private key ") + pkey.substr(7)
                + "\n ERROR: " + ERR_error_string(ERR_get_error(), nullptr));
        EVP_PKEY_free(returnedValue);
        returnedValue = nullptr;
    }

    return returnedValue;
}

} // namespace detail
} // namespace security
} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

