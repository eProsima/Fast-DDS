// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "BlackboxTests.hpp"

#if HAVE_SECURITY

#include <algorithm>
#include <atomic>
#include <fstream>
#include <map>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <fastdds/utils/IPFinder.hpp>

#include "../utils/filter_helpers.hpp"
#include "PubSubParticipant.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "PubSubWriterReader.hpp"
#include "UDPMessageSender.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

static void fill_pub_auth(
        PropertyPolicy& policy)
{
    policy.properties().emplace_back("dds.sec.auth.plugin", "builtin.PKI-DH");
    policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem");
    policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainpubcert.pem");
    policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainpubkey.pem");

    // Select the key agreement algorithm based on process id
    switch (static_cast<uint32_t>(GET_PID()) % 4u)
    {
        // Automatic selection
        case 1u:
            policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.preferred_key_agreement", "AUTO");
            break;
        // Force DH
        case 2u:
            policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.preferred_key_agreement", "DH");
            break;
        // Force ECDH
        case 3u:
            policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.preferred_key_agreement", "ECDH");
            break;
        // Leave default
        case 0u:
        default:
            break;
    }
}

static void fill_sub_auth(
        PropertyPolicy& policy)
{
    policy.properties().emplace_back("dds.sec.auth.plugin", "builtin.PKI-DH");
    policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem");
    policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainsubcert.pem");
    policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainsubkey.pem");

    // Select the key agreement algorithm based on process id
    switch (static_cast<uint32_t>(GET_PID()) % 4u)
    {
        // Automatic selection
        case 1u:
            policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.preferred_key_agreement", "AUTO");
            break;
        // Force DH
        case 2u:
            policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.preferred_key_agreement", "DH");
            break;
        // Force ECDH
        case 3u:
            policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.preferred_key_agreement", "ECDH");
            break;
        // Leave default
        case 0u:
        default:
            break;
    }
}

class Security : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

class SecurityPkcs : public ::testing::Test
{
public:

    struct HsmToken
    {
        std::string pin;
        std::string id;
        std::string serial;
        std::map<std::string, std::string> urls;
    };

    static void create_hsm_token(
            const char* token_id)
    {
        // Init the token
        std::stringstream cmd;
        cmd << "softhsm2-util --init-token --free --label " << token_id << " --pin " << hsm_token_pin
            << " --so-pin " << hsm_token_pin << "";
        ASSERT_EQ(0, std::system (cmd.str().c_str()));
        tokens[token_id] = HsmToken();
        tokens[token_id].pin = hsm_token_pin;
        tokens[token_id].id = token_id;

        // Get the serial number of the HSM slot
        std::stringstream serial_stream;
#ifdef _WIN32 // We are running windows
        ASSERT_EQ(0,
                std::system ("powershell -C \"softhsm2-util --show-slots | sls 'Serial number:\\s*([\\d\\w]+)' | " \
                "% { $_.Matches.Groups[1].Value } | Out-File -FilePath softhsm_serial -Encoding ASCII\""));
#else // We are running something with sh
        ASSERT_EQ(0,
                std::system ("softhsm2-util --show-slots | grep -oP 'Serial number:\\s*\\K(\\d|\\w)+' > softhsm_serial"));
#endif // _WIN32
        serial_stream << std::ifstream("softhsm_serial").rdbuf();
        std::remove ("softhsm_serial");

        // Read each serial number one by one
        while (!serial_stream.eof())
        {
            std::string serial;
            serial_stream >> serial;
            if (!serial.empty())
            {
                if (tokens.end() == std::find_if(tokens.begin(), tokens.end(), [&serial](std::pair<const char* const,
                        const HsmToken> t)
                        {
                            return t.second.serial == serial;
                        }))
                {
                    tokens[token_id].serial = serial;
                    break;
                }
            }
        }
    }

    static void delete_hsm_token(
            const char* token_id)
    {
        auto it = tokens.find(token_id);
        if (it != tokens.end())
        {
            // Delete the token
            std::stringstream cmd;
            cmd << "softhsm2-util --delete-token --token " << token_id << " --pin " << hsm_token_pin
                << " --so-pin " << hsm_token_pin << "";
            ASSERT_EQ(0, std::system (cmd.str().c_str()));
            tokens.erase(it);
        }
    }

    static void SetUpTestCase()
    {
        // Init the tokens
        create_hsm_token(hsm_token_id_no_pin);
        create_hsm_token(hsm_token_id_url_pin);
        create_hsm_token(hsm_token_id_env_pin);

        // Add the keys to the tokens
        import_private_key(std::string(certs_path) + "/mainsubkey.pem", hsm_mainsubkey_label,
                "1A2B3C", hsm_token_id_no_pin);
        import_private_key(std::string(certs_path) + "/mainpubkey.pem", hsm_mainpubkey_label,
                "ABCDEF", hsm_token_id_no_pin);
        import_private_key(std::string(certs_path) + "/mainsubkey.pem", hsm_mainsubkey_label,
                "123456", hsm_token_id_url_pin);
        import_private_key(std::string(certs_path) + "/mainpubkey.pem", hsm_mainpubkey_label,
                "789ABC", hsm_token_id_url_pin);
        import_private_key(std::string(certs_path) + "/mainsubkey.pem", hsm_mainsubkey_label,
                "2468AC", hsm_token_id_env_pin);
        import_private_key(std::string(certs_path) + "/mainpubkey.pem", hsm_mainpubkey_label,
                "13579B", hsm_token_id_env_pin);
    }

    static void TearDownTestCase()
    {
        // delete the tokens
        delete_hsm_token(hsm_token_id_no_pin);
        delete_hsm_token(hsm_token_id_url_pin);
        delete_hsm_token(hsm_token_id_env_pin);
    }

    static void import_private_key(
            const std::string& key_file,
            const char* key_label,
            const char* key_id,
            const char* token_id)
    {
        ASSERT_NE(tokens.end(), tokens.find(token_id));

        std::stringstream cmd;
        cmd << "softhsm2-util --import " << key_file << " --token " << token_id << " --label " << key_label
            << " --pin " << hsm_token_pin << " --id " << key_id << "";
        // Import the key
        ASSERT_EQ(0,
                std::system(cmd.str().c_str()));
        // Construct the key URL
        std::stringstream id_url;
        for (unsigned int i = 0; i < strlen(key_id); i += 2)
        {
            id_url << "%" << key_id[i] << key_id[i + 1];
        }

        tokens[token_id].urls[key_label] = "pkcs11:model=SoftHSM%20v2;manufacturer=SoftHSM%20project;serial=" +
                tokens[token_id].serial + ";token=" + token_id + ";id=" + id_url.str() + ";object=" + key_label +
                ";type=private";
    }

    static const char* const hsm_token_id_no_pin;
    static const char* const hsm_token_id_url_pin;
    static const char* const hsm_token_id_env_pin;

    static constexpr const char* hsm_token_pin = "1234";
    static constexpr const char* hsm_mainsubkey_label = "mainsubkey";
    static constexpr const char* hsm_mainpubkey_label = "mainpubkey";

    static std::map<const char*, HsmToken> tokens;
};

std::map<const char*, SecurityPkcs::HsmToken> SecurityPkcs::tokens;
const char* const SecurityPkcs::hsm_token_id_no_pin = "testing_token_no_pin";
const char* const SecurityPkcs::hsm_token_id_url_pin = "testing_token_url_pin";
const char* const SecurityPkcs::hsm_token_id_env_pin = "testing_token_env_pin";

TEST_P(Security, BuiltinAuthenticationPlugin_PKIDH_validation_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_property_policy);

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);

    writer.history_depth(10).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Used to detect Github issue #106
TEST_P(Security, BuiltinAuthenticationPlugin_PKIDH_validation_ok_same_participant)
{
    PubSubWriterReader<HelloWorldPubSubType> wreader(TEST_TOPIC_NAME);

    PropertyPolicy property_policy;

    fill_pub_auth(property_policy);

    wreader.sub_history_depth(10).sub_reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    wreader.pub_history_depth(10);
    wreader.property_policy(property_policy).init();

    ASSERT_TRUE(wreader.isInitialized());

    // Wait for discovery.
    wreader.wait_discovery();

    auto data = default_helloworld_data_generator();

    wreader.startReception(data);

    // Send data
    wreader.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    wreader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationPlugin_PKIDH_validation_fail)
{
    {
        PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
        PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

        PropertyPolicy pub_property_policy;

        reader.history_depth(10).
                reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

        ASSERT_TRUE(reader.isInitialized());

        fill_pub_auth(pub_property_policy);

        writer.history_depth(10).
                property_policy(pub_property_policy).init();

        ASSERT_TRUE(writer.isInitialized());

        // Wait for authorization
        writer.waitUnauthorized();
    }
    {
        PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
        PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

        PropertyPolicy sub_property_policy;
        fill_sub_auth(sub_property_policy);

        reader.history_depth(10).
                reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
                property_policy(sub_property_policy).init();

        ASSERT_TRUE(reader.isInitialized());

        writer.history_depth(10).init();

        ASSERT_TRUE(writer.isInitialized());

        // Wait for authorization
        reader.waitUnauthorized();
    }
}

