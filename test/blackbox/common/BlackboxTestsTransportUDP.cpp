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

#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>
#include <fastdds/utils/IPFinder.hpp>

#include "BlackboxTests.hpp"
#include "DatagramInjectionTransport.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "UDPMessageSender.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT
};

class TransportUDP : public testing::TestWithParam<std::tuple<communication_type, bool>>
{
public:

    void SetUp() override
    {
        test_transport_.reset();
        use_udpv4 = std::get<1>(GetParam());
        if (use_udpv4)
        {
            test_transport_ = std::make_shared<UDPv4TransportDescriptor>();
        }
        else
        {
#ifdef __APPLE__
            // TODO: fix IPv6 issues related with zone ID
            GTEST_SKIP() << "UDPv6 tests are disabled in Mac";
#endif // ifdef __APPLE__
            test_transport_ = std::make_shared<UDPv6TransportDescriptor>();
        }
    }

    void TearDown() override
    {
        use_udpv4 = true;
    }

    void get_ip_address(
            LocatorList_t* loc)
    {
        if (use_udpv4)
        {
            eprosima::fastdds::rtps::IPFinder::getIP4Address(loc);
        }
        else
        {
            eprosima::fastdds::rtps::IPFinder::getIP6Address(loc);
        }
    }

    std::shared_ptr<UDPTransportDescriptor> test_transport_;
    std::string ip0;
    std::string ip1;
    std::string ip2;
};

TEST_P(TransportUDP, UDPTransportWrongConfigMaxMessageSize)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    test_transport_->maxMessageSize = 100000;

    writer.disable_builtin_transport().
            add_user_transport_to_pparams(test_transport_).init();

    ASSERT_FALSE(writer.isInitialized());
}

TEST_P(TransportUDP, UDPTransportWrongConfigSendBufferSize)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    test_transport_->sendBufferSize = 64000;

    writer.disable_builtin_transport().
            add_user_transport_to_pparams(test_transport_).init();

    ASSERT_FALSE(writer.isInitialized());
}

TEST_P(TransportUDP, UDPTransportWrongConfigReceiveBufferSize)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    test_transport_->receiveBufferSize = 64000;

    writer.disable_builtin_transport().
            add_user_transport_to_pparams(test_transport_).init();

    ASSERT_FALSE(writer.isInitialized());
}

// TODO - GASCO: UDPMaxInitialPeer tests should use static discovery through initial peers.
TEST_P(TransportUDP, UDPMaxInitialPeer_P0_4_P3)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastdds::rtps::LocatorList_t loc;
    get_ip_address(&loc);

    if (!use_udpv4)
    {
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

    reader.participant_id(0).max_initial_peers_range(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.participant_id(3).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());
}

TEST_P(TransportUDP, UDPMaxInitialPeer_P0_4_P4)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastdds::rtps::LocatorList_t loc;
    get_ip_address(&loc);

    if (!use_udpv4)
    {
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

    reader.participant_id(0).max_initial_peers_range(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.participant_id(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());
}

TEST_P(TransportUDP, UDPMaxInitialPeer_P5_4_P4)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastdds::rtps::LocatorList_t loc;
    get_ip_address(&loc);

    if (!use_udpv4)
    {
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

    reader.participant_id(5).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.max_initial_peers_range(4).participant_id(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));

    ASSERT_FALSE(writer.is_matched());
    ASSERT_FALSE(reader.is_matched());
}

TEST_P(TransportUDP, UDPMaxInitialPeer_P5_6_P4)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // Disallow multicast discovery
    eprosima::fastdds::rtps::LocatorList_t loc;
    get_ip_address(&loc);

    if (!use_udpv4)
    {
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

    reader.participant_id(5).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.max_initial_peers_range(6).participant_id(4).metatraffic_unicast_locator_list(loc).initial_peers(loc).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());
}

