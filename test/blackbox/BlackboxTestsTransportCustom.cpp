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

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <fastdds/rtps/transport/low-bandwidth/HeaderReductionTransport.h>
#include <fastdds/rtps/transport/low-bandwidth/PayloadCompressionTransport.h>
#include <fastdds/rtps/transport/low-bandwidth/SourceTimestampTransport.h>

using namespace eprosima;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

TEST(CustomTransport, PubSubHeaderReductionHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    auto udpTr = std::make_shared<UDPv4TransportDescriptor>();
    auto trDesc = std::make_shared<HeaderReductionTransportDescriptor>(udpTr);
    fastrtps::rtps::PropertyPolicy policies;
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.remove_protocol", "true"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.remove_version", "true"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.remove_vendor_id", "true"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.compress_guid_prefix",
            "25,32,32"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.submessage.combine_id_and_flags",
            "true"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.submessage.remove_extra_flags",
            "true"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.submessage.compress_entitiy_ids",
            "8,8"));
    policies.properties().emplace_back(fastrtps::rtps::Property(
                "rtps.header_reduction.submessage.compress_sequence_number", "16"));


    reader.property_policy(policies).disable_builtin_transport().add_user_transport_to_pparams(trDesc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.property_policy(policies).disable_builtin_transport().add_user_transport_to_pparams(trDesc).reliability(
        eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

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

TEST(CustomTransport, PubSubSrcTimestampHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    auto udpTr = std::make_shared<UDPv4TransportDescriptor>();
    auto trDesc = std::make_shared<SourceTimestampTransportDescriptor>(udpTr);
    int callback_num = 0;
    trDesc->callback_parameter = &callback_num;
    trDesc->callback = [] (void* p, int32_t /*s_time*/, int32_t /*r_time*/, uint32_t /*len*/)
            {
                int* pp = (int*)p;
                *pp = *pp + 1;
            };

    reader.disable_builtin_transport().add_user_transport_to_pparams(trDesc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.disable_builtin_transport().add_user_transport_to_pparams(trDesc).reliability(
        eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

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

    ASSERT_GE(callback_num, 2);
}

#if HAVE_ZLIB
TEST(CustomTransport, PubSubCompressZLibHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    auto udpTr = std::make_shared<UDPv4TransportDescriptor>();
    auto trDesc = std::make_shared<PayloadCompressionTransportDescriptor>(udpTr);
    fastrtps::rtps::PropertyPolicy policies;
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.payload_compression.compression_library",
            "ZLIB"));

    reader.property_policy(policies).disable_builtin_transport().add_user_transport_to_pparams(trDesc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.property_policy(policies).disable_builtin_transport().add_user_transport_to_pparams(trDesc).reliability(
        eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

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
#endif

#if HAVE_BZIP2
TEST(CustomTransport, PubSubCompressBZip2Helloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    auto udpTr = std::make_shared<UDPv4TransportDescriptor>();
    auto trDesc = std::make_shared<PayloadCompressionTransportDescriptor>(udpTr);
    fastrtps::rtps::PropertyPolicy policies;
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.payload_compression.compression_library",
            "BZIP2"));

    reader.property_policy(policies).disable_builtin_transport().add_user_transport_to_pparams(trDesc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.property_policy(policies).disable_builtin_transport().add_user_transport_to_pparams(trDesc).reliability(
        eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

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
#endif

#if HAVE_ZLIB || HAVE_BZIP2
TEST(CustomTransport, PubSubCompressAutoHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    auto udpTr = std::make_shared<UDPv4TransportDescriptor>();
    auto trDesc = std::make_shared<PayloadCompressionTransportDescriptor>(udpTr);
    fastrtps::rtps::PropertyPolicy policies;
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.payload_compression.compression_library",
            "AUTOMATIC"));

    reader.property_policy(policies).disable_builtin_transport().add_user_transport_to_pparams(trDesc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.property_policy(policies).disable_builtin_transport().add_user_transport_to_pparams(trDesc).reliability(
        eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

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
#endif

TEST(CustomTransport, PubSubChainReductionsHelloworld)
{
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    fastrtps::rtps::PropertyPolicy policies;

    // Lowest level is UDP
    auto udpTr = std::make_shared<UDPv4TransportDescriptor>();

    // Timestamp on top of UDP
    auto tsDesc = std::make_shared<SourceTimestampTransportDescriptor>(udpTr);
    int callback_num = 0;
    tsDesc->callback_parameter = &callback_num;
    tsDesc->callback = [](void* p, int32_t /*s_time*/, int32_t /*r_time*/, uint32_t /*len*/)
            {
                int* pp = (int*)p;
                *pp = *pp + 1;
            };

    // Compression (if available) on top of timestamp
#if HAVE_ZLIB || HAVE_BZIP2
    auto zDesc = std::make_shared<PayloadCompressionTransportDescriptor>(tsDesc);
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.payload_compression.compression_library",
            "AUTOMATIC"));
#endif

    // Header reduction on top of compression (or timestamp if compression not available)
#if HAVE_ZLIB || HAVE_BZIP2
    auto hrDesc = std::make_shared<HeaderReductionTransportDescriptor>(zDesc);
#else
    auto hrDesc = std::make_shared<HeaderReductionTransportDescriptor>(tsDesc);
#endif
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.remove_protocol", "true"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.remove_version", "true"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.remove_vendor_id", "true"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.compress_guid_prefix",
            "25,32,32"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.submessage.combine_id_and_flags",
            "true"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.submessage.remove_extra_flags",
            "true"));
    policies.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.submessage.compress_entitiy_ids",
            "8,8"));
    policies.properties().emplace_back(fastrtps::rtps::Property(
                "rtps.header_reduction.submessage.compress_sequence_number", "16"));

    reader.property_policy(policies).disable_builtin_transport().add_user_transport_to_pparams(hrDesc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.property_policy(policies).disable_builtin_transport().add_user_transport_to_pparams(hrDesc).reliability(
        eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

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
