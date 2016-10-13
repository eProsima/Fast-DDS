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

#include <iostream>

#include "AuthenticationPluginTests.hpp"

static const char* certs_path = nullptr;

TEST_F(AuthenticationPluginTest, validate_local_identity_validation_ok_with_pwd)
{
    ASSERT_TRUE(plugin != nullptr);

    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result= ValidationResult_t::VALIDATION_FAILED;

    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/pwdpubcert.pem"));
    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.password",
                    "file://" + std::string(certs_path) + "testkey"));

    result = plugin->validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_OK);
    ASSERT_TRUE(local_identity_handle != nullptr);
}

TEST_F(AuthenticationPluginTest, validate_local_identity_wrong_identity_ca)
{
    ASSERT_TRUE(plugin != nullptr);

    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result= ValidationResult_t::VALIDATION_FAILED;

    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/wrongcacert.pem"));
    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));

    result = plugin->validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
}

TEST_F(AuthenticationPluginTest, validate_local_identity_wrong_identity_certificate)
{
    ASSERT_TRUE(plugin != nullptr);

    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result= ValidationResult_t::VALIDATION_FAILED;

    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/wrongpubcert.pem"));

    result = plugin->validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
}

TEST_F(AuthenticationPluginTest, validate_local_identity_wrong_validation)
{
    ASSERT_TRUE(plugin != nullptr);

    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result= ValidationResult_t::VALIDATION_FAILED;

    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/seccacert.pem"));
    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));

    result = plugin->validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
}

TEST_F(AuthenticationPluginTest, validate_local_identity_revoked_certificate)
{
    ASSERT_TRUE(plugin != nullptr);

    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result= ValidationResult_t::VALIDATION_FAILED;

    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/revokedpubcert.pem"));
    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_crl",
                    "file://" + std::string(certs_path) + "/maincrl.pem"));

    result = plugin->validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
}

TEST_F(AuthenticationPluginTest, validate_local_identity_revoked_certificate_with_joined_pem)
{
    ASSERT_TRUE(plugin != nullptr);

    IdentityHandle* local_identity_handle = nullptr;
    GUID_t adjusted_participant_key;
    uint32_t domain_id = 0;
    RTPSParticipantAttributes participant_attr;
    GUID_t candidate_participant_key;
    SecurityException exception;
    ValidationResult_t result= ValidationResult_t::VALIDATION_FAILED;

    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/joinedcacertcrl.pem"));
    participant_attr.properties.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/revokedpubcert.pem"));

    result = plugin->validate_local_identity(&local_identity_handle,
            adjusted_participant_key,
            domain_id,
            participant_attr,
            candidate_participant_key,
            exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_FAILED);
    ASSERT_TRUE(local_identity_handle == nullptr);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    certs_path = std::getenv("CERTS_PATH");

    if(certs_path == nullptr)
    {
        std::cout << "Cannot get enviroment variable CERTS_PATH" << std::endl;
        exit(-1);
    }

    AuthenticationPluginTest::property_policy.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
    AuthenticationPluginTest::property_policy.properties().
        emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
                    "file://" + std::string(certs_path) + "/mainpubcert.pem"));

    return RUN_ALL_TESTS();
}

