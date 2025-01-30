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
 * @file FileProvider.cpp
 */

#include <security/artifact_providers/FileProvider.hpp>

#include <cassert>
#include <cstring>
#include <iostream>

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION " (" __FILE__ ":" S2(__LINE__) ")"
#define _SecurityException_(str) SecurityException(std::string(str) + LOCATION)


namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {
namespace detail {

X509_STORE* FileProvider::load_ca(
        const std::string& ca,
        bool& there_are_crls,
        std::string& ca_sn,
        std::string& ca_algo,
        std::function<bool(X509*, std::string&, SecurityException&)> get_signature_algorithm,
        SecurityException& exception)
{
    X509_STORE* store = X509_STORE_new();

    if (store != nullptr)
    {
        BIO* in = BIO_new(BIO_s_file());

        if (in != nullptr)
        {
            if (BIO_read_filename(in, ca.substr(7).c_str()) > 0)
            {
                STACK_OF(X509_INFO) * inf = PEM_X509_INFO_read_bio(in, NULL, NULL, NULL);
                X509* ca_cert = nullptr;

                if (inf != nullptr)
                {
                    int i, count = 0;
                    there_are_crls = false;

                    for (i = 0; i < sk_X509_INFO_num(inf); i++)
                    {
                        X509_INFO* itmp = sk_X509_INFO_value(inf, i);

                        if (itmp->x509)
                        {
                            if (nullptr == ca_cert)
                            {
                                ca_cert = itmp->x509;
                            }

                            // Retrieve subject name for future use.
                            if (ca_sn.empty())
                            {
                                X509_NAME* ca_subject_name = X509_get_subject_name(itmp->x509);
                                assert(ca_subject_name != nullptr);
                                char* ca_subject_name_str = X509_NAME_oneline(ca_subject_name, 0, 0);
                                assert(ca_subject_name_str != nullptr);
                                ca_sn = ca_subject_name_str;
                                OPENSSL_free(ca_subject_name_str);
                            }

                            // Retrieve signature algorithm
                            if (ca_algo.empty())
                            {
                                if (get_signature_algorithm(itmp->x509, ca_algo, exception))
                                {
                                    X509_STORE_add_cert(store, itmp->x509);
                                    count++;
                                }
                            }
                            else
                            {
                                X509_STORE_add_cert(store, itmp->x509);
                                count++;
                            }
                        }
                        if (itmp->crl)
                        {
                            X509_STORE_add_crl(store, itmp->crl);
                            there_are_crls = true;
                        }
                    }

                    sk_X509_INFO_pop_free(inf, X509_INFO_free);

                    if (count > 0)
                    {
                        // Verify CA certificate.
                        unsigned long flags = 0;
                        flags |= X509_V_FLAG_CHECK_SS_SIGNATURE | X509_V_FLAG_POLICY_CHECK;
                        flags |= X509_V_FLAG_X509_STRICT;
                        X509_STORE_CTX* ctx = X509_STORE_CTX_new();
                        if (nullptr != ctx)
                        {
                            X509_STORE_CTX_init(ctx, store, ca_cert, NULL);
                            X509_STORE_CTX_set_flags(ctx, flags);
                            if (X509_verify_cert(ctx) == 1)
                            {
                                X509_STORE_CTX_free(ctx);
                                BIO_free(in);
                                return store;
                            }

                            int error_code = X509_STORE_CTX_get_error(ctx);
                            const char* error_msg = X509_verify_cert_error_string(error_code);

                            exception = _SecurityException_(
                                    "Error '" + std::to_string(error_code) + "' verifying CA certificate for " +
                                    ca_sn + ": " + error_msg);
                            X509_STORE_CTX_free(ctx);
                        }
                        else
                        {
                            exception = _SecurityException_("Error creating X509 store context");
                        }
                    }
                }
                else
                {
                    exception = _SecurityException_(std::string(
                                        "OpenSSL library cannot read X509 info in file ") + ca.substr(7));
                }
            }
            else
            {
                exception = _SecurityException_(std::string(
                                    "OpenSSL library cannot read file ") + ca.substr(7));
            }

            BIO_free(in);
        }
        else
        {
            exception = _SecurityException_("OpenSSL library cannot allocate file");
        }

        X509_STORE_free(store);
    }
    else
    {
        exception = _SecurityException_("Creation of X509 storage");
    }

    return nullptr;
}

X509* FileProvider::load_certificate(
        const std::string& identity_cert,
        SecurityException& exception)
{
    X509* returnedValue = nullptr;
    BIO* in = BIO_new(BIO_s_file());

    if (in != nullptr)
    {
        if (BIO_read_filename(in, identity_cert.substr(7).c_str()) > 0)
        {
            returnedValue = PEM_read_bio_X509_AUX(in, NULL, NULL, NULL);
        }
        else
        {
            exception =
                    _SecurityException_(std::string("OpenSSL library cannot read file ") + identity_cert.substr(7));
        }

        BIO_free(in);
    }
    else
    {
        exception = _SecurityException_("OpenSSL library cannot allocate file");
    }

    return returnedValue;
}

static int private_key_password_callback(
        char* buf,
        int bufsize,
        int /*verify*/,
        const char* password)
{
    assert(password != nullptr);

    int returnedValue = static_cast<int>(strlen(password));

    if (returnedValue > bufsize)
    {
        returnedValue = bufsize;
    }

    memcpy(buf, password, returnedValue);
    return returnedValue;
}

EVP_PKEY* FileProvider::load_private_key(
        X509* certificate,
        const std::string& pkey,
        const std::string& password,
        SecurityException& exception)
{
    EVP_PKEY* returnedValue = nullptr;
    BIO* in = BIO_new(BIO_s_file());

    if (in != nullptr)
    {
        if (BIO_read_filename(in, pkey.substr(7).c_str()) > 0)
        {
            returnedValue =
                    PEM_read_bio_PrivateKey(in, NULL, (pem_password_cb*)private_key_password_callback,
                            (void*)password.c_str());

            // Verify private key.
            if (nullptr == returnedValue)
            {
                exception = _SecurityException_(std::string("Error obtaining private key ") + pkey.substr(7));
            }
            else if (!X509_check_private_key(certificate, returnedValue))
            {
                exception = _SecurityException_(std::string("Error verifying private key ") + pkey.substr(7));
                EVP_PKEY_free(returnedValue);
                returnedValue = nullptr;
            }
        }
        else
        {
            exception = _SecurityException_(std::string("OpenSSL library cannot read file ") + pkey.substr(7));
        }

        BIO_free(in);
    }
    else
    {
        exception = _SecurityException_("OpenSSL library cannot allocate file");
    }

    return returnedValue;
}

X509_CRL* FileProvider::load_crl(
        const std::string& identity_crl,
        SecurityException& exception)
{
    X509_CRL* returnedValue = nullptr;

    BIO* in = BIO_new(BIO_s_file());

    if (in != nullptr)
    {
        if (BIO_read_filename(in, identity_crl.substr(7).c_str()) > 0)
        {
            returnedValue = PEM_read_bio_X509_CRL(in, NULL, NULL, NULL);
        }
        else
        {
            exception = _SecurityException_(std::string("OpenSSL library cannot read file ") + identity_crl.substr(7));
        }

        BIO_free(in);
    }
    else
    {
        exception = _SecurityException_("OpenSSL library cannot allocate file");
    }

    return returnedValue;
}

} // namespace detail
} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