// Used to reproduce VPN environment issue with multicast.
TEST_P(TransportUDP, MulticastCommunicationBadReader)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    ip0 = use_udpv4 ? "127.0.0.1" : "::1";
    ip1 = use_udpv4 ? "239.255.1.4" : "ff1e::ffff:efff:104";
    ip2 = use_udpv4 ? "239.255.1.5" : "ff1e::ffff:efff:105";

    test_transport_->interfaceWhiteList.push_back(ip0);

    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    writer.add_to_metatraffic_multicast_locator_list(ip2, global_port);
    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldPubSubType> readerMultiBad(TEST_TOPIC_NAME);
    readerMultiBad.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    readerMultiBad.add_to_metatraffic_multicast_locator_list(ip1, global_port);
    readerMultiBad.init();

    ASSERT_TRUE(readerMultiBad.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    readerMultiBad.wait_discovery(std::chrono::seconds(3));
    ASSERT_FALSE(writer.is_matched());
    ASSERT_FALSE(readerMultiBad.is_matched());
}

// Used to reproduce VPN environment issue with multicast.
TEST_P(TransportUDP, MulticastCommunicationOkReader)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    ip0 = use_udpv4 ? "127.0.0.1" : "::1";
    ip2 = use_udpv4 ? "239.255.1.5" : "ff1e::ffff:efff:105";

    // TODO(jlbueno) When announcing from localhost to multicast the RTPS packets are being sent (wireshark captures
    // them) but the packets are not received in the remote participant.
    // Using any other interface different from localhost, the test passes.
    // Disabling multicast and setting initial peers, the test also passes.
    if (use_udpv4)
    {
        test_transport_->interfaceWhiteList.push_back(ip0);

        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.add_to_metatraffic_multicast_locator_list(ip2, global_port);
        writer.init();

        ASSERT_TRUE(writer.isInitialized());

        PubSubReader<HelloWorldPubSubType> readerMultiOk(TEST_TOPIC_NAME);
        readerMultiOk.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        readerMultiOk.add_to_metatraffic_multicast_locator_list(ip2, global_port);
        readerMultiOk.init();

        ASSERT_TRUE(readerMultiOk.isInitialized());

        writer.wait_discovery();
        readerMultiOk.wait_discovery();
        ASSERT_TRUE(writer.is_matched());
        ASSERT_TRUE(readerMultiOk.is_matched());
    }
}

// #4420 Using whitelists in localhost sometimes UDP doesn't receive the release input channel message.
TEST_P(TransportUDP, whitelisting_udp_localhost_multi)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    ip0 = use_udpv4 ? "127.0.0.1" : "::1";

    // TODO(jlbueno) When announcing from localhost to multicast the RTPS packets are being sent (wireshark captures
    // them) but the packets are not received in the remote participant.
    // Using any other interface different from localhost, the test passes.
    // Disabling multicast and setting initial peers, the test also passes.
    if (use_udpv4)
    {
        test_transport_->interfaceWhiteList.push_back(ip0);

        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.init();

        ASSERT_TRUE(writer.isInitialized());

        for (int i = 0; i < 200; ++i)
        {
            PubSubReader<HelloWorldPubSubType> readerMultiOk(TEST_TOPIC_NAME);
            readerMultiOk.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
            readerMultiOk.init();

            ASSERT_TRUE(readerMultiOk.isInitialized());

            writer.wait_discovery();
            readerMultiOk.wait_discovery();
            ASSERT_TRUE(writer.is_matched());
            ASSERT_TRUE(readerMultiOk.is_matched());
        }
    }
}

// Checking correct copying of participant user data locators to the writers/readers
TEST_P(TransportUDP, DefaultMulticastLocatorsParticipant)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    size_t writer_samples = 5;

    ip1 = use_udpv4 ? "239.255.0.1" : "ff1e::ffff:efff:1";
    if (!use_udpv4)
    {
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

    writer.add_to_default_multicast_locator_list(ip1, 22222);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    reader.add_to_default_multicast_locator_list(ip1, 22222);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));
    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());

    auto data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Checking correct copying of participant metatraffic locators to the datawriters/datatreaders
