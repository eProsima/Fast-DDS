// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <string>

#include <gtest/gtest.h>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/messages/CDRMessage.hpp>
#include <security/accesscontrol/AccessPermissionsHandle.h>
#include <security/accesscontrol/Permissions.h>
#include <security/authentication/PKIDH.h>
#include <security/authentication/PKIHandshakeHandle.h>
#include <security/authentication/PKIIdentityHandle.h>
#include <security/OpenSSLInit.hpp>

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::rtps::security;

static const char* certs_path = nullptr;

class AccessControlTest : public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        fill_candidate_participant_key();
        permissions_ca = "maincacert.pem";
        permissions_file = "permissions_access_control_tests.smime";
        governance_file = "governance_helloworld_all_enable.smime";
    }

    virtual void TearDown()
    {
    }

    AccessControlTest()
    {
        static eprosima::fastdds::rtps::security::OpenSSLInit openssl_init;
    }

    void fill_common_participant_security_attributes(
            RTPSParticipantAttributes& participant_attr);
    void fill_publisher_participant_security_attributes(
            RTPSParticipantAttributes& participant_attr);
    void fill_subscriber_participant_security_attributes(
            RTPSParticipantAttributes& participant_attr);

    void get_access_handle(
            const RTPSParticipantAttributes& participant_attr,
            PermissionsHandle** access_handle,
            bool should_success = true);
    void fill_candidate_participant_key();

    GUID_t candidate_participant_key;

public:

    void check_local_datawriter(
            const RTPSParticipantAttributes& participant_attr,
            bool success);
    void check_local_datareader(
            const RTPSParticipantAttributes& participant_attr,
            bool success);
    void check_remote_datawriter(
            const RTPSParticipantAttributes& participant_attr,
            bool success);
    void check_remote_datareader(
            const RTPSParticipantAttributes& participant_attr,
            bool success);

    PKIDH authentication_plugin;
    Permissions access_plugin;
    uint32_t domain_id = 0;
    std::string topic_name;
    std::vector<std::string> partitions;
    std::string permissions_ca;
    std::string governance_file;
    std::string permissions_file;
};

void AccessControlTest::fill_candidate_participant_key()
{
    candidate_participant_key.guidPrefix.value[0] = 1;
    candidate_participant_key.guidPrefix.value[1] = 2;
    candidate_participant_key.guidPrefix.value[2] = 3;
    candidate_participant_key.guidPrefix.value[3] = 4;
    candidate_participant_key.guidPrefix.value[4] = 5;
    candidate_participant_key.guidPrefix.value[5] = 6;
    candidate_participant_key.guidPrefix.value[6] = 7;
    candidate_participant_key.guidPrefix.value[7] = 8;
    candidate_participant_key.guidPrefix.value[8] = 9;
    candidate_participant_key.guidPrefix.value[9] = 10;
    candidate_participant_key.guidPrefix.value[10] = 11;
    candidate_participant_key.guidPrefix.value[11] = 12;
    candidate_participant_key.entityId.value[0] = 0x0;
    candidate_participant_key.entityId.value[1] = 0x0;
    candidate_participant_key.entityId.value[2] = 0x1;
    candidate_participant_key.entityId.value[3] = 0xc1;
}

void AccessControlTest::fill_subscriber_participant_security_attributes(
        RTPSParticipantAttributes& participant_attr)
{
    fill_common_participant_security_attributes(participant_attr);

    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainsubkey.pem"));
}

void AccessControlTest::fill_publisher_participant_security_attributes(
        RTPSParticipantAttributes& participant_attr)
{
    fill_common_participant_security_attributes(participant_attr);

    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainpubkey.pem"));
}

void AccessControlTest::fill_common_participant_security_attributes(
        RTPSParticipantAttributes& participant_attr)
{
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.auth.builtin.PKI-DH.password", "testkey"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.access.authentication_plugin", "builtin.Access-Permissions"));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions_ca",
            "file://" + std::string(certs_path) + "/" + permissions_ca));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/" + governance_file));
    participant_attr.properties.properties().
            emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/" + permissions_file));
}

void AccessControlTest::get_access_handle(
        const RTPSParticipantAttributes& participant_attr,
        PermissionsHandle** access_handle,
        bool should_success)
{
    IdentityHandle* identity_handle = nullptr;

    ValidationResult_t result = ValidationResult_t::VALIDATION_FAILED;
    SecurityException exception;
    GUID_t adjusted_participant_key;

    result = authentication_plugin.validate_local_identity(
        &identity_handle,
        adjusted_participant_key,
        domain_id,
        participant_attr,
        candidate_participant_key,
        exception);

    ASSERT_TRUE(result == ValidationResult_t::VALIDATION_OK) << exception.what();
    ASSERT_TRUE(identity_handle != nullptr) << exception.what();
    ASSERT_TRUE(adjusted_participant_key != GUID_t::unknown());

    *access_handle = access_plugin.validate_local_permissions(
        authentication_plugin,
        *identity_handle,
        domain_id,
        participant_attr,
        exception);

    bool success = *access_handle != nullptr;
    ASSERT_EQ(success, should_success) << exception.what();
    ASSERT_TRUE(authentication_plugin.return_identity_handle(identity_handle, exception)) << exception.what();
}

