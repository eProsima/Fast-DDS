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

// TODO This isn't a proper fix for compatibility with OpenSSL 3.0, but
// suppresses the warnings until true OpenSSL 3.0 APIs can be used.
#ifdef OPENSSL_API_COMPAT
#undef OPENSSL_API_COMPAT
#endif // ifdef OPENSSL_API_COMPAT
#define OPENSSL_API_COMPAT 10101

#include <security/authentication/PKIDH.h>

#include <openssl/opensslv.h>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/log/Log.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/security/logging/Logging.h>
#include <rtps/messages/CDRMessage.hpp>
#include <security/authentication/PKIIdentityHandle.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define IS_OPENSSL_1_1 1
#define OPENSSL_CONST const
#else
#define IS_OPENSSL_1_1 0
#define OPENSSL_CONST
#endif // if OPENSSL_VERSION_NUMBER >= 0x10100000L

#if OPENSSL_VERSION_NUMBER >= 0x10101040L
#define IS_OPENSSL_1_1_1d 1
#else
#define IS_OPENSSL_1_1_1d 0
#endif // if OPENSSL_VERSION_NUMBER >= 0x10101040L

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/obj_mac.h>

#include <security/artifact_providers/FileProvider.hpp>
#include <security/artifact_providers/Pkcs11Provider.hpp>

#include <cassert>
#include <algorithm>
#include <utility>

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION " (" __FILE__ ":" S2(__LINE__) ")"
#define _SecurityException_(str) SecurityException(std::string(str) + LOCATION)

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace security;

using ParameterList = eprosima::fastdds::dds::ParameterList;

static const unsigned char* BN_deserialize_raw(
        BIGNUM** bn,
        const unsigned char* raw_pointer,
        size_t length,
        SecurityException& exception)
{
    BIGNUM* bnn = BN_new();

    if (bnn != nullptr)
    {
        if (BN_bin2bn(raw_pointer, static_cast<int>(length), bnn) != nullptr)
        {
            *bn = bnn;
            return raw_pointer + length;
        }
        else
        {
            exception = _SecurityException_("Cannot deserialize DH");
        }

        BN_free(bnn);
    }
    else
    {
        exception = _SecurityException_("OpenSSL library cannot create bignum");
    }

    return nullptr;
}

static bool get_signature_algorithm(
        X509* certificate,
        std::string& signature_algorithm,
        SecurityException& exception)
{
    bool returnedValue = false;
    BUF_MEM* ptr = nullptr;
    OPENSSL_CONST X509_ALGOR* sigalg = nullptr;
    OPENSSL_CONST ASN1_BIT_STRING* sig = nullptr;

    BIO* out = BIO_new(BIO_s_mem());

    if (out != nullptr)
    {
        X509_get0_signature(&sig, &sigalg, certificate);

        if (sigalg != nullptr)
        {
            if (i2a_ASN1_OBJECT(out, sigalg->algorithm) > 0)
            {
                BIO_get_mem_ptr(out, &ptr);

                if (ptr != nullptr)
                {
                    if (strncmp(ptr->data, "ecdsa-with-SHA256", ptr->length) == 0)
                    {
                        signature_algorithm = ECDSA_SHA256;
                        returnedValue = true;
                    }
                    else if (strncmp(ptr->data, "sha256WithRSAEncryption", ptr->length) == 0)
                    {
                        signature_algorithm = RSA_SHA256;
                        returnedValue = true;
                    }
                    else if (strncmp(ptr->data, "sha1WithRSAEncryption", ptr->length) == 0)
                    {
                        signature_algorithm = RSA_SHA256;
                        returnedValue = true;
                    }
                }
                else
                {
                    exception = _SecurityException_("OpenSSL library cannot retrieve mem ptr");
                }
            }
        }
        else
        {
            exception = _SecurityException_("OpenSSL library cannot write cert");
        }

        BIO_free(out);
    }
    else
    {
        exception = _SecurityException_("OpenSSL library cannot allocate mem");
    }

    return returnedValue;
}

// Auxiliary functions
static X509_STORE* load_identity_ca(
        const std::string& identity_ca,
        bool& there_are_crls,
        std::string& ca_sn,
        std::string& ca_algo,
        SecurityException& exception)
{
    if (identity_ca.size() >= 7 && identity_ca.compare(0, 7, "file://") == 0)
    {
        return detail::FileProvider::load_ca(identity_ca, there_are_crls, ca_sn, ca_algo, get_signature_algorithm,
                       exception);
    }

    exception = _SecurityException_(std::string("Unsupported URI format ") + identity_ca);
    return nullptr;
}

static X509* load_certificate(
        const std::string& identity_cert,
        SecurityException& exception)
{
    if (identity_cert.size() >= 7 && identity_cert.compare(0, 7, "file://") == 0)
    {
        return detail::FileProvider::load_certificate(identity_cert, exception);
    }

    exception = _SecurityException_(std::string("Unsupported URI format ") + identity_cert);
    return nullptr;
}

static X509* load_certificate(
        const std::vector<uint8_t>& data)
{
    X509* returnedValue = nullptr;

    if (data.size() <= static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        BIO* cid = BIO_new_mem_buf(data.data(), static_cast<int>(data.size()));

        if (cid != nullptr)
        {
            returnedValue = PEM_read_bio_X509_AUX(cid, NULL, NULL, NULL);
            BIO_free(cid);
        }
    }

    return returnedValue;
}

static bool verify_certificate(
        X509_STORE* store,
        X509* cert,
        const bool there_are_crls)
{
    assert(store);
    assert(cert);

    bool returnedValue = false;

    X509_STORE_CTX* ctx = X509_STORE_CTX_new();

    unsigned long flags = there_are_crls ? X509_V_FLAG_CRL_CHECK : 0;
    if (X509_STORE_CTX_init(ctx, store, cert, NULL) > 0)
    {
        X509_STORE_CTX_set_flags(ctx, flags | /*X509_V_FLAG_X509_STRICT |*/
                X509_V_FLAG_CHECK_SS_SIGNATURE | X509_V_FLAG_POLICY_CHECK);

        if (X509_verify_cert(ctx) > 0)
        {
            returnedValue = true;
        }
        else
        {
            int errorCode = X509_STORE_CTX_get_error(ctx);
            if (errorCode == X509_V_OK)
            {
                EPROSIMA_LOG_WARNING(SECURITY_AUTHENTICATION,
                        "Invalidation error of certificate, but no error code returned.");
            }
            else
            {
                EPROSIMA_LOG_WARNING(SECURITY_AUTHENTICATION, "Invalidation error of certificate  (" << X509_verify_cert_error_string(
                            errorCode) << ")");
            }
        }

        X509_STORE_CTX_cleanup(ctx);
    }
    else
    {
        EPROSIMA_LOG_WARNING(SECURITY_AUTHENTICATION, "Cannot init context for verifying certificate");
    }

    X509_STORE_CTX_free(ctx);

    return returnedValue;
}

static EVP_PKEY* load_private_key(
        X509* certificate,
        const std::string& file,
        const std::string& password,
        SecurityException& exception,
        PKIDH& pkidh)
{
    EVP_PKEY* key = nullptr;

    if (file.size() >= 7 && file.compare(0, 7, "file://") == 0)
    {
        key = detail::FileProvider::load_private_key(certificate, file, password, exception);
    }
    else if (file.size() >= 7 && file.compare(0, 7, "pkcs11:") == 0)
    {
        if (!pkidh.pkcs11_provider)
        {
            pkidh.pkcs11_provider.reset(new detail::Pkcs11Provider());
        }

        key = pkidh.pkcs11_provider->load_private_key(certificate, file, password, exception);
    }
    else
    {
        exception = _SecurityException_(std::string("Unsupported URI format ") + file);
    }

    return key;
}

static bool store_certificate_in_buffer(
        X509* certificate,
        BUF_MEM** ptr,
        SecurityException& exception)
{
    bool returnedValue = false;

    BIO* out = BIO_new(BIO_s_mem());

    if (out != nullptr)
    {
        if (PEM_write_bio_X509(out, certificate) > 0 )
        {
            BIO_get_mem_ptr(out, ptr);

            if (*ptr != nullptr)
            {
                (void)BIO_set_close(out, BIO_NOCLOSE);
                returnedValue = true;
            }
            else
            {
                exception = _SecurityException_("OpenSSL library cannot retrieve mem ptr");
            }
        }
        else
        {
            exception = _SecurityException_("OpenSSL library cannot write cert");
        }

        BIO_free(out);
    }
    else
    {
        exception = _SecurityException_("OpenSSL library cannot allocate mem");
    }

    return returnedValue;
}

static bool sign_sha256(
        EVP_PKEY* private_key,
        const unsigned char* data,
        const size_t data_length,
        std::vector<uint8_t>& signature,
        SecurityException& exception)
{
    assert(private_key);
    assert(data);

    bool returnedValue = false;

#if IS_OPENSSL_1_1
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
#else
    EVP_MD_CTX* ctx = (EVP_MD_CTX*)malloc(sizeof(EVP_MD_CTX));
#endif // if IS_OPENSSL_1_1
    EVP_MD_CTX_init(ctx);
    EVP_PKEY_CTX* pkey;

    auto md = EVP_sha256();
    if (EVP_DigestSignInit(ctx, &pkey, md, NULL, private_key) == 1)
    {
        // TODO (Miguel) don't do this for ECDSA
        EVP_PKEY_CTX_set_rsa_padding(pkey, RSA_PKCS1_PSS_PADDING);
        EVP_PKEY_CTX_set_rsa_mgf1_md(pkey, md);
        EVP_PKEY_CTX_set_rsa_pss_saltlen(pkey, -1);

        if (EVP_DigestSignUpdate(ctx, data, data_length) == 1)
        {
            size_t length = 0;
            if (EVP_DigestSignFinal(ctx, NULL, &length) == 1 && length > 0)
            {
                signature.resize(length);

                if (EVP_DigestSignFinal(ctx, signature.data(), &length) ==  1)
                {
                    signature.resize(length);
                    returnedValue = true;
                }
                else
                {
                    exception =
                            _SecurityException_(std::string("Cannot finish signature (") +
                                    std::to_string(ERR_get_error()) + std::string(")"));
                }
            }
            else
            {
                exception =
                        _SecurityException_(std::string("Cannot retrieve signature length (") +
                                std::to_string(ERR_get_error()) + std::string(")"));
            }
        }
        else
        {
            exception = _SecurityException_(std::string("Cannot sign data (") + std::to_string(
                                ERR_get_error()) + std::string(")"));
        }
    }
    else
    {
        exception = _SecurityException_(std::string("Cannot init signature (") + std::to_string(
                            ERR_get_error()) + std::string(")"));
    }

#if IS_OPENSSL_1_1
    EVP_MD_CTX_free(ctx);
#else
    EVP_MD_CTX_cleanup(ctx);
    free(ctx);
#endif // if IS_OPENSSL_1_1

    return returnedValue;
}

