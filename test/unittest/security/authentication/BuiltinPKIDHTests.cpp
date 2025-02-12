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

// TODO This isn't a proper fix for compatibility with OpenSSL 3.0, but
// suppresses the warnings until true OpenSSL 3.0 APIs can be used.
#ifdef OPENSSL_API_COMPAT
#undef OPENSSL_API_COMPAT
#endif // ifdef OPENSSL_API_COMPAT
#define OPENSSL_API_COMPAT 10101

#include <iostream>
#include <openssl/opensslv.h>
#include <openssl/pem.h>

#include <rtps/messages/CDRMessage.hpp>

#include "AuthenticationPluginTests.hpp"
#include <security/authentication/PKIHandshakeHandle.h>
#include <security/authentication/PKIIdentityHandle.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define IS_OPENSSL_1_1 1
#else
#define IS_OPENSSL_1_1 0
#endif // if OPENSSL_VERSION_NUMBER >= 0x10100000L

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::rtps::security;

static const char* certs_path = nullptr;

PropertyPolicy AuthenticationPluginTest::get_valid_policy()
{
    PropertyPolicy property_policy;

    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainpubkey.pem"));

    return property_policy;
}

PropertyPolicy AuthenticationPluginTest::get_wrong_policy()
{
    PropertyPolicy property_policy;

    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/seccacert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainpubkey.pem"));

    return property_policy;
}

IdentityToken AuthenticationPluginTest::generate_remote_identity_token_ok(
        const IdentityHandle& local_identity_handle)
{
    IdentityToken token;
    const PKIIdentityHandle& h = PKIIdentityHandle::narrow(local_identity_handle);
    token.class_id("DDS:Auth:PKI-DH:1.0");

    Property property;

    // dds.cert.sn
    property.name("dds.cert.sn");
    X509_NAME* cert_sn = X509_get_subject_name(h->cert_);
    char* cert_sn_str = X509_NAME_oneline(cert_sn, 0, 0);
    property.value() = cert_sn_str;
    OPENSSL_free(cert_sn_str);
    token.properties().emplace_back(std::move(property));

    // dds.cert.algo
    property.name("dds.cert.algo");
    property.value(h->sign_alg_);
    token.properties().emplace_back(std::move(property));

    // dds.ca.sn
    property.name("dds.ca.sn");
    property.value(h->sn);
    token.properties().emplace_back(std::move(property));

    // dds.ca.algo
    property.name("dds.ca.algo");
    property.value(h->algo);
    token.properties().emplace_back(std::move(property));

    return token;
}

void AuthenticationPluginTest::check_local_identity_handle(
        const IdentityHandle& handle)
{
    const PKIIdentityHandle& h = PKIIdentityHandle::narrow(handle);
    ASSERT_TRUE(h.nil() == false);
    ASSERT_TRUE(h->store_ != nullptr);
    ASSERT_TRUE(h->cert_ != nullptr);
    ASSERT_TRUE(h->cert_content_ != nullptr);
    // TODO(Ricardo)
}

void AuthenticationPluginTest::check_remote_identity_handle(
        const IdentityHandle& handle)
{
    const PKIIdentityHandle& h = PKIIdentityHandle::narrow(handle);
    ASSERT_TRUE(h.nil() == false);
}

