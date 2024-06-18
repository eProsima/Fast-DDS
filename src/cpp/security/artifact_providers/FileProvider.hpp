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
 * @file FileProvider.hpp
 */

#ifndef _SECURITY_ARTIFACTPROVIDERS_FILEPROVIDER_HPP_
#define _SECURITY_ARTIFACTPROVIDERS_FILEPROVIDER_HPP_

#include <functional>

#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <rtps/security/exceptions/SecurityException.h>


namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {
namespace detail {

class FileProvider
{

public:

    static X509_STORE* load_ca(
            const std::string& ca,
            bool& there_are_crls,
            std::string& ca_sn,
            std::string& ca_algo,
            std::function<bool(X509*, std::string&, SecurityException&)> get_signature_algorithm,
            SecurityException& exception);

    static EVP_PKEY* load_private_key(
            X509* certificate,
            const std::string& file,
            const std::string& password,
            SecurityException& exception);

    static X509* load_certificate(
            const std::string& identity_cert,
            SecurityException& exception);

    static X509_CRL* load_crl(
            const std::string& identity_crl,
            SecurityException& exception);

};

} // namespace detail
} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif  // _SECURITY_ARTIFACTPROVIDERS_FILEPROVIDER_HPP_