static bool check_sign_sha256(
        X509* certificate,
        const unsigned char* data,
        const size_t data_length,
        const std::vector<uint8_t>& signature,
        SecurityException& exception)
{
    assert(certificate);
    assert(data);

    bool returnedValue = false;

    EVP_MD_CTX* ctx =
#if IS_OPENSSL_1_1
            EVP_MD_CTX_new();
#else
            (EVP_MD_CTX*)malloc(sizeof(EVP_MD_CTX));
#endif // if IS_OPENSSL_1_1
    EVP_MD_CTX_init(ctx);

    EVP_PKEY* pubkey = X509_get_pubkey(certificate);

    if (pubkey != nullptr)
    {
        auto md = EVP_sha256();
        EVP_PKEY_CTX* pkey;
        if (EVP_DigestVerifyInit(ctx, &pkey, md, NULL, pubkey) == 1)
        {
            // TODO (Miguel) don't do this for ECDSA
            EVP_PKEY_CTX_set_rsa_padding(pkey, RSA_PKCS1_PSS_PADDING);
            EVP_PKEY_CTX_set_rsa_mgf1_md(pkey, md);
            EVP_PKEY_CTX_set_rsa_pss_saltlen(pkey, -2);

            if (EVP_DigestVerifyUpdate(ctx, data, data_length) == 1)
            {
                if (EVP_DigestVerifyFinal(ctx, signature.data(), signature.size()) == 1)
                {
                    returnedValue = true;
                }
                else
                {
                    EPROSIMA_LOG_WARNING(SECURITY_AUTHENTICATION,
                            "Signature verification error (" << ERR_get_error() << ")");
                }
            }
            else
            {
                exception =
                        _SecurityException_(std::string("Cannot update signature check (") +
                                std::to_string(ERR_get_error()) + std::string(")"));
            }

        }
        else
        {
            exception =
                    _SecurityException_(std::string("Cannot init signature check (") + std::to_string(
                                ERR_get_error()) + std::string(")"));
        }

        EVP_PKEY_free(pubkey);
    }
    else
    {
        exception = _SecurityException_("Cannot get public key from certificate");
    }

#if IS_OPENSSL_1_1
    EVP_MD_CTX_free(ctx);
#else
    EVP_MD_CTX_cleanup(ctx);
    free(ctx);
#endif // if IS_OPENSSL_1_1

    return returnedValue;
}

static X509_CRL* load_crl(
        const std::string& identity_crl,
        SecurityException& exception)
{
    if (identity_crl.size() >= 7 && identity_crl.compare(0, 7, "file://") == 0)
    {
        return detail::FileProvider::load_crl(identity_crl, exception);
    }

    exception = _SecurityException_(std::string("Unsupported URI format ") + identity_crl);
    return nullptr;
}

static bool adjust_participant_key(
        X509* cert,
        const GUID_t& candidate_participant_key,
        GUID_t& adjusted_participant_key,
        SecurityException& exception)
{
    assert(cert != nullptr);

    X509_NAME* cert_sn = X509_get_subject_name(cert);
    assert(cert_sn != nullptr);

    unsigned char md[SHA256_DIGEST_LENGTH];
    unsigned int length = 0;

    if (!X509_NAME_digest(cert_sn, EVP_sha256(), md, &length) || length != SHA256_DIGEST_LENGTH)
    {
        exception = _SecurityException_("OpenSSL library cannot hash sha256");
        return false;
    }

    adjusted_participant_key.guidPrefix.value[0] = 0x80 | (md[0] >> 1);
    adjusted_participant_key.guidPrefix.value[1] = (md[0] << 7) | (md[1] >> 1);
    adjusted_participant_key.guidPrefix.value[2] = (md[1] << 7) | (md[2] >> 1);
    adjusted_participant_key.guidPrefix.value[3] = (md[2] << 7) | (md[3] >> 1);
    adjusted_participant_key.guidPrefix.value[4] = (md[3] << 7) | (md[4] >> 1);
    adjusted_participant_key.guidPrefix.value[5] = (md[4] << 7) | (md[5] >> 1);

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

    if (!EVP_Digest(&key, 16, md, NULL, EVP_sha256(), NULL))
    {
        exception = _SecurityException_("OpenSSL library cannot hash sha256");
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

static int get_dh_type(
        const std::string& algorithm)
{
    auto raw_alg = algorithm.c_str();
    if (strcmp(DH_2048_256, raw_alg) == 0)
    {
        return EVP_PKEY_DH;
    }
    else if (strcmp(ECDH_prime256v1, raw_alg) == 0)
    {
        return EVP_PKEY_EC;
    }

    return 0;
}

static EVP_PKEY* generate_dh_key(
        int type,
        SecurityException& exception)
{
    EVP_PKEY_CTX* pctx = nullptr;
    EVP_PKEY* params = nullptr;

    if (type == EVP_PKEY_EC)
    {
        pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
        if (pctx != nullptr)
        {
            if ((1 != EVP_PKEY_paramgen_init(pctx)) ||
                    (1 != EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1)) ||
                    (1 != EVP_PKEY_paramgen(pctx, &params)))
            {
                exception = _SecurityException_("Cannot set default parameters: ");
                EVP_PKEY_CTX_free(pctx);
                return nullptr;
            }
        }
        else
        {
            exception = _SecurityException_("Cannot allocate EVP parameters");
            return nullptr;
        }
    }
    else if (type == EVP_PKEY_DH)
    {
        params = EVP_PKEY_new();
        if (params != nullptr)
        {
            DH* dh = DH_get_2048_256();
            if (dh != nullptr)
            {
#if IS_OPENSSL_1_1_1d
                int dh_type = DH_get0_q(dh) == NULL ? EVP_PKEY_DH : EVP_PKEY_DHX;
                if (EVP_PKEY_assign(params, dh_type, dh) <= 0)
#else
                if (EVP_PKEY_assign_DH(params, dh) <= 0)
#endif // if IS_OPENSSL_1_1_1d
                {
                    exception = _SecurityException_("Cannot set default parameters: ");
                    DH_free(dh);
                    EVP_PKEY_free(params);
                    return nullptr;
                }
            }
        }
        else
        {
            exception = _SecurityException_("Cannot allocate EVP parameters");
            return nullptr;
        }
    }
    else
    {
        exception = _SecurityException_("Wrong DH kind");
        return nullptr;
    }

    EVP_PKEY* keys = nullptr;
    EVP_PKEY_CTX* kctx = EVP_PKEY_CTX_new(params, NULL);

    if (kctx != nullptr)
    {
        if (1 == EVP_PKEY_keygen_init(kctx))
        {
            if (1 != EVP_PKEY_keygen(kctx, &keys))
            {
                exception = _SecurityException_("Cannot generate EVP key");
            }
        }
        else
        {
            exception = _SecurityException_("Cannot init EVP key");
        }

        EVP_PKEY_CTX_free(kctx);
    }
    else
    {
        exception = _SecurityException_("Cannot create EVP context");
    }

    ERR_clear_error();
    EVP_PKEY_free(params);
    if (pctx != nullptr)
    {
        EVP_PKEY_CTX_free(pctx);
    }
    return keys;
}

static bool store_dh_public_key(
        EVP_PKEY* dhkey,
        int type,
        std::vector<uint8_t>& buffer,
        SecurityException& exception)
{
    bool returnedValue = false;

    if (type == EVP_PKEY_DH)
    {
        const DH* dh =
#if IS_OPENSSL_1_1
                EVP_PKEY_get0_DH(dhkey);
#else
                dhkey->pkey.dh;
#endif // if IS_OPENSSL_1_1

        if (dh != nullptr)
        {
#if IS_OPENSSL_1_1
            const BIGNUM* pub_key = nullptr;
            const BIGNUM* priv_key = nullptr;
            DH_get0_key(dh, &pub_key, &priv_key);

#else
            const BIGNUM* pub_key = dh->pub_key;
#endif // if IS_OPENSSL_1_1

            int len = BN_num_bytes(pub_key);
            buffer.resize(len);
            unsigned char* pointer = buffer.data();
            if (BN_bn2bin(pub_key, pointer) == len)
            {
                returnedValue = true;
            }
            else
            {
                exception = _SecurityException_("Cannot serialize public key");
            }
        }
        else
        {
            exception = _SecurityException_("OpenSSL library doesn't retrieve DH");
        }
    }
    else if (type == EVP_PKEY_EC)
    {
        const EC_KEY* ec =
#if IS_OPENSSL_1_1
                EVP_PKEY_get0_EC_KEY(dhkey);
#else
                dhkey->pkey.ec;
#endif // if IS_OPENSSL_1_1
        if (ec != nullptr)
        {
            auto grp = EC_KEY_get0_group(ec);
            auto pub_key = EC_KEY_get0_public_key(ec);
            auto len = EC_POINT_point2oct(grp, pub_key, EC_KEY_get_conv_form(ec), NULL, 0, NULL);
            buffer.resize(len);
            if (EC_POINT_point2oct(grp, pub_key, EC_KEY_get_conv_form(ec), buffer.data(), len, NULL) == len)
            {
                returnedValue = true;
            }
            else
            {
                exception = _SecurityException_("Cannot serialize public key");
            }
        }
        else
        {
            exception = _SecurityException_("OpenSSL library doesn't retrieve DH");
        }
    }
    else
    {
        exception = _SecurityException_("Wrong DH kind");
    }

    return returnedValue;
}

static EVP_PKEY* generate_dh_peer_key(
        const std::vector<uint8_t>& buffer,
        SecurityException& exception,
        int alg_kind)
{
    if (alg_kind == EVP_PKEY_DH)
    {
        DH* dh = DH_get_2048_256();

        if (dh != nullptr)
        {
            const unsigned char* pointer = buffer.data();

#if IS_OPENSSL_1_1
            BIGNUM* pub_key_ptr;
            BIGNUM** pub_key = &pub_key_ptr;
#else
            BIGNUM** pub_key = &dh->pub_key;
#endif // if IS_OPENSSL_1_1

            if ((pointer = BN_deserialize_raw(pub_key, buffer.data(), buffer.size(), exception)) != nullptr)
            {
#if IS_OPENSSL_1_1
                DH_set0_key(dh, *pub_key, NULL);
#endif // if IS_OPENSSL_1_1
                EVP_PKEY* key = EVP_PKEY_new();

                if (key != nullptr)
                {
#if IS_OPENSSL_1_1_1d
                    int type = DH_get0_q(dh) == NULL ? EVP_PKEY_DH : EVP_PKEY_DHX;
                    if (EVP_PKEY_assign(key, type, dh) > 0)
#else
                    if (EVP_PKEY_assign_DH(key, dh) > 0)
#endif // if IS_OPENSSL_1_1_1d
                    {
                        return key;
                    }
                    else
                    {
                        exception = _SecurityException_("OpenSSL library cannot set dh in pkey");
                        DH_free(dh);
                    }

                    EVP_PKEY_free(key);
                }
                else
                {
                    exception = _SecurityException_("OpenSSL library cannot create pkey");
                }
            }
            else
            {
                exception = _SecurityException_("Cannot deserialize public key");
            }
        }
        else
        {
            exception = _SecurityException_("OpenSSL library cannot create dh");
        }
    }
    else if (alg_kind == EVP_PKEY_EC)
    {
        EC_KEY* ec = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);

        if (ec != nullptr)
        {
            const unsigned char* pointer = buffer.data();

#if IS_OPENSSL_1_1
            if (EC_KEY_oct2key(ec, pointer, buffer.size(), NULL) > 0)
#else
            if (o2i_ECPublicKey(&ec, &pointer, (long) buffer.size()) != nullptr)
#endif // if IS_OPENSSL_1_1
            {
                EVP_PKEY* key = EVP_PKEY_new();

                if (key != nullptr)
                {
                    if (EVP_PKEY_assign_EC_KEY(key, ec) > 0)
                    {
                        return key;
                    }
                    else
                    {
                        exception = _SecurityException_("OpenSSL library cannot set ec in pkey");
                    }

                    EVP_PKEY_free(key);
                }
                else
                {
                    exception = _SecurityException_("OpenSSL library cannot create pkey");
                }
            }
            else
            {
                exception = _SecurityException_("Cannot deserialize public key");
            }

            EC_KEY_free(ec);
        }
        else
        {
            exception = _SecurityException_("OpenSSL library cannot create ec");
        }
    }
    else
    {
        exception = _SecurityException_("Wrong DH kind");
    }

    return nullptr;
}