TEST_P(Security, BuiltinAuthenticationPlugin_PKIDH_lossy_conditions)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_property_policy);

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).init();

    // To simulate lossy conditions, we are going to remove the default
    // bultin transport, and instead use a lossy shim layer variant.
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->sendBufferSize = 65536;
    testTransport->receiveBufferSize = 65536;
    // We drop 20% of all data frags
    testTransport->dropDataMessagesPercentage = 40;
    testTransport->dropLogLength = 10;
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(testTransport);

    fill_pub_auth(pub_property_policy);

    writer.history_depth(10).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();
}

// Regresion test for Refs #13295, github #2362
TEST(Security, BuiltinAuthenticationPlugin_second_participant_creation_loop)
{
    constexpr size_t n_loops = 101;

    using Log = eprosima::fastdds::dds::Log;
    using LogConsumer = eprosima::fastdds::dds::LogConsumer;

    // A LogConsumer that just counts the number of entries consumed
    struct TestConsumer : public LogConsumer
    {
        TestConsumer(
                std::atomic_size_t& n_logs_ref)
            : n_logs_(n_logs_ref)
        {
        }

        void Consume(
                const Log::Entry&) override
        {
            ++n_logs_;
        }

    private:

        std::atomic_size_t& n_logs_;
    };

    // Counter for log entries
    std::atomic<size_t>n_logs{};

    // Prepare Log module to check that no SECURITY errors are produced
    Log::SetCategoryFilter(std::regex("SECURITY"));
    Log::SetVerbosity(Log::Kind::Error);
    Log::ClearConsumers();
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(new TestConsumer(n_logs)));

    // Class to allow waiting for the authentication message to be sent
    class AuthMessageSendStatus
    {
        bool message_sent_ = false;
        std::mutex mutex_;
        std::condition_variable cv_;

    public:

        void reset()
        {
            std::lock_guard < std::mutex> guard(mutex_);
            message_sent_ = false;
        }

        void notify()
        {
            std::lock_guard<std::mutex> guard(mutex_);
            message_sent_ = true;
            cv_.notify_one();
        }

        void wait()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() -> bool
                    {
                        return message_sent_;
                    });
        }

    };

    // Prepare transport to check that the authentication message is sent
    auto transport = std::make_shared<test_UDPv4TransportDescriptor>();
    AuthMessageSendStatus auth_message_send_status;
    transport->drop_data_messages_filter_ = [&auth_message_send_status](eprosima::fastdds::rtps::CDRMessage_t& msg)
            -> bool
            {
                auto old_pos = msg.pos;

                // Jump to writer entity id
                msg.pos += 2 + 2 + 4;

                // Read writer entity id
                eprosima::fastdds::rtps::GUID_t writer_guid;
                writer_guid.entityId = eprosima::fastdds::helpers::cdr_parse_entity_id(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos = old_pos;

                if (writer_guid.entityId == eprosima::fastdds::rtps::participant_stateless_message_writer_entity_id)
                {
                    auth_message_send_status.notify();
                }

                return false;
            };

    // Prepare participant properties
    PropertyPolicy property_policy;
    fill_pub_auth(property_policy);

    // Create the participant being checked
    PubSubReader<HelloWorldPubSubType> main_participant("HelloWorldTopic");
    main_participant.disable_builtin_transport().add_user_transport_to_pparams(transport);
    main_participant.property_policy(property_policy).init();
    EXPECT_TRUE(main_participant.isInitialized());

    // Perform a loop in which we create another participant, and destroy it just after it has been discovered.
    // This is the best reproducer of the issue, as authentication messages should be sent when a remote participant
    // is discovered.
    for (size_t n = 1; n <= n_loops; ++n)
    {
        std::cout << "Iteration " << n << std::endl;

        // Wait for undiscovery so we can wait for discovery below
        EXPECT_TRUE(main_participant.wait_participant_undiscovery());
        auth_message_send_status.reset();

        // Create another participant with authentication enabled
        PubSubParticipant<HelloWorldPubSubType> other_participant(0, 0, 0, 0);
        EXPECT_TRUE(other_participant.property_policy(property_policy).init_participant());

        // Wait for the main participant to send an authentication message to the other participant
        auth_message_send_status.wait();

        // The created participant gets out of scope here, and is destroyed
    }

    // No SECURITY error logs should have been produced
    Log::Flush();
    EXPECT_EQ(0u, n_logs);
}

TEST_P(Security, BuiltinAuthenticationPlugin_ensure_same_guid_reconnection)
{
    constexpr size_t n_loops = 10;

    using Log = eprosima::fastdds::dds::Log;
    using LogConsumer = eprosima::fastdds::dds::LogConsumer;

    // A LogConsumer that just counts the number of entries consumed
    struct TestConsumer : public LogConsumer
    {
        TestConsumer(
                std::atomic_size_t& n_logs_ref)
            : n_logs_(n_logs_ref)
        {
        }

        void Consume(
                const Log::Entry&) override
        {
            ++n_logs_;
        }

    private:

        std::atomic_size_t& n_logs_;
    };

    // Counter for log entries
    std::atomic<size_t>n_logs{};

    // Prepare Log module to check that no SECURITY errors are produced
    Log::SetCategoryFilter(std::regex("SECURITY"));
    Log::SetVerbosity(Log::Kind::Error);
    Log::ClearConsumers();
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(new TestConsumer(n_logs)));

    // Prepare participant properties
    PropertyPolicy property_policy;
    fill_pub_auth(property_policy);

    // Create the participant being checked
    PubSubWriter<HelloWorldPubSubType> main_participant("HelloWorldTopic");
    main_participant.property_policy(property_policy).init();
    EXPECT_TRUE(main_participant.isInitialized());

    eprosima::fastdds::rtps::GuidPrefix_t guid_prefix;
    memset(guid_prefix.value, 0xBB, sizeof(guid_prefix.value));

    // Perform a loop in which we create another participant, and destroy it just after it has been discovered.
    // This is the best reproducer of the issue, as authentication messages should be sent when a remote participant
    // is discovered.
    for (size_t n = 1; n <= n_loops; ++n)
    {
        std::cout << "Iteration " << n << std::endl;

        // Wait for undiscovery so we can wait for discovery below
        EXPECT_TRUE(main_participant.wait_participant_undiscovery());

        // Create another participant with authentication enabled and custom GUID
        PubSubReader<HelloWorldPubSubType> other_participant("HelloWorldTopic");
        other_participant.property_policy(property_policy).guid_prefix(guid_prefix).init();
        EXPECT_TRUE(other_participant.isInitialized());

        // Wait for mutual discovery and authentication
        main_participant.wait_discovery();
        other_participant.wait_discovery();

        // The created participant gets out of scope here, and is destroyed
    }

    // No SECURITY error logs should have been produced
    Log::Flush();
    EXPECT_EQ(0u, n_logs);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_rtps_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_property_policy);
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_shm_transport_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    auto shm_transport = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);
    reader.disable_builtin_transport();
    reader.add_user_transport_to_pparams(shm_transport);
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(shm_transport);

    PropertyPolicy pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_property_policy);
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_shm_udp_transport_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    auto shm_transport = std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
    const uint32_t segment_size = 1024 * 1024;
    shm_transport->segment_size(segment_size);
    shm_transport->max_message_size(segment_size);
    reader.disable_builtin_transport();
    reader.add_user_transport_to_pparams(shm_transport);
    reader.add_user_transport_to_pparams(udp_transport);
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(shm_transport);
    writer.add_user_transport_to_pparams(udp_transport);

    PropertyPolicy pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_property_policy);
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_rtps_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_property_policy);
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Used to detect Github issue #106
TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_rtps_ok_same_participant)
{
    PubSubWriterReader<HelloWorldPubSubType> wreader(TEST_TOPIC_NAME);

    PropertyPolicy property_policy;

    fill_pub_auth(property_policy);
    property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    wreader.pub_history_depth(10).sub_history_depth(10).sub_reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_durability_kind(eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS);
    wreader.property_policy(property_policy).init();

    ASSERT_TRUE(wreader.isInitialized());

    // Wait for discovery.
    wreader.wait_discovery();

    auto data = default_helloworld_data_generator();

    wreader.startReception(data);

    // Send data
    wreader.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    wreader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_large_string)
{
    PubSubReader<StringTestPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringTestPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_property_policy);
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_rtps_large_string)
{
    PubSubReader<StringTestPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringTestPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_property_policy);
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_rtps_data300kb)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_property_policy);
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(5).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 500;

    writer.history_depth(5).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, bytesPerPeriod, periodInMs).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_rtps_data300kb)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_property_policy);
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    reader.history_depth(5).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, bytesPerPeriod, periodInMs).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_submessage_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_submessage_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Used to detect Github issue #106
TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_submessage_ok_same_participant)
{
    PubSubWriterReader<HelloWorldPubSubType> wreader(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy,
            property_policy;

    fill_pub_auth(pub_property_policy);
    property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    wreader.pub_history_depth(10).sub_history_depth(10).sub_reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_durability_kind(eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS);
    wreader.property_policy(property_policy).
            pub_property_policy(pub_property_policy).
            sub_property_policy(sub_property_policy).init();

    ASSERT_TRUE(wreader.isInitialized());

    // Wait for discovery.
    wreader.wait_discovery();

    auto data = default_helloworld_data_generator();

    wreader.startReception(data);

    // Send data
    wreader.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    wreader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_submessage_large_string)
{
    PubSubReader<StringTestPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringTestPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_submessage_large_string)
{
    PubSubReader<StringTestPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringTestPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_submessage_data300kb)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    reader.history_depth(5).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 500;

    writer.history_depth(5).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, bytesPerPeriod, periodInMs).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_submessage_data300kb)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    reader.history_depth(5).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, bytesPerPeriod, periodInMs).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_payload_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_payload_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Used to detect Github issue #106
TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_payload_ok_same_participant)
{
    PubSubWriterReader<HelloWorldPubSubType> wreader(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy,
            property_policy;

    fill_pub_auth(property_policy);
    property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    wreader.pub_history_depth(10).sub_history_depth(10).sub_reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_durability_kind(eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS);
    wreader.property_policy(property_policy).
            pub_property_policy(pub_property_policy).
            sub_property_policy(sub_property_policy).init();

    ASSERT_TRUE(wreader.isInitialized());

    // Wait for discovery.
    wreader.wait_discovery();

    auto data = default_helloworld_data_generator();

    wreader.startReception(data);

    // Send data
    wreader.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    wreader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_payload_ok_same_participant_300kb)
{
    PubSubWriterReader<Data1mbPubSubType> wreader(TEST_TOPIC_NAME);

    PropertyPolicy pub_property_policy, sub_property_policy, property_policy;

    fill_pub_auth(property_policy);
    property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    wreader.pub_history_depth(10).sub_history_depth(10).sub_reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_durability_kind(eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS);
    wreader.property_policy(property_policy).
            pub_property_policy(pub_property_policy).
            sub_property_policy(sub_property_policy).init();

    ASSERT_TRUE(wreader.isInitialized());

    // Wait for discovery.
    wreader.wait_discovery();

    auto data = default_data300kb_data_generator();

    wreader.startReception(data);

    // Send data
    wreader.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    wreader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_payload_large_string)
{
    PubSubReader<StringTestPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringTestPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_payload_large_string)
{
    PubSubReader<StringTestPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringTestPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_payload_data300kb)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(5).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 500;

    writer.history_depth(5).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, bytesPerPeriod, periodInMs).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_payload_data300kb)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(5).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, bytesPerPeriod, periodInMs).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_all_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_all_ok)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_all_large_string)
{
    PubSubReader<StringTestPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringTestPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_all_large_string)
{
    PubSubReader<StringTestPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<StringTestPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(10).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_large_string_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_besteffort_all_data300kb)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(5).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 1000;

    writer.history_depth(5).
            reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, bytesPerPeriod, periodInMs).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_all_data300kb)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(5).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    // When doing fragmentation, it is necessary to have some degree of
    // flow control not to overrun the receive buffer.
    uint32_t bytesPerPeriod = 65536;
    uint32_t periodInMs = 50;

    writer.history_depth(5).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            add_flow_controller_descriptor_to_pparams(
        eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO, bytesPerPeriod, periodInMs).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_data_generator(5);

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Regression test of Refs #2457
TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_reliable_all_data300kb_mix)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.history_depth(5).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(2).resource_limits_max_samples(2).resource_limits_allocated_samples(2).
            asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_data300kb_mix_data_generator(10);

    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        // Send data
        writer.send_sample(data_sample);
        ++count;
        if (count % 2 == 0)
        {
            // Block reader until reception finished or timeout.
            reader.block_for_at_least(count);
        }
    }
}

// Regression test of Refs #2457, Github ros2 #438.
TEST_P(Security, BuiltinAuthenticationAndCryptoPlugin_user_data)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy pub_part_property_policy, sub_part_property_policy,
            pub_property_policy, sub_property_policy;

    fill_pub_auth(pub_part_property_policy);
    pub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    pub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    writer.history_depth(100).
            user_data({ 'a', 'b', 'c', 'd', 'e' }).
            property_policy(pub_part_property_policy).
            entity_property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    fill_sub_auth(sub_part_property_policy);
    sub_part_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_part_property_policy.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.submessage_protection_kind", "ENCRYPT");
    sub_property_policy.properties().emplace_back("rtps.endpoint.payload_protection_kind", "ENCRYPT");

    reader.set_on_discovery_function([&writer](const ParticipantBuiltinTopicData& info,
            ParticipantDiscoveryStatus /*status*/) -> bool
            {
                if (info.guid == writer.participant_guid())
                {
                    std::cout << "Received USER_DATA from the writer: ";
                    for (auto i : info.user_data)
                    {
                        std::cout << i << ' ';
                    }
                    return info.user_data == std::vector<octet>({ 'a', 'b', 'c', 'd', 'e' });
                }

                return false;
            });

    reader.history_depth(100).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_part_property_policy).
            entity_property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    reader.wait_discovery();
    writer.wait_discovery();

    reader.wait_discovery_result();
}

TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_governance_rule_order)
{
    {
        // Governance rule for topic *HelloWorldTopic* with enable_read/write_access_contrl set to false
        // Governance rule for topic * with enable_read/write_access_contrl set to true
        // Permission denied for topic HelloWorldTopic
        // Creation of reader and writer is allowed
        PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
        PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");
        std::string governance_file("governance_rule_order_test.smime");

        PropertyPolicy pub_property_policy, sub_property_policy;

        fill_sub_auth(sub_property_policy);
        sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        sub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions.smime"));

        reader.history_depth(10).
                reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
                property_policy(sub_property_policy).init();

        ASSERT_TRUE(reader.isInitialized());

        fill_pub_auth(pub_property_policy);
        pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        pub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions.smime"));

        writer.history_depth(10).
                property_policy(pub_property_policy).init();

        ASSERT_TRUE(writer.isInitialized());

        // Wait for authorization
        reader.waitAuthorized();
        writer.waitAuthorized();

        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        auto data = default_helloworld_data_generator();

        reader.startReception(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        reader.block_for_all();
    }

    {
        // Governance rule for topic * with enable_read/write_access_contrl set to true
        // Governance rule for topic *HelloWorldTopic* with enable_read/write_access_contrl set to false
        // Permission denied for topic HelloWorldTopic
        // Creation of reader and writer is denied
        PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
        PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");
        std::string governance_file("governance_rule_order_test_inverse.smime");

        PropertyPolicy pub_property_policy, sub_property_policy;

        fill_sub_auth(sub_property_policy);
        sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        sub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions.smime"));

        reader.property_policy(sub_property_policy).init();

        ASSERT_FALSE(reader.isInitialized());

        fill_pub_auth(pub_property_policy);
        pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        pub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions.smime"));

        writer.property_policy(pub_property_policy).init();

        ASSERT_FALSE(writer.isInitialized());
    }
}

TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_multiple_endpoints_matching)
{
    {
        std::string governance_file("governance_helloworld_all_enable.smime");
        std::string permissions_file("permissions_helloworld.smime");

        PropertyPolicy pub_property_policy;
        fill_pub_auth(pub_property_policy);
        pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        pub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/" + permissions_file));

        PubSubParticipant<HelloWorldPubSubType> publishers(3u, 0u, 9u, 0u);
        publishers.property_policy(pub_property_policy)
                .pub_topic_name("HelloWorldTopic");
        ASSERT_TRUE(publishers.init_participant());

        // Initializing two publishers in the same participant
        ASSERT_TRUE(publishers.init_publisher(0u));
        ASSERT_TRUE(publishers.init_publisher(1u));

        PropertyPolicy sub_property_policy;
        fill_sub_auth(sub_property_policy);
        sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        sub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/" + permissions_file));

        PubSubParticipant<HelloWorldPubSubType> subscribers(0u, 3u, 0u, 9u);
        subscribers.property_policy(sub_property_policy)
                .sub_topic_name("HelloWorldTopic");
        ASSERT_TRUE(subscribers.init_participant());

        // Initializing two subscribers in the same participant
        ASSERT_TRUE(subscribers.init_subscriber(0u));
        ASSERT_TRUE(subscribers.init_subscriber(1u));

        // Wait for discovery: 2 subs x 2 pubs
        publishers.pub_wait_discovery(4u);
        subscribers.sub_wait_discovery(4u);

        // Initializing one late joiner in the participants
        ASSERT_TRUE(subscribers.init_subscriber(2u));
        ASSERT_TRUE(publishers.init_publisher(2u));

        // Wait for discovery: 3 subs x 3 pubs
        publishers.pub_wait_discovery();
        subscribers.sub_wait_discovery();
    }
}