TEST_P(TransportUDP, MetatrafficMulticastLocatorsParticipant)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Warning);

    size_t writer_samples = 5;

    ip1 = use_udpv4 ? "239.255.1.1" : "ff1e::ffff:efff:101";

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    writer.add_to_metatraffic_multicast_locator_list(ip1, 22222);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    reader.add_to_metatraffic_multicast_locator_list(ip1, 22222);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));
    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());

    auto data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Checking correct copying of participant user data locators to the writers/readers
TEST_P(TransportUDP, DefaultMulticastLocatorsParticipantZeroPort)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    size_t writer_samples = 5;

    ip1 = use_udpv4 ? "239.255.0.1" : "ff1e::ffff:efff:1";
    if (!use_udpv4)
    {
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    }

    writer.add_to_default_multicast_locator_list(ip1, 0);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    reader.add_to_default_multicast_locator_list(ip1, 0);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));
    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());

    auto data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Checking correct copying of participant metatraffic locators to the datawriters/datatreaders
TEST_P(TransportUDP, MetatrafficMulticastLocatorsParticipantZeroPort)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Warning);

    size_t writer_samples = 5;

    ip1 = use_udpv4 ? "239.255.1.1" : "ff1e::ffff:efff:101";

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    writer.add_to_metatraffic_multicast_locator_list(ip1, 0);
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
    reader.add_to_metatraffic_multicast_locator_list(ip1, 0);
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));
    ASSERT_TRUE(writer.is_matched());
    ASSERT_TRUE(reader.is_matched());

    auto data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// #4420 Using whitelists in localhost sometimes UDP doesn't receive the release input channel message.
TEST_P(TransportUDP, whitelisting_udp_localhost_alone)
{
    ip0 = use_udpv4 ? "127.0.0.1" : "::1";

    test_transport_->interfaceWhiteList.push_back(ip0);

    for (int i = 0; i < 200; ++i)
    {
        PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
        writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport_);
        writer.init();
        ASSERT_TRUE(writer.isInitialized());
    }
}

TEST(TransportUDP, DatagramInjection)
{
    using eprosima::fastdds::rtps::DatagramInjectionTransportDescriptor;
    using eprosima::fastdds::rtps::DatagramInjectionTransport;

    auto low_level_transport = std::make_shared<UDPv4TransportDescriptor>();
    auto transport = std::make_shared<DatagramInjectionTransportDescriptor>(low_level_transport);

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.disable_builtin_transport().add_user_transport_to_pparams(transport).init();
    ASSERT_TRUE(writer.isInitialized());

    auto receivers = transport->get_receivers();
    ASSERT_FALSE(receivers.empty());

    DatagramInjectionTransport::deliver_datagram_from_file(receivers, "datagrams/16784.bin");
    DatagramInjectionTransport::deliver_datagram_from_file(receivers, "datagrams/20140.bin");
    DatagramInjectionTransport::deliver_datagram_from_file(receivers, "datagrams/20574.bin");
    DatagramInjectionTransport::deliver_datagram_from_file(receivers, "datagrams/20660.bin");
}