static bool generate_challenge(
        std::vector<uint8_t>& vector,
        SecurityException& exception)
{
    bool returnedValue = false;
    BIGNUM* bn = BN_new();

    if (BN_rand(bn, 256, 0 /*BN_RAND_TOP_ONE*/, 0 /*BN_RAND_BOTTOM_ANY*/))
    {
        int len = BN_num_bytes(bn);
        vector.resize(len);

        if (BN_bn2bin(bn, vector.data()) == len)
        {
            returnedValue = true;
        }
        else
        {
            exception = _SecurityException_("OpenSSL library cannot store challenge");
        }
    }

    BN_clear_free(bn);

    return returnedValue;
}

/**
 * @brief Convert a signature algortihm before adding it to an IdentityToken.
 *
 * This methods converts the signature algorithm to the format used in the IdentityToken.
 * Depending on the value of the use_legacy parameter, the algorithm will be converted to the legacy format or to the
 * one specified in the DDS-SEC 1.1 specification.
 *
 * @param algorithm The algorithm to convert.
 * @param use_legacy Whether to use the legacy format or not.
 *
 * @return The converted algorithm.
 */
static std::string convert_to_token_algo(
        const std::string& algorithm,
        bool use_legacy)
{
    // Leave as internal format when legacy is used
    if (use_legacy)
    {
        return algorithm;
    }

    // Convert to token format
    if (algorithm == RSA_SHA256)
    {
        return RSA_SHA256_FOR_TOKENS;
    }
    else if (algorithm == ECDSA_SHA256)
    {
        return ECDSA_SHA256_FOR_TOKENS;
    }

    return algorithm;
}

/**
 * @brief Parse a signature algorithm from an IdentityToken.
 *
 * This method parses the signature algorithm from an IdentityToken.
 * It converts the algorithm to the internal (legacy) format used by the library.
 *
 * @param algorithm The algorithm to parse.
 *
 * @return The parsed algorithm.
 */
static std::string parse_token_algo(
        const std::string& algorithm)
{
    // Convert to internal format, allowing both legacy and new formats
    if (algorithm == RSA_SHA256_FOR_TOKENS)
    {
        return RSA_SHA256;
    }
    else if (algorithm == ECDSA_SHA256_FOR_TOKENS)
    {
        return ECDSA_SHA256;
    }

    return algorithm;
}

std::shared_ptr<SecretHandle> PKIDH::generate_sharedsecret(
        EVP_PKEY* private_key,
        EVP_PKEY* public_key,
        SecurityException& exception) const
{
    assert(private_key);
    assert(public_key);

    std::shared_ptr<SharedSecretHandle> handle;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(private_key, NULL);

    if (ctx != nullptr)
    {
        if (EVP_PKEY_derive_init(ctx) > 0)
        {
            if (EVP_PKEY_derive_set_peer(ctx, public_key) > 0)
            {
                size_t length = 0;
                if (EVP_PKEY_derive(ctx, NULL, &length) > 0)
                {
                    SharedSecret::BinaryData data;
                    data.name("SharedSecret");
                    data.value().resize(length);

                    if (EVP_PKEY_derive(ctx, data.value().data(), &length) > 0)
                    {
                        uint8_t md[32];
                        if (EVP_Digest(data.value().data(), length, md, NULL, EVP_sha256(), NULL))
                        {
                            data.value().assign(md, md + 32);
                            handle = std::dynamic_pointer_cast<SharedSecretHandle>(
                                get_shared_secret(SharedSecretHandle::nil_handle, exception));
                            (*handle)->data_.push_back(std::move(data));
                        }
                        else
                        {
                            exception = _SecurityException_("OpenSSL library failed while getting derived key");
                        }
                    }
                    else
                    {
                        exception = _SecurityException_("OpenSSL library cannot get derive");
                    }
                }
                else
                {
                    exception = _SecurityException_("OpenSSL library cannot get length");
                }
            }
            else
            {
                exception = _SecurityException_("OpenSSL library cannot set peer");
            }
        }
        else
        {
            exception = _SecurityException_("OpenSSL library cannot init derive");
        }

        EVP_PKEY_CTX_free(ctx);
    }
    else
    {
        exception = _SecurityException_("OpenSSL library cannot allocate context");
    }

    return std::dynamic_pointer_cast<SecretHandle>(handle);
}

static bool generate_identity_token(
        PKIIdentityHandle& handle,
        bool transmit_legacy_algorithms)
{
    Property property;
    IdentityToken& token = handle->identity_token_;
    token.class_id("DDS:Auth:PKI-DH:1.0");

    property.name("dds.cert.sn");
    property.value() = handle->cert_sn_;
    property.propagate(true);
    token.properties().push_back(std::move(property));

    property.name("dds.cert.algo");
    property.value() = convert_to_token_algo(handle->sign_alg_, transmit_legacy_algorithms);
    property.propagate(true);
    token.properties().push_back(std::move(property));

    property.name("dds.ca.sn");
    property.value() = handle->sn;
    property.propagate(true);
    token.properties().push_back(std::move(property));

    property.name("dds.ca.algo");
    property.value() = convert_to_token_algo(handle->algo, transmit_legacy_algorithms);
    property.propagate(true);
    token.properties().push_back(std::move(property));

    return true;
}