// Regression test of Refs #5346, Github #441.
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_fail_on_topic_wildcards)
{
    {
        // Wildcards are only considered on PERMISSIONS, Topic values should be treated as plain strings
        PubSubReader<HelloWorldPubSubType> reader("*");
        PubSubWriter<HelloWorldPubSubType> writer("*");
        std::string governance_file("governance_helloworld_all_enable.smime");

        PropertyPolicy pub_property_policy, sub_property_policy;

        fill_sub_auth(sub_property_policy);
        sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        sub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions_helloworld.smime"));

        reader.setManualTopicName("*").
                property_policy(sub_property_policy).init();
        ASSERT_FALSE(reader.isInitialized());

        fill_pub_auth(pub_property_policy);
        pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        pub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions_helloworld.smime"));

        writer.setManualTopicName("*").
                property_policy(pub_property_policy).init();

        ASSERT_FALSE(writer.isInitialized());
    }

    {
        // Wildcards are only considered on PERMISSIONS, Topic values should be treated as plain strings
        PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
        PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");
        std::string governance_file("governance_helloworld_all_enable.smime");

        PropertyPolicy pub_property_policy, sub_property_policy;

        fill_sub_auth(sub_property_policy);
        sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        sub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions_helloworld.smime"));

        reader.history_depth(10).
                reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
                property_policy(sub_property_policy).init();

        ASSERT_TRUE(reader.isInitialized());

        fill_pub_auth(pub_property_policy);
        pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        pub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions_helloworld.smime"));

        writer.history_depth(10).
                property_policy(pub_property_policy).init();

        ASSERT_TRUE(writer.isInitialized());

        // Wait for authorization
        reader.waitAuthorized();
        writer.waitAuthorized();

        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        auto data = default_helloworld_data_generator();

        reader.startReception(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        reader.block_for_all();
    }
}

// Regression test of Refs #5346, Github #441.
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_fail_on_partition_wildcards)
{
    {
        // Wildcards are only considered on PERMISSIONS, partition values should be treated as plain strings
        PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
        PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");
        std::string governance_file("governance_helloworld_all_enable.smime");

        PropertyPolicy pub_property_policy, sub_property_policy;

        fill_sub_auth(sub_property_policy);
        sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        sub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions_helloworld_partitions.smime"));

        reader.partition("*").
                property_policy(sub_property_policy).init();
        ASSERT_FALSE(reader.isInitialized());

        fill_pub_auth(pub_property_policy);
        pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        pub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions_helloworld_partitions.smime"));

        writer.partition("*").
                property_policy(pub_property_policy).init();

        ASSERT_FALSE(writer.isInitialized());
    }

    {
        // Wildcards are only considered on PERMISSIONS, partition values should be treated as plain strings
        PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
        PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");
        std::string governance_file("governance_helloworld_all_enable.smime");

        PropertyPolicy pub_property_policy, sub_property_policy;

        fill_sub_auth(sub_property_policy);
        sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        sub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions_helloworld_partitions.smime"));

        reader.history_depth(10).
                reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
                property_policy(sub_property_policy).
                partition("Partition1").init();

        ASSERT_TRUE(reader.isInitialized());

        fill_pub_auth(pub_property_policy);
        pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                "builtin.Access-Permissions"));
        pub_property_policy.properties().emplace_back(Property(
                    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://" + std::string(certs_path) + "/maincacert.pem"));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                "file://" + std::string(certs_path) + "/" + governance_file));
        pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                "file://" + std::string(certs_path) + "/permissions_helloworld_partitions.smime"));

        writer.history_depth(10).
                property_policy(pub_property_policy).
                partition("Partition*").init();

        ASSERT_TRUE(writer.isInitialized());

        // Wait for authorization
        reader.waitAuthorized();
        writer.waitAuthorized();

        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        auto data = default_helloworld_data_generator();

        reader.startReception(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        reader.block_for_all();
    }
}

// Regression test of Refs #20658, Github #4553.
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_toggle_partition)
{
    PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");
    PubSubReader<HelloWorldPubSubType> reader_p_1("HelloWorldTopic");
    PubSubReader<HelloWorldPubSubType> reader_p_2("HelloWorldTopic");

    std::string governance_file("governance_helloworld_all_enable.smime");

    // Prepare subscriptions security properties
    PropertyPolicy sub_property_policy;
    fill_sub_auth(sub_property_policy);
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    sub_property_policy.properties().emplace_back(Property(
                "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/" + governance_file));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/permissions_helloworld_partitions.smime"));

    // Initialize one reader on each partition
    reader_p_1.partition("Partition1").
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).
            init();
    ASSERT_TRUE(reader_p_1.isInitialized());

    reader_p_2.partition("Partition2").
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).
            init();
    ASSERT_TRUE(reader_p_2.isInitialized());

    // Prepare publication security properties
    PropertyPolicy pub_property_policy;
    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    pub_property_policy.properties().emplace_back(Property(
                "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/" + governance_file));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/permissions_helloworld_partitions.smime"));

    // Initialize a writer on both partitions
    writer.partition("Partition1").partition("Partition2").
            property_policy(pub_property_policy).
            init();
    ASSERT_TRUE(writer.isInitialized());

    // Wait for all entities to discover each other
    reader_p_1.wait_discovery();
    reader_p_2.wait_discovery();
    writer.wait_discovery(2u);

    constexpr size_t num_samples = 100;
    auto data = default_helloworld_data_generator(num_samples);
    reader_p_1.startReception(data);
    reader_p_2.startReception(data);

    for (size_t i = 0; i < num_samples; ++i)
    {
        // Switch to third partition and wait for all entities to unmatch
        writer.update_partition("Partition3");
        reader_p_1.wait_writer_undiscovery();
        reader_p_2.wait_writer_undiscovery();
        writer.wait_discovery(0u);

        // Switch partition and wait for the corresponding reader to discover the writer
        if (0 == i % 2)
        {
            writer.update_partition("Partition1");
            reader_p_1.wait_discovery();
        }
        else
        {
            writer.update_partition("Partition2");
            reader_p_2.wait_discovery();
        }

        // Ensure the writer matches the reader before sending the sample
        writer.wait_discovery(1u);
        writer.send_sample(data.front());
        data.pop_front();
        writer.waitForAllAcked(std::chrono::milliseconds(100));
    }

    EXPECT_EQ(num_samples / 2u, reader_p_1.getReceivedCount());
    EXPECT_EQ(num_samples / 2u, reader_p_2.getReceivedCount());
}

