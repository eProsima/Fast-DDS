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
 * @file PKIDH.cpp
 */

#include "PKIDH.h"
#include <fastrtps/log/Log.h>

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <cassert>
#include <algorithm>

using namespace eprosima::fastrtps;
using namespace ::rtps;
using namespace ::security;

// Auxiliary functions
X509_STORE* load_identity_ca(std::string& identity_ca)
{
    X509_STORE* store = X509_STORE_new();

    if(store != nullptr)
    {
        OpenSSL_add_all_algorithms();

        if(identity_ca.size() >= 7 && identity_ca.compare(0, 7, "file://") == 0)
        {
            BIO *in = BIO_new(BIO_s_file());

            if(in != nullptr)
            {
                if(BIO_read_filename(in, identity_ca.substr(7).c_str()) > 0)
                {
                    bool failed = false;
                    int i, count = 0;
                    for (;;)
                    {
                        X509* pem = PEM_read_bio_X509_AUX(in, NULL, NULL, NULL);

                        if(pem == nullptr)
                        {
                            if ((ERR_GET_REASON(ERR_peek_last_error()) ==
                                        PEM_R_NO_START_LINE) && (count > 0)) {
                                ERR_clear_error();
                                break;
                            } else {
                                failed = true;
                                logError(AUTHENTICATION, "OpenSSL library cannot read perm in file");
                                break;
                            }
                        }

                        i = X509_STORE_add_cert(store, pem);

                        X509_free(pem);
                        pem = nullptr;

                        if (!i)
                        {
                            failed = true;
                            logError(AUTHENTICATION, "OpenSSL library cannot store perm");
                            break;
                        }

                        count++;
                    }

                    if(!failed && count > 0)
                    {
                        BIO_free(in);
                        return store;
                    }

                }
                else
                    logError(AUTHENTICATION, "OpenSSL library cannot read file " << identity_ca.substr(7));

                BIO_free(in);
            }
            else
                logError(AUTHENTICATION, "OpenSSL library cannot allocate file");
        }

        X509_STORE_free(store);
    }
    else
        logError(AUTHENTICATION, "Creation of X509 storage");

    return nullptr;
}

X509* load_certificate(std::string& identity_cert)
{
    X509* returnedValue = nullptr;

    if(identity_cert.size() >= 7 && identity_cert.compare(0, 7, "file://") == 0)
    {
        BIO *in = BIO_new(BIO_s_file());

        if(in != nullptr)
        {
            if(BIO_read_filename(in, identity_cert.substr(7).c_str()) > 0)
            {
                returnedValue = PEM_read_bio_X509_AUX(in, NULL, NULL, NULL);
            }
            else
                logError(AUTHENTICATION, "OpenSSL library cannot read file " << identity_cert.substr(7));

            BIO_free(in);
        }
        else
            logError(AUTHENTICATION, "OpenSSL library cannot allocate file");
    }

    return returnedValue;
}

ValidationResult_t PKIDH::validate_local_identity(IdentityHandle** local_identity_handle,
        GUID_t& adjusted_participant_key,
        const uint32_t domain_id,
        const RTPSParticipantAttributes& participant_attr,
        const GUID_t& candidate_participant_key,
        SecurityException& exception)
{
    assert(local_identity_handle);
    ValidationResult_t returnedValue =  ValidationResult_t::VALIDATION_FAILED;

    PropertyPolicy auth_properties = PropertyPolicyHelper::get_properties_with_prefix(participant_attr.properties, "dds.sec.auth.builtin.PKI-DH.");

    if(PropertyPolicyHelper::length(auth_properties) == 0)
    {
        logError(AUTHENTICATION, "Not found any dds.sec.auth.builtin.PKI-DH property");
        return returnedValue;
    }

    std::string* identity_ca = PropertyPolicyHelper::find_property(auth_properties, "identity_ca");

    if(identity_ca == nullptr)
    {
        logError(AUTHENTICATION, "Not found dds.sec.auth.builtin.PKI-DH.identity_ca property");
        return returnedValue;
    }

    std::string* identity_cert = PropertyPolicyHelper::find_property(auth_properties, "identity_certificate");

    if(identity_cert == nullptr)
    {
        logError(AUTHENTICATION, "Not found dds.sec.auth.builtin.PKI-DH.identity_certificate property");
        return returnedValue;
    }


    X509_STORE* store = load_identity_ca(*identity_ca);

    if(store != nullptr)
    {
        ERR_clear_error();

        X509* cert = load_certificate(*identity_cert);

        if(cert != nullptr)
        {
            X509_STORE_CTX *ctx = X509_STORE_CTX_new();

            if(ctx != nullptr)
            {
                X509_STORE_CTX_init(ctx, store, cert, NULL);
                X509_STORE_CTX_set_flags(ctx, X509_V_FLAG_X509_STRICT |
                        X509_V_FLAG_CHECK_SS_SIGNATURE | X509_V_FLAG_POLICY_CHECK);

                if(X509_verify_cert(ctx) > 0 && X509_STORE_CTX_get_error(ctx) == X509_V_OK)
                {
                    returnedValue = ValidationResult_t::VALIDATION_OK;
                }
                else
                    logError(AUTHENTICATION, "Invalidation error of certificate " << *identity_cert
                            << " (" << X509_STORE_CTX_get_error(ctx) << ")");

                X509_STORE_CTX_free(ctx);
            }

            X509_free(cert);
        }
        else
            logError(AUTHENTICATION, "Cannot read file " << *identity_cert);
    }

    X509_STORE_free(store);

    ERR_clear_error();

    return returnedValue;
}

ValidationResult_t PKIDH::validate_remote_identity(IdentityHandle** remote_identity_handle,
        const IdentityHandle& local_identity_handle,
        const IdentityToken& remote_identity_token,
        const GUID_t remote_participant_key,
        SecurityException& exception)
{
    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::begin_handshake_request(HandshakeHandle** handshake_handle,
        HandshakeMessageToken& handshake_message,
        const IdentityHandle& initiator_identity_handle,
        const IdentityHandle& replier_identity_handle,
        SecurityException& exception)
{
    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::begin_handshake_reply(HandshakeHandle** handshake_handle,
        HandshakeMessageToken& handshake_message_out,
        const HandshakeMessageToken& handshake_message_in,
        const IdentityHandle& initiator_identity_handle,
        const IdentityHandle& replier_identity_handle,
        SecurityException& exception)
{
    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::process_handshake(HandshakeMessageToken& handshake_message_out,
        const HandshakeMessageToken& handshake_message_in,
        const HandshakeHandle& handshake_handle,
        SecurityException& exception)
{
    return ValidationResult_t::VALIDATION_FAILED;
}

SharedSecretHandle* PKIDH::get_shared_secret(const HandshakeHandle& handshake_handle,
        SecurityException& exception)
{
    return nullptr;
}

bool PKIDH::set_listener(AuthenticationListener* listener,
        SecurityException& exception)
{
    return false;
}

bool PKIDH::get_identity_token(IdentityToken** identity_token,
        const IdentityHandle& handle,
        SecurityException& exception)
{
    return false;
}

bool PKIDH::return_identity_token(IdentityToken* token,
        SecurityException& exception)
{
    return false;
}

bool PKIDH::return_handshake_handle(HandshakeHandle* handshake_handle,
        SecurityException& exception)
{
    return false;
}

bool PKIDH::return_identity_handle(IdentityHandle* identity_handle,
        SecurityException& exception)
{
    return false;
}

bool PKIDH::return_sharedsecret_handle(SharedSecretHandle* sharedsecret_handle,
        SecurityException& exception)
{
    return false;
}