void AuthenticationPluginTest::check_handshake_request_message(
        const HandshakeHandle& handle,
        const HandshakeMessageToken& message)
{
    const PKIHandshakeHandle& handshake_handle = PKIHandshakeHandle::narrow(handle);
    ASSERT_TRUE(handshake_handle.nil() == false);
    const std::vector<uint8_t>* cid = DataHolderHelper::find_binary_property_value(message, "c.id");
    ASSERT_TRUE(cid != nullptr);

    // Read certificate.
    BIO* cid_in = BIO_new_mem_buf(cid->data(), static_cast<int>(cid->size()));
    ASSERT_TRUE(cid_in != nullptr);
    X509* cid_cert = PEM_read_bio_X509_AUX(cid_in, NULL, NULL, NULL);
    ASSERT_TRUE(cid_cert != nullptr);
    ASSERT_TRUE(X509_cmp((*handshake_handle->local_identity_handle_)->cert_, cid_cert) == 0);
    X509_free(cid_cert);
    BIO_free(cid_in);


    const std::vector<uint8_t>* csign_alg = DataHolderHelper::find_binary_property_value(message, "c.dsign_algo");
    ASSERT_TRUE(csign_alg != nullptr);
    std::string sign_alg(csign_alg->begin(), csign_alg->end());
    ASSERT_TRUE(sign_alg.compare("ECDSA-SHA256") == 0 ||
            sign_alg.compare("RSASSA-PSS-SHA256") == 0);

    const std::vector<uint8_t>* ckagree_alg = DataHolderHelper::find_binary_property_value(message, "c.kagree_algo");
    ASSERT_TRUE(ckagree_alg != nullptr);
    std::string kagree_alg(ckagree_alg->begin(), ckagree_alg->end());
    ASSERT_TRUE(kagree_alg.compare("DH+MODP-2048-256") == 0 ||
            kagree_alg.compare("ECDH+prime256v1-CEUM") == 0);

    const std::vector<uint8_t>* hash_c1 = DataHolderHelper::find_binary_property_value(message, "hash_c1");
    ASSERT_TRUE(hash_c1 != nullptr);
    ASSERT_TRUE(hash_c1->size() == SHA256_DIGEST_LENGTH);
    CDRMessage_t cdrmessage(static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(message.binary_properties())));
    cdrmessage.msg_endian = BIGEND;
    CDRMessage::addBinaryPropertySeq(&cdrmessage, message.binary_properties(), "c.", false);
    unsigned char md[SHA256_DIGEST_LENGTH];
    ASSERT_TRUE(EVP_Digest(cdrmessage.buffer, cdrmessage.length, md, NULL, EVP_sha256(), NULL));
    ASSERT_TRUE(memcmp(md, hash_c1->data(), SHA256_DIGEST_LENGTH) == 0);

    const std::vector<uint8_t>* dh1 = DataHolderHelper::find_binary_property_value(message, "dh1");
    ASSERT_TRUE(dh1 != nullptr);
    DH* dh = EVP_PKEY_get1_DH(handshake_handle->dhkeys_);
    ASSERT_TRUE(dh != nullptr);
    const unsigned char* pointer = dh1->data();
    size_t length = dh1->size();
    BIGNUM* bn = BN_new();
    ASSERT_TRUE(BN_bin2bn(pointer, static_cast<int>(length), bn) !=  nullptr);

    int check_result;
    ASSERT_TRUE(DH_check_pub_key(dh, bn, &check_result));
    ASSERT_TRUE(!check_result);
    BN_clear_free(bn);
    DH_free(dh);

    const std::vector<uint8_t>* challenge1 = DataHolderHelper::find_binary_property_value(message, "challenge1");
    ASSERT_TRUE(challenge1 != nullptr);
}