void AccessControlTest::check_local_datareader(
        const RTPSParticipantAttributes& participant_attr,
        bool success)
{
    PermissionsHandle* access_handle;
    get_access_handle(participant_attr, &access_handle);

    SecurityException exception;

    bool result = access_plugin.check_create_datareader(
        *access_handle,
        domain_id,
        topic_name,
        partitions,
        exception);

    if (success)
    {
        ASSERT_TRUE(result) << exception.what();
    }
    else
    {
        ASSERT_FALSE(result);
    }
    ASSERT_TRUE(access_plugin.return_permissions_handle(access_handle, exception)) << exception.what();
}

void AccessControlTest::check_remote_datareader(
        const RTPSParticipantAttributes& participant_attr,
        bool success)
{
    PermissionsHandle* access_handle;
    get_access_handle(participant_attr, &access_handle);

    SecurityException exception;

    ReaderProxyData reader_proxy_data(1, 1);
    reader_proxy_data.topic_name = eprosima::fastcdr::string_255(topic_name);
    reader_proxy_data.partition.setNames(partitions);
    bool relay_only;
    bool result = access_plugin.check_remote_datareader(
        *access_handle,
        domain_id,
        reader_proxy_data,
        relay_only,
        exception);

    if (success)
    {
        ASSERT_TRUE(result) << exception.what();
    }
    else
    {
        ASSERT_FALSE(result);
    }
    ASSERT_TRUE(access_plugin.return_permissions_handle(access_handle, exception)) << exception.what();
}

void AccessControlTest::check_local_datawriter(
        const RTPSParticipantAttributes& participant_attr,
        bool success)
{
    PermissionsHandle* access_handle;
    get_access_handle(participant_attr, &access_handle);

    SecurityException exception;

    bool result = access_plugin.check_create_datawriter(
        *access_handle,
        domain_id,
        topic_name,
        partitions,
        exception);

    if (success)
    {
        ASSERT_TRUE(result) << exception.what();
    }
    else
    {
        ASSERT_FALSE(result);
    }
    ASSERT_TRUE(access_plugin.return_permissions_handle(access_handle, exception)) << exception.what();
}

void AccessControlTest::check_remote_datawriter(
        const RTPSParticipantAttributes& participant_attr,
        bool success)
{
    PermissionsHandle* access_handle;
    get_access_handle(participant_attr, &access_handle);

    SecurityException exception;

    WriterProxyData writer_proxy_data(1, 1);
    writer_proxy_data.topic_name = eprosima::fastcdr::string_255(topic_name);
    writer_proxy_data.partition.setNames(partitions);
    bool result = access_plugin.check_remote_datawriter(
        *access_handle,
        domain_id,
        writer_proxy_data,
        exception);

    if (success)
    {
        ASSERT_TRUE(result) << exception.what();
    }
    else
    {
        ASSERT_FALSE(result);
    }
    ASSERT_TRUE(access_plugin.return_permissions_handle(access_handle, exception)) << exception.what();
}

TEST_F(AccessControlTest, validate_topic_access_ok_no_partitions)
{
    topic_name = "HelloWorldTopic_no_partitions";

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);
    check_local_datareader(subscriber_participant_attr, true);
    check_remote_datareader(subscriber_participant_attr, true);

    RTPSParticipantAttributes publisher_participant_attr;
    fill_publisher_participant_security_attributes(publisher_participant_attr);
    check_local_datawriter(publisher_participant_attr, true);
    check_remote_datawriter(publisher_participant_attr, true);
}

TEST_F(AccessControlTest, validate_topic_access_fail_wildcards)
{
    topic_name = "HelloWorldTopic_*";

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);
    check_local_datareader(subscriber_participant_attr, false);
    check_remote_datareader(subscriber_participant_attr, false);

    RTPSParticipantAttributes publisher_participant_attr;
    fill_publisher_participant_security_attributes(publisher_participant_attr);
    check_local_datawriter(publisher_participant_attr, false);
    check_remote_datawriter(publisher_participant_attr, false);
}

TEST_F(AccessControlTest, validate_topic_access_ok_permission_wildcards)
{
    topic_name = "HelloWorldTopic_no_partitions_wildcards_1";

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);
    check_local_datareader(subscriber_participant_attr, true);
    check_remote_datareader(subscriber_participant_attr, true);

    RTPSParticipantAttributes publisher_participant_attr;
    fill_publisher_participant_security_attributes(publisher_participant_attr);
    check_local_datawriter(publisher_participant_attr, true);
    check_remote_datawriter(publisher_participant_attr, true);
}

TEST_F(AccessControlTest, validate_partition_access_ok)
{
    topic_name = "HelloWorldTopic_single_partition";
    partitions.push_back("Partition");

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);
    check_local_datareader(subscriber_participant_attr, true);
    check_remote_datareader(subscriber_participant_attr, true);

    RTPSParticipantAttributes publisher_participant_attr;
    fill_publisher_participant_security_attributes(publisher_participant_attr);
    check_local_datawriter(publisher_participant_attr, true);
    check_remote_datawriter(publisher_participant_attr, true);
}

