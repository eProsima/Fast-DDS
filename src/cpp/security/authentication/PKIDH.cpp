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
#include "PKIHandshakeHandle.h"
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/messages/CDRMessage.h>

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/obj_mac.h>

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

X509* load_certificate(const std::vector<uint8_t>& data)
{
    X509* returnedValue = nullptr;
    BIO* cid = BIO_new_mem_buf(data.data(), data.size());

    if(cid != nullptr)
    {
        returnedValue = PEM_read_bio_X509_AUX(cid, NULL, NULL, NULL);
    }

    return returnedValue;
}

bool verify_certificate(X509_STORE* store, X509* cert, const bool there_are_crls)
{
    assert(store);
    assert(cert);

    bool returnedValue = false;

    X509_STORE_CTX *ctx = X509_STORE_CTX_new();

    if(ctx != nullptr)
    {
        unsigned long flags = there_are_crls ? X509_V_FLAG_CRL_CHECK : 0;
        X509_STORE_CTX_init(ctx, store, cert, NULL);
        X509_STORE_CTX_set_flags(ctx, flags | X509_V_FLAG_X509_STRICT |
                X509_V_FLAG_CHECK_SS_SIGNATURE | X509_V_FLAG_POLICY_CHECK);

        if(X509_verify_cert(ctx) > 0 && X509_STORE_CTX_get_error(ctx) == X509_V_OK)
            returnedValue = true;
        else
            logError(AUTHENTICATION, "Invalidation error of certificate  (" << X509_STORE_CTX_get_error(ctx) << ")");

        X509_STORE_CTX_free(ctx);
    }

    return returnedValue;
}

int private_key_password_callback(char* buf, int bufsize, int verify, const char* password)
{
    assert(password != nullptr);

    int returnedValue = strlen(password);

    if(returnedValue > bufsize)
        returnedValue = bufsize;

    memcpy(buf, password, returnedValue);
    return returnedValue;
}