void AuthenticationPluginTest::check_handshake_reply_message(
        const HandshakeHandle& handle,
        const HandshakeMessageToken& message,
        const HandshakeMessageToken& request_message)
{
    const PKIHandshakeHandle& handshake_handle = PKIHandshakeHandle::narrow(handle);
    ASSERT_TRUE(handshake_handle.nil() == false);
    const std::vector<uint8_t>* cid = DataHolderHelper::find_binary_property_value(message, "c.id");
    ASSERT_TRUE(cid != nullptr);
    // Read certificate.
    BIO* cid_in = BIO_new_mem_buf(cid->data(), static_cast<int>(cid->size()));
    ASSERT_TRUE(cid_in != nullptr);
    X509* cid_cert = PEM_read_bio_X509_AUX(cid_in, NULL, NULL, NULL);
    ASSERT_TRUE(cid_cert != nullptr);
    ASSERT_TRUE(X509_cmp((*handshake_handle->local_identity_handle_)->cert_, cid_cert) == 0);
    X509_free(cid_cert);
    BIO_free(cid_in);


    const std::vector<uint8_t>* csign_alg = DataHolderHelper::find_binary_property_value(message, "c.dsign_algo");
    ASSERT_TRUE(csign_alg != nullptr);
    std::string sign_alg(csign_alg->begin(), csign_alg->end());
    ASSERT_TRUE(sign_alg.compare("ECDSA-SHA256") == 0 ||
            sign_alg.compare("RSASSA-PSS-SHA256") == 0);

    const std::vector<uint8_t>* ckagree_alg = DataHolderHelper::find_binary_property_value(message, "c.kagree_algo");
    ASSERT_TRUE(ckagree_alg != nullptr);
    std::string kagree_alg(ckagree_alg->begin(), ckagree_alg->end());
    ASSERT_TRUE(kagree_alg.compare("DH+MODP-2048-256") == 0 ||
            kagree_alg.compare("ECDH+prime256v1-CEUM") == 0);

    const std::vector<uint8_t>* hash_c2 = DataHolderHelper::find_binary_property_value(message, "hash_c2");
    ASSERT_TRUE(hash_c2 != nullptr);
    ASSERT_TRUE(hash_c2->size() == SHA256_DIGEST_LENGTH);
    CDRMessage_t cdrmessage(static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(message.binary_properties())));
    cdrmessage.msg_endian = BIGEND;
    CDRMessage::addBinaryPropertySeq(&cdrmessage, message.binary_properties(), "c.", false);
    unsigned char md[SHA256_DIGEST_LENGTH];
    ASSERT_TRUE(EVP_Digest(cdrmessage.buffer, cdrmessage.length, md, NULL, EVP_sha256(), NULL));
    ASSERT_TRUE(memcmp(md, hash_c2->data(), SHA256_DIGEST_LENGTH) == 0);

    const std::vector<uint8_t>* dh2 = DataHolderHelper::find_binary_property_value(message, "dh2");
    ASSERT_TRUE(dh2 != nullptr);
    DH* dh = EVP_PKEY_get1_DH(handshake_handle->dhkeys_);
    ASSERT_TRUE(dh != nullptr);
    const unsigned char* pointer = dh2->data();
    size_t length = dh2->size();
    BIGNUM* bn = BN_new();
    ASSERT_TRUE(BN_bin2bn(pointer, static_cast<int>(length), bn) !=  nullptr);

    int check_result;
    ASSERT_TRUE(DH_check_pub_key(dh, bn, &check_result));
    ASSERT_TRUE(!check_result);
    BN_clear_free(bn);
    DH_free(dh);

    const std::vector<uint8_t>* hash_c1 = DataHolderHelper::find_binary_property_value(message, "hash_c1");
    ASSERT_TRUE(hash_c1 != nullptr);
    const std::vector<uint8_t>* request_hash_c1 = DataHolderHelper::find_binary_property_value(request_message,
                    "hash_c1");
    ASSERT_TRUE(request_hash_c1 != nullptr);
    ASSERT_TRUE(*hash_c1 == *request_hash_c1);

    const std::vector<uint8_t>* dh1 = DataHolderHelper::find_binary_property_value(message, "dh1");
    ASSERT_TRUE(dh1 != nullptr);
    const std::vector<uint8_t>* request_dh1 = DataHolderHelper::find_binary_property_value(request_message, "dh1");
    ASSERT_TRUE(request_dh1 != nullptr);
    ASSERT_TRUE(*dh1 == *request_dh1);

    const std::vector<uint8_t>* challenge1 = DataHolderHelper::find_binary_property_value(message, "challenge1");
    ASSERT_TRUE(challenge1 != nullptr);
    const std::vector<uint8_t>* request_challenge1 = DataHolderHelper::find_binary_property_value(request_message,
                    "challenge1");
    ASSERT_TRUE(request_challenge1 != nullptr);
    ASSERT_TRUE(*challenge1 == *request_challenge1);

    const std::vector<uint8_t>* challenge2 = DataHolderHelper::find_binary_property_value(message, "challenge2");
    ASSERT_TRUE(challenge2 != nullptr);

    const std::vector<uint8_t>* signature = DataHolderHelper::find_binary_property_value(message, "signature");
    ASSERT_TRUE(signature != nullptr);
    // signature
    CDRMessage_t cdrmessage2(static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(message.binary_properties())));
    cdrmessage2.msg_endian = BIGEND;
    // add sequence length
    CDRMessage::addUInt32(&cdrmessage2, 6);
    //add hash_c2
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(message, "hash_c2"));
    //add challenge2
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(message, "challenge2"));
    //add dh2
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(message, "dh2"));
    //add challenge1
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(message, "challenge1"));
    //add dh1
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(message, "dh1"));
    //add hash_c1
    CDRMessage::addBinaryProperty(&cdrmessage2, *DataHolderHelper::find_binary_property(message, "hash_c1"), false);
    EVP_MD_CTX* ctx =
