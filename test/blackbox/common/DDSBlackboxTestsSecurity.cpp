// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <gtest/gtest.h>

#include <thread>

#include <asio.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>

namespace fastdds = ::eprosima::fastdds::dds;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

void set_authentication_config(
        rtps::PropertySeq& props)
{
    static constexpr auto kAuthPlugin{ "dds.sec.auth.plugin" };
    static constexpr auto kAuthPluginNameValue{ "builtin.PKI-DH" };
    static constexpr auto kIdentityCA{ "dds.sec.auth.builtin.PKI-DH.identity_ca" };
    static constexpr auto kIdentityCert{ "dds.sec.auth.builtin.PKI-DH.identity_certificate" };
    static constexpr auto kIdentityPrivateKey{ "dds.sec.auth.builtin.PKI-DH.private_key" };

    props.emplace_back(kAuthPlugin, kAuthPluginNameValue);
    props.emplace_back(kIdentityCA, "file://" + std::string(certs_path) + "/maincacert.pem");
    props.emplace_back(kIdentityCert, "file://" + std::string(certs_path) + "/mainsubcert.pem");
    props.emplace_back(kIdentityPrivateKey, "file://" + std::string(certs_path) + "/mainsubkey.pem");
}

void set_participant_crypto_config(
        rtps::PropertySeq& props)
{
    static constexpr auto kCryptoPlugin{ "dds.sec.crypto.plugin" };
    static constexpr auto kCryptoPluginNameValue{ "builtin.AES-GCM-GMAC" };
    static constexpr auto kRtpsProtectionKind{ "rtps.participant.rtps_protection_kind" };
    static constexpr auto kProtectionKindValue{ "ENCRYPT" };

    props.emplace_back(kCryptoPlugin, kCryptoPluginNameValue);
    props.emplace_back(kRtpsProtectionKind, kProtectionKindValue);
}

void test_big_message_corner_case(
        const std::string& name,
        uint32_t array_length)
{
    std::cout << "==== Checking with length = " << array_length << " ====" << std::endl;

    struct WriterListener : public fastdds::DataWriterListener
    {
        WriterListener(
                std::atomic_bool& cond_ref)
            : cond_(cond_ref)
        {
            cond_ = false;
        }

        void on_publication_matched(
                fastdds::DataWriter* writer,
                const fastdds::PublicationMatchedStatus& info)
        {
            static_cast<void>(writer);
            static_cast<void>(info);

            cond_ = true;
        }

    private:

        std::atomic_bool& cond_;
    };

    auto qos{ fastdds::PARTICIPANT_QOS_DEFAULT };
    set_authentication_config(qos.properties().properties());
    set_participant_crypto_config(qos.properties().properties());
    auto transport = std::make_shared<rtps::UDPv4TransportDescriptor>();
    transport->interfaceWhiteList.push_back("127.0.0.1");
    qos.transport().use_builtin_transports = false;
    qos.transport().user_transports.push_back(transport);
    auto participant = fastdds::DomainParticipantFactory::get_instance()->create_participant(0, qos);
    ASSERT_NE(nullptr, participant);

    std::vector<uint32_t> lengths = { array_length };
    DynamicType_ptr base_type = DynamicTypeBuilderFactory::get_instance()->create_char8_type();
    DynamicTypeBuilder_ptr builder =
            DynamicTypeBuilderFactory::get_instance()->create_array_builder(base_type, lengths);
    builder->set_name(name);
    DynamicType_ptr array_type = builder->build();

    fastdds::TypeSupport type_support(new eprosima::fastrtps::types::DynamicPubSubType(array_type));
    type_support.get()->auto_fill_type_information(false);
    type_support.get()->auto_fill_type_object(false);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->register_type(type_support));

    auto topic = participant->create_topic(name, name, fastdds::TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic);
    auto subscriber = participant->create_subscriber(fastdds::SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);
    auto reader = subscriber->create_datareader(topic, fastdds::DATAREADER_QOS_DEFAULT);
    ASSERT_NE(nullptr, reader);
    auto publisher = participant->create_publisher(fastdds::PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher);

    std::atomic_bool writer_matched{ false };
    WriterListener wlistener(writer_matched);
    auto writer = publisher->create_datawriter(topic, fastdds::DATAWRITER_QOS_DEFAULT, &wlistener,
                    fastdds::StatusMask::publication_matched());
    ASSERT_NE(nullptr, writer);

    // Wait for discovery to occur
    while (!writer_matched)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(array_type);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, writer->write(data, fastdds::HANDLE_NIL));

    fastdds::SampleInfo info{};
    bool taken = false;
    for (size_t n_tries = 0; n_tries < 6u; ++n_tries)
    {
        if (ReturnCode_t::RETCODE_OK == reader->take_next_sample(data, &info))
        {
            taken = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    EXPECT_TRUE(taken);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_contained_entities());
    fastdds::DomainParticipantFactory::get_instance()->delete_participant(participant);
}

