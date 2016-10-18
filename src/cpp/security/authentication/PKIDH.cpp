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
#include "PKIIdentityHandle.h"
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/messages/CDRMessage.h>

#include <openssl/pem.h>
#include <openssl/err.h>

#include <cassert>
#include <algorithm>

using namespace eprosima::fastrtps;
using namespace ::rtps;
using namespace ::security;

// Auxiliary functions
X509_STORE* load_identity_ca(const std::string& identity_ca, bool& there_are_crls)
{
    X509_STORE* store = X509_STORE_new();

    if(store != nullptr)
    {
        OpenSSL_add_all_algorithms();

        if(identity_ca.size() >= 7 && identity_ca.compare(0, 7, "file://") == 0)
        {
            BIO* in = BIO_new(BIO_s_file());

            if(in != nullptr)
            {
                if(BIO_read_filename(in, identity_ca.substr(7).c_str()) > 0)
                {
                    STACK_OF(X509_INFO)* inf = PEM_X509_INFO_read_bio(in, NULL, NULL, NULL);

                    if(inf != nullptr)
                    {
                        int i, count = 0;
                        there_are_crls = false;

                        for (i = 0; i < sk_X509_INFO_num(inf); i++)
                        {
                            X509_INFO* itmp = sk_X509_INFO_value(inf, i);

                            if (itmp->x509)
                            {
                                X509_STORE_add_cert(store, itmp->x509);
                                count++;
                            }
                            if (itmp->crl)
                            {
                                X509_STORE_add_crl(store, itmp->crl);
                                count++;
                                there_are_crls = true;
                            }
                        }

                        sk_X509_INFO_pop_free(inf, X509_INFO_free);
                        BIO_free(in);

                        return store;
                    }
                    else
                        logError(AUTHENTICATION, "OpenSSL library cannot read X509 info in file" << identity_ca.substr(7));

                    sk_X509_INFO_pop_free(inf, X509_INFO_free);
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

X509* load_certificate(const std::string& identity_cert)
{
    X509* returnedValue = nullptr;

    if(identity_cert.size() >= 7 && identity_cert.compare(0, 7, "file://") == 0)
    {
        BIO* in = BIO_new(BIO_s_file());

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

bool store_certificate_in_buffer(X509* certificate, BUF_MEM** ptr)
{
    bool returnedValue = false;

    BIO *out = BIO_new(BIO_s_mem());

    if(out != nullptr)
    {
        if(PEM_write_bio_X509(out, certificate) > 0 )
        {
            BIO_get_mem_ptr(out, ptr);

            if(*ptr != nullptr)
            {
                BIO_set_close(out, BIO_NOCLOSE);
                returnedValue = true;
            }
            else
                logError(AUTHENTICATION, "OpenSSL library cannot retrieve mem ptr");
        }
        else
            logError(AUTHENTICATION, "OpenSSL library cannot write cert");

        BIO_free(out);
    }
    else
        logError(AUTHENTICATION, "OpenSSL library cannot allocate mem");

    return returnedValue;
}

bool get_signature_algorithm(X509* certificate, std::string& signature_algorithm)
{
    bool returnedValue = false;
    BUF_MEM* ptr = nullptr;
    X509_ALGOR* sigalg = nullptr;
    ASN1_BIT_STRING* sig = nullptr;

    BIO *out = BIO_new(BIO_s_mem());

    if(out != nullptr)
    {
        X509_get0_signature(&sig, &sigalg, certificate);

        if(sigalg != nullptr)
        {
            if(i2a_ASN1_OBJECT(out, sigalg->algorithm) > 0)
            {
                BIO_get_mem_ptr(out, &ptr);

                if(ptr != nullptr)
                {
                    if(strncmp(ptr->data, "ecdsa-with-SHA256", ptr->length) == 0)
                    {
                        signature_algorithm = "ECDSA-SHA256";
                        returnedValue = true;
                    }
                }
                else
                    logError(AUTHENTICATION, "OpenSSL library cannot retrieve mem ptr");
            }
        }
        else
            logError(AUTHENTICATION, "OpenSSL library cannot write cert");

        BIO_free(out);
    }
    else
        logError(AUTHENTICATION, "OpenSSL library cannot allocate mem");

    return returnedValue;
}


X509_CRL* load_crl(const std::string& identity_crl)
{
    X509_CRL* returnedValue = nullptr;

    if(identity_crl.size() >= 7 && identity_crl.compare(0, 7, "file://") == 0)
    {
        BIO *in = BIO_new(BIO_s_file());

        if(in != nullptr)
        {
            if(BIO_read_filename(in, identity_crl.substr(7).c_str()) > 0)
            {
                returnedValue = PEM_read_bio_X509_CRL(in, NULL, NULL, NULL);
            }
            else
                logError(AUTHENTICATION, "OpenSSL library cannot read file " << identity_crl.substr(7));

            BIO_free(in);
        }
        else
            logError(AUTHENTICATION, "OpenSSL library cannot allocate file");
    }

    return returnedValue;
}

bool adjust_participant_key(X509* cert, const GUID_t& candidate_participant_key,
        GUID_t& adjusted_participant_key)
{
    assert(cert != nullptr);

    X509_NAME* cert_sn = cert->cert_info->subject;

    assert(cert_sn != nullptr);

    unsigned long returnedValue = 0;
    unsigned char md[SHA256_DIGEST_LENGTH];

    i2d_X509_NAME(cert_sn, NULL);
    if(!EVP_Digest(cert_sn->canon_enc, cert_sn->canon_enclen, md, NULL, EVP_sha256(), NULL))
    {
        logError(AUTHENTICATION, "OpenSSL library cannot hash sha256");
        return false;
    }

    adjusted_participant_key.guidPrefix.value[0] = 0x80 | md[0];
    adjusted_participant_key.guidPrefix.value[1] = md[1];
    adjusted_participant_key.guidPrefix.value[2] = md[2];
    adjusted_participant_key.guidPrefix.value[3] = md[3];
    adjusted_participant_key.guidPrefix.value[4] = md[4];
    adjusted_participant_key.guidPrefix.value[5] = md[5];

    unsigned char key[16] = {
        candidate_participant_key.guidPrefix.value[0],
        candidate_participant_key.guidPrefix.value[1],
        candidate_participant_key.guidPrefix.value[2],
        candidate_participant_key.guidPrefix.value[3],
        candidate_participant_key.guidPrefix.value[4],
        candidate_participant_key.guidPrefix.value[5],
        candidate_participant_key.guidPrefix.value[6],
        candidate_participant_key.guidPrefix.value[7],
        candidate_participant_key.guidPrefix.value[8],
        candidate_participant_key.guidPrefix.value[9],
        candidate_participant_key.guidPrefix.value[10],
        candidate_participant_key.guidPrefix.value[11],
        candidate_participant_key.entityId.value[0],
        candidate_participant_key.entityId.value[1],
        candidate_participant_key.entityId.value[2],
        candidate_participant_key.entityId.value[3]
    };

    if(!EVP_Digest(&key, 16, md, NULL, EVP_sha256(), NULL))
    {
        logError(AUTHENTICATION, "OpenSSL library cannot hash sha256");
        return false;
    }

    adjusted_participant_key.guidPrefix.value[6] = md[0];
    adjusted_participant_key.guidPrefix.value[7] = md[1];
    adjusted_participant_key.guidPrefix.value[8] = md[2];
    adjusted_participant_key.guidPrefix.value[9] = md[3];
    adjusted_participant_key.guidPrefix.value[10] = md[4];
    adjusted_participant_key.guidPrefix.value[11] = md[5];

    adjusted_participant_key.entityId.value[0] = candidate_participant_key.entityId.value[0];
    adjusted_participant_key.entityId.value[1] = candidate_participant_key.entityId.value[1];
    adjusted_participant_key.entityId.value[2] = candidate_participant_key.entityId.value[2];
    adjusted_participant_key.entityId.value[3] = candidate_participant_key.entityId.value[3];

    return true;
}

bool generate_dh_key_parameters()
{
    //EVP_PKEY_CTX_new_id();
    return false;
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

    std::string* identity_crl = PropertyPolicyHelper::find_property(auth_properties, "identity_crl");

    PKIIdentityHandle* ih = new PKIIdentityHandle();
    bool there_are_crls = false;

    (*ih)->store_ = load_identity_ca(*identity_ca, there_are_crls);

    if((*ih)->store_ != nullptr)
    {
        bool configuration_error = false;

        if(identity_crl != nullptr)
        {
            X509_CRL* crl = load_crl(*identity_crl);

            if(crl != nullptr)
            {
                X509_STORE_add_crl((*ih)->store_, crl);
                there_are_crls = true;
            }
            else
                configuration_error = true;
        }

        ERR_clear_error();

        if(!configuration_error)
        {
            (*ih)->cert_ = load_certificate(*identity_cert);

            if((*ih)->cert_ != nullptr)
            {
                X509_STORE_CTX *ctx = X509_STORE_CTX_new();

                if(ctx != nullptr)
                {
                    unsigned long flags = there_are_crls ? X509_V_FLAG_CRL_CHECK : 0;
                    X509_STORE_CTX_init(ctx, (*ih)->store_, (*ih)->cert_, NULL);
                    X509_STORE_CTX_set_flags(ctx, flags | X509_V_FLAG_X509_STRICT |
                            X509_V_FLAG_CHECK_SS_SIGNATURE | X509_V_FLAG_POLICY_CHECK);

                    if(X509_verify_cert(ctx) > 0 && X509_STORE_CTX_get_error(ctx) == X509_V_OK)
                    {
                        if(store_certificate_in_buffer((*ih)->cert_, &(*ih)->cert_content_))
                        {
                            if(get_signature_algorithm((*ih)->cert_, (*ih)->sign_alg_))
                            {
                                if(adjust_participant_key((*ih)->cert_, candidate_participant_key, adjusted_participant_key))
                                {
                                    (*ih)->participant_key_ = adjusted_participant_key;
                                    *local_identity_handle = ih;

                                    returnedValue = ValidationResult_t::VALIDATION_OK;
                                }
                            }
                        }
                    }
                    else
                        logError(AUTHENTICATION, "Invalidation error of certificate " << *identity_cert
                                << " (" << X509_STORE_CTX_get_error(ctx) << ")");

                    X509_STORE_CTX_free(ctx);
                }
            }
            else
                logError(AUTHENTICATION, "Cannot read file " << *identity_cert);
        }
    }

    if(returnedValue != ValidationResult_t::VALIDATION_OK)
    {
        delete ih;
    }

    ERR_clear_error();

    return returnedValue;
}

ValidationResult_t PKIDH::validate_remote_identity(IdentityHandle** remote_identity_handle,
        const IdentityHandle& local_identity_handle,
        const IdentityToken& remote_identity_token,
        const GUID_t remote_participant_key,
        SecurityException& exception)
{
    ValidationResult_t returnedValue = ValidationResult_t::VALIDATION_FAILED;

    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(local_identity_handle);

    if(!lih.nil())
    {
        PKIIdentityHandle* rih = new PKIIdentityHandle();

        (*rih)->participant_key_ = remote_participant_key;

        if(lih->participant_key_ < remote_participant_key )
            returnedValue = ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST;
        else
            returnedValue = ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE;
    }

    return  returnedValue;
}

ValidationResult_t PKIDH::begin_handshake_request(HandshakeHandle** handshake_handle,
        HandshakeMessageToken& handshake_message,
        const IdentityHandle& initiator_identity_handle,
        const IdentityHandle& replier_identity_handle,
        SecurityException& exception)
{
    ValidationResult_t returnedValue = ValidationResult_t::VALIDATION_FAILED;
    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(initiator_identity_handle);

    if(!lih.nil())
    {
        unsigned char md[SHA256_DIGEST_LENGTH];

        handshake_message.class_id("DDS:Auth:PKI-DH:1.0+Req");

        BinaryProperty bproperty;

        // c.id
        bproperty.name("c.id");
        bproperty.value().assign(lih->cert_content_->data,
                lih->cert_content_->data + lih->cert_content_->length);
        bproperty.propagate(true);
        handshake_message.binary_properties().emplace_back(std::move(bproperty));

        // TODO(Ricardo) c.pdata
        
        // c.dsign_algo.
        bproperty.name("c.dsign_algo");
        bproperty.value().assign(lih->sign_alg_.begin(),
                lih->sign_alg_.end());
        bproperty.propagate(true);
        handshake_message.binary_properties().emplace_back(std::move(bproperty));

        // TODO(Ricardo) Only support right now DH+MODP-2048-256
        bproperty.name("c.kagree_algo");
        bproperty.value().assign(lih->kagree_alg_.begin(),
                lih->kagree_alg_.end());
        bproperty.propagate(true);
        handshake_message.binary_properties().emplace_back(std::move(bproperty));

        CDRMessage_t message(BinaryPropertyHelper::serialized_size(handshake_message.binary_properties()));
        message.msg_endian = BIGEND;
        CDRMessage::addBinaryPropertySeq(&message, handshake_message.binary_properties());
        if(!EVP_Digest(message.buffer, message.length, md, NULL, EVP_sha256(), NULL))
        {
            logError(AUTHENTICATION, "OpenSSL library cannot hash sha256");
            return ValidationResult_t::VALIDATION_FAILED;
        }
        bproperty.name("hash_c1");
        bproperty.value().assign(md, md + SHA256_DIGEST_LENGTH);
        bproperty.propagate(true);
        handshake_message.binary_properties().emplace_back(std::move(bproperty));

        
        returnedValue = ValidationResult_t::VALIDATION_OK;
    }

    return returnedValue;
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