TEST(TransportUDP, MaliciousManipulatedDataOctetsToNextHeaderIgnore)
{
    // Force using UDP transport
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    PubSubWriter<UnboundedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<UnboundedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    struct MaliciousManipulatedDataOctetsToNextHeader
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
                uint8_t flags = 0x04;
#else
                uint8_t flags = 0x05;
#endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
                uint16_t octets_to_next_header = 0x30;
                uint16_t extra_flags = 0;
                uint16_t octets_to_inline_qos = 0x2d;
                EntityId_t reader_id{};
                EntityId_t writer_id{};
                SequenceNumber_t sn{100};
            };

            struct SerializedData
            {
                uint16_t encapsulation;
                uint16_t encapsulation_opts;
                octet data[24];
            };

            Header header;
            SerializedData payload;
        }
        data;

        uint8_t additional_bytes[8] {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    };

    UDPMessageSender fake_msg_sender;

    // Set common QoS
    reader.disable_builtin_transport().add_user_transport_to_pparams(udp_transport)
            .history_depth(10).reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    writer.history_depth(10).reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);

    // Set custom reader locator so we can send malicious data to a known location
    Locator_t reader_locator;
    ASSERT_TRUE(IPLocator::setIPv4(reader_locator, "127.0.0.1"));
    reader_locator.port = 7000;
    reader.add_to_unicast_locator_list("127.0.0.1", 7000);

    // Initialize and wait for discovery
    reader.init();
    ASSERT_TRUE(reader.isInitialized());
    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    reader.wait_discovery();
    writer.wait_discovery();

    auto data = default_unbounded_helloworld_data_generator();
    reader.startReception(data);
    writer.send(data);
    ASSERT_TRUE(data.empty());

    // Send malicious data
    {
        auto writer_guid = writer.datawriter_guid();

        MaliciousManipulatedDataOctetsToNextHeader malicious_packet{};
        malicious_packet.sender_prefix = writer_guid.guidPrefix;
        malicious_packet.data.header.writer_id = writer_guid.entityId;
        malicious_packet.data.header.reader_id = reader.datareader_guid().entityId;
        malicious_packet.data.payload.encapsulation = CDR_LE;

        CDRMessage_t msg(0);
        uint32_t msg_len = static_cast<uint32_t>(sizeof(malicious_packet));
        msg.init(reinterpret_cast<octet*>(&malicious_packet), msg_len);
        msg.length = msg_len;
        msg.pos = msg_len;
        fake_msg_sender.send(msg, reader_locator);
    }

    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

/**
 * This is a regression test for redmine issue #21707.
 */