TEST(DDSSecurity, big_message_corner_case)
{
    const uint32_t array_lengths[] =
    {
        // Working case
        65200,
        // Failing cases
        65240, 65252, 65260, 65300,
        // Working case
        65400
    };

    std::string topic_name = TEST_TOPIC_NAME;
    for (uint32_t len : array_lengths)
    {
        test_big_message_corner_case(topic_name, len);
    }
}

namespace {

void fill_pub_auth(
        PropertyPolicy& policy)
{
    policy.properties().emplace_back("dds.sec.auth.plugin", "builtin.PKI-DH");
    policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem");
    policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainpubcert.pem");
    policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainpubkey.pem");
}

void fill_access(
        PropertyPolicy& policy,
        const std::string& governance_file,
        const std::string& permissions_file,
        const std::string& permissions_ca_file = "maincacert.pem")
{
    policy.properties().emplace_back("dds.sec.access.plugin", "builtin.Access-Permissions");
    policy.properties().emplace_back("dds.sec.access.builtin.Access-Permissions.permissions_ca",
            "file://" + std::string(certs_path) + "/" + permissions_ca_file);
    policy.properties().emplace_back("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/" + governance_file);
    policy.properties().emplace_back("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/" + permissions_file);
}

void fill_crypto(
        PropertyPolicy& policy)
{
    policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
}

} // namespace

class DualTopicParticipant
{

    class ParticipantListener_ : public fastdds::DomainParticipantListener
    {
    public:

        ParticipantListener_(
                DualTopicParticipant* owner)
            : owner_(owner)
        {
        }

        void onParticipantAuthentication(
                fastdds::DomainParticipant*,
                ParticipantAuthenticationInfo&& info) override
        {
            if (info.status == ParticipantAuthenticationInfo::UNAUTHORIZED_PARTICIPANT)
            {
                std::lock_guard<std::mutex> lock(owner_->mtx_);
                ++owner_->unauthorized_count_;
                owner_->cv_.notify_all();
            }
        }

    private:

        DualTopicParticipant* owner_;
    };

    class WriterListener_ : public fastdds::DataWriterListener
    {
    public:

        WriterListener_(
                std::atomic<int>& counter,
                std::mutex& mtx,
                std::condition_variable& cv)
            : counter_(counter)
            , mtx_(mtx)
            , cv_(cv)
        {
        }

        void on_publication_matched(
                fastdds::DataWriter*,
                const fastdds::PublicationMatchedStatus& info) override
        {
            counter_.fetch_add(info.current_count_change);
            std::lock_guard<std::mutex> lock(mtx_);
            cv_.notify_all();
        }

    private:

        std::atomic<int>& counter_;
        std::mutex& mtx_;
        std::condition_variable& cv_;
    };

    class ReaderListener_ : public fastdds::DataReaderListener
    {
    public:

        ReaderListener_(
                std::atomic<int>& counter,
                std::mutex& mtx,
                std::condition_variable& cv)
            : counter_(counter)
            , mtx_(mtx)
            , cv_(cv)
        {
        }

        void on_subscription_matched(
                fastdds::DataReader*,
                const fastdds::SubscriptionMatchedStatus& info) override
        {
            counter_.fetch_add(info.current_count_change);
            std::lock_guard<std::mutex> lock(mtx_);
            cv_.notify_all();
        }

    private:

        std::atomic<int>& counter_;
        std::mutex& mtx_;
        std::condition_variable& cv_;
    };

public:

