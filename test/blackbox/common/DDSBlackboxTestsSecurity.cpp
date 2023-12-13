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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
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

#endif  // HAVE_SECURITY