template <typename DataType>
void prepare_pkcs11_nodes(
        PubSubReader<DataType>& reader,
        PubSubWriter<DataType>& writer,
        const std::string& reader_private_key_url,
        const std::string& writer_private_key_url)
{
    std::string governance_file("governance_helloworld_all_enable.smime");

    // With no PIN, the load of the private key fails
    PropertyPolicy pub_property_policy;
    PropertyPolicy sub_property_policy;

    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
            "builtin.PKI-DH"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            reader_private_key_url));
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    sub_property_policy.properties().emplace_back(Property(
                "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/" + governance_file));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/permissions_helloworld.smime"));

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).init();

    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
            "builtin.PKI-DH"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainpubcert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            writer_private_key_url));
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    pub_property_policy.properties().emplace_back(Property(
                "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/" + governance_file));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/permissions_helloworld.smime"));

    writer.history_depth(10).
            property_policy(pub_property_policy).init();
}

TEST_F(SecurityPkcs, BuiltinAuthenticationAndAccessAndCryptoPlugin_pkcs11_key)
{
    {
        PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
        PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");
        prepare_pkcs11_nodes(reader, writer,
                tokens[hsm_token_id_no_pin].urls[hsm_mainsubkey_label],
                tokens[hsm_token_id_no_pin].urls[hsm_mainpubkey_label]);

        ASSERT_FALSE(reader.isInitialized());
        ASSERT_FALSE(writer.isInitialized());
    }
    {
        PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
        PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");
        prepare_pkcs11_nodes(reader, writer,
                tokens[hsm_token_id_url_pin].urls[hsm_mainsubkey_label] + "?pin-value=" + hsm_token_pin,
                tokens[hsm_token_id_url_pin].urls[hsm_mainpubkey_label] + "?pin-value=" + hsm_token_pin);

        ASSERT_TRUE(reader.isInitialized());
        ASSERT_TRUE(writer.isInitialized());

        // Wait for authorization
        reader.waitAuthorized();
        writer.waitAuthorized();

        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        auto data = default_helloworld_data_generator();

        reader.startReception(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        reader.block_for_all();

    }
    {
        // Set the PIN on the environment variable
#ifdef _WIN32
        _putenv_s("FASTDDS_PKCS11_PIN", "1234");
#else
        setenv("FASTDDS_PKCS11_PIN", "1234", 1);
#endif // ifdef _WIN32

        PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
        PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");
        prepare_pkcs11_nodes(reader, writer,
                tokens[hsm_token_id_env_pin].urls[hsm_mainsubkey_label],
                tokens[hsm_token_id_env_pin].urls[hsm_mainpubkey_label]);

        ASSERT_TRUE(reader.isInitialized());
        ASSERT_TRUE(writer.isInitialized());

        // Wait for authorization
        reader.waitAuthorized();
        writer.waitAuthorized();

        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        auto data = default_helloworld_data_generator();

        reader.startReception(data);

        // Send data
        writer.send(data);
        // In this test all data should be sent.
        ASSERT_TRUE(data.empty());
        // Block reader until reception finished or timeout.
        reader.block_for_all();

        // unset the PIN environment variable for the next round
#ifdef _WIN32
        _putenv_s("FASTDDS_PKCS11_PIN", "");
#else
        unsetenv("FASTDDS_PKCS11_PIN");
#endif // ifdef _WIN32
    }
}

static void CommonPermissionsConfigure(
        PubSubReader<HelloWorldPubSubType>& reader,
        const std::string& governance_file,
        const std::string& permissions_file,
        const PropertyPolicy& extra_properties)
{
    PropertyPolicy sub_property_policy(extra_properties);
    fill_sub_auth(sub_property_policy);
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/" + governance_file));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/" + permissions_file));

    reader.property_policy(sub_property_policy);
}

static void CommonPermissionsConfigure(
        PubSubWriter<HelloWorldPubSubType>& writer,
        const std::string& governance_file,
        const std::string& permissions_file,
        const PropertyPolicy& extra_properties)
{
    PropertyPolicy pub_property_policy(extra_properties);

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/" + governance_file));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/" + permissions_file));

    writer.property_policy(pub_property_policy);
}

static void CommonPermissionsConfigure(
        PubSubReader<HelloWorldPubSubType>& reader,
        PubSubWriter<HelloWorldPubSubType>& writer,
        const std::string& governance_file,
        const std::string& permissions_file,
        const PropertyPolicy& extra_properties = PropertyPolicy())
{
    CommonPermissionsConfigure(reader, governance_file, permissions_file, extra_properties);
    CommonPermissionsConfigure(writer, governance_file, permissions_file, extra_properties);
}