static void KeyOnlyBigPayloadIgnored(
        PubSubWriter<KeyedHelloWorldPubSubType>& writer,
        PubSubReader<KeyedHelloWorldPubSubType>& reader)
{
    struct KeyOnlyBigPayloadDatagram
    {
        struct RTPSHeader
        {
            std::array<char, 4> rtps_id{ {'R', 'T', 'P', 'S'} };
            std::array<uint8_t, 2> protocol_version{ {2, 3} };
            std::array<uint8_t, 2> vendor_id{ {0x01, 0x0F} };
            GuidPrefix_t sender_prefix{};
        }
        header;

        static_assert(sizeof(RTPSHeader) == RTPSMESSAGE_HEADER_SIZE, "Unexpected size for RTPS header");

        struct DataSubMsg
        {
            struct Header
            {
                uint8_t submessage_id = 0x15;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
                uint8_t flags = 0x0A; // Serialized key, inline QoS
#else
                uint8_t flags = 0x0B; // Serialized key, inline QoS, endianness
#endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
                uint16_t octets_to_next_header = 0x48;
                uint16_t extra_flags = 0;
                uint16_t octets_to_inline_qos = 0x10;
                EntityId_t reader_id{};
                EntityId_t writer_id{};
                SequenceNumber_t sn{ 2 };
            };

            static_assert(sizeof(Header) == RTPSMESSAGE_DATA_MIN_LENGTH, "Unexpected size for DATA header");

            struct InlineQoS
            {
                // PID_STATUS_INFO (unregistered + disposed)
                struct StatusInfo
                {
                    uint16_t pid = 0x0071;
                    uint16_t length = 0x0004;
                    std::array<uint8_t, 4> status_value{ {0x00, 0x00, 0x00, 0x03} };
                };
                // PID_SENTINEL
                struct Sentinel
                {
                    uint16_t pid = 0x0001;
                    uint16_t length = 0x0000;
                };

                StatusInfo status_info;
                Sentinel sentinel;
            };

            static_assert(sizeof(InlineQoS) == 8 + 4, "Unexpected size for InlineQoS");

            struct SerializedData
            {
                std::array<uint8_t, 2> encapsulation {{0}};
                std::array<uint8_t, 2> encapsulation_opts {{0}};
                std::array<uint8_t, 0x24> data {{0}};
            };

            static_assert(sizeof(SerializedData) == 0x24 + 2 + 2, "Unexpected size for SerializedData");

            Header header;
            InlineQoS qos;
            SerializedData payload;
        }
        data;

        static_assert(
            sizeof(DataSubMsg) ==
            sizeof(DataSubMsg::Header) + sizeof(DataSubMsg::InlineQoS) + sizeof(DataSubMsg::SerializedData),
            "Unexpected size for DataSubMsg");
    };

    static_assert(
        sizeof(KeyOnlyBigPayloadDatagram) ==
        sizeof(KeyOnlyBigPayloadDatagram::RTPSHeader) + sizeof(KeyOnlyBigPayloadDatagram::DataSubMsg),
        "Unexpected size for KeyOnlyBigPayloadDatagram");

    UDPMessageSender fake_msg_sender;

    // Force using UDP transport
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    // Set common QoS
    reader.disable_builtin_transport().add_user_transport_to_pparams(udp_transport);

    // Set custom reader locator so we can send malicious data to a known location
    Locator_t reader_locator;
    ASSERT_TRUE(IPLocator::setIPv4(reader_locator, "127.0.0.1"));
    reader_locator.port = 7000;
    reader.add_to_unicast_locator_list("127.0.0.1", 7000);

    // Initialize and wait for discovery
    reader.init();
    ASSERT_TRUE(reader.isInitialized());
    writer.init();
    ASSERT_TRUE(writer.isInitialized());
    reader.wait_discovery();
    writer.wait_discovery();

    // Send one sample
    std::list<KeyedHelloWorld> data;
    KeyedHelloWorld sample;
    sample.key(0);
    sample.index(1);
    sample.message("KeyedHelloWorld 1 (key = 0)");
    data.push_back(sample);
    reader.startReception(data);
    writer.send(data);
    ASSERT_TRUE(data.empty());

    // Wait for the reader to receive the sample
    reader.block_for_all();

    // Send unregister disposed without PID_KEY_HASH, and long key-only payload
    {
        auto writer_guid = writer.datawriter_guid();

        KeyOnlyBigPayloadDatagram malicious_packet{};
        malicious_packet.header.sender_prefix = writer_guid.guidPrefix;
        malicious_packet.data.header.writer_id = writer_guid.entityId;
        malicious_packet.data.header.reader_id = reader.datareader_guid().entityId;
        malicious_packet.data.payload.encapsulation[1] = CDR_LE;
        malicious_packet.data.payload.data.fill(0x00);

        CDRMessage_t msg(0);
        uint32_t msg_len = static_cast<uint32_t>(sizeof(malicious_packet));
        msg.init(reinterpret_cast<octet*>(&malicious_packet), msg_len);
        msg.length = msg_len;
        msg.pos = msg_len;
        fake_msg_sender.send(msg, reader_locator);
    }

    // Wait some time to let the message be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

}

TEST(TransportUDP, KeyOnlyBigPayloadIgnored_Reliable)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    // Set reliability
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);

    KeyOnlyBigPayloadIgnored(writer, reader);
}

TEST(TransportUDP, KeyOnlyBigPayloadIgnored_BestEffort)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    // Set reliability
    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);

    KeyOnlyBigPayloadIgnored(writer, reader);
}

// Test for ==operator UDPTransportDescriptor is not required as it is an abstract class and in UDPv4 is same method
// Test for copy UDPTransportDescriptor is not required as it is an abstract class and in UDPv4 is same method