TEST_F(AccessControlTest, validate_partition_access_fail_on_wrong_partition)
{
    topic_name = "HelloWorldTopic_single_partition";
    partitions.push_back("Partition3");

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);
    check_local_datareader(subscriber_participant_attr, false);
    check_remote_datareader(subscriber_participant_attr, false);

    RTPSParticipantAttributes publisher_participant_attr;
    fill_publisher_participant_security_attributes(publisher_participant_attr);
    check_local_datawriter(publisher_participant_attr, false);
    check_remote_datawriter(publisher_participant_attr, false);
}

TEST_F(AccessControlTest, validate_partition_access_fail_on_wildcards)
{
    topic_name = "HelloWorldTopic_single_partition";
    partitions.push_back("*");

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);
    check_local_datareader(subscriber_participant_attr, false);
    check_remote_datareader(subscriber_participant_attr, false);

    RTPSParticipantAttributes publisher_participant_attr;
    fill_publisher_participant_security_attributes(publisher_participant_attr);
    check_local_datawriter(publisher_participant_attr, false);
    check_remote_datawriter(publisher_participant_attr, false);
}

TEST_F(AccessControlTest, validate_partition_access_ok_on_subpartition)
{
    topic_name = "HelloWorldTopic_multiple_partition";
    partitions.push_back("Partition1");
    partitions.push_back("Partition2");

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);
    check_local_datareader(subscriber_participant_attr, true);
    check_remote_datareader(subscriber_participant_attr, true);

    RTPSParticipantAttributes publisher_participant_attr;
    fill_publisher_participant_security_attributes(publisher_participant_attr);
    check_local_datawriter(publisher_participant_attr, true);
    check_remote_datawriter(publisher_participant_attr, true);
}

TEST_F(AccessControlTest, validate_partition_access_fail_on_not_subpartition)
{
    topic_name = "HelloWorldTopic_multiple_partition";
    partitions.push_back("Partition1");
    partitions.push_back("Partition5");

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);
    check_local_datareader(subscriber_participant_attr, false);
    check_remote_datareader(subscriber_participant_attr, false);

    RTPSParticipantAttributes publisher_participant_attr;
    fill_publisher_participant_security_attributes(publisher_participant_attr);
    check_local_datawriter(publisher_participant_attr, false);
    check_remote_datawriter(publisher_participant_attr, false);
}

TEST_F(AccessControlTest, validate_partition_access_ok_on_permission_wildcards)
{
    topic_name = "HelloWorldTopic_partition_wildcards";
    partitions.push_back("Partition1");
    partitions.push_back("Partition2");

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);
    check_local_datareader(subscriber_participant_attr, true);
    check_remote_datareader(subscriber_participant_attr, true);

    RTPSParticipantAttributes publisher_participant_attr;
    fill_publisher_participant_security_attributes(publisher_participant_attr);
    check_local_datawriter(publisher_participant_attr, true);
    check_remote_datawriter(publisher_participant_attr, true);
}

/* Regression test for Redmine 17099 (Github 3239).
 *
 * Using a modified permissions file signed with the subscriber identity certificate should fail.
 */
TEST_F(AccessControlTest, validate_fail_on_self_signed_permissions)
{
    permissions_file = "permissions_access_control_tests_malicious.smime";

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);

    PermissionsHandle* access_handle;
    get_access_handle(subscriber_participant_attr, &access_handle, false);
}

/* Regression test for advisory GHSA-w33g-jmm2-8983
 *
 * Creating a participant with an expired Permissions CA should fail.
 */
TEST_F(AccessControlTest, validate_fail_on_expired_permissions_ca)
{
    permissions_ca = "expired_ca_cert.pem";
    permissions_file = "permissions_signed_by_expired_ca.smime";
    governance_file = "governance_signed_by_expired_ca.smime";

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);

    PermissionsHandle* access_handle;
    get_access_handle(subscriber_participant_attr, &access_handle, false);
}

/* Regression test for advisory GHSA-w33g-jmm2-8983
 *
 * Permissions signed by an intermediate CA should be valid.
 */
TEST_F(AccessControlTest, validation_ok_on_chained_ca)
{
    permissions_ca = "chainedcacert.pem";
    permissions_file = "permissions_signed_by_chained_ca.smime";
    governance_file = "governance_signed_by_chained_ca.smime";

    topic_name = "HelloWorldTopic_no_partitions";

    RTPSParticipantAttributes subscriber_participant_attr;
    fill_subscriber_participant_security_attributes(subscriber_participant_attr);
    check_local_datareader(subscriber_participant_attr, true);
    check_remote_datareader(subscriber_participant_attr, true);

    RTPSParticipantAttributes publisher_participant_attr;
    fill_publisher_participant_security_attributes(publisher_participant_attr);
    check_local_datawriter(publisher_participant_attr, true);
    check_remote_datawriter(publisher_participant_attr, true);
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