EVP_PKEY* load_private_key(X509* certificate, const std::string& file, const std::string& password)
{
    EVP_PKEY* returnedValue = nullptr;
    if(file.size() >= 7 && file.compare(0, 7, "file://") == 0)
    {
        BIO* in = BIO_new(BIO_s_file());

        if(in != nullptr)
        {
            if(BIO_read_filename(in, file.substr(7).c_str()) > 0)
            {
                returnedValue = PEM_read_bio_PrivateKey(in, NULL, (pem_password_cb*)private_key_password_callback, (void*)password.c_str());

                // Verify private key.
                if(!X509_check_private_key(certificate, returnedValue))
                {
                    logError(AUTHENTICATION, "Error verifying private key " << file.substr(7));
                    EVP_PKEY_free(returnedValue);
                    returnedValue = nullptr;
                }
            }
            else
                logError(AUTHENTICATION, "OpenSSL library cannot read file " << file.substr(7));

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
                        signature_algorithm = ECDSA_SHA256;
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

bool sign_sha256(EVP_PKEY* private_key, const unsigned char* data, size_t data_length,
        std::vector<uint8_t>& buffer)
{
    assert(private_key);
    assert(data);

    bool returnedValue = false;
    EVP_MD_CTX ctx;
    EVP_MD_CTX_init(&ctx);

    if(EVP_DigestSignInit(&ctx, NULL, EVP_sha256(), NULL, private_key) == 1)
    {
        if(EVP_DigestSignUpdate(&ctx, data, data_length) == 1)
        {
            size_t length = 0;
            if(EVP_DigestSignFinal(&ctx, NULL, &length) == 1 && length > 0)
            {
                size_t aux_length = length;
                buffer.resize(length);

                std::cout << "len = " << length << std::endl;
                if(EVP_DigestSignFinal(&ctx, buffer.data(), &length) ==  1 && length == aux_length)
                    returnedValue = true; 
                else
                    logError(AUTHENTICATION, "Cannot finish signature (" << ERR_get_error() << ")");

                std::cout << "len2 = " << length << std::endl;
            }
        }
        else
            logError(AUTHENTICATION, "Cannot sign data (" << ERR_get_error() << ")");
    }
    else
        logError(AUTHENTICATION, "Cannot init signature (" << ERR_get_error() << ")");

    EVP_MD_CTX_cleanup(&ctx);

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

int get_dh_type(const std::string& algorithm)
{
    if(algorithm.compare(DH_2048_256) == 0)
        return EVP_PKEY_DH;
    else if(algorithm.compare(ECDH_prime256v1) == 0)
        return EVP_PKEY_EC;

    return 0;
}

EVP_PKEY* generate_dh_key(int type)
{
    EVP_PKEY* keys = nullptr;
    EVP_PKEY* params = EVP_PKEY_new();
    EVP_PKEY_CTX* context = nullptr;

    if(params != nullptr)
    {
        int ret = 0;

        switch(type)
        {
            case EVP_PKEY_DH:
                ret = EVP_PKEY_set1_DH(params, DH_get_2048_256());
                break;
        };

        if(ret)
        {
            if((context = EVP_PKEY_CTX_new(params, NULL)) != nullptr)
            {
                if(EVP_PKEY_keygen_init(context))
                {
                    if(!EVP_PKEY_keygen(context, &keys))
                        logError(AUTHENTICATION, "Cannot generate EVP key");
                }
                else
                    logError(AUTHENTICATION, "Cannot init EVP key");

                EVP_PKEY_CTX_free(context);
            }
            else
                logError(AUTHENTICATION, "Cannot create EVP context");
        }
        else
            logError(AUTHENTICATION, "Cannot set default paremeters");

        EVP_PKEY_free(params);
    }
    else
        logError(AUTHENTICATION, "Cannot allcoate EVP parameters");

    /*
    EVP_PKEY_CTX* context = EVP_PKEY_CTX_new_id(type, NULL);

    if(context != nullptr)
    {
        // Generate parameters
        if(EVP_PKEY_paramgen_init(context))
        {
            bool generated_param = false;
            switch(type)
            {
                case EVP_PKEY_EC:
                    if(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(context, NID_X9_62_prime256v1))
                        generated_param = true;
                    break;
                case EVP_PKEY_DH:
                    if(EVP_PKEY_CTX_set_dh_paramgen_prime_len(context, 2048))
                        generated_param = true;
                    break;
            }

            if(generated_param)
            {
                std::cout << "PARAM" << std::endl;
                if(EVP_PKEY_paramgen(context, &param))
                {
                std::cout << "END PARAM" << std::endl;
                    if(EVP_PKEY_keygen_init(context))
                    {
                        if(!EVP_PKEY_keygen(context, &keys))
                            logError(AUTHENTICATION, "Cannot generate EVP key");
                    }
                    else
                        logError(AUTHENTICATION, "Cannot init EVP key");

                    //EVP_PKEY_free(param);
                }
                else
                    logError(AUTHENTICATION, "Cannot generate EVP parameters");
            }
            else
                logError(AUTHENTICATION, "Cannot set EVP parameters");
        }
        else
            logError(AUTHENTICATION, "Cannot init EVP context");

        EVP_PKEY_CTX_free(context);
    }
    else
        logError(AUTHENTICATION, "Cannot generate EVP context");
    */


    return keys;
}

bool store_dh_public_key(EVP_PKEY* dhkey, std::vector<uint8_t>& vector)
{
    DH* dh = EVP_PKEY_get1_DH(dhkey);

    if(dh != nullptr)
    {
        const BIGNUM* pub_key = dh->pub_key;
        int len = BN_num_bytes(pub_key);
        vector.resize(len + 4);

        if(DEFAULT_ENDIAN == BIGEND)
        {
            vector.at(0) = ((char*)&len)[0];
            vector.at(1) = ((char*)&len)[1];
            vector.at(2) = ((char*)&len)[2];
            vector.at(3) = ((char*)&len)[3];
        }
        else
        {
            vector.at(0) = ((char*)&len)[3];
            vector.at(1) = ((char*)&len)[2];
            vector.at(2) = ((char*)&len)[1];
            vector.at(3) = ((char*)&len)[0];
        }

        if(BN_bn2bin(pub_key, vector.data() + 4) == len)
            return true;
        else
            logError(AUTHENTICATION, "OpenSSL library cannot store public key");
    }
    else
        logError(AUTHENTICATION, "OpenSSL library doesn't retrieve DH");

    return false;
}

bool generate_challenge(std::vector<uint8_t>& vector)
{
    bool returnedValue = false;
    BIGNUM* bn = BN_new();

    if(bn != nullptr)
    {
        if(BN_rand(bn, 512, 0 /*BN_RAND_TOP_ONE*/, 0 /*BN_RAND_BOTTOM_ANY*/));
        {
            int len = BN_num_bytes(bn);
            vector.resize(len);

            if(BN_bn2bin(bn, vector.data()) == len)
                returnedValue = true;
            else
                logError(AUTHENTICATION, "OpenSSL library cannot store challenge");
        }

        BN_free(bn);
    }
    else
        logError(AUTHENTICATION, "OpenSSL library cannot allocate random number");

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

    PropertyPolicy auth_properties = PropertyPolicyHelper::get_properties_with_prefix(participant_attr.properties, "dds.sec.auth.builtin.PKI-DH.");

    if(PropertyPolicyHelper::length(auth_properties) == 0)
    {
        logError(AUTHENTICATION, "Not found any dds.sec.auth.builtin.PKI-DH property");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    std::string* identity_ca = PropertyPolicyHelper::find_property(auth_properties, "identity_ca");

    if(identity_ca == nullptr)
    {
        logError(AUTHENTICATION, "Not found dds.sec.auth.builtin.PKI-DH.identity_ca property");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    std::string* identity_cert = PropertyPolicyHelper::find_property(auth_properties, "identity_certificate");

    if(identity_cert == nullptr)
    {
        logError(AUTHENTICATION, "Not found dds.sec.auth.builtin.PKI-DH.identity_certificate property");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    std::string* identity_crl = PropertyPolicyHelper::find_property(auth_properties, "identity_crl");

    std::string* private_key = PropertyPolicyHelper::find_property(auth_properties, "private_key");

    if(private_key == nullptr)
    {
        logError(AUTHENTICATION, "Not found dds.sec.auth.builtin.PKI-DH.private_key property");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    std::string* password = PropertyPolicyHelper::find_property(auth_properties, "password");
    std::string empty_password;

    if(password == nullptr)
        password = &empty_password;

    PKIIdentityHandle* ih = new PKIIdentityHandle();

    (*ih)->store_ = load_identity_ca(*identity_ca, (*ih)->there_are_crls_);

    if((*ih)->store_ != nullptr)
    {
        ERR_clear_error();

        if(identity_crl != nullptr)
        {
            X509_CRL* crl = load_crl(*identity_crl);

            if(crl != nullptr)
            {
                X509_STORE_add_crl((*ih)->store_, crl);
                (*ih)->there_are_crls_ = true;
            }
            else
            {
                delete ih;
                return ValidationResult_t::VALIDATION_FAILED;
            }
        }

        ERR_clear_error();

        (*ih)->cert_ = load_certificate(*identity_cert);

        if((*ih)->cert_ != nullptr)
        {
            if(verify_certificate((*ih)->store_, (*ih)->cert_, (*ih)->there_are_crls_))
            {
                if(store_certificate_in_buffer((*ih)->cert_, &(*ih)->cert_content_))
                {
                    if(get_signature_algorithm((*ih)->cert_, (*ih)->sign_alg_))
                    {
                        (*ih)->pkey_ = load_private_key((*ih)->cert_, *private_key, *password);

                        if((*ih)->pkey_ != nullptr)
                        {
                            if(adjust_participant_key((*ih)->cert_, candidate_participant_key, adjusted_participant_key))
                            {
                                (*ih)->participant_key_ = adjusted_participant_key;
                                *local_identity_handle = ih;

                                return ValidationResult_t::VALIDATION_OK;
                            }
                        }
                        else
                            logError(AUTHENTICATION, "Invalid private key in " << *private_key);
                    }
                }
            }
            else
                logError(AUTHENTICATION, "Verification failed for " << *identity_cert);
        }
        else
            logError(AUTHENTICATION, "Error loading file " << *identity_cert);
    }

    delete ih;

    ERR_clear_error();

    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::validate_remote_identity(IdentityHandle** remote_identity_handle,
        const IdentityHandle& local_identity_handle,
        const IdentityToken& remote_identity_token,
        const GUID_t remote_participant_key,
        SecurityException& exception)
{
    assert(remote_identity_handle);
    assert(local_identity_handle.nil() == false);

    ValidationResult_t returnedValue = ValidationResult_t::VALIDATION_FAILED;

    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(local_identity_handle);

    if(!lih.nil())
    {
        PKIIdentityHandle* rih = new PKIIdentityHandle();

        (*rih)->participant_key_ = remote_participant_key;
        *remote_identity_handle = rih;

        if(lih->participant_key_ < remote_participant_key )
            returnedValue = ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST;
        else
            returnedValue = ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE;
    }

    return  returnedValue;
}

ValidationResult_t PKIDH::begin_handshake_request(HandshakeHandle** handshake_handle,
        HandshakeMessageToken** handshake_message,
        const IdentityHandle& initiator_identity_handle,
        const IdentityHandle& replier_identity_handle,
        SecurityException& exception)
{
    assert(handshake_handle);
    assert(handshake_message);
    assert(initiator_identity_handle.nil() == false);
    assert(replier_identity_handle.nil() == false);

    ValidationResult_t returnedValue = ValidationResult_t::VALIDATION_FAILED;
    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(initiator_identity_handle);
    const PKIIdentityHandle& rih = PKIIdentityHandle::narrow(replier_identity_handle);

    if(lih.nil() || rih.nil())
    {
        logError(AUTHENTICATION, "Bad precondition");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    unsigned char md[SHA256_DIGEST_LENGTH];

    // New handshake
    PKIHandshakeHandle* handshake_handle_aux = new PKIHandshakeHandle();
    (*handshake_handle_aux)->kagree_alg_ = lih->kagree_alg_;
    (*handshake_handle_aux)->handshake_message_.class_id("DDS:Auth:PKI-DH:1.0+Req");

    BinaryProperty bproperty;

    // c.id
    bproperty.name("c.id");
    bproperty.value().assign(lih->cert_content_->data,
            lih->cert_content_->data + lih->cert_content_->length);
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

    // TODO(Ricardo) c.pdata

    // c.dsign_algo.
    bproperty.name("c.dsign_algo");
    bproperty.value().assign(lih->sign_alg_.begin(),
            lih->sign_alg_.end());
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

    // TODO(Ricardo) Only support right now DH+MODP-2048-256
    // c.kagree_algo.
    bproperty.name("c.kagree_algo");
    bproperty.value().assign(lih->kagree_alg_.begin(),
            lih->kagree_alg_.end());
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

    // hash_c1
    // TODO(Ricardo) Have to add +3 because current serialization add alignment bytes at the end.
    CDRMessage_t message(BinaryPropertyHelper::serialized_size((*handshake_handle_aux)->handshake_message_.binary_properties()) + 3);
    message.msg_endian = BIGEND;
    CDRMessage::addBinaryPropertySeq(&message, (*handshake_handle_aux)->handshake_message_.binary_properties());
    if(!EVP_Digest(message.buffer, message.length, md, NULL, EVP_sha256(), NULL))
    {
        logError(AUTHENTICATION, "OpenSSL library cannot hash sha256");
        delete handshake_handle_aux;
        return ValidationResult_t::VALIDATION_FAILED;
    }
    bproperty.name("hash_c1");
    bproperty.value().assign(md, md + SHA256_DIGEST_LENGTH);
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

    // dh1
    if(((*handshake_handle_aux)->dhkeys_ = generate_dh_key(get_dh_type((*handshake_handle_aux)->kagree_alg_))) != nullptr)
    {
        bproperty.name("dh1");
        bproperty.propagate(true);

        if(store_dh_public_key((*handshake_handle_aux)->dhkeys_, bproperty.value()))
        {
            (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

            // challenge1
            bproperty.name("challenge1");
            bproperty.propagate(true);
            if(generate_challenge(bproperty.value()))
            {
                (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

                (*handshake_handle_aux)->local_identity_handle_ = &lih;
                (*handshake_handle_aux)->remote_identity_handle_ = &rih;
                *handshake_handle = handshake_handle_aux;
                *handshake_message = &(*handshake_handle_aux)->handshake_message_;
                return ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE;
            }
        }
    }

    delete handshake_handle_aux;

    ERR_clear_error();

    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::begin_handshake_reply(HandshakeHandle** handshake_handle,
        HandshakeMessageToken** handshake_message_out,
        const HandshakeMessageToken& handshake_message_in,
        IdentityHandle& initiator_identity_handle,
        const IdentityHandle& replier_identity_handle,
        SecurityException& exception)
{
    assert(handshake_handle);
    assert(handshake_message_out);
    assert(initiator_identity_handle.nil() == false);
    assert(replier_identity_handle.nil() == false);

    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(replier_identity_handle);
    PKIIdentityHandle& rih = PKIIdentityHandle::narrow(initiator_identity_handle);

    if(lih.nil() || rih.nil())
    {
        logError(AUTHENTICATION, "Bad precondition");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check incomming handshake.
    // Check c.id
    const std::vector<uint8_t>* cid = DataHolderHelper::find_binary_property_value(handshake_message_in, "c.id");
    if(cid == nullptr)
    {
        logError(AUTHENTICATION, "Cannot find property c.id");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    rih->cert_ = load_certificate(*cid);

    if(rih->cert_ == nullptr)
    {
        logError(AUTHENTICATION, "Cannot load certificate");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if(!verify_certificate(lih->store_, rih->cert_, lih->there_are_crls_))
    {
        logError(AUTHENTICATION, "Error verifying certificate");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // TODO(Ricardo) c.pdata and check participant_builtin_key
        
    // c.dsign_algo
    const std::vector<uint8_t>* dsign_algo = DataHolderHelper::find_binary_property_value(handshake_message_in, "c.dsign_algo");

    if(dsign_algo == nullptr)
    {
        logError(AUTHENTICATION, "Cannot find property c.dsign_algo");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check signature algorithm
    std::string s_dsign_algo(dsign_algo->begin(), dsign_algo->end());
    if(s_dsign_algo.compare(RSA_SHA256) != 0 &&
            s_dsign_algo.compare(ECDSA_SHA256) != 0)
    {
        logError(AUTHENTICATION, "Not supported signature algorithm (" << s_dsign_algo << ")");
        return ValidationResult_t::VALIDATION_FAILED;
    }
    rih->sign_alg_ = std::move(s_dsign_algo);

    // c.kagree_algo
    const std::vector<uint8_t>* kagree_algo = DataHolderHelper::find_binary_property_value(handshake_message_in, "c.kagree_algo");

    if(kagree_algo == nullptr)
    {
        logError(AUTHENTICATION, "Cannot find property c.kagree_algo");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check key agreement algorithm
    std::string s_kagree_algo(kagree_algo->begin(), kagree_algo->end());
    if(s_kagree_algo.compare(DH_2048_256) != 0 &&
            s_kagree_algo.compare(ECDH_prime256v1) != 0)
    {
        logError(AUTHENTICATION, "Not supported key agreement algorithm (" << s_kagree_algo << ")");
        return ValidationResult_t::VALIDATION_FAILED;
    }
    rih->kagree_alg_ = std::move(s_kagree_algo);

    // hash_c1
    const std::vector<uint8_t>* hash_c1 = DataHolderHelper::find_binary_property_value(handshake_message_in, "hash_c1");

    if(hash_c1 == nullptr)
    {
        logError(AUTHENTICATION, "Cannot find property hash_c1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if(hash_c1->size() != SHA256_DIGEST_LENGTH)
    {
        logError(AUTHENTICATION, "Wrong size of hash_c1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // TODO(Ricardo) Have to add +3 because current serialization add alignment bytes at the end.
    CDRMessage_t cdrmessage(BinaryPropertyHelper::serialized_size(handshake_message_in.binary_properties())+ 3);
    cdrmessage.msg_endian = BIGEND;
    CDRMessage::addBinaryPropertySeq(&cdrmessage, handshake_message_in.binary_properties(), "hash_c1");
    unsigned char md[SHA256_DIGEST_LENGTH];

    if(!EVP_Digest(cdrmessage.buffer, cdrmessage.length, md, NULL, EVP_sha256(), NULL))
    {
        logError(AUTHENTICATION, "Cannot generate SHA256 of request");
        return ValidationResult_t::VALIDATION_FAILED;
    }
    
    if(memcmp(md, hash_c1->data(), SHA256_DIGEST_LENGTH) != 0)
    {
        logError(AUTHENTICATION, "Wrong hash_c1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // dh1
    const std::vector<uint8_t>* dh1 = DataHolderHelper::find_binary_property_value(handshake_message_in, "dh1");

    if(dh1 == nullptr)
    {
        logError(AUTHENTICATION, "Cannot find property dh1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // challenge1
    const std::vector<uint8_t>* challenge1 = DataHolderHelper::find_binary_property_value(handshake_message_in, "challenge1");

    if(challenge1 == nullptr)
    {
        logError(AUTHENTICATION, "Cannot find property challenge1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Generate handshake reply message token.
    PKIHandshakeHandle* handshake_handle_aux = new PKIHandshakeHandle();
    (*handshake_handle_aux)->kagree_alg_ = rih->kagree_alg_;
    (*handshake_handle_aux)->handshake_message_.class_id("DDS:Auth:PKI-DH:1.0+Reply");

    BinaryProperty bproperty;

    // c.id
    bproperty.name("c.id");
    bproperty.value().assign(lih->cert_content_->data,
            lih->cert_content_->data + lih->cert_content_->length);
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

    // TODO(Ricardo) c.pdata

    // c.dsign_algo.
    bproperty.name("c.dsign_algo");
    bproperty.value().assign(lih->sign_alg_.begin(),
            lih->sign_alg_.end());
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

    // TODO(Ricardo) Only support right now DH+MODP-2048-256
    // c.kagree_algo.
    bproperty.name("c.kagree_algo");
    bproperty.value().assign((*handshake_handle_aux)->kagree_alg_.begin(),
            (*handshake_handle_aux)->kagree_alg_.end());
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

    // hash_c2
    // TODO(Ricardo) Have to add +3 because current serialization add alignment bytes at the end.
    CDRMessage_t message(BinaryPropertyHelper::serialized_size((*handshake_handle_aux)->handshake_message_.binary_properties()) + 3);
    message.msg_endian = BIGEND;
    CDRMessage::addBinaryPropertySeq(&message, (*handshake_handle_aux)->handshake_message_.binary_properties());
    if(!EVP_Digest(message.buffer, message.length, md, NULL, EVP_sha256(), NULL))
    {
        logError(AUTHENTICATION, "OpenSSL library cannot hash sha256");
        delete handshake_handle_aux;
        return ValidationResult_t::VALIDATION_FAILED;
    }
    bproperty.name("hash_c2");
    bproperty.value().assign(md, md + SHA256_DIGEST_LENGTH);
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));
    

    // dh2
    if(((*handshake_handle_aux)->dhkeys_ = generate_dh_key(get_dh_type((*handshake_handle_aux)->kagree_alg_))) != nullptr)
    {
        bproperty.name("dh2");
        bproperty.propagate(true);

        if(store_dh_public_key((*handshake_handle_aux)->dhkeys_, bproperty.value()))
        {
            (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

            // hash_c1
            bproperty.name("hash_c1");
            bproperty.value(std::move(*hash_c1));
            bproperty.propagate(true);
            (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

            // dh1
            bproperty.name("dh1");
            bproperty.value(std::move(*dh1));
            bproperty.propagate(true);
            (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

            // challenge1
            bproperty.name("challenge1");
            bproperty.value(std::move(*challenge1));
            bproperty.propagate(true);
            (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

            // challenge2
            bproperty.name("challenge2");
            bproperty.propagate(true);
            if(generate_challenge(bproperty.value()))
            {
                (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

                // signature
                CDRMessage_t cdrmessage(BinaryPropertyHelper::serialized_size((*handshake_handle_aux)->handshake_message_.binary_properties()));
                cdrmessage.msg_endian = BIGEND;
                // add sequence length
                CDRMessage::addUInt32(&cdrmessage, 6);
                //add hash_c2
                CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_, "hash_c2"));
                //add challenge2
                CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_, "challenge2"));
                //add dh2
                CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_, "dh2"));
                //add challenge1
                CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_, "challenge1"));
                //add dh1
                CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_, "dh1"));
                //add hash_c1
                CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_, "hash_c1"));

                bproperty.name("signature");
                bproperty.propagate("true");
                if(sign_sha256(lih->pkey_, cdrmessage.buffer, cdrmessage.length, bproperty.value()))
                {
                    (*handshake_handle_aux)->handshake_message_.binary_properties().emplace_back(std::move(bproperty));

                    (*handshake_handle_aux)->local_identity_handle_ = &lih;
                    (*handshake_handle_aux)->remote_identity_handle_ = &rih;
                    *handshake_handle = handshake_handle_aux;
                    *handshake_message_out = &(*handshake_handle_aux)->handshake_message_;
                }

                return ValidationResult_t::VALIDATION_PENDING_CHALLENGE_MESSAGE;
            }
        }
    }

    delete handshake_handle_aux;

    ERR_clear_error();

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