ValidationResult_t PKIDH::validate_local_identity(
        IdentityHandle** local_identity_handle,
        GUID_t& adjusted_participant_key,
        const uint32_t /*domain_id*/,
        const RTPSParticipantAttributes& participant_attr,
        const GUID_t& candidate_participant_key,
        SecurityException& exception)
{
    assert(local_identity_handle);

    PropertyPolicy auth_properties = PropertyPolicyHelper::get_properties_with_prefix(participant_attr.properties,
                    "dds.sec.auth.builtin.PKI-DH.");

    if (PropertyPolicyHelper::length(auth_properties) == 0)
    {
        exception = _SecurityException_("Not found any dds.sec.auth.builtin.PKI-DH property");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    bool transmit_legacy_algorithms = false;
    std::string* legacy = PropertyPolicyHelper::find_property(auth_properties, "transmit_algorithms_as_legacy");
    if (legacy != nullptr)
    {
        transmit_legacy_algorithms = (*legacy == "true");
    }

    std::string* identity_ca = PropertyPolicyHelper::find_property(auth_properties, "identity_ca");

    if (identity_ca == nullptr)
    {
        exception = _SecurityException_("Not found dds.sec.auth.builtin.PKI-DH.identity_ca property");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    std::string* identity_cert = PropertyPolicyHelper::find_property(auth_properties, "identity_certificate");

    if (identity_cert == nullptr)
    {
        exception = _SecurityException_("Not found dds.sec.auth.builtin.PKI-DH.identity_certificate property");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    std::string* identity_crl = PropertyPolicyHelper::find_property(auth_properties, "identity_crl");

    std::string* private_key = PropertyPolicyHelper::find_property(auth_properties, "private_key");

    if (private_key == nullptr)
    {
        exception = _SecurityException_("Not found dds.sec.auth.builtin.PKI-DH.private_key property");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    std::string* password = PropertyPolicyHelper::find_property(auth_properties, "password");
    std::string empty_password;

    if (password == nullptr)
    {
        password = &empty_password;
    }

    std::string key_agreement_algorithm = "AUTO";
    std::string* key_agreement_property =
            PropertyPolicyHelper::find_property(auth_properties, "preferred_key_agreement");
    if (nullptr != key_agreement_property)
    {
        const std::pair<std::string, std::string> key_agreement_allowed_values[] = {
            {DH_2048_256, DH_2048_256},
            {ECDH_prime256v1, ECDH_prime256v1},
            {"ECDH", ECDH_prime256v1},
            {"DH", DH_2048_256},
            {"AUTO", "AUTO"}
        };

        key_agreement_algorithm = "";
        for (const auto& allowed_value : key_agreement_allowed_values)
        {
            if (key_agreement_property->compare(allowed_value.first) == 0)
            {
                key_agreement_algorithm = allowed_value.second;
                break;
            }
        }

        if (key_agreement_algorithm.empty())
        {
            exception = _SecurityException_("Invalid key agreement algorithm '" + *key_agreement_property + "'");
            EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
            return ValidationResult_t::VALIDATION_FAILED;
        }
    }

    PKIIdentityHandle* ih = &PKIIdentityHandle::narrow(*get_identity_handle(exception));

    (*ih)->store_ = load_identity_ca(*identity_ca, (*ih)->there_are_crls_, (*ih)->sn, (*ih)->algo,
                    exception);

    if ((*ih)->store_ != nullptr)
    {
        ERR_clear_error();

        if (key_agreement_algorithm == "AUTO")
        {
            if ((*ih)->algo == RSA_SHA256)
            {
                key_agreement_algorithm = DH_2048_256;
            }
            else
            {
                key_agreement_algorithm = ECDH_prime256v1;
            }
        }

        (*ih)->kagree_alg_ = key_agreement_algorithm;

        if (identity_crl != nullptr)
        {
            X509_CRL* crl = load_crl(*identity_crl, exception);

            if (crl != nullptr)
            {
                X509_STORE_add_crl((*ih)->store_, crl);
                X509_CRL_free(crl);
                (*ih)->there_are_crls_ = true;
            }
            else
            {
                delete ih;
                EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
                return ValidationResult_t::VALIDATION_FAILED;
            }
        }

        ERR_clear_error();

        (*ih)->cert_ = load_certificate(*identity_cert, exception);

        if ((*ih)->cert_ != nullptr)
        {
            // Get subject name.
            X509_NAME* cert_sn = X509_get_subject_name((*ih)->cert_);
            assert(cert_sn != nullptr);
            char* cert_sn_str = X509_NAME_oneline(cert_sn, 0, 0);
            assert(cert_sn_str != nullptr);
            (*ih)->cert_sn_ = cert_sn_str;
            OPENSSL_free(cert_sn_str);
            BIO* cert_sn_rfc2253_str = BIO_new(BIO_s_mem());
            X509_NAME_print_ex(cert_sn_rfc2253_str, cert_sn, 0, XN_FLAG_RFC2253 & ~ASN1_STRFLGS_ESC_MSB);
            const int bufsize = 1024;
            char buffer[bufsize];
            int length = BIO_read(cert_sn_rfc2253_str, buffer, bufsize);
            BIO_free(cert_sn_rfc2253_str);
            (*ih)->cert_sn_rfc2253_.assign(buffer, length);


            if (verify_certificate((*ih)->store_, (*ih)->cert_, (*ih)->there_are_crls_))
            {
                if (store_certificate_in_buffer((*ih)->cert_, &(*ih)->cert_content_, exception))
                {
                    if (get_signature_algorithm((*ih)->cert_, (*ih)->sign_alg_, exception))
                    {
                        (*ih)->pkey_ = load_private_key((*ih)->cert_, *private_key, *password, exception, *this);

                        if ((*ih)->pkey_ != nullptr)
                        {
                            if (adjust_participant_key((*ih)->cert_, candidate_participant_key,
                                    adjusted_participant_key, exception))
                            {
                                // Generate IdentityToken.
                                if (generate_identity_token(*ih, transmit_legacy_algorithms))
                                {
                                    (*ih)->participant_key_ = adjusted_participant_key;
                                    *local_identity_handle = ih;

                                    return ValidationResult_t::VALIDATION_OK;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());

    delete ih;

    ERR_clear_error();

    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::validate_remote_identity(
        IdentityHandle** remote_identity_handle,
        const IdentityHandle& local_identity_handle,
        const IdentityToken& remote_identity_token,
        const GUID_t& remote_participant_key,
        SecurityException& exception)
{
    assert(remote_identity_handle);
    assert(local_identity_handle.nil() == false);

    ValidationResult_t returnedValue = ValidationResult_t::VALIDATION_FAILED;

    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(local_identity_handle);

    if (!lih.nil() && !remote_identity_token.is_nil())
    {
        // dds.ca.sn
        const std::string* ca_sn = DataHolderHelper::find_property_value(remote_identity_token, "dds.ca.sn");

        // dds.cert.sn
        // const std::string* cert_sn = DataHolderHelper::find_property_value(remote_identity_token, "dds.cert.sn");

        // dds.cert.algo
        const std::string* cert_algo = DataHolderHelper::find_property_value(remote_identity_token, "dds.cert.algo");

        PKIIdentityHandle* rih = &PKIIdentityHandle::narrow(*get_identity_handle(exception));

        (*rih)->sn = ca_sn ? *ca_sn : "";
        (*rih)->cert_sn_ = ""; // cert_sn ? *cert_sn : "";
        (*rih)->algo = cert_algo ? parse_token_algo(*cert_algo) : "";
        (*rih)->participant_key_ = remote_participant_key;
        *remote_identity_handle = rih;

        if (lih->participant_key_ < remote_participant_key )
        {
            returnedValue = ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_REQUEST;
        }
        else
        {
            returnedValue = ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE;
        }
    }

    return returnedValue;
}

ValidationResult_t PKIDH::begin_handshake_request(
        HandshakeHandle** handshake_handle,
        HandshakeMessageToken** handshake_message,
        const IdentityHandle& initiator_identity_handle,
        IdentityHandle& replier_identity_handle,
        const CDRMessage_t& cdr_participant_data,
        SecurityException& exception)
{
    assert(handshake_handle);
    assert(handshake_message);
    assert(initiator_identity_handle.nil() == false);
    assert(replier_identity_handle.nil() == false);

    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(initiator_identity_handle);
    PKIIdentityHandle& rih = PKIIdentityHandle::narrow(replier_identity_handle);

    if (lih.nil() || rih.nil())
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if (cdr_participant_data.length == 0)
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
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
    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

    // c.perm
    if (lih->permissions_credential_token_.class_id().compare("DDS:Access:PermissionsCredential") == 0)
    {
        const Property* permissions_file = DataHolderHelper::find_property(lih->permissions_credential_token_,
                        "dds.perm.cert");

        if (permissions_file != nullptr)
        {
            bproperty.name("c.perm");
            bproperty.value().assign(permissions_file->value().begin(), permissions_file->value().end());
            bproperty.propagate(true);
            (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));
        }
        else
        {
            exception = _SecurityException_("Cannot find permissions file in permissions credential token");
            EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
            delete handshake_handle_aux;
            return ValidationResult_t::VALIDATION_FAILED;
        }
    }

    // c.pdata
    bproperty.name("c.pdata");
    bproperty.value().assign(cdr_participant_data.buffer,
            cdr_participant_data.buffer + cdr_participant_data.length);
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

    // c.dsign_algo.
    bproperty.name("c.dsign_algo");
    bproperty.value().assign(lih->sign_alg_.begin(),
            lih->sign_alg_.end());
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

    // c.kagree_algo.
    bproperty.name("c.kagree_algo");
    bproperty.value().assign(lih->kagree_alg_.begin(),
            lih->kagree_alg_.end());
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

    // hash_c1
    CDRMessage_t message(static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(
                (*handshake_handle_aux)->handshake_message_.binary_properties())));
    message.msg_endian = BIGEND;
    CDRMessage::addBinaryPropertySeq(&message, (*handshake_handle_aux)->handshake_message_.binary_properties(), false);
    if (!EVP_Digest(message.buffer, message.length, md, NULL, EVP_sha256(), NULL))
    {
        exception = _SecurityException_("OpenSSL library cannot hash sha256");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        delete handshake_handle_aux;
        return ValidationResult_t::VALIDATION_FAILED;
    }
    bproperty.name("hash_c1");
    bproperty.value().assign(md, md + SHA256_DIGEST_LENGTH);
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

    int kagree_kind = get_dh_type((*handshake_handle_aux)->kagree_alg_);

    // dh1
    if (((*handshake_handle_aux)->dhkeys_ = generate_dh_key(kagree_kind, exception)) != nullptr)
    {
        bproperty.name("dh1");
        bproperty.propagate(true);

        if (store_dh_public_key((*handshake_handle_aux)->dhkeys_, kagree_kind, bproperty.value(), exception))
        {
            (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

            // challenge1
            bproperty.name("challenge1");
            bproperty.propagate(true);
            if (generate_challenge(bproperty.value(), exception))
            {
                (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

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

ValidationResult_t PKIDH::begin_handshake_reply(
        HandshakeHandle** handshake_handle,
        HandshakeMessageToken** handshake_message_out,
        HandshakeMessageToken&& handshake_message_in,
        IdentityHandle& initiator_identity_handle,
        const IdentityHandle& replier_identity_handle,
        const CDRMessage_t& cdr_participant_data,
        SecurityException& exception)
{
    assert(handshake_handle);
    assert(handshake_message_out);
    assert(initiator_identity_handle.nil() == false);
    assert(replier_identity_handle.nil() == false);

    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(replier_identity_handle);
    PKIIdentityHandle& rih = PKIIdentityHandle::narrow(initiator_identity_handle);

    if (lih.nil() || rih.nil())
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if (cdr_participant_data.length == 0)
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check TokenMessage
    if (handshake_message_in.class_id().compare("DDS:Auth:PKI-DH:1.0+Req") != 0)
    {
        WARNING_SECURITY_LOGGING("PKIDH", std::string("Bad HandshakeMessageToken (") +
                handshake_message_in.class_id() + ")");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check incoming handshake.
    // Check c.id
    const std::vector<uint8_t>* cid = DataHolderHelper::find_binary_property_value(handshake_message_in, "c.id");
    if (cid == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property c.id");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    //! Release the memory in case of having an old certificate
    if (rih->cert_ != nullptr)
    {
        X509_free(rih->cert_);
        rih->cert_ = nullptr;
    }

    rih->cert_ = load_certificate(*cid);

    if (rih->cert_ == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot load certificate");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    X509_NAME* cert_sn = X509_get_subject_name(rih->cert_);
    assert(cert_sn != nullptr);
    char* cert_sn_str = X509_NAME_oneline(cert_sn, 0, 0);
    assert(cert_sn_str != nullptr);
    if (!rih->cert_sn_.empty() && rih->cert_sn_.compare(cert_sn_str) != 0)
    {
        OPENSSL_free(cert_sn_str);
        WARNING_SECURITY_LOGGING("PKIDH", "Certificated subject name invalid");
        return ValidationResult_t::VALIDATION_FAILED;
    }
    rih->cert_sn_.assign(cert_sn_str);
    OPENSSL_free(cert_sn_str);
    BIO* cert_sn_rfc2253_str = BIO_new(BIO_s_mem());
    X509_NAME_print_ex(cert_sn_rfc2253_str, cert_sn, 0, XN_FLAG_RFC2253 & ~ASN1_STRFLGS_ESC_MSB);
    const int bufsize = 1024;
    char buffer[bufsize];
    int str_length = BIO_read(cert_sn_rfc2253_str, buffer, bufsize);
    BIO_free(cert_sn_rfc2253_str);
    rih->cert_sn_rfc2253_.assign(buffer, str_length);

    if (!verify_certificate(lih->store_, rih->cert_, lih->there_are_crls_))
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Error verifying certificate");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // c.perm
    if (lih->permissions_credential_token_.class_id().compare("DDS:Access:PermissionsCredential") == 0)
    {
        const std::vector<uint8_t>* perm = DataHolderHelper::find_binary_property_value(handshake_message_in,
                        "c.perm");

        if (perm == nullptr)
        {
            WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property c.perm");
            return ValidationResult_t::VALIDATION_FAILED;
        }

        rih->permissions_credential_token_.class_id("DDS:Access:PermissionsCredential");
        Property permission_file;
        permission_file.name("dds.perm.cert");
        permission_file.value().assign(perm->begin(), perm->end());
        rih->permissions_credential_token_.properties().push_back(std::move(permission_file));
    }

    const std::vector<uint8_t>* pdata = DataHolderHelper::find_binary_property_value(handshake_message_in, "c.pdata");

    if (pdata == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property c.pdata");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    GUID_t participant_guid;
    CDRMessage_t cdr_pdata(0);
    cdr_pdata.wraps = true;
    cdr_pdata.msg_endian = BIGEND;
    cdr_pdata.length = (uint32_t)pdata->size();
    cdr_pdata.max_size = (uint32_t)pdata->size();
    cdr_pdata.buffer = (octet*)pdata->data();

    if (!ParameterList::read_guid_from_cdr_msg(cdr_pdata, fastdds::dds::PID_PARTICIPANT_GUID, participant_guid))
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot deserialize ParticipantProxyData in property c.pdata");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if ((participant_guid.guidPrefix.value[0] & 0x80) != 0x80)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Bad participant_key's first bit in c.pdata");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    unsigned char md[SHA256_DIGEST_LENGTH];
    unsigned char hash_c1[SHA256_DIGEST_LENGTH];
    unsigned int length = 0;

    if (!X509_NAME_digest(cert_sn, EVP_sha256(), md, &length) || length != SHA256_DIGEST_LENGTH)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot generate SHA256 of subject name");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    md[5] &= 0xFE;
    unsigned char bytes[6]{
        static_cast<unsigned char>((participant_guid.guidPrefix.value[0] << 1) |
        (participant_guid.guidPrefix.value[1] >> 7)),
        static_cast<unsigned char>((participant_guid.guidPrefix.value[1] << 1) |
        (participant_guid.guidPrefix.value[2] >> 7)),
        static_cast<unsigned char>((participant_guid.guidPrefix.value[2] << 1) |
        (participant_guid.guidPrefix.value[3] >> 7)),
        static_cast<unsigned char>((participant_guid.guidPrefix.value[3] << 1) |
        (participant_guid.guidPrefix.value[4] >> 7)),
        static_cast<unsigned char>((participant_guid.guidPrefix.value[4] << 1) |
        (participant_guid.guidPrefix.value[5] >> 7)),
        static_cast<unsigned char>(participant_guid.guidPrefix.value[5] << 1)
    };

    if (memcmp(md, bytes, 6) != 0)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Bad participant_key's 47bits in c.pdata");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // c.dsign_algo
    const std::vector<uint8_t>* dsign_algo = DataHolderHelper::find_binary_property_value(handshake_message_in,
                    "c.dsign_algo");

    if (dsign_algo == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property c.dsign_algo");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check signature algorithm
    std::string s_dsign_algo(dsign_algo->begin(), dsign_algo->end());
    if (strcmp(RSA_SHA256, s_dsign_algo.c_str()) != 0 &&
            strcmp(ECDSA_SHA256, s_dsign_algo.c_str()) != 0)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Not supported signature algorithm (" + s_dsign_algo + ")");
        return ValidationResult_t::VALIDATION_FAILED;
    }
    rih->sign_alg_ = std::move(s_dsign_algo);

    // c.kagree_algo
    const std::vector<uint8_t>* kagree_algo = DataHolderHelper::find_binary_property_value(handshake_message_in,
                    "c.kagree_algo");

    if (kagree_algo == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property c.kagree_algo");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check key agreement algorithm
    std::string s_kagree_algo(kagree_algo->begin(), kagree_algo->end());
    if (strcmp(DH_2048_256, s_kagree_algo.c_str()) != 0 &&
            strcmp(ECDH_prime256v1, s_kagree_algo.c_str()) != 0)
    {
        WARNING_SECURITY_LOGGING("PKIDH", std::string("Not supported key agreement algorithm (") + s_kagree_algo + ")");
        return ValidationResult_t::VALIDATION_FAILED;
    }
    rih->kagree_alg_ = std::move(s_kagree_algo);

    CDRMessage_t cdrmessage(static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(
                handshake_message_in.binary_properties())));
    cdrmessage.msg_endian = BIGEND;
    CDRMessage::addBinaryPropertySeq(&cdrmessage, handshake_message_in.binary_properties(), "c.", false);

    if (!EVP_Digest(cdrmessage.buffer, cdrmessage.length, hash_c1, NULL, EVP_sha256(), NULL))
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot generate SHA256 of request");
        return ValidationResult_t::VALIDATION_FAILED;
    }
    else
    {
        // hash_c1
        std::vector<uint8_t>* hash_c1_vec =
                DataHolderHelper::find_binary_property_value(handshake_message_in, "hash_c1");

        if (hash_c1_vec != nullptr)
        {
            if ((hash_c1_vec->size() == SHA256_DIGEST_LENGTH) &&
                    (memcmp(hash_c1, hash_c1_vec->data(), SHA256_DIGEST_LENGTH) != 0))
            {
                WARNING_SECURITY_LOGGING("PKIDH", "Wrong hash_c1");
            }
        }
    }

    // dh1
    std::vector<uint8_t>* dh1 = DataHolderHelper::find_binary_property_value(handshake_message_in, "dh1");

    if (dh1 == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property dh1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // challenge1
    std::vector<uint8_t>* challenge1 = DataHolderHelper::find_binary_property_value(handshake_message_in, "challenge1");

    if (challenge1 == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property challenge1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Generate handshake reply message token.
    PKIHandshakeHandle* handshake_handle_aux = new PKIHandshakeHandle();
    (*handshake_handle_aux)->kagree_alg_ = rih->kagree_alg_;
    (*handshake_handle_aux)->handshake_message_.class_id("DDS:Auth:PKI-DH:1.0+Reply");

    int kagree_kind = get_dh_type((*handshake_handle_aux)->kagree_alg_);

    // Store dh1
    if (((*handshake_handle_aux)->peerkeys_ = generate_dh_peer_key(*dh1, exception, kagree_kind)) == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot store peer key from dh1");
        delete handshake_handle_aux;
        return ValidationResult_t::VALIDATION_FAILED;
    }

    BinaryProperty bproperty;

    // c.id
    bproperty.name("c.id");
    bproperty.value().assign(lih->cert_content_->data,
            lih->cert_content_->data + lih->cert_content_->length);
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

    // c.perm
    if (lih->permissions_credential_token_.class_id().compare("DDS:Access:PermissionsCredential") == 0)
    {
        const Property* permissions_file = DataHolderHelper::find_property(lih->permissions_credential_token_,
                        "dds.perm.cert");

        if (permissions_file != nullptr)
        {
            bproperty.name("c.perm");
            bproperty.value().assign(permissions_file->value().begin(), permissions_file->value().end());
            bproperty.propagate(true);
            (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));
        }
        else
        {
            exception = _SecurityException_("Cannot find permissions file in permissions credential token");
            EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
            return ValidationResult_t::VALIDATION_FAILED;
        }
    }

    // c.pdata
    bproperty.name("c.pdata");
    bproperty.value().assign(cdr_participant_data.buffer,
            cdr_participant_data.buffer + cdr_participant_data.length);
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

    // c.dsign_algo.
    bproperty.name("c.dsign_algo");
    bproperty.value().assign(lih->sign_alg_.begin(),
            lih->sign_alg_.end());
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

    // c.kagree_algo.
    bproperty.name("c.kagree_algo");
    bproperty.value().assign((*handshake_handle_aux)->kagree_alg_.begin(),
            (*handshake_handle_aux)->kagree_alg_.end());
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

    // hash_c2
    CDRMessage_t message(static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(
                (*handshake_handle_aux)->handshake_message_.binary_properties())));
    message.msg_endian = BIGEND;
    CDRMessage::addBinaryPropertySeq(&message, (*handshake_handle_aux)->handshake_message_.binary_properties(), false);
    if (!EVP_Digest(message.buffer, message.length, md, NULL, EVP_sha256(), NULL))
    {
        exception = _SecurityException_("OpenSSL library cannot hash sha256");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        delete handshake_handle_aux;
        return ValidationResult_t::VALIDATION_FAILED;
    }
    bproperty.name("hash_c2");
    bproperty.value().assign(md, md + SHA256_DIGEST_LENGTH);
    bproperty.propagate(true);
    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

    // dh2
    if (((*handshake_handle_aux)->dhkeys_ = generate_dh_key(kagree_kind, exception)) != nullptr)
    {
        bproperty.name("dh2");
        bproperty.propagate(true);

        if (store_dh_public_key((*handshake_handle_aux)->dhkeys_, kagree_kind, bproperty.value(), exception))
        {
            (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

            // hash_c1
            bproperty.name("hash_c1");
            bproperty.value().assign(hash_c1, hash_c1 + SHA256_DIGEST_LENGTH);
            bproperty.propagate(true);
            (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

            // dh1
            bproperty.name("dh1");
            bproperty.value(std::move(*dh1));
            bproperty.propagate(true);
            (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

            // challenge1
            bproperty.name("challenge1");
            bproperty.value(std::move(*challenge1));
            bproperty.propagate(true);
            (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

            // challenge2
            bproperty.name("challenge2");
            bproperty.propagate(true);
            if (generate_challenge(bproperty.value(), exception))
            {
                (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

                // signature
                CDRMessage_t cdrmessage2(static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(
                            (*handshake_handle_aux)->handshake_message_.binary_properties())));
                cdrmessage2.msg_endian = BIGEND;
                // add sequence length
                CDRMessage::addUInt32(&cdrmessage2, 6);
                //add hash_c2
                CDRMessage::addBinaryProperty(&cdrmessage2,
                        *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_,
                        "hash_c2"));
                //add challenge2
                CDRMessage::addBinaryProperty(&cdrmessage2,
                        *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_,
                        "challenge2"));
                //add dh2
                CDRMessage::addBinaryProperty(&cdrmessage2,
                        *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_, "dh2"));
                //add challenge1
                CDRMessage::addBinaryProperty(&cdrmessage2,
                        *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_,
                        "challenge1"));
                //add dh1
                CDRMessage::addBinaryProperty(&cdrmessage2,
                        *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_, "dh1"));
                //add hash_c1
                CDRMessage::addBinaryProperty(&cdrmessage2,
                        *DataHolderHelper::find_binary_property((*handshake_handle_aux)->handshake_message_,
                        "hash_c1"), false);

                bproperty.name("signature");
                bproperty.propagate(true);
                if (sign_sha256(lih->pkey_, cdrmessage2.buffer, cdrmessage2.length, bproperty.value(), exception))
                {
                    (*handshake_handle_aux)->handshake_message_.binary_properties().push_back(std::move(bproperty));

                    (*handshake_handle_aux)->local_identity_handle_ = &lih;
                    (*handshake_handle_aux)->remote_identity_handle_ = &rih;
                    *handshake_handle = handshake_handle_aux;
                    *handshake_message_out = &(*handshake_handle_aux)->handshake_message_;

                    return ValidationResult_t::VALIDATION_PENDING_HANDSHAKE_MESSAGE;
                }
            }
        }
    }

    delete handshake_handle_aux;

    ERR_clear_error();

    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::process_handshake(
        HandshakeMessageToken** handshake_message_out,
        HandshakeMessageToken&& handshake_message_in,
        HandshakeHandle& handshake_handle,
        SecurityException& exception)
{
    ValidationResult_t returnedValue = ValidationResult_t::VALIDATION_FAILED;

    PKIHandshakeHandle& handshake = PKIHandshakeHandle::narrow(handshake_handle);

    if (!handshake.nil())
    {
        if (handshake->handshake_message_.class_id().compare("DDS:Auth:PKI-DH:1.0+Req") == 0)
        {
            returnedValue = process_handshake_request(handshake_message_out, std::move(handshake_message_in),
                            handshake, exception);
        }
        else if (handshake->handshake_message_.class_id().compare("DDS:Auth:PKI-DH:1.0+Reply") == 0)
        {
            returnedValue = process_handshake_reply(handshake_message_out, std::move(handshake_message_in),
                            handshake, exception);
        }
        else
        {
            WARNING_SECURITY_LOGGING("PKIDH",
                    std::string("Handshake message not supported (") + handshake->handshake_message_.class_id() + ")");
        }
    }

    return returnedValue;
}

ValidationResult_t PKIDH::process_handshake_request(
        HandshakeMessageToken** handshake_message_out,
        HandshakeMessageToken&& handshake_message_in,
        PKIHandshakeHandle& handshake_handle,
        SecurityException& exception)
{
    const PKIIdentityHandle& lih = *handshake_handle->local_identity_handle_;
    PKIIdentityHandle& rih = *handshake_handle->remote_identity_handle_;

    // Check TokenMessage
    if (handshake_message_in.class_id().compare("DDS:Auth:PKI-DH:1.0+Reply") != 0)
    {
        WARNING_SECURITY_LOGGING("PKIDH", std::string("Bad HandshakeMessageToken (") +
                handshake_message_in.class_id() + ")");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check incoming handshake.
    // Check c.id
    const std::vector<uint8_t>* cid = DataHolderHelper::find_binary_property_value(handshake_message_in, "c.id");
    if (cid == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property c.id");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    rih->cert_ = load_certificate(*cid);

    if (rih->cert_ == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot load certificate");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    X509_NAME* cert_sn = X509_get_subject_name(rih->cert_);
    assert(cert_sn != nullptr);
    char* cert_sn_str = X509_NAME_oneline(cert_sn, 0, 0);
    assert(cert_sn_str != nullptr);
    if (!rih->cert_sn_.empty() && rih->cert_sn_.compare(cert_sn_str) != 0)
    {
        OPENSSL_free(cert_sn_str);
        WARNING_SECURITY_LOGGING("PKIDH", "Certificated subject name invalid");
        return ValidationResult_t::VALIDATION_FAILED;
    }
    OPENSSL_free(cert_sn_str);
    BIO* cert_sn_rfc2253_str = BIO_new(BIO_s_mem());
    X509_NAME_print_ex(cert_sn_rfc2253_str, cert_sn, 0, XN_FLAG_RFC2253 & ~ASN1_STRFLGS_ESC_MSB);
    const int bufsize = 1024;
    char buffer[bufsize];
    int str_length = BIO_read(cert_sn_rfc2253_str, buffer, bufsize);
    BIO_free(cert_sn_rfc2253_str);
    rih->cert_sn_rfc2253_.assign(buffer, str_length);

    if (!verify_certificate(lih->store_, rih->cert_, lih->there_are_crls_))
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Error verifying certificate");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // c.perm
    if (lih->permissions_credential_token_.class_id().compare("DDS:Access:PermissionsCredential") == 0)
    {
        const std::vector<uint8_t>* perm = DataHolderHelper::find_binary_property_value(handshake_message_in,
                        "c.perm");

        if (perm == nullptr)
        {
            WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property c.perm");
            return ValidationResult_t::VALIDATION_FAILED;
        }

        rih->permissions_credential_token_.class_id("DDS:Access:PermissionsCredential");
        Property permission_file;
        permission_file.name("dds.perm.cert");
        permission_file.value().assign(perm->begin(), perm->end());
        rih->permissions_credential_token_.properties().push_back(std::move(permission_file));
    }

    const std::vector<uint8_t>* pdata = DataHolderHelper::find_binary_property_value(handshake_message_in, "c.pdata");

    if (pdata == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property c.pdata");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    GUID_t participant_guid;
    CDRMessage_t cdr_pdata(0);
    cdr_pdata.wraps = true;
    cdr_pdata.msg_endian = BIGEND;
    cdr_pdata.length = (uint32_t)pdata->size();
    cdr_pdata.max_size = (uint32_t)pdata->size();
    cdr_pdata.buffer = (octet*)pdata->data();

    if (!ParameterList::read_guid_from_cdr_msg(cdr_pdata, fastdds::dds::PID_PARTICIPANT_GUID, participant_guid))
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot deserialize ParticipantProxyData in property c.pdata");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if ((participant_guid.guidPrefix.value[0] & 0x80) != 0x80)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Bad participant_key's first bit in c.pdata");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    unsigned char md[SHA256_DIGEST_LENGTH];
    unsigned int length = 0;

    if (!X509_NAME_digest(cert_sn, EVP_sha256(), md, &length) || length != SHA256_DIGEST_LENGTH)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot generate SHA256 of subject name");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    md[5] &= 0xFE;
    unsigned char bytes[6]{
        static_cast<unsigned char>((participant_guid.guidPrefix.value[0] << 1) |
        (participant_guid.guidPrefix.value[1] >> 7)),
        static_cast<unsigned char>((participant_guid.guidPrefix.value[1] << 1) |
        (participant_guid.guidPrefix.value[2] >> 7)),
        static_cast<unsigned char>((participant_guid.guidPrefix.value[2] << 1) |
        (participant_guid.guidPrefix.value[3] >> 7)),
        static_cast<unsigned char>((participant_guid.guidPrefix.value[3] << 1) |
        (participant_guid.guidPrefix.value[4] >> 7)),
        static_cast<unsigned char>((participant_guid.guidPrefix.value[4] << 1) |
        (participant_guid.guidPrefix.value[5] >> 7)),
        static_cast<unsigned char>(participant_guid.guidPrefix.value[5] << 1)
    };

    if (memcmp(md, bytes, 6) != 0)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Bad participant_key's 47bits in c.pdata");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // c.dsign_algo
    const std::vector<uint8_t>* dsign_algo = DataHolderHelper::find_binary_property_value(handshake_message_in,
                    "c.dsign_algo");

    if (dsign_algo == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property c.dsign_algo");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check signature algorithm
    std::string s_dsign_algo(dsign_algo->begin(), dsign_algo->end());
    if (s_dsign_algo.compare(RSA_SHA256) != 0 &&
            s_dsign_algo.compare(ECDSA_SHA256) != 0)
    {
        WARNING_SECURITY_LOGGING("PKIDH", std::string("Not supported signature algorithm (") + s_dsign_algo + ")");
        return ValidationResult_t::VALIDATION_FAILED;
    }
    rih->sign_alg_ = std::move(s_dsign_algo);

    // c.kagree_algo
    const std::vector<uint8_t>* kagree_algo = DataHolderHelper::find_binary_property_value(handshake_message_in,
                    "c.kagree_algo");

    if (kagree_algo == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property c.kagree_algo");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check key agreement algorithm
    std::string s_kagree_algo(kagree_algo->begin(), kagree_algo->end());
    if (s_kagree_algo.compare(handshake_handle->kagree_alg_) != 0)
    {
        WARNING_SECURITY_LOGGING("PKIDH", std::string("Invalid key agreement algorithm. Received ") +
                s_kagree_algo + ", expected " + handshake_handle->kagree_alg_);
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // hash_c2
    BinaryProperty* hash_c2 = DataHolderHelper::find_binary_property(handshake_message_in, "hash_c2");

    if (hash_c2 == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property hash_c2");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if (hash_c2->value().size() != SHA256_DIGEST_LENGTH)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Wrong size of hash_c2");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    uint32_t digestInLen =
            static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(handshake_message_in.binary_properties()));
    CDRMessage_t cdrmessage(digestInLen + 3);
    cdrmessage.msg_endian = BIGEND;
    CDRMessage::addBinaryPropertySeq(&cdrmessage, handshake_message_in.binary_properties(), "c.", false);

    if (!EVP_Digest(cdrmessage.buffer, cdrmessage.length, md, NULL, EVP_sha256(), NULL))
    {
        exception = _SecurityException_("Cannot generate SHA256 of request");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if (memcmp(md, hash_c2->value().data(), SHA256_DIGEST_LENGTH) != 0)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Wrong hash_c2");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // dh2
    BinaryProperty* dh2 = DataHolderHelper::find_binary_property(handshake_message_in, "dh2");
    if (dh2 == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property dh2");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    int kagree_kind = get_dh_type(s_kagree_algo);

    if ((handshake_handle->peerkeys_ = generate_dh_peer_key(dh2->value(), exception, kagree_kind)) == nullptr)
    {
        exception = _SecurityException_("Cannot store peer key from dh2");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    BinaryProperty* challenge2 = DataHolderHelper::find_binary_property(handshake_message_in, "challenge2");

    if (challenge2 == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property challenge2");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // hash_c1
    BinaryProperty* hash_c1 = DataHolderHelper::find_binary_property(handshake_message_in, "hash_c1");

    if (hash_c1 == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property hash_c1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    const std::vector<uint8_t>* hash_c1_request = DataHolderHelper::find_binary_property_value(
        handshake_handle->handshake_message_, "hash_c1");

    if (hash_c1_request == nullptr)
    {
        exception = _SecurityException_("Cannot find property hash_c1 in request message");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if (hash_c1->value() != *hash_c1_request)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Invalid property hash_c1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // dh1
    BinaryProperty* dh1 = DataHolderHelper::find_binary_property(handshake_message_in, "dh1");

    if (dh1 == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property dh1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    const std::vector<uint8_t>* dh1_request = DataHolderHelper::find_binary_property_value(
        handshake_handle->handshake_message_, "dh1");

    if (dh1_request == nullptr)
    {
        exception = _SecurityException_("Cannot find property dh1 in request message");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if (dh1->value() != *dh1_request)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Invalid property dh1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    BinaryProperty* challenge1 = DataHolderHelper::find_binary_property(handshake_message_in, "challenge1");

    if (challenge1 == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property challenge1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    const std::vector<uint8_t>* challenge1_request = DataHolderHelper::find_binary_property_value(
        handshake_handle->handshake_message_, "challenge1");

    if (challenge1_request == nullptr)
    {
        exception = _SecurityException_("Cannot find property challenge1 in request message");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if (challenge1->value() != *challenge1_request)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Invalid property challenge1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    const std::vector<uint8_t>* signature = DataHolderHelper::find_binary_property_value(handshake_message_in,
                    "signature");

    if (signature == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property signature");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // signature
    CDRMessage_t cdrmessage2(static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(
                handshake_message_in.binary_properties())));
    cdrmessage2.msg_endian = BIGEND;
    // add sequence length
    CDRMessage::addUInt32(&cdrmessage2, 6);
    //add hash_c2
    CDRMessage::addBinaryProperty(&cdrmessage2, *hash_c2);
    //add challenge2
    CDRMessage::addBinaryProperty(&cdrmessage2, *challenge2);
    //add dh2
    CDRMessage::addBinaryProperty(&cdrmessage2, *dh2);
    //add challenge1
    CDRMessage::addBinaryProperty(&cdrmessage2, *challenge1);
    //add dh1
    CDRMessage::addBinaryProperty(&cdrmessage2, *dh1);
    //add hash_c1
    CDRMessage::addBinaryProperty(&cdrmessage2, *hash_c1, false);

    if (!check_sign_sha256(rih->cert_, cdrmessage2.buffer, cdrmessage2.length, *signature, exception))
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Error verifying signature");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Generate handshake final message token.
    HandshakeMessageToken final_message;
    final_message.binary_properties().clear();
    final_message.class_id("DDS:Auth:PKI-DH:1.0+Final");

    BinaryProperty bproperty;

    // hash_c1
    bproperty.name("hash_c1");
    bproperty.value(std::move(hash_c1->value()));
    bproperty.propagate(true);
    final_message.binary_properties().push_back(std::move(bproperty));

    // hash_c2
    bproperty.name("hash_c2");
    bproperty.value(std::move(hash_c2->value()));
    bproperty.propagate(true);
    final_message.binary_properties().push_back(std::move(bproperty));

    // dh1
    bproperty.name("dh1");
    bproperty.value(std::move(dh1->value()));
    bproperty.propagate(true);
    final_message.binary_properties().push_back(std::move(bproperty));

    // dh2
    bproperty.name("dh2");
    bproperty.value(std::move(dh2->value()));
    bproperty.propagate(true);
    final_message.binary_properties().push_back(std::move(bproperty));

    // challenge1
    bproperty.name("challenge1");
    bproperty.value(std::move(challenge1->value()));
    bproperty.propagate(true);
    final_message.binary_properties().push_back(std::move(bproperty));

    // challenge2
    bproperty.name("challenge2");
    bproperty.value(std::move(challenge2->value()));
    bproperty.propagate(true);
    final_message.binary_properties().push_back(std::move(bproperty));

    // signature
    cdrmessage2.length = 0;
    cdrmessage2.pos = 0;
    // add sequence length
    CDRMessage::addUInt32(&cdrmessage2, 6);
    //add hash_c1
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(final_message, "hash_c1"));
    //add challenge1
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(final_message, "challenge1"));
    //add dh1
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(final_message, "dh1"));
    //add challenge2
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(final_message, "challenge2"));
    //add dh2
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(final_message, "dh2"));
    //add hash_c2
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(final_message, "hash_c2"),
            false);

    bproperty.name("signature");
    bproperty.propagate(true);
    if (sign_sha256(lih->pkey_, cdrmessage2.buffer, cdrmessage2.length, bproperty.value(), exception))
    {
        final_message.binary_properties().push_back(std::move(bproperty));

        handshake_handle->sharedsecret_ =
                std::dynamic_pointer_cast<SharedSecretHandle>(
            generate_sharedsecret(handshake_handle->dhkeys_, handshake_handle->peerkeys_,
            exception));

        if (handshake_handle->sharedsecret_ != nullptr)
        {
            // Save challenge1 y challenge2 in sharedsecret
            (*handshake_handle->sharedsecret_)->data_.emplace_back(SharedSecret::BinaryData("Challenge1",
                    *DataHolderHelper::find_binary_property_value(final_message, "challenge1")));
            (*handshake_handle->sharedsecret_)->data_.emplace_back(SharedSecret::BinaryData("Challenge2",
                    *DataHolderHelper::find_binary_property_value(final_message, "challenge2")));

            handshake_handle->handshake_message_ = std::move(final_message);
            *handshake_message_out = &handshake_handle->handshake_message_;

            return ValidationResult_t::VALIDATION_OK_WITH_FINAL_MESSAGE;
        }
    }

    ERR_clear_error();

    return ValidationResult_t::VALIDATION_FAILED;
}

ValidationResult_t PKIDH::process_handshake_reply(
        HandshakeMessageToken** /*handshake_message_out*/,
        HandshakeMessageToken&& handshake_message_in,
        PKIHandshakeHandle& handshake_handle,
        SecurityException& exception)
{
    PKIIdentityHandle& rih = *handshake_handle->remote_identity_handle_;

    // Check TokenMessage
    if (handshake_message_in.class_id().compare("DDS:Auth:PKI-DH:1.0+Final") != 0)
    {
        WARNING_SECURITY_LOGGING("PKIDH", std::string("Bad HandshakeMessageToken (") +
                handshake_message_in.class_id() + ")");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // Check incoming handshake.

    // challenge1 (mandatory)
    BinaryProperty* challenge1 = DataHolderHelper::find_binary_property(handshake_message_in, "challenge1");
    if (challenge1 == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property challenge1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    std::vector<uint8_t>* challenge1_reply = DataHolderHelper::find_binary_property_value(
        handshake_handle->handshake_message_, "challenge1");
    if (challenge1_reply == nullptr)
    {
        exception = _SecurityException_("Cannot find property challenge1 in reply message");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if (challenge1->value() != *challenge1_reply)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Invalid challenge1");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // challenge2 (mandatory)
    BinaryProperty* challenge2 = DataHolderHelper::find_binary_property(handshake_message_in, "challenge2");
    if (challenge2 == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property challenge2");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    std::vector<uint8_t>* challenge2_reply = DataHolderHelper::find_binary_property_value(
        handshake_handle->handshake_message_, "challenge2");
    if (challenge2_reply == nullptr)
    {
        exception = _SecurityException_("Cannot find property challenge2 in reply message");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    if (challenge2->value() != *challenge2_reply)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Invalid challenge2");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // signature (mandatory)
    const std::vector<uint8_t>* signature = DataHolderHelper::find_binary_property_value(handshake_message_in,
                    "signature");
    if (signature == nullptr)
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Cannot find property signature");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    // hash_c1 (optional)
    BinaryProperty* hash_c1_reply = DataHolderHelper::find_binary_property(handshake_handle->handshake_message_,
                    "hash_c1");
    if (hash_c1_reply == nullptr)
    {
        exception = _SecurityException_("Cannot find property hash_c1 in reply message");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    BinaryProperty* hash_c1 = DataHolderHelper::find_binary_property(handshake_message_in, "hash_c1");
    if (hash_c1 != nullptr)
    {
        if (hash_c1->value() != hash_c1_reply->value())
        {
            WARNING_SECURITY_LOGGING("PKIDH", "Invalid hash_c1");
        }
    }

    // hash_c2 (optional)
    BinaryProperty* hash_c2_reply = DataHolderHelper::find_binary_property(handshake_handle->handshake_message_,
                    "hash_c2");
    if (hash_c2_reply == nullptr)
    {
        exception = _SecurityException_("Cannot find property hash_c2 in reply message");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    BinaryProperty* hash_c2 = DataHolderHelper::find_binary_property(handshake_message_in, "hash_c2");
    if (hash_c2 != nullptr)
    {
        if (hash_c2->value() != hash_c2_reply->value())
        {
            WARNING_SECURITY_LOGGING("PKIDH", "Invalid hash_c2");
        }
    }

    // dh1 (optional)
    BinaryProperty* dh1_reply = DataHolderHelper::find_binary_property(handshake_handle->handshake_message_, "dh1");
    if (dh1_reply == nullptr)
    {
        exception = _SecurityException_("Cannot find property dh1 in reply message");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    BinaryProperty* dh1 = DataHolderHelper::find_binary_property(handshake_message_in, "dh1");
    if (dh1 != nullptr)
    {
        if (dh1->value() != dh1_reply->value())
        {
            WARNING_SECURITY_LOGGING("PKIDH", "Invalid dh1");
        }
    }

    // dh2 (optional)
    BinaryProperty* dh2_reply = DataHolderHelper::find_binary_property(handshake_handle->handshake_message_, "dh2");
    if (dh2_reply == nullptr)
    {
        exception = _SecurityException_("Cannot find property dh2 in reply message");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        return ValidationResult_t::VALIDATION_FAILED;
    }

    BinaryProperty* dh2 = DataHolderHelper::find_binary_property(handshake_message_in, "dh2");
    if (dh2 != nullptr)
    {
        if (dh2->value() != dh2_reply->value())
        {
            WARNING_SECURITY_LOGGING("PKIDH", "Invalid dh2");
        }
    }

    // signature
    CDRMessage_t cdrmessage(static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(
                handshake_handle->handshake_message_.binary_properties())));
    cdrmessage.msg_endian = BIGEND;
    // add sequence length
    CDRMessage::addUInt32(&cdrmessage, 6);
    //add hash_c1
    CDRMessage::addBinaryProperty(&cdrmessage, *hash_c1_reply);
    //add challenge1
    CDRMessage::addBinaryProperty(&cdrmessage, *challenge1);
    //add dh1
    CDRMessage::addBinaryProperty(&cdrmessage, *dh1_reply);
    //add challenge2
    CDRMessage::addBinaryProperty(&cdrmessage, *challenge2);
    //add dh2
    CDRMessage::addBinaryProperty(&cdrmessage, *dh2_reply);
    //add hash_c2
    CDRMessage::addBinaryProperty(&cdrmessage, *hash_c2_reply, false);

    if (!check_sign_sha256(rih->cert_, cdrmessage.buffer, cdrmessage.length, *signature, exception))
    {
        WARNING_SECURITY_LOGGING("PKIDH", "Error verifying signature");
        return ValidationResult_t::VALIDATION_FAILED;
    }

    handshake_handle->sharedsecret_ =
            std::dynamic_pointer_cast<SharedSecretHandle>(
        generate_sharedsecret(handshake_handle->dhkeys_, handshake_handle->peerkeys_,
        exception));

    if (handshake_handle->sharedsecret_ != nullptr)
    {
        // Save challenge1 y challenge2 in sharedsecret
        (*handshake_handle->sharedsecret_)->data_.emplace_back(SharedSecret::BinaryData("Challenge1",
                challenge1->value()));
        (*handshake_handle->sharedsecret_)->data_.emplace_back(SharedSecret::BinaryData("Challenge2",
                challenge2->value()));

        return ValidationResult_t::VALIDATION_OK;
    }

    ERR_clear_error();

    return ValidationResult_t::VALIDATION_FAILED;
}

std::shared_ptr<SecretHandle> PKIDH::get_shared_secret(
        const HandshakeHandle& handshake_handle,
        SecurityException& exception) const
{
    const PKIHandshakeHandle& handshake = PKIHandshakeHandle::narrow(handshake_handle);

    if (!handshake.nil())
    {
        auto secret = get_shared_secret(SharedSecretHandle::nil_handle, exception);
        auto sharedsecret = std::dynamic_pointer_cast<SharedSecretHandle>(secret);
        (*sharedsecret)->data_ = (*handshake->sharedsecret_)->data_;
        return secret;
    }

    // create ad hoc deleter because this object can only be created/release from the friend factory
    auto p = new (std::nothrow) SharedSecretHandle;
    return std::dynamic_pointer_cast<SecretHandle>(
        std::shared_ptr<SharedSecretHandle>(p, [](SharedSecretHandle* p)
        {
            delete p;
        }));
}

bool PKIDH::set_listener(
        AuthenticationListener* /*listener*/,
        SecurityException& /*exception*/)
{
    return false;
}

bool PKIDH::get_identity_token(
        IdentityToken** identity_token,
        const IdentityHandle& handle,
        SecurityException& /*exception*/)
{
    const PKIIdentityHandle& ihandle = PKIIdentityHandle::narrow(handle);

    if (!ihandle.nil())
    {
        *identity_token = new IdentityToken(ihandle->identity_token_);
        return true;
    }

    return false;
}

bool PKIDH::return_identity_token(
        IdentityToken* token,
        SecurityException& /*exception*/)
{
    delete token;
    return true;
}

bool PKIDH::return_handshake_handle(
        HandshakeHandle* handshake_handle,
        SecurityException& /*exception*/)
{
    PKIHandshakeHandle* handle = &PKIHandshakeHandle::narrow(*handshake_handle);

    if (!handle->nil())
    {
        delete handle;
        return true;
    }

    return false;
}

IdentityHandle* PKIDH::get_identity_handle(
        SecurityException&)
{
    return new (std::nothrow) PKIIdentityHandle();
}

bool PKIDH::return_identity_handle(
        IdentityHandle* identity_handle,
        SecurityException& /*exception*/)
{
    PKIIdentityHandle* handle = &PKIIdentityHandle::narrow(*identity_handle);

    if (!handle->nil())
    {
        delete handle;
        return true;
    }

    return false;
}

bool PKIDH::return_sharedsecret_handle(
        std::shared_ptr<SecretHandle>& sharedsecret_handle,
        SecurityException& /*exception*/) const
{
    sharedsecret_handle.reset();
    return true;
}

bool PKIDH::set_permissions_credential_and_token(
        IdentityHandle& identity_handle,
        PermissionsCredentialToken& permissions_credential_token,
        SecurityException& exception)
{
    PKIIdentityHandle& ihandle = PKIIdentityHandle::narrow(identity_handle);

    if (!ihandle.nil())
    {
        ihandle->permissions_credential_token_ = std::move(permissions_credential_token);
        return true;
    }
    else
    {
        exception = _SecurityException_("Invalid identity handle");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
    }

    return false;
}

bool PKIDH::get_authenticated_peer_credential_token(
        PermissionsCredentialToken** token,
        const IdentityHandle& identity_handle,
        SecurityException& exception)
{
    const PKIIdentityHandle& handle = PKIIdentityHandle::narrow(identity_handle);

    if (!handle.nil())
    {
        *token = new PermissionsCredentialToken(handle->permissions_credential_token_);
        return true;
    }
    else
    {
        exception = _SecurityException_("Invalid handshake handle");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
    }

    return false;
}

bool PKIDH::return_authenticated_peer_credential_token(
        PermissionsCredentialToken* token,
        SecurityException&)
{
    delete token;
    return true;
}

bool PKIDH::check_guid_comes_from(
        IdentityHandle* identity_handle,
        const GUID_t& adjusted,
        const GUID_t& original)
{
    SecurityException exception;

    if (identity_handle != nullptr)
    {
        PKIIdentityHandle* pkiih = &PKIIdentityHandle::narrow(*identity_handle);
        GUID_t adjusted_original_guid;

        if (pkiih != nullptr && !(*pkiih).nil() && (*pkiih)->cert_ != nullptr)
        {
            adjust_participant_key((*pkiih)->cert_, original, adjusted_original_guid, exception);
            return adjusted == adjusted_original_guid;
        }
        else
        {
            exception = _SecurityException_("Invalid PKI Identity handle or Invalid Certificate");
            EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
        }

    }
    else
    {
        exception = _SecurityException_("Invalid Identity handle");
        EMERGENCY_SECURITY_LOGGING("PKIDH", exception.what());
    }

    return adjusted == original;

}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