static void BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(
        PubSubReader<HelloWorldPubSubType>& reader,
        PubSubWriter<HelloWorldPubSubType>& writer,
        const std::string& governance_file)
{
    CommonPermissionsConfigure(reader, writer, governance_file, "permissions.smime");

    reader.history_depth(10).reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(10).init();
    ASSERT_TRUE(writer.isInitialized());

    // Wait for authorization
    reader.waitAuthorized();
    writer.waitAuthorized();

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Regression test of Refs #16168, Github #3102.
TEST_P(Security, RemoveParticipantProxyDataonSecurityManagerLeaseExpired_validation_no_deadlock)
{
    std::string governance_file("governance_helloworld_disable_liveliness.smime");
    std::string permissions_file("permissions_helloworld.smime");

    //!Lambda for configuring publisher participant qos and security properties
    auto secure_participant_pub_configurator = [&governance_file,
                    &permissions_file](const std::shared_ptr<PubSubWriter<HelloWorldPubSubType>>& part,
                    const std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface>& transport_interface)
            {
                part->lease_duration(3, 1);
                part->disable_builtin_transport().add_user_transport_to_pparams(transport_interface);

                PropertyPolicy property_policy;

                fill_pub_auth(property_policy);

                property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
                property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                        "builtin.Access-Permissions"));
                property_policy.properties().emplace_back(Property(
                            "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                            "file://" + std::string(certs_path) + "/maincacert.pem"));
                property_policy.properties().emplace_back(Property(
                            "dds.sec.access.builtin.Access-Permissions.governance",
                            "file://" + std::string(certs_path) + "/" + governance_file));
                property_policy.properties().emplace_back(Property(
                            "dds.sec.access.builtin.Access-Permissions.permissions",
                            "file://" + std::string(certs_path) + "/" + permissions_file));

                std::cout << " Configuring Publisher Participant Properties " << std::endl;

                part->property_policy(property_policy);

            };
    //!Lambda for configuring subscriber participant qos and security properties
    auto secure_participant_sub_configurator = [&governance_file,
                    &permissions_file](const std::shared_ptr<PubSubReader<HelloWorldPubSubType>>& part,
                    const std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface>& transport_interface)
            {
                part->lease_duration(3, 1);
                part->disable_builtin_transport().add_user_transport_to_pparams(transport_interface);

                PropertyPolicy property_policy;

                fill_sub_auth(property_policy);
                property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
                property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                        "builtin.Access-Permissions"));
                property_policy.properties().emplace_back(Property(
                            "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                            "file://" + std::string(certs_path) + "/maincacert.pem"));
                property_policy.properties().emplace_back(Property(
                            "dds.sec.access.builtin.Access-Permissions.governance",
                            "file://" + std::string(certs_path) + "/" + governance_file));
                property_policy.properties().emplace_back(Property(
                            "dds.sec.access.builtin.Access-Permissions.permissions",
                            "file://" + std::string(certs_path) + "/" + permissions_file));

                std::cout << " Configuring Subscriber Participant Properties " << std::endl;

                part->property_policy(property_policy);

            };

    //! 1.Spawn a couple of participants writer/reader
    std::string topic_name = "HelloWorldTopic";
    auto pubsub_writer = std::make_shared<PubSubWriter<HelloWorldPubSubType>>(topic_name);
    auto pubsub_reader = std::make_shared<PubSubReader<HelloWorldPubSubType>>(topic_name);

    // Initialization of all the participants
    std::cout << "Initializing PubSubs for topic " << topic_name << std::endl;

    auto test_udptransport = std::make_shared<test_UDPv4TransportDescriptor>();
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    // 2.Configure the participants
    secure_participant_pub_configurator(pubsub_writer, test_udptransport);
    pubsub_writer->init();
    ASSERT_EQ(pubsub_writer->isInitialized(), true);

    secure_participant_sub_configurator(pubsub_reader, udp_transport);
    pubsub_reader->init();
    ASSERT_EQ(pubsub_reader->isInitialized(), true);

    std::cout << std::endl << "Waiting discovery between participants." << std::endl;

    // 3.Wait for authorization
    pubsub_reader->waitAuthorized();
    pubsub_writer->waitAuthorized();

    // 4.Wait for discovery.
    pubsub_reader->wait_discovery();
    pubsub_writer->wait_discovery();

    auto data = default_helloworld_data_generator();

    pubsub_reader->startReception(data);

    // 5.Send data
    pubsub_writer->send(data);

    // 6.Block reader until reception finished or timeout.
    pubsub_reader->block_for_at_least(2);

    std::cout << "Reader received at least two samples, shutting down publisher " << std::endl;

    //! 7.Simulate a force-quit (cntrl+c) on the publisher by dropping connection
    test_udptransport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = true;

    bool pubsub_writer_undiscovered;

    //! 8.Wait reader to remove writer participant
    //! Writer participant lease duration will expire in 3 secs
    //! Check if deadlock is produced when accessing ResourceEvent collection
    //! to unregister a TimedEvent() in ResourceEvent
    pubsub_writer_undiscovered = pubsub_reader->wait_participant_undiscovery(std::chrono::seconds(6));

    //! 9.Assert if last operation timed out
    ASSERT_TRUE(pubsub_writer_undiscovered);

}

TEST(Security, AllowUnauthenticatedParticipants_EntityCreationFailsIfRTPSProtectionIsNotNONE)
{
    PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
    std::string governance_file("governance_allow_unauth_rtps_encrypt.smime");

    PropertyPolicy property_policy;

    fill_sub_auth(property_policy);
    property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    property_policy.properties().emplace_back(Property(
                "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                "file://" + std::string(certs_path) + "/maincacert.pem"));
    property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/" + governance_file));
    property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/permissions_helloworld.smime"));

    reader.property_policy(property_policy).init();

    //! If allow_unauthenticated_participants TRUE and rtps_protection is not NONE
    //! Entity creation must fail
    ASSERT_FALSE(reader.isInitialized());
}


TEST(Security, AllowUnauthenticatedParticipants_TwoSecureParticipantsWithDifferentCertificatesAreAbleToMatch)
{
    //! Create
    PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
    PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");

    PropertyPolicy pub_property_policy, sub_property_policy;

    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
            "builtin.PKI-DH"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/othercacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/othersubcert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/othersubkey.pem"));
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions_ca",
            "file://" + std::string(certs_path) + "/othercacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/governance_allow_unauth_all_disabled_access_none_other_ca.smime"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/permissions_helloworld_securehelloworld_other_ca.smime"));

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/governance_allow_unauth_all_disabled_access_none.smime"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/permissions_helloworld_securehelloworld.smime"));

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    //! Wait for the authorization to fail (~15secs)
    writer.waitUnauthorized();

    //! Wait for the discovery
    writer.wait_discovery();

    //! check that the writer matches the reader because of having allow_unauthenticated_participants enabled
    ASSERT_TRUE(writer.is_matched());

    //! Data is correctly sent and received
    auto data = default_helloworld_data_generator();

    reader.startReception(data);

    writer.send(data);

    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Block reader until reception finished or timeout.
    reader.block_for_all();

}

TEST(Security, AllowUnauthenticatedParticipants_TwoParticipantsDifferentCertificatesWithReadWriteProtectionDoNotMatch)
{
    //! Create
    PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic");
    PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic");

    PropertyPolicy pub_property_policy, sub_property_policy;

    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.plugin",
            "builtin.PKI-DH"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/othercacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/othersubcert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/othersubkey.pem"));
    sub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions_ca",
            "file://" + std::string(certs_path) + "/othercacert.pem"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) +
            "/governance_allow_unauth_all_disabled_read_write_enabled_other_ca.smime"));
    sub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/permissions_helloworld_securehelloworld_other_ca.smime"));

    reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(sub_property_policy).init();

    ASSERT_TRUE(reader.isInitialized());

    fill_pub_auth(pub_property_policy);
    pub_property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/governance_allow_unauth_all_disabled_read_write_enabled.smime"));
    pub_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/permissions_helloworld_securehelloworld.smime"));

    writer.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
            property_policy(pub_property_policy).init();

    ASSERT_TRUE(writer.isInitialized());

    //! Wait for the authorization to fail (~15secs)
    writer.waitUnauthorized();

    //! Wait some time afterwards (this will time out)
    writer.wait_discovery(std::chrono::seconds(1));

    //! check that the writer does not match the reader because of
    //! having read and write protection enabled
    //! despite allow_unauthenticated_participants is enabled
    ASSERT_FALSE(writer.is_matched());
}

// Regresion test for redmine issue 20166
TEST(Security, InANonSecureParticipantWithTwoSecureParticipantScenario_TheTwoSecureParticipantsCorrectlyCommunicate)
{
    // Create
    PubSubReader<HelloWorldPubSubType> non_secure_reader("HelloWorldTopic");
    PubSubReader<HelloWorldPubSubType> secure_reader("HelloWorldTopic");
    PubSubWriter<HelloWorldPubSubType> secure_writer("HelloWorldTopic");

    // Configure security
    const std::string governance_file("governance_helloworld_all_enable.smime");
    const std::string permissions_file("permissions_helloworld.smime");
    CommonPermissionsConfigure(secure_reader, secure_writer, governance_file, permissions_file);

    secure_writer.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(secure_writer.isInitialized());

    non_secure_reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(non_secure_reader.isInitialized());

    secure_reader.history_depth(10).
            reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(secure_reader.isInitialized());

    // Wait for the authorization
    secure_reader.waitAuthorized();
    secure_writer.waitAuthorized();

    // Wait for discovery
    secure_writer.wait_discovery(std::chrono::seconds(5));
    secure_reader.wait_discovery(std::chrono::seconds(5));

    // Data is correctly sent and received
    auto data = default_helloworld_data_generator();

    secure_reader.startReception(data);

    secure_writer.send(data);

    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    secure_reader.block_for_all();
    EXPECT_EQ(non_secure_reader.getReceivedCount(), 0u);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_enable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_disable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_disable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_enable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_enable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_disable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_disable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryDisableAccessNone_validation_ok_enable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}


// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_enable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_disable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_disable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_enable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_enable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_disable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_disable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsDisableDiscoveryEnableAccessNone_validation_ok_enable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_disable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_disable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessEncrypt_validation_ok_enable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_enable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_disable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_disable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_enable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_enable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_disable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_disable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryDisableAccessNone_validation_ok_enable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_disable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_disable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessEncrypt_validation_ok_enable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_encrypt.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_enable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_disable_discovery_enable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_disable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_enable_discovery_disable_access_encrypt)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_enable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_disable_discovery_enable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_disable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

// *INDENT-OFF*
TEST_P(Security, BuiltinAuthenticationAndAccessAndCryptoPlugin_PermissionsEnableDiscoveryEnableAccessNone_validation_ok_enable_discovery_disable_access_none)
// *INDENT-ON*
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string governance_file("governance_enable_discovery_enable_access_none.smime");

    BuiltinAuthenticationAndAccessAndCryptoPlugin_Permissions_validation_ok_common(reader, writer, governance_file);
}