    DualTopicParticipant()
        : participant_(nullptr)
        , pub_(nullptr)
        , sub_(nullptr)
        , secure_writer_(nullptr)
        , plain_writer_(nullptr)
        , secure_reader_(nullptr)
        , plain_reader_(nullptr)
        , secure_topic_(nullptr)
        , plain_topic_(nullptr)
        , participant_listener_(this)
        , secure_writer_matched_(0)
        , plain_writer_matched_(0)
        , secure_reader_matched_(0)
        , plain_reader_matched_(0)
        , unauthorized_count_(0)
        , secure_writer_listener_(secure_writer_matched_, mtx_, cv_)
        , plain_writer_listener_(plain_writer_matched_, mtx_, cv_)
        , secure_reader_listener_(secure_reader_matched_, mtx_, cv_)
        , plain_reader_listener_(plain_reader_matched_, mtx_, cv_)
    {
    }

    ~DualTopicParticipant()
    {
        auto* factory = fastdds::DomainParticipantFactory::get_instance();
        if (participant_ != nullptr)
        {
            if (pub_)
            {
                if (secure_writer_)
                {
                    pub_->delete_datawriter(secure_writer_);
                }
                if (plain_writer_)
                {
                    pub_->delete_datawriter(plain_writer_);
                }
                participant_->delete_publisher(pub_);
            }
            if (sub_)
            {
                if (secure_reader_)
                {
                    sub_->delete_datareader(secure_reader_);
                }
                if (plain_reader_)
                {
                    sub_->delete_datareader(plain_reader_);
                }
                participant_->delete_subscriber(sub_);
            }
            if (secure_topic_)
            {
                participant_->delete_topic(secure_topic_);
            }
            if (plain_topic_)
            {
                participant_->delete_topic(plain_topic_);
            }
            factory->delete_participant(participant_);
        }
    }

    bool init(
            const PropertyPolicy& property_policy,
            const std::string& secure_topic_name,
            const std::string& plain_topic_name,
            bool as_writer)
    {
        auto* factory = fastdds::DomainParticipantFactory::get_instance();

        fastdds::DomainParticipantQos pqos;
        pqos.properties() = property_policy;
        pqos.wire_protocol().builtin.discovery_config.leaseDuration = { 3, 0 };
        pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = { 1, 0 };

        participant_ = factory->create_participant(
            static_cast<uint32_t>(GET_PID()) % 230,
            pqos,
            &participant_listener_,
            fastdds::StatusMask::none());

        if (!participant_)
        {
            return false;
        }

        fastdds::TypeSupport type(new HelloWorldPubSubType());
        if (ReturnCode_t::RETCODE_OK != participant_->register_type(type))
        {
            return false;
        }

        std::ostringstream suffix;
        suffix << "_" << asio::ip::host_name() << "_" << GET_PID();
        std::string secure_name = secure_topic_name + suffix.str();
        std::string plain_name  = plain_topic_name  + suffix.str();

        secure_topic_ = participant_->create_topic(secure_name, type->getName(), fastdds::TOPIC_QOS_DEFAULT);
        plain_topic_  = participant_->create_topic(plain_name,  type->getName(), fastdds::TOPIC_QOS_DEFAULT);

        if (!secure_topic_ || !plain_topic_)
        {
            return false;
        }

        if (as_writer)
        {
            pub_ = participant_->create_publisher(fastdds::PUBLISHER_QOS_DEFAULT);
            if (!pub_)
            {
                return false;
            }

            fastdds::DataWriterQos wqos = fastdds::DATAWRITER_QOS_DEFAULT;
            wqos.data_sharing().off();

            secure_writer_ = pub_->create_datawriter(secure_topic_, wqos, &secure_writer_listener_);
            plain_writer_  = pub_->create_datawriter(plain_topic_,  wqos, &plain_writer_listener_);

            return secure_writer_ != nullptr && plain_writer_ != nullptr;
        }
        else
        {
            sub_ = participant_->create_subscriber(fastdds::SUBSCRIBER_QOS_DEFAULT);
            if (!sub_)
            {
                return false;
            }

            fastdds::DataReaderQos rqos = fastdds::DATAREADER_QOS_DEFAULT;
            rqos.data_sharing().off();

            secure_reader_ = sub_->create_datareader(secure_topic_, rqos, &secure_reader_listener_);
            plain_reader_  = sub_->create_datareader(plain_topic_,  rqos, &plain_reader_listener_);

            return secure_reader_ != nullptr && plain_reader_ != nullptr;
        }
    }