#if IS_OPENSSL_1_1
            EVP_MD_CTX_new();
#else
            (EVP_MD_CTX*)malloc(sizeof(EVP_MD_CTX));
#endif // if IS_OPENSSL_1_1
    EVP_MD_CTX_init(ctx);
    EVP_PKEY* pubkey = X509_get_pubkey((*handshake_handle->local_identity_handle_)->cert_);
    ASSERT_TRUE(pubkey != nullptr);
    ASSERT_TRUE(EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, pubkey) == 1);
    ASSERT_TRUE(EVP_DigestVerifyUpdate(ctx, cdrmessage2.buffer, cdrmessage2.length) == 1);
    ASSERT_TRUE(EVP_DigestVerifyFinal(ctx, signature->data(), signature->size()) == 1);
    EVP_PKEY_free(pubkey);
#if IS_OPENSSL_1_1
    EVP_MD_CTX_free(ctx);
#else
    free(ctx);
#endif // if IS_OPENSSL_1_1
}

void AuthenticationPluginTest::check_handshake_final_message(
        const HandshakeHandle& handle,
        const HandshakeMessageToken& message,
        const HandshakeMessageToken& reply_message)
{
    const PKIHandshakeHandle& handshake_handle = PKIHandshakeHandle::narrow(handle);
    ASSERT_TRUE(handshake_handle.nil() == false);

    const std::vector<uint8_t>* hash_c1 = DataHolderHelper::find_binary_property_value(message, "hash_c1");
    ASSERT_TRUE(hash_c1 != nullptr);
    const std::vector<uint8_t>* reply_hash_c1 = DataHolderHelper::find_binary_property_value(reply_message, "hash_c1");
    ASSERT_TRUE(reply_hash_c1 != nullptr);
    ASSERT_TRUE(*hash_c1 == *reply_hash_c1);

    const std::vector<uint8_t>* hash_c2 = DataHolderHelper::find_binary_property_value(message, "hash_c2");
    ASSERT_TRUE(hash_c2 != nullptr);
    const std::vector<uint8_t>* reply_hash_c2 = DataHolderHelper::find_binary_property_value(reply_message, "hash_c2");
    ASSERT_TRUE(reply_hash_c2 != nullptr);
    ASSERT_TRUE(*hash_c2 == *reply_hash_c2);

    const std::vector<uint8_t>* dh1 = DataHolderHelper::find_binary_property_value(message, "dh1");
    ASSERT_TRUE(dh1 != nullptr);
    const std::vector<uint8_t>* reply_dh1 = DataHolderHelper::find_binary_property_value(reply_message, "dh1");
    ASSERT_TRUE(reply_dh1 != nullptr);
    ASSERT_TRUE(*dh1 == *reply_dh1);

    const std::vector<uint8_t>* dh2 = DataHolderHelper::find_binary_property_value(message, "dh2");
    ASSERT_TRUE(dh2 != nullptr);
    const std::vector<uint8_t>* reply_dh2 = DataHolderHelper::find_binary_property_value(reply_message, "dh2");
    ASSERT_TRUE(reply_dh2 != nullptr);
    ASSERT_TRUE(*dh2 == *reply_dh2);

    const std::vector<uint8_t>* challenge1 = DataHolderHelper::find_binary_property_value(message, "challenge1");
    ASSERT_TRUE(challenge1 != nullptr);
    const std::vector<uint8_t>* reply_challenge1 = DataHolderHelper::find_binary_property_value(reply_message,
                    "challenge1");
    ASSERT_TRUE(reply_challenge1 != nullptr);
    ASSERT_TRUE(*challenge1 == *reply_challenge1);

    const std::vector<uint8_t>* challenge2 = DataHolderHelper::find_binary_property_value(message, "challenge2");
    ASSERT_TRUE(challenge2 != nullptr);
    const std::vector<uint8_t>* reply_challenge2 = DataHolderHelper::find_binary_property_value(reply_message,
                    "challenge2");
    ASSERT_TRUE(reply_challenge2 != nullptr);
    ASSERT_TRUE(*challenge2 == *reply_challenge2);

    const std::vector<uint8_t>* signature = DataHolderHelper::find_binary_property_value(message, "signature");
    ASSERT_TRUE(signature != nullptr);
    // signature
    CDRMessage_t cdrmessage(static_cast<uint32_t>(BinaryPropertyHelper::serialized_size(message.binary_properties())));
    cdrmessage.msg_endian = BIGEND;
    // add sequence length
    CDRMessage::addUInt32(&cdrmessage, 6);
    //add hash_c1
    CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property(message, "hash_c1"));
    //add challenge1
    CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property(message, "challenge1"));
    //add dh1
    CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property(message, "dh1"));
    //add challenge2
    CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property(message, "challenge2"));
    //add dh2
    CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property(message, "dh2"));
    //add hash_c2
    CDRMessage::addBinaryProperty(&cdrmessage, *DataHolderHelper::find_binary_property(message, "hash_c2"), false);
    EVP_MD_CTX* ctx =