TEST(Security, MaliciousHeartbeatIgnore)
{
    PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic_MaliciousHeartbeatIgnore");
    PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic_MaliciousHeartbeatIgnore");

    struct MaliciousHeartbeat
    {
        std::array<char, 4> rtps_id{{'R', 'T', 'P', 'S'}};
        std::array<uint8_t, 2> protocol_version{{2, 3}};
        std::array<uint8_t, 2> vendor_id{{0x01, 0x0F}};
        GuidPrefix_t sender_prefix;

        uint8_t submessage_id = 0x07;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
        uint8_t flags = 0;
#else
        uint8_t flags = 0x01;
#endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
        uint16_t submessage_length = 4 + 4 + 8 + 8 + 4;
        EntityId_t reader_id{};
        EntityId_t writer_id{};
        SequenceNumber_t first_sn{};
        SequenceNumber_t last_sn{};
        int32_t count = 0;
    };

    // Set custom transport on both participants
    auto transport = std::make_shared<test_UDPv4TransportDescriptor>();
    std::atomic<bool> avoid_sec_submessages{false};
    transport->sub_messages_filter_ = [&avoid_sec_submessages](CDRMessage_t& msg) -> bool
            {
                return avoid_sec_submessages.load() && (0x30 == (msg.buffer[msg.pos] & 0xF0));
            };

    UDPMessageSender fake_msg_sender;

    writer.disable_builtin_transport().add_user_transport_to_pparams(transport);
    reader.disable_builtin_transport().add_user_transport_to_pparams(transport);

    // Set custom reader locator so we can send malicious data to a known location
    Locator_t reader_locator;
    ASSERT_TRUE(IPLocator::setIPv4(reader_locator, "127.0.0.1"));
    reader_locator.port = 7000;
    reader.add_to_unicast_locator_list("127.0.0.1", 7000);

    // Set common QoS
    reader.history_depth(10).reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    writer.history_depth(10).reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);

    // Configure security
    const std::string governance_file("governance_helloworld_all_enable.smime");
    const std::string permissions_file("permissions_helloworld.smime");
    CommonPermissionsConfigure(reader, writer, governance_file, permissions_file);

    // Initialize and wait for discovery
    reader.init();
    ASSERT_TRUE(reader.isInitialized());
    writer.init();
    ASSERT_TRUE(writer.isInitialized());
    reader.wait_discovery();
    writer.wait_discovery();

    // Disable secure submessages and send data
    avoid_sec_submessages.store(true);
    auto data = default_helloworld_data_generator();
    reader.startReception(data);
    writer.send(data);
    ASSERT_TRUE(data.empty());

    // Send malicious heartbeat
    {
        auto writer_guid = writer.datawriter_guid();

        MaliciousHeartbeat hb{};
        hb.sender_prefix = writer_guid.guidPrefix;
        hb.writer_id = writer_guid.entityId;
        hb.reader_id = EntityId_t::unknown();
        hb.first_sn.low = 100;
        hb.last_sn.low = 100;
        hb.count = 100;

        CDRMessage_t msg(0);
        uint32_t msg_len = static_cast<uint32_t>(sizeof(hb));
        msg.init(reinterpret_cast<octet*>(&hb), msg_len);
        msg.length = msg_len;
        msg.pos = msg_len;
        fake_msg_sender.send(msg, reader_locator);
    }

    // Enable secure submessages
    avoid_sec_submessages.store(false);
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(Security, MaliciousParticipantRemovalIgnore)
{
    PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic_MaliciousParticipantRemovalIgnore");
    PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic_MaliciousParticipantRemovalIgnore");

    struct MaliciousParticipantRemoval
    {
        std::array<char, 4> rtps_id{ {'R', 'T', 'P', 'S'} };
        std::array<uint8_t, 2> protocol_version{ {2, 3} };
        std::array<uint8_t, 2> vendor_id{ {0x01, 0x0F} };
        GuidPrefix_t sender_prefix{};

        struct DataSubMsg
        {
            struct Header
            {
                uint8_t submessage_id = 0x15;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
                uint8_t flags = 0x02;
#else
                uint8_t flags = 0x03;
#endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
                uint16_t submessage_length = 2 + 2 + 4 + 4 + 8;
                uint16_t extra_flags = 0;
                uint16_t octets_to_inline_qos = 4 + 4 + 8;
                EntityId_t reader_id{};
                EntityId_t writer_id{};
                SequenceNumber_t sn{};
            };

            struct InlineQos
            {
                struct KeyHash
                {
                    uint16_t pid = 0x0070;  // PID_KEY_HASH
                    uint16_t plen = 16;
                    GUID_t guid{};
                };

                struct StatusInfo
                {
                    uint16_t pid = 0x0071;  // PID_STATUS_INFO
                    uint16_t plen = 4;
                    uint8_t flags[4] = { 0x00, 0x00, 0x00, 0x03 };
                };

                struct Sentinel
                {
                    uint16_t pid = 0x0001;  // PID_SENTINEL
                    uint16_t plen = 0;
                };

                KeyHash hash;
                StatusInfo status;
                Sentinel sentinel;
            };

            Header header;
            InlineQos inline_qos;
        }
        data;
    };

    // Set common QoS
    reader.history_depth(10).reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    writer.history_depth(10).reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);

    // Configure security
    const std::string governance_file("governance_helloworld_all_enable.smime");
    const std::string permissions_file("permissions_helloworld.smime");
    CommonPermissionsConfigure(reader, writer, governance_file, permissions_file);

    // Initialize and wait for authorization and discovery
    reader.init();
    ASSERT_TRUE(reader.isInitialized());
    writer.init();
    ASSERT_TRUE(writer.isInitialized());
    reader.waitAuthorized();
    writer.waitAuthorized();
    reader.wait_discovery();
    writer.wait_discovery();

    // Send fake DATA(p[UD])
    UDPMessageSender fake_msg_sender;
    {
        auto writer_guid = writer.datawriter_guid();
        auto participant_guid = writer.participant_guid();
        auto domain_id = static_cast<uint32_t>(GET_PID() % 230);

        MaliciousParticipantRemoval packet{};
        packet.sender_prefix = writer_guid.guidPrefix;
        packet.data.header.submessage_length += sizeof(packet.data.inline_qos);
        packet.data.header.writer_id = c_EntityId_SPDPWriter;
        packet.data.header.reader_id = c_EntityId_SPDPReader;
        packet.data.header.sn.low = 100;
        packet.data.inline_qos.hash.guid = participant_guid;

        Locator_t mcast_locator;
        ASSERT_TRUE(IPLocator::setIPv4(mcast_locator, "239.255.0.1"));
        mcast_locator.port = 7400 + 250 * domain_id;

        CDRMessage_t msg(0);
        uint32_t msg_len = static_cast<uint32_t>(sizeof(packet));
        msg.init(reinterpret_cast<octet*>(&packet), msg_len);
        msg.length = msg_len;
        msg.pos = msg_len;
        fake_msg_sender.send(msg, mcast_locator);
    }

    EXPECT_FALSE(reader.wait_participant_undiscovery(std::chrono::seconds(1)));

    auto data = default_helloworld_data_generator();
    reader.startReception(data);
    writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();
}

TEST(Security, ValidateAuthenticationHandshakePropertiesParsing)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy property_policy;

    fill_sub_auth(property_policy);
    property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");

    // max_handshake_requests out of bounds
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.max_handshake_requests",
            "0"));

    writer.property_policy(property_policy).init();

    // Writer creation should fail
    ASSERT_FALSE(writer.isInitialized());

    writer.destroy();

    property_policy.properties().pop_back();

    // initial_handshake_resend_period out of bounds
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.initial_handshake_resend_period",
            "-200"));

    writer.property_policy(property_policy).init();

    // Writer creation should fail
    ASSERT_FALSE(writer.isInitialized());

    writer.destroy();

    property_policy.properties().pop_back();

    // handshake_resend_period_gain out of bounds
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.handshake_resend_period_gain",
            "0.5"));

    writer.property_policy(property_policy).init();

    // Writer creation should fail
    ASSERT_FALSE(writer.isInitialized());

    writer.destroy();

    property_policy.properties().pop_back();

    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.max_handshake_requests",
            "5"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.initial_handshake_resend_period",
            "200"));
    property_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.handshake_resend_period_gain",
            "1.75"));

    writer.property_policy(property_policy).init();

    // Writer should correctly initialize
    ASSERT_TRUE(writer.isInitialized());
}

TEST(Security, ValidateAuthenticationHandshakeProperties)
{
    // Create
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    PropertyPolicy property_policy;
    std::string xml_file = "auth_handshake_props_profile.xml";
    std::string profile_name = "auth_handshake_props";

    // Set a configuration that makes participant authentication
    // to be performed quickly so that we receive handshake
    // in 0.15 secs approx
    writer.set_xml_filename(xml_file);
    writer.set_participant_profile(profile_name);

    reader.set_xml_filename(xml_file);
    reader.set_participant_profile(profile_name);

    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // If the settings were correctly applied
    // we expect to be authorized in less than 0.5 seconds
    // In reality, this time could be 0.2 perfectly,
    // but some padding is left because of the ci
    // or slower platforms
    std::chrono::duration<double, std::milli> max_time(500);
    auto t0 = std::chrono::steady_clock::now();
    reader.waitAuthorized();
    auto auth_elapsed_time = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - t0);

    // Both should be authorized
    writer.waitAuthorized();

    ASSERT_TRUE(auth_elapsed_time < max_time);
}

