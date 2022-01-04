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

#include "BlackboxTests.hpp"

#if HAVE_SECURITY

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>

#include <gtest/gtest.h>

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

TEST(DDSSecurity, big_message_corner_case)
{
    auto qos{ fastdds::PARTICIPANT_QOS_DEFAULT };
    set_authentication_config(qos.properties().properties());
    set_participant_crypto_config(qos.properties().properties());
    auto transport = std::make_shared<rtps::UDPv4TransportDescriptor>();
    transport->interfaceWhiteList.push_back("127.0.0.1");
    qos.transport().use_builtin_transports = false;
    qos.transport().user_transports.push_back(transport);
    auto participant = fastdds::DomainParticipantFactory::get_instance()->create_participant(0, qos);
    ASSERT_NE(nullptr, participant);

    std::vector<uint32_t> lengths = { 65252 };
    DynamicType_ptr base_type = DynamicTypeBuilderFactory::get_instance()->create_char8_type();
    DynamicTypeBuilder_ptr builder =
        DynamicTypeBuilderFactory::get_instance()->create_array_builder(base_type, lengths);
    builder->set_name(TEST_TOPIC_NAME);
    DynamicType_ptr array_type = builder->build();

    fastdds::TypeSupport type_support(new eprosima::fastrtps::types::DynamicPubSubType(array_type));
    type_support.get()->auto_fill_type_information(false);
    type_support.get()->auto_fill_type_object(false);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->register_type(type_support));

    auto topic = participant->create_topic(TEST_TOPIC_NAME, TEST_TOPIC_NAME, fastdds::TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic);
    auto subscriber = participant->create_subscriber(fastdds::SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);
    auto reader = subscriber->create_datareader(topic, fastdds::DATAREADER_QOS_DEFAULT);
    ASSERT_NE(nullptr, reader);
    auto publisher = participant->create_publisher(fastdds::PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher);
    auto writer = publisher->create_datawriter(topic, fastdds::DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(nullptr, writer);

    // TODO: wait for discovery
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto data = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(array_type);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, writer->write(data, fastdds::HANDLE_NIL));

    fastdds::SampleInfo info{};
    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    } while (ReturnCode_t::RETCODE_OK != reader->take_next_sample(data, &info));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_contained_entities());
    fastdds::DomainParticipantFactory::get_instance()->delete_participant(participant);
}

#endif  // HAVE_SECURITY