#if IS_OPENSSL_1_1
            EVP_MD_CTX_new();
#else
            (EVP_MD_CTX*)malloc(sizeof(EVP_MD_CTX));
#endif // if IS_OPENSSL_1_1
    EVP_MD_CTX_init(ctx);
    EVP_PKEY* pubkey = X509_get_pubkey((*handshake_handle->local_identity_handle_)->cert_);
    ASSERT_TRUE(pubkey != nullptr);
    ASSERT_TRUE(EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, pubkey) == 1);
    ASSERT_TRUE(EVP_DigestVerifyUpdate(ctx, cdrmessage.buffer, cdrmessage.length) == 1);
    ASSERT_TRUE(EVP_DigestVerifyFinal(ctx, signature->data(), signature->size()) == 1);
    EVP_PKEY_free(pubkey);
#if IS_OPENSSL_1_1
    EVP_MD_CTX_free(ctx);
#else
    free(ctx);
#endif // if IS_OPENSSL_1_1
}

void AuthenticationPluginTest::check_shared_secrets(
        const SecretHandle& sharedsecret1,
        const SecretHandle& sharedsecret2)
{
    const std::vector<uint8_t>* challenge1_1 = SharedSecretHelper::find_data_value(sharedsecret1, "Challenge1");
    ASSERT_TRUE(challenge1_1 != nullptr);
    const std::vector<uint8_t>* challenge1_2 = SharedSecretHelper::find_data_value(sharedsecret2, "Challenge1");
    ASSERT_TRUE(challenge1_2 != nullptr);
    ASSERT_TRUE(*challenge1_1 == *challenge1_2);
    const std::vector<uint8_t>* challenge2_1 = SharedSecretHelper::find_data_value(sharedsecret1, "Challenge2");
    ASSERT_TRUE(challenge2_1 != nullptr);
    const std::vector<uint8_t>* challenge2_2 = SharedSecretHelper::find_data_value(sharedsecret2, "Challenge2");
    ASSERT_TRUE(challenge2_2 != nullptr);
    ASSERT_TRUE(*challenge2_1 == *challenge2_2);
    const std::vector<uint8_t>* sharedsecret_1 = SharedSecretHelper::find_data_value(sharedsecret1, "SharedSecret");
    ASSERT_TRUE(sharedsecret_1 != nullptr);
    const std::vector<uint8_t>* sharedsecret_2 = SharedSecretHelper::find_data_value(sharedsecret2, "SharedSecret");
    ASSERT_TRUE(sharedsecret_2 != nullptr);
    ASSERT_TRUE(*sharedsecret_1 == *sharedsecret_2);
}