// Test == operator for UDPv4
TEST(BlackBox, UDPv4_equal_operator)
{
    // UDPv4TransportDescriptor
    UDPv4TransportDescriptor udpv4_transport_1;
    UDPv4TransportDescriptor udpv4_transport_2;

    // Compare equal in defult values
    ASSERT_EQ(udpv4_transport_1, udpv4_transport_2);

    // Modify some default values in 1
    udpv4_transport_1.non_blocking_send = !udpv4_transport_1.non_blocking_send; // change default value
    udpv4_transport_1.m_output_udp_socket = udpv4_transport_1.m_output_udp_socket + 10; // change default value

    ASSERT_FALSE(udpv4_transport_1 == udpv4_transport_2); // operator== != operator!=, using operator== == false instead

    // Modify default values in 2
    udpv4_transport_2.non_blocking_send = !udpv4_transport_2.non_blocking_send; // change default value
    udpv4_transport_2.m_output_udp_socket = udpv4_transport_2.m_output_udp_socket + 10; // change default value

    ASSERT_EQ(udpv4_transport_1, udpv4_transport_2);
}

// Test copy constructor and copy assignment for UDPv4
TEST(BlackBox, UDPv4_copy)
{
    UDPv4TransportDescriptor udpv4_transport;
    udpv4_transport.non_blocking_send = !udpv4_transport.non_blocking_send; // change default value
    udpv4_transport.m_output_udp_socket = udpv4_transport.m_output_udp_socket + 10; // change default value

    // Copy constructor
    UDPv4TransportDescriptor udpv4_transport_copy_constructor(udpv4_transport);
    EXPECT_EQ(udpv4_transport, udpv4_transport_copy_constructor);

    // Copy assignment
    UDPv4TransportDescriptor udpv4_transport_copy = udpv4_transport;
    EXPECT_EQ(udpv4_transport_copy, udpv4_transport);
}

// Test == operator for UDPv6
TEST(BlackBox, UDPv6_equal_operator)
{
    // UDPv6TransportDescriptor
    eprosima::fastdds::rtps::UDPv6TransportDescriptor udpv6_transport_1;
    eprosima::fastdds::rtps::UDPv6TransportDescriptor udpv6_transport_2;

    // Compare equal in defult values
    ASSERT_EQ(udpv6_transport_1, udpv6_transport_2);

    // Modify some default values in 1
    udpv6_transport_1.non_blocking_send = !udpv6_transport_1.non_blocking_send; // change default value
    udpv6_transport_1.m_output_udp_socket = udpv6_transport_1.m_output_udp_socket + 10; // change default value

    ASSERT_FALSE(udpv6_transport_1 == udpv6_transport_2); // operator== != operator!=, using operator== == false instead


    // Modify some default values in 2
    udpv6_transport_2.non_blocking_send = !udpv6_transport_2.non_blocking_send; // change default value
    udpv6_transport_2.m_output_udp_socket = udpv6_transport_2.m_output_udp_socket + 10; // change default value

    ASSERT_EQ(udpv6_transport_1, udpv6_transport_2);
}

// Test copy constructor and copy assignment for UDPv6
TEST(BlackBox, UDPv6_copy)
{
    // Change some varibles in order to check the non default cretion
    eprosima::fastdds::rtps::UDPv6TransportDescriptor udpv6_transport;
    udpv6_transport.non_blocking_send = !udpv6_transport.non_blocking_send; // change default value
    udpv6_transport.m_output_udp_socket = udpv6_transport.m_output_udp_socket + 10; // change default value

    // Copy constructor
    eprosima::fastdds::rtps::UDPv6TransportDescriptor udpv6_transport_copy_constructor(udpv6_transport);
    EXPECT_EQ(udpv6_transport, udpv6_transport_copy_constructor);

    // Copy assignment
    eprosima::fastdds::rtps::UDPv6TransportDescriptor udpv6_transport_copy = udpv6_transport;
    EXPECT_EQ(udpv6_transport_copy, udpv6_transport);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(TransportUDP,
        TransportUDP,
        testing::Combine(testing::Values(TRANSPORT), testing::Values(false, true)),
        [](const testing::TestParamInfo<TransportUDP::ParamType>& info)
        {
            bool udpv4 = std::get<1>(info.param);
            std::string suffix = udpv4 ? "UDPv4" : "UDPv6";
            switch (std::get<0>(info.param))
            {
                case TRANSPORT:
                default:
                    return "Transport" + suffix;
            }

        });