    void waitUnauthorized(
            std::chrono::seconds timeout,
            unsigned int expected)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait_for(lock, timeout, [&]()
                {
                    return unauthorized_count_ >= expected;
                });
    }

    unsigned int unauthorized_count() const
    {
        return unauthorized_count_.load();
    }

    bool wait_secure_writer_matched(
            int expected,
            std::chrono::seconds timeout)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        return cv_.wait_for(lock, timeout, [&]()
                       {
                           return secure_writer_matched_.load() == expected;
                       });
    }

    bool wait_secure_reader_matched(
            int expected,
            std::chrono::seconds timeout)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        return cv_.wait_for(lock, timeout, [&]()
                       {
                           return secure_reader_matched_.load() == expected;
                       });
    }

    bool wait_plain_writer_matched(
            int expected,
            std::chrono::seconds timeout)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        return cv_.wait_for(lock, timeout, [&]()
                       {
                           return plain_writer_matched_.load() == expected;
                       });
    }

    bool wait_plain_reader_matched(
            int expected,
            std::chrono::seconds timeout)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        return cv_.wait_for(lock, timeout, [&]()
                       {
                           return plain_reader_matched_.load() == expected;
                       });
    }

    int secure_writer_matched() const
    {
        return secure_writer_matched_.load();
    }

    int plain_writer_matched()  const
    {
        return plain_writer_matched_.load();
    }

    int secure_reader_matched() const
    {
        return secure_reader_matched_.load();
    }

    int plain_reader_matched()  const
    {
        return plain_reader_matched_.load();
    }

    static bool roundtrip(
            fastdds::DataWriter* writer,
            fastdds::DataReader* reader,
            uint16_t index,
            const std::string& message,
            const std::string& label,
            std::chrono::seconds timeout)
    {
        HelloWorld sample;
        sample.index(index);
        sample.message(message);

        if (!writer->write(&sample))
        {
            std::cout << "[" << label << "] write failed" << std::endl;
            return false;
        }

        auto deadline = std::chrono::steady_clock::now() + timeout;
        HelloWorld recv;
        fastdds::SampleInfo info;
        while (std::chrono::steady_clock::now() < deadline)
        {
            if (ReturnCode_t::RETCODE_OK == reader->take_next_sample(&recv, &info))
            {
                if (info.valid_data)
                {
                    std::cout << "[" << label << "] received: index=" << recv.index()
                              << " message=\"" << recv.message() << "\"" << std::endl;
                    return recv.index() == index;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "[" << label << "] timed out waiting for sample" << std::endl;
        return false;
    }

    fastdds::DataWriter* secure_writer()
    {
        return secure_writer_;
    }

    fastdds::DataWriter* plain_writer()
    {
        return plain_writer_;
    }

    fastdds::DataReader* secure_reader()
    {
        return secure_reader_;
    }

    fastdds::DataReader* plain_reader()
    {
        return plain_reader_;
    }

private:

    fastdds::DomainParticipant*   participant_;
    fastdds::Publisher*           pub_;
    fastdds::Subscriber*          sub_;
    fastdds::DataWriter*          secure_writer_;
    fastdds::DataWriter*          plain_writer_;
    fastdds::DataReader*          secure_reader_;
    fastdds::DataReader*          plain_reader_;
    fastdds::Topic*               secure_topic_;
    fastdds::Topic*               plain_topic_;
    ParticipantListener_ participant_listener_;
    std::atomic<int>     secure_writer_matched_;
    std::atomic<int>     plain_writer_matched_;
    std::atomic<int>     secure_reader_matched_;
    std::atomic<int>     plain_reader_matched_;
    std::atomic<unsigned int> unauthorized_count_;
    std::mutex mtx_;
    std::condition_variable cv_;
    WriterListener_ secure_writer_listener_;
    WriterListener_ plain_writer_listener_;
    ReaderListener_ secure_reader_listener_;
    ReaderListener_ plain_reader_listener_;
};

/**
 * This is a regression test for redmine issue #24414
 * It checks that 2 participants communicating on different topics can continue to communicate in an unprotected topic,
 * even if one of them has a secure topic that has been unmatched with the other participant.
 */
TEST(DDSSecurity, SecureTopicUnmatchDoesNotAffectUnsecuredTopic)
{
    std::string governance_file("governance_allow_unauth_all_disabled_access_none.smime");
    std::string permissions_file("permissions_expiration_two_topics.smime");

#ifdef _WIN32
    std::string running_command = "python generate_expiring_ca.py";
    FILE* pipe = _popen(running_command.c_str(), "r");
#else
    std::string running_command = "python3 generate_expiring_ca.py";
    FILE* pipe = popen(running_command.c_str(), "r");
#endif // _WIN32

    int return_code_ = -1;
    if (pipe)
    {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            std::cout << buffer;
        }
#ifdef _WIN32
        return_code_ = _pclose(pipe);
#else
        return_code_ = pclose(pipe);
#endif // ifdef _WIN32
    }

    if (return_code_ != 0)
    {
        std::cout << "Error running system" << std::endl;
    }

    std::string secure_topic_name("SecureHelloWorldTopic");
    std::string plain_topic_name("HelloWorldTopic");

    PropertyPolicy prop_a;
    fill_pub_auth(prop_a);
    fill_access(prop_a, governance_file, permissions_file);
    fill_crypto(prop_a);

    PropertyPolicy prop_b;
    prop_b.properties().emplace_back("dds.sec.auth.plugin", "builtin.PKI-DH");
    prop_b.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem");
    prop_b.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/modifiablesubcert.pem");
    prop_b.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/modifiablesubkey.pem");
    fill_access(prop_b, governance_file, permissions_file);
    fill_crypto(prop_b);

    DualTopicParticipant participant_a;
    DualTopicParticipant participant_b;

    ASSERT_TRUE(participant_a.init(prop_a, secure_topic_name, plain_topic_name, true));
    ASSERT_TRUE(participant_b.init(prop_b, secure_topic_name, plain_topic_name, false));

    std::cout << std::endl << "Waiting for endpoint discovery on both topics." << std::endl;

    ASSERT_TRUE(participant_a.wait_secure_writer_matched(1, std::chrono::seconds(10)));
    ASSERT_TRUE(participant_a.wait_plain_writer_matched(1, std::chrono::seconds(10)));
    ASSERT_TRUE(participant_b.wait_secure_reader_matched(1, std::chrono::seconds(10)));
    ASSERT_TRUE(participant_b.wait_plain_reader_matched(1, std::chrono::seconds(10)));

    ASSERT_EQ(participant_a.secure_writer_matched(), 1);
    ASSERT_EQ(participant_a.plain_writer_matched(),  1);
    ASSERT_EQ(participant_b.secure_reader_matched(), 1);
    ASSERT_EQ(participant_b.plain_reader_matched(),  1);

    std::cout << std::endl << "Verifying data flow before certificate expiration." << std::endl;
    ASSERT_TRUE(DualTopicParticipant::roundtrip(
                participant_a.secure_writer(), participant_b.secure_reader(),
                1, "pre-revoke secure", "secure pre", std::chrono::seconds(5)));
    ASSERT_TRUE(DualTopicParticipant::roundtrip(
                participant_a.plain_writer(), participant_b.plain_reader(),
                2, "pre-revoke plain", "plain pre", std::chrono::seconds(5)));

    std::this_thread::sleep_for(std::chrono::seconds(30));

    participant_a.waitUnauthorized(std::chrono::seconds(15), 1);
    ASSERT_GE(participant_a.unauthorized_count(), 1u);

    participant_b.waitUnauthorized(std::chrono::seconds(15), 1);
    ASSERT_GE(participant_b.unauthorized_count(), 1u);

    ASSERT_TRUE(participant_a.wait_secure_writer_matched(0, std::chrono::seconds(10)));
    ASSERT_EQ(participant_a.secure_writer_matched(), 0);
    ASSERT_EQ(participant_b.secure_reader_matched(), 0);

    ASSERT_EQ(participant_a.plain_writer_matched(), 1);
    ASSERT_EQ(participant_b.plain_reader_matched(), 1);

    ASSERT_FALSE(DualTopicParticipant::roundtrip(
                participant_a.secure_writer(), participant_b.secure_reader(),
                3, "post-revoke secure", "secure post", std::chrono::seconds(3)));

    ASSERT_TRUE(DualTopicParticipant::roundtrip(
                participant_a.plain_writer(), participant_b.plain_reader(),
                4, "post-revoke plain", "plain post", std::chrono::seconds(5)));
}

#endif  // HAVE_SECURITY