TEST_F(AuthenticationPluginTest, validate_local_identity_kagree_algo)
{
    const std::string correct_values[] =
    {
        "DH",
        "ECDH",
        "DH+MODP-2048-256",
        "ECDH+prime256v1-CEUM"
    };

    const std::string wrong_values[] =
    {
        "RSA+MODP-2048-256",
        "ECDH+MODP-2048-256",
        "RSA",
        "ECDH+prime256v1",
        "unknown",
        ""
    };

    auto test_fn = [this](
        const std::string& alg,
        ValidationResult_t expected_result) -> void
            {
                IdentityHandle* local_identity_handle = nullptr;
                GUID_t adjusted_participant_key;
                uint32_t domain_id = 0;
                RTPSParticipantAttributes participant_attr;
                GUID_t candidate_participant_key;
                SecurityException exception;
                ValidationResult_t result = ValidationResult_t::VALIDATION_FAILED;

                fill_candidate_participant_key(candidate_participant_key);
                participant_attr.properties = get_valid_policy();
                participant_attr.properties.properties().emplace_back(
                    Property("dds.sec.auth.builtin.PKI-DH.preferred_key_agreement", alg));
                result = plugin.validate_local_identity(&local_identity_handle,
                                adjusted_participant_key,
                                domain_id,
                                participant_attr,
                                candidate_participant_key,
                                exception);

                ASSERT_TRUE(result == expected_result);
                if (ValidationResult_t::VALIDATION_OK == result)
                {
                    ASSERT_TRUE(local_identity_handle != nullptr);
                    check_local_identity_handle(*local_identity_handle);
                    ASSERT_TRUE(adjusted_participant_key != GUID_t::unknown());
                    ASSERT_TRUE(plugin.return_identity_handle(local_identity_handle, exception));
                }
                else
                {
                    ASSERT_TRUE(local_identity_handle == nullptr);
                    ASSERT_TRUE(adjusted_participant_key == GUID_t::unknown());
                }
            };

    for (const std::string& value : correct_values)
    {
        test_fn(value, ValidationResult_t::VALIDATION_OK);
    }

    for (const std::string& value : wrong_values)
    {
        test_fn(value, ValidationResult_t::VALIDATION_FAILED);
    }
}

TEST_F(AuthenticationPluginTest, validate_local_identity_validation_ok_with_pwd)
{
    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result = ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/pwdpubcert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/pwdpubkey.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.password", "testkey"));

    result = plugin.validate_local_identity(&local_identity_handle,
                    adjusted_participant_key,
                    domain_id,
                    participant_attr,
                    candidate_participant_key,
                    exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_OK);
    ASSERT_TRUE(local_identity_handle != nullptr);
    ASSERT_TRUE(adjusted_participant_key != GUID_t::unknown());

    ASSERT_TRUE(plugin.return_identity_handle(local_identity_handle, exception));
}