// Regression test for Redmine issue #20181
// Two simple secure participants with tcp transport and initial peers must match.
// It basically tests that the PDPSecurityInitiatorListener
// in PDPSimple answers back with the proxy data.
TEST(Security, security_with_initial_peers_over_tcpv4_correctly_behaves)
{
    // Create
    PubSubWriter<HelloWorldPubSubType> tcp_client("HelloWorldTopic_TCP");
    PubSubReader<HelloWorldPubSubType> tcp_server("HelloWorldTopic_TCP");

    // Search for a valid WAN address
    LocatorList_t all_locators;
    Locator_t wan_locator;
    IPFinder::getIP4Address(&all_locators);

    for (auto& locator : all_locators)
    {
        if (!IPLocator::isLocal(locator))
        {
            wan_locator = locator;
            break;
        }
    }

    uint16_t server_listening_port = 11810;
    wan_locator.port = server_listening_port;
    wan_locator.kind = LOCATOR_KIND_TCPv4;

    auto tcp_client_transport_descriptor = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
    LocatorList_t initial_peers;
    initial_peers.push_back(wan_locator);
    tcp_client.disable_builtin_transport()
            .add_user_transport_to_pparams(tcp_client_transport_descriptor)
            .initial_peers(initial_peers);

    auto tcp_server_transport_descriptor = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
    tcp_server_transport_descriptor->listening_ports.push_back(server_listening_port);
    IPLocator::copyIPv4(wan_locator, tcp_server_transport_descriptor->wan_addr);

    std::cout << "SETTING WAN address to " <<  wan_locator << std::endl;

    tcp_server.disable_builtin_transport()
            .add_user_transport_to_pparams(tcp_server_transport_descriptor);

    // Configure security
    const std::string governance_file("governance_helloworld_all_enable.smime");
    const std::string permissions_file("permissions_helloworld.smime");
    CommonPermissionsConfigure(tcp_server, tcp_client, governance_file, permissions_file);

    tcp_server.init();
    tcp_client.init();

    ASSERT_TRUE(tcp_server.isInitialized());
    ASSERT_TRUE(tcp_client.isInitialized());

    tcp_server.waitAuthorized();
    tcp_client.waitAuthorized();

    tcp_server.wait_discovery();
    tcp_client.wait_discovery();

    ASSERT_TRUE(tcp_server.is_matched());
    ASSERT_TRUE(tcp_client.is_matched());

    auto data = default_helloworld_data_generator();
    tcp_server.startReception(data);
    tcp_client.send(data);
    ASSERT_TRUE(data.empty());
    tcp_server.block_for_all(std::chrono::seconds(10));
}

// Regression test for Redmine issue #22033
// Authorized participants shall remove the changes from the
// participants secure stateless msgs pool
TEST(Security, participant_stateless_secure_writer_pool_change_is_removed_upon_participant_authentication)
{
    struct TestConsumer : public eprosima::fastdds::dds::LogConsumer
    {
        TestConsumer(
                std::atomic_size_t& n_logs_ref)
            : n_logs_(n_logs_ref)
        {
        }

        void Consume(
                const eprosima::fastdds::dds::Log::Entry&) override
        {
            ++n_logs_;
        }

    private:

        std::atomic_size_t& n_logs_;
    };

    // Counter for log entries
    std::atomic<size_t>n_logs{};

    // Prepare Log module to check that no SECURITY errors are produced
    eprosima::fastdds::dds::Log::SetCategoryFilter(std::regex("SECURITY"));
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Error);
    eprosima::fastdds::dds::Log::RegisterConsumer(std::unique_ptr<eprosima::fastdds::dds::LogConsumer>(new TestConsumer(
                n_logs)));

    const size_t n_participants = 20;

    // Create 21 secure participants
    std::vector<std::shared_ptr<PubSubReader<HelloWorldPubSubType>>> participants;
    participants.reserve(n_participants + 1);

    for (size_t i = 0; i < n_participants + 1; ++i)
    {
        participants.emplace_back(std::make_shared<PubSubReader<HelloWorldPubSubType>>("HelloWorldTopic"));
        // Configure security
        const std::string governance_file("governance_helloworld_all_enable.smime");
        const std::string permissions_file("permissions_helloworld.smime");

        PropertyPolicy handshake_prop_policy;

        handshake_prop_policy.properties().emplace_back(Property("dds.sec.auth.builtin.PKI-DH.max_handshake_requests",
                "10000000"));
        handshake_prop_policy.properties().emplace_back(Property(
                    "dds.sec.auth.builtin.PKI-DH.initial_handshake_resend_period",
                    "250"));
        handshake_prop_policy.properties().emplace_back(Property(
                    "dds.sec.auth.builtin.PKI-DH.handshake_resend_period_gain",
                    "1.0"));

        CommonPermissionsConfigure(*participants.back(), governance_file, permissions_file, handshake_prop_policy);

        // Init all except the latest one
        if (i != n_participants)
        {
            participants.back()->init();
            ASSERT_TRUE(participants.back()->isInitialized());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Wait for the first participant to authenticate the rest
    participants.front()->waitAuthorized(std::chrono::seconds::zero(), n_participants - 1);

    // Init the last one
    participants.back()->init();
    ASSERT_TRUE(participants.back()->isInitialized());

    participants.front()->waitAuthorized(std::chrono::seconds::zero(), n_participants);

    // No SECURITY error logs should have been produced
    eprosima::fastdds::dds::Log::Flush();
    EXPECT_EQ(0u, n_logs);
}

// Regression test for Redmine issue #22024
// OpenSSL assertion is not thrown when the library is abruptly finished.
TEST(Security, openssl_correctly_finishes)
{
    // Create
    PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic_openssl_is_correctly_finished");
    PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic_openssl_is_correctly_finished");

    const std::string governance_file("governance_helloworld_all_enable.smime");
    const std::string permissions_file("permissions_helloworld.smime");

    CommonPermissionsConfigure(reader, writer, governance_file, permissions_file);

    reader.init();
    writer.init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Here we force the atexit function from openssl to be abruptly called
    // i.e in a disordered way
    // If OpenSSL is not correctly finished, a SIGSEGV will be thrown
    std::exit(0);
}

// Regression test for Redmine issue #19925
TEST(Security, legacy_token_algorithms_communicate)
{
    auto test_run = [](bool legacy_pub, bool legacy_sub) -> void
            {
                // Create
                PubSubWriter<HelloWorldPubSubType> writer("HelloWorldTopic_legacy_token_algorithms_communicate");
                PubSubReader<HelloWorldPubSubType> reader("HelloWorldTopic_legacy_token_algorithms_communicate");
                const std::string governance_file("governance_helloworld_all_enable.smime");
                const std::string permissions_file("permissions_helloworld.smime");

                // Configure Writer
                {
                    PropertyPolicy extra_policy;
                    const char* value = legacy_pub ? "true" : "false";
                    auto& properties = extra_policy.properties();
                    properties.emplace_back(
                        "dds.sec.auth.builtin.PKI-DH.transmit_algorithms_as_legacy", value);
                    properties.emplace_back(
                        "dds.sec.access.builtin.Access-Permissions.transmit_algorithms_as_legacy", value);
                    CommonPermissionsConfigure(writer, governance_file, permissions_file, extra_policy);
                }

                // Configure Reader
                {
                    PropertyPolicy extra_policy;
                    const char* value = legacy_sub ? "true" : "false";
                    auto& properties = extra_policy.properties();
                    properties.emplace_back(
                        "dds.sec.auth.builtin.PKI-DH.transmit_algorithms_as_legacy", value);
                    properties.emplace_back(
                        "dds.sec.access.builtin.Access-Permissions.transmit_algorithms_as_legacy", value);
                    CommonPermissionsConfigure(reader, governance_file, permissions_file, extra_policy);
                }

                // Initialize
                reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();
                ASSERT_TRUE(reader.isInitialized());
                writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();
                ASSERT_TRUE(writer.isInitialized());

                // Wait for discovery
                reader.waitAuthorized();
                writer.waitAuthorized();
                reader.wait_discovery();
                writer.wait_discovery();
                ASSERT_TRUE(reader.is_matched());
                ASSERT_TRUE(writer.is_matched());

                // Perform communication
                auto data = default_helloworld_data_generator(1);
                reader.startReception(data);
                writer.send(data);
                ASSERT_TRUE(data.empty());
                reader.block_for_all();
            };

    // Test all possible combinations
    test_run(false, false);
    test_run(false, true);
    test_run(true, false);
    test_run(true, true);
}

void blackbox_security_init()
{
    certs_path = std::getenv("CERTS_PATH");

    if (certs_path == nullptr)
    {
        std::cout << "Cannot get enviroment variable CERTS_PATH" << std::endl;
        exit(-1);
    }
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(Security,
        Security,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<Security::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });


#endif // if HAVE_SECURITY
