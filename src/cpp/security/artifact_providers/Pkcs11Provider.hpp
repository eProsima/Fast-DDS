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
 * @file Pkcs11Provider.hpp
 */

#ifndef _SECURITY_ARTIFACTPROVIDERS_PKCS11PROVIDER_HPP_
#define _SECURITY_ARTIFACTPROVIDERS_PKCS11PROVIDER_HPP_

#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <rtps/security/exceptions/SecurityException.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {
namespace detail {

class Pkcs11Provider
{

public:

    EVP_PKEY* load_private_key(
            X509* certificate,
            const std::string& file,
            const std::string& password,
            SecurityException& exception);

    Pkcs11Provider();

    ~Pkcs11Provider();

private:

    EVP_PKEY* load_private_key_impl(
            X509* certificate,
            const std::string& file,
            const std::string& password,
            SecurityException& exception);

    SecurityException initialization_exception_;
    bool has_initialization_error_ = false;
    ENGINE* pkcs11_ = nullptr;
    UI_METHOD* ui_method_ = nullptr;
};

} // namespace detail
} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif  // _SECURITY_ARTIFACTPROVIDERS_PKCS11PROVIDER_HPP_