TEST_F(AuthenticationPluginTest, validate_local_identity_no_pwd)
{
    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result = ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/pwdpubcert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/pwdpubkey.pem"));

    result = plugin.validate_local_identity(&local_identity_handle,
                    adjusted_participant_key,
                    domain_id,
                    participant_attr,
                    candidate_participant_key,
                    exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
    ASSERT_TRUE(adjusted_participant_key == GUID_t::unknown());
}

TEST_F(AuthenticationPluginTest, validate_local_identity_wrong_pwd)
{
    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result = ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/pwdpubcert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/pwdpubkey.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.password", "wrongpasswd"));

    result = plugin.validate_local_identity(&local_identity_handle,
                    adjusted_participant_key,
                    domain_id,
                    participant_attr,
                    candidate_participant_key,
                    exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
    ASSERT_TRUE(adjusted_participant_key == GUID_t::unknown());
}

TEST_F(AuthenticationPluginTest, validate_local_identity_wrong_identity_ca)
{
    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result = ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/wrongcacert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainpubkey.pem"));

    result = plugin.validate_local_identity(&local_identity_handle,
                    adjusted_participant_key,
                    domain_id,
                    participant_attr,
                    candidate_participant_key,
                    exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
    ASSERT_TRUE(adjusted_participant_key == GUID_t::unknown());
}

TEST_F(AuthenticationPluginTest, validate_local_identity_wrong_identity_certificate)
{
    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result = ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/wrongpubcert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainpubcert.pem"));

    result = plugin.validate_local_identity(&local_identity_handle,
                    adjusted_participant_key,
                    domain_id,
                    participant_attr,
                    candidate_participant_key,
                    exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
    ASSERT_TRUE(adjusted_participant_key == GUID_t::unknown());
}

TEST_F(AuthenticationPluginTest, validate_local_identity_revoked_certificate)
{
    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result = ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/revokedpubcert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/revokedpubkey.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_crl",
            "file://" + std::string(certs_path) + "/maincrl.pem"));

    result = plugin.validate_local_identity(&local_identity_handle,
                    adjusted_participant_key,
                    domain_id,
                    participant_attr,
                    candidate_participant_key,
                    exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
    ASSERT_TRUE(adjusted_participant_key == GUID_t::unknown());
}

TEST_F(AuthenticationPluginTest, validate_local_identity_revoked_certificate_with_joined_pem)
{
    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result = ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/joinedcacertcrl.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/revokedpubcert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/revokedpubkey.pem"));

    result = plugin.validate_local_identity(&local_identity_handle,
                    adjusted_participant_key,
                    domain_id,
                    participant_attr,
                    candidate_participant_key,
                    exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
    ASSERT_TRUE(adjusted_participant_key == GUID_t::unknown());
}

TEST_F(AuthenticationPluginTest, validate_local_identity_expired_certificate)
{
    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result = ValidationResult_t::VALIDATION_FAILED;

    fill_candidate_participant_key(candidate_participant_key);
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/expiredpubcert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/expiredpubkey.pem"));

    result = plugin.validate_local_identity(&local_identity_handle,
                    adjusted_participant_key,
                    domain_id,
                    participant_attr,
                    candidate_participant_key,
                    exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
    ASSERT_TRUE(adjusted_participant_key == GUID_t::unknown());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (!::testing::GTEST_FLAG(list_tests))
    {
        certs_path = std::getenv("CERTS_PATH");

        if (certs_path == nullptr)
        {
            std::cout << "Cannot get enviroment variable CERTS_PATH" << std::endl;
            exit(-1);
        }
    }

    return RUN_ALL_TESTS();
}
