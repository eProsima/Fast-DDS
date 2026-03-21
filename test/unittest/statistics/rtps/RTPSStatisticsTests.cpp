// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <map>
#include <thread>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif // if defined(_WIN32)

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/statistics/IListeners.hpp>

#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/transport/test_UDPv4Transport.h>
#include <rtps/writer/BaseWriter.hpp>
#include <statistics/rtps/monitor-service/Interfaces.hpp>
#include <statistics/types/monitorservice_types.hpp>
#include <statistics/types/types.hpp>
#include <utils/SystemInfo.hpp>
#include <xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

class RTPSParticipantMock : public fastdds::rtps::RTPSParticipant
{

public:

    fastdds::rtps::RTPSParticipantImpl* get_impl()
    {
        return mp_impl;
    }

private:

    ~RTPSParticipantMock();
};

struct MockListener : IListener
{
    MockListener()
    {
        EXPECT_CALL(*this, on_unexpected_kind).Times(0);
    }

    void on_statistics_data(
            const Data& data) override
    {
        auto kind = data._d();
        switch (kind)
        {
            case EventKind::HISTORY2HISTORY_LATENCY:
                on_history_latency(data.writer_reader_data());
                break;
            case EventKind::RTPS_SENT:
                on_rtps_sent(data.entity2locator_traffic());
                break;
            case EventKind::RTPS_LOST:
                on_rtps_lost(data.entity2locator_traffic());
                break;
            case EventKind::NETWORK_LATENCY:
                on_network_latency(data.locator2locator_data());
                break;
            case EventKind::HEARTBEAT_COUNT:
                on_heartbeat_count(data.entity_count());
                break;
            case EventKind::ACKNACK_COUNT:
                on_acknack_count(data.entity_count());
                break;
            case EventKind::DATA_COUNT:
                on_data_count(data.entity_count());
                break;
            case EventKind::RESENT_DATAS:
                on_resent_count(data.entity_count());
                break;
            case EventKind::GAP_COUNT:
                on_gap_count(data.entity_count());
                break;
            case EventKind::NACKFRAG_COUNT:
                on_nackfrag_count(data.entity_count());
                break;
            case EventKind::DISCOVERED_ENTITY:
                on_entity_discovery(data.discovery_time());
                break;
            case EventKind::PDP_PACKETS:
                on_pdp_packets(data.entity_count());
                break;
            case EventKind::EDP_PACKETS:
                on_edp_packets(data.entity_count());
                break;
            case EventKind::SAMPLE_DATAS:
                on_sample_datas(data.sample_identity_count());
                break;
            case EventKind::PUBLICATION_THROUGHPUT:
                on_publisher_throughput(data.entity_data());
                break;
            case EventKind::SUBSCRIPTION_THROUGHPUT:
                on_subscriber_throughput(data.entity_data());
                break;
            default:
                on_unexpected_kind(kind);
                break;
        }
    }

    MOCK_METHOD1(on_history_latency, void(const eprosima::fastdds::statistics::WriterReaderData&));
    MOCK_METHOD1(on_rtps_sent, void(const eprosima::fastdds::statistics::Entity2LocatorTraffic&));
    MOCK_METHOD1(on_rtps_lost, void(const eprosima::fastdds::statistics::Entity2LocatorTraffic&));
    MOCK_METHOD1(on_network_latency, void(const eprosima::fastdds::statistics::Locator2LocatorData&));
    MOCK_METHOD1(on_heartbeat_count, void(const eprosima::fastdds::statistics::EntityCount&));
    MOCK_METHOD1(on_acknack_count, void(const eprosima::fastdds::statistics::EntityCount&));
    MOCK_METHOD1(on_data_count, void(const eprosima::fastdds::statistics::EntityCount&));
    MOCK_METHOD1(on_resent_count, void(const eprosima::fastdds::statistics::EntityCount&));
    MOCK_METHOD1(on_gap_count, void(const eprosima::fastdds::statistics::EntityCount&));
    MOCK_METHOD1(on_nackfrag_count, void(const eprosima::fastdds::statistics::EntityCount&));
    MOCK_METHOD1(on_entity_discovery, void(const eprosima::fastdds::statistics::DiscoveryTime&));
    MOCK_METHOD1(on_pdp_packets, void(const eprosima::fastdds::statistics::EntityCount&));
    MOCK_METHOD1(on_edp_packets, void(const eprosima::fastdds::statistics::EntityCount&));
    MOCK_METHOD1(on_sample_datas, void(const eprosima::fastdds::statistics::SampleIdentityCount&));
    MOCK_METHOD1(on_publisher_throughput, void(const eprosima::fastdds::statistics::EntityData&));
    MOCK_METHOD1(on_subscriber_throughput, void(const eprosima::fastdds::statistics::EntityData&));
    MOCK_METHOD1(on_unexpected_kind, void(uint32_t));
};

class RTPSStatisticsTestsImpl
{
    using test_Descriptor = fastdds::rtps::test_UDPv4TransportDescriptor;

    // transport filter, that would delegate into a custom one if provided
    // Filters have specific getters and setters methods.
    class TransportFilter
    {
        friend class RTPSStatisticsTestsImpl;

        test_Descriptor::filter external_filter_;

    public:

        TransportFilter() = default;
        TransportFilter(
                const TransportFilter&) = delete;
        TransportFilter(
                TransportFilter&&) = delete;

        bool operator ()(
                fastdds::rtps::CDRMessage_t& msg) const noexcept
        {
            try
            {
                // filter through the external functor
                return external_filter_(msg);
            }
            catch ( std::bad_function_call&)
            {
                return false; // don't filter
            }
        }

    };

    std::map<fastdds::rtps::SubmessageId, TransportFilter> filters_;

protected:

    fastdds::rtps::WriterHistory* writer_history_ = nullptr;
    fastdds::rtps::ReaderHistory* reader_history_ = nullptr;

    fastdds::rtps::RTPSParticipant* participant_ = nullptr;
    fastdds::rtps::RTPSWriter* writer_ = nullptr;
    fastdds::rtps::RTPSReader* reader_ = nullptr;
    std::shared_ptr<test_Descriptor> test_transport_descriptor_;

    // Getters and setters for the transport filter
    using filter = fastdds::rtps::test_UDPv4TransportDescriptor::filter;

    template<class F>
    void set_transport_filter(
            fastdds::rtps::SubmessageId id,
            F f) noexcept
    {
        filters_[id].external_filter_ = f;
    }

    void set_transport_filter(
            fastdds::rtps::SubmessageId id,
            std::nullptr_t) noexcept
    {
        filters_[id].external_filter_ = nullptr;
    }

    test_Descriptor::filter get_transport_filter(
            fastdds::rtps::SubmessageId id) noexcept
    {
        return filters_[id].external_filter_;
    }

public:

    void create_participant()
    {
        using namespace fastdds::rtps;

        // create the participant
        RTPSParticipantAttributes p_attr;

        // use leaky transport
        // as filter use a fixture provided functor
        test_transport_descriptor_ = std::make_shared<test_Descriptor>();

        // initialize filters
        test_transport_descriptor_->drop_data_messages_filter_  = std::ref(filters_[DATA]);
        test_transport_descriptor_->drop_heartbeat_messages_filter_ = std::ref(filters_[HEARTBEAT]);
        test_transport_descriptor_->drop_ack_nack_messages_filter_ = std::ref(filters_[ACKNACK]);
        test_transport_descriptor_->drop_gap_messages_filter_ = std::ref(filters_[GAP]);
        test_transport_descriptor_->drop_data_frag_messages_filter_ = std::ref(filters_[DATA_FRAG]);

        p_attr.useBuiltinTransports = false;
        p_attr.userTransports.push_back(test_transport_descriptor_);

        // random domain_id
#if defined(__cplusplus_winrt)
        uint32_t domain_id = static_cast<uint32_t>(GetCurrentProcessId()) % 100;
#elif defined(_WIN32)
        uint32_t domain_id = static_cast<uint32_t>(_getpid()) % 100;
#else
        uint32_t domain_id = static_cast<uint32_t>(getpid()) % 100;
#endif // if defined(__cplusplus_winrt)

        participant_ = RTPSDomain::createParticipant(
            domain_id, true, p_attr);
    }

    void remove_participant()
    {
        using namespace fastdds::rtps;

        // Remove the endpoints
        destroy_endpoints();

        // Remove the participant
        RTPSDomain::removeRTPSParticipant(participant_);
    }

    void create_reader(
            uint32_t payloadMaxSize,
            fastdds::rtps::ReliabilityKind_t reliability_qos = fastdds::rtps::ReliabilityKind_t::RELIABLE,
            fastdds::rtps::DurabilityKind_t durability_qos = fastdds::rtps::DurabilityKind_t::VOLATILE)
    {
        using namespace fastdds::rtps;

        HistoryAttributes history_attributes;
        history_attributes.payloadMaxSize = payloadMaxSize;
        reader_history_ = new ReaderHistory(history_attributes);

        ReaderAttributes r_att;
        r_att.endpoint.reliabilityKind = reliability_qos;
        r_att.endpoint.durabilityKind = durability_qos;

        // Setting localhost as the only locator ensures that DATA submessages will be sent only once.
        Locator_t local_locator;
        IPLocator::setIPv4(local_locator, 127, 0, 0, 1);
        r_att.endpoint.unicastLocatorList.clear();
        r_att.endpoint.unicastLocatorList.push_back(local_locator);

        reader_ = RTPSDomain::createRTPSReader(participant_, r_att, reader_history_);
    }

    void create_writer(
            uint32_t payloadMaxSize,
            fastdds::rtps::ReliabilityKind_t reliability_qos = fastdds::rtps::ReliabilityKind_t::RELIABLE,
            fastdds::rtps::DurabilityKind_t durability_qos = fastdds::rtps::DurabilityKind_t::TRANSIENT_LOCAL)
    {
        using namespace fastdds::rtps;

        HistoryAttributes history_attributes;
        history_attributes.payloadMaxSize = payloadMaxSize;
        writer_history_ = new WriterHistory(history_attributes);

        WriterAttributes w_att;
        w_att.times.heartbeat_period.seconds = 0;
        w_att.times.heartbeat_period.nanosec = 250 * 1000 * 1000; // reduce acknowledgement wait
        w_att.endpoint.reliabilityKind = reliability_qos;
        w_att.endpoint.durabilityKind = durability_qos;

        writer_ = RTPSDomain::createRTPSWriter(participant_, w_att, writer_history_);
    }

    void create_lazy_writer(
            uint32_t payloadMaxSize,
            fastdds::rtps::ReliabilityKind_t reliability_qos = fastdds::rtps::ReliabilityKind_t::RELIABLE,
            fastdds::rtps::DurabilityKind_t durability_qos = fastdds::rtps::DurabilityKind_t::TRANSIENT_LOCAL)
    {
        using namespace fastdds::rtps;

        HistoryAttributes history_attributes;
        history_attributes.payloadMaxSize = payloadMaxSize;
        writer_history_ = new WriterHistory(history_attributes);

        WriterAttributes w_att;
        w_att.times.heartbeat_period.seconds = 3;
        w_att.times.heartbeat_period.nanosec = 0;
        w_att.times.nack_response_delay.seconds = 0;
        w_att.times.nack_response_delay.nanosec = 300 * 1000 * 1000; // increase ACKNACK response delay
        w_att.endpoint.reliabilityKind = reliability_qos;
        w_att.endpoint.durabilityKind = durability_qos;

        writer_ = RTPSDomain::createRTPSWriter(participant_, w_att, writer_history_);
    }

    void create_endpoints(
            uint32_t payloadMaxSize,
            fastdds::rtps::ReliabilityKind_t reliability_qos = fastdds::rtps::ReliabilityKind_t::RELIABLE)
    {
        create_reader(payloadMaxSize, reliability_qos);
        create_writer(payloadMaxSize, reliability_qos);
    }

    void match_endpoints(
            bool /* key */,
            fastcdr::string_255 data_type,
            fastcdr::string_255 topic_name)
    {
        using namespace fastdds;
        using namespace fastdds::rtps;

        TopicDescription topic_desc;
        topic_desc.type_name = data_type;
        topic_desc.topic_name = topic_name;

        dds::WriterQos Wqos;
        auto& watt = writer_->getAttributes();
        Wqos.m_durability.durabilityKind(watt.durabilityKind);
        Wqos.m_reliability.kind =
                RELIABLE ==
                watt.reliabilityKind ? dds::RELIABLE_RELIABILITY_QOS : dds::BEST_EFFORT_RELIABILITY_QOS;

        dds::ReaderQos Rqos;
        auto& ratt = writer_->getAttributes();
        Rqos.m_durability.durabilityKind(ratt.durabilityKind);
        Rqos.m_reliability.kind =
                RELIABLE ==
                ratt.reliabilityKind ? dds::RELIABLE_RELIABILITY_QOS : dds::BEST_EFFORT_RELIABILITY_QOS;

        participant_->register_writer(writer_, topic_desc, Wqos);
        participant_->register_reader(reader_, topic_desc, Rqos);
    }

    void destroy_endpoints()
    {
        using namespace fastdds::rtps;

        if (nullptr != writer_ )
        {
            RTPSDomain::removeRTPSWriter(writer_);
            delete writer_history_;
            writer_ = nullptr;
            writer_history_ = nullptr;
        }

        if (nullptr != reader_)
        {
            RTPSDomain::removeRTPSReader(reader_);
            delete reader_history_;
            reader_ = nullptr;
            reader_history_ = nullptr;
        }
    }

    void write_small_sample(
            uint32_t length)
    {
        using namespace fastdds::rtps;

        ASSERT_NE(nullptr, writer_);

        auto writer_change = writer_history_->create_change(length, ALIVE);
        ASSERT_NE(nullptr, writer_change);

        std::string str("https://github.com/eProsima/Fast-DDS.git");
        uint32_t change_length = std::min(length, static_cast<uint32_t>(str.length()));
        memcpy(writer_change->serializedPayload.data, str.c_str(), change_length);
        writer_change->serializedPayload.length = change_length;

        ASSERT_TRUE(writer_history_->add_change(writer_change));
    }

    void write_large_sample(
            uint32_t length,
            uint16_t fragment_size)
    {
        using namespace fastdds::rtps;

        ASSERT_NE(nullptr, writer_);

        auto writer_change = writer_history_->create_change(length, ALIVE);
        ASSERT_NE(nullptr, writer_change);

        {
            memset(writer_change->serializedPayload.data, 'e', length);
            writer_change->serializedPayload.length = length;
            writer_change->setFragmentSize(fragment_size, true);
        }

        ASSERT_TRUE(writer_history_->add_change(writer_change));
    }

};

class RTPSStatisticsTests
    : public ::testing::Test
    , public RTPSStatisticsTestsImpl
{
public:

    // Sets up the test fixture.
    void SetUp() override
    {
        using namespace fastdds;

        // Intraprocess must be disable in order to receive DATA callbacks
        LibrarySettings att;
        att.intraprocess_delivery = INTRAPROCESS_OFF;
        fastdds::rtps::RTPSDomain::set_library_settings(att);

        create_participant();
    }

    // Tears down the test fixture.
    void TearDown() override
    {
        remove_participant();
    }

};

/*
 * This test checks RTPSParticipant, RTPSWriter and RTPSReader statistics module related APIs.
 * Creates dummy listener objects and associates them to RTPS entities of each kind covering
 * the different possible cases: already registered, non-registered, already unregistered.
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_listener_management)
{
    // Check API add and remove interfaces
    using namespace std;
    using namespace fastdds::rtps;

    // Create the testing endpoints
    create_endpoints(255);

    auto listener1 = make_shared<MockListener>();
    auto listener2 = make_shared<MockListener>();
    auto nolistener = listener1;
    nolistener.reset();

    uint32_t kind = EventKind::PUBLICATION_THROUGHPUT;
    uint32_t another_kind = EventKind::SUBSCRIPTION_THROUGHPUT;
    uint32_t yet_another_kind = EventKind::NETWORK_LATENCY;

    // test the participant apis
    // + fails to remove an empty listener
    EXPECT_FALSE(participant_->remove_statistics_listener(nolistener, kind));
    // + fails if no listener has been yet added
    EXPECT_FALSE(participant_->remove_statistics_listener(listener1, kind));
    // + fails to add an empty listener
    EXPECT_FALSE(participant_->add_statistics_listener(nolistener, kind));
    // + succeeds to add a new listener
    ASSERT_TRUE(participant_->add_statistics_listener(listener1, kind));
    // + fails to add multiple times the same listener...
    EXPECT_FALSE(participant_->add_statistics_listener(listener1, kind));
    //   ... unless it's associated to other entity
    EXPECT_TRUE(participant_->add_statistics_listener(listener1, another_kind));
    // + fails if an unknown listener is removed
    EXPECT_FALSE(participant_->remove_statistics_listener(listener2, kind));
    // + fails if a known listener is removed with a non registered entity
    EXPECT_FALSE(participant_->remove_statistics_listener(listener1, yet_another_kind));
    // + succeeds to remove a known listener
    EXPECT_TRUE(participant_->remove_statistics_listener(listener1, kind));
    EXPECT_TRUE(participant_->remove_statistics_listener(listener1, another_kind));
    // + fails if a listener is already removed
    EXPECT_FALSE(participant_->remove_statistics_listener(listener1, kind));
    // + The EventKind is an actual mask that allow register multiple entities simultaneously
    EXPECT_TRUE(participant_->add_statistics_listener(listener1, kind | another_kind));
    // + When using a mask of multiple entities the return value succeeds only if all
    //   entity driven operations succeeds. The following operation must fail because one
    //   of the entities has not that registered listener
    EXPECT_FALSE(participant_->remove_statistics_listener(listener1,
            kind | another_kind | yet_another_kind));

    // test the writer apis
    // + fails to remove an empty listener
    EXPECT_FALSE(writer_->remove_statistics_listener(nolistener));
    // + fails if no listener has been yet added
    EXPECT_FALSE(writer_->remove_statistics_listener(listener1));
    // + fails to add an empty listener
    EXPECT_FALSE(writer_->add_statistics_listener(nolistener));
    // + succeeds to add a new listener
    ASSERT_TRUE(writer_->add_statistics_listener(listener1));
    // + fails to add multiple times the same listener
    EXPECT_FALSE(writer_->add_statistics_listener(listener1));
    // + fails if an unknown listener is removed
    EXPECT_FALSE(writer_->remove_statistics_listener(listener2));
    // + succeeds to remove a known listener
    EXPECT_TRUE(writer_->remove_statistics_listener(listener1));
    // + fails if a listener is already removed
    EXPECT_FALSE(writer_->remove_statistics_listener(listener1));

    // test the reader apis
    // + fails to remove an empty listener
    EXPECT_FALSE(reader_->remove_statistics_listener(nolistener));
    // + fails if no listener has been yet added
    EXPECT_FALSE(reader_->remove_statistics_listener(listener1));
    // + fails to add an empty listener
    EXPECT_FALSE(reader_->add_statistics_listener(nolistener));
    // + succeeds to add a new listener
    ASSERT_TRUE(reader_->add_statistics_listener(listener1));
    // + fails to add multiple times the same listener
    EXPECT_FALSE(reader_->add_statistics_listener(listener1));
    // + fails if an unknown listener is removed
    EXPECT_FALSE(reader_->remove_statistics_listener(listener2));
    // + succeeds to remove a known listener
    EXPECT_TRUE(reader_->remove_statistics_listener(listener1));
    // + fails if a listener is already removed
    EXPECT_FALSE(reader_->remove_statistics_listener(listener1));
}

/*
 * This test checks RTPSParticipant, RTPSWriter and RTPSReader statistics module related APIs.
 * - RTPS_SENT callbacks are performed
 * - RTPS_LOST callbacks are performed
 * - NETWORK_LATENCY callbacks are performed
 * - HISTORY2HISTORY_LATENCY callbacks are performed
 * - DATA_COUNT callbacks are performed for DATA submessages
 * - RESENT_DATAS callbacks are performed for DATA submessages demanded by the readers
 * - ACKNACK_COUNT callbacks are performed
 * - HEARBEAT_COUNT callbacks are performed
 * - SAMPLE_DATAS callbacks are performed
 * - PUBLICATION_THROUGHPUT callbacks are performed
 * - SUBSCRIPTION_THROUGHPUT callbacks are performed
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_listener_callbacks)
{
    using namespace ::testing;
    using namespace fastdds;
    using namespace fastdds::rtps;
    using namespace std;

    std::atomic<unsigned int> samples_filtered{0};

    // make sure some messages are lost to assure the RESENT_DATAS callback
    set_transport_filter(
        DATA,
        [&samples_filtered](fastdds::rtps::CDRMessage_t& msg)-> bool
        {
            uint32_t old_pos = msg.pos;

            // see RTPS DDS 9.4.5.3 Data Submessage
            EntityId_t readerID, writerID;
            SequenceNumber_t sn;

            msg.pos += 2; // flags
            msg.pos += 2; // octets to inline quos
            CDRMessage::readEntityId(&msg, &readerID);
            CDRMessage::readEntityId(&msg, &writerID);
            CDRMessage::readSequenceNumber(&msg, &sn);

            // restore buffer pos
            msg.pos = old_pos;

            // generate losses
            if ( samples_filtered < 10 // only a few times (mind the interfaces)
            && (writerID.value[3] & 0xC0) == 0      // only user endpoints
            && (sn == SequenceNumber_t{0, 1}))     // only first sample
            {
                ++samples_filtered;
                return true;
            }

            return false;
        });

    uint16_t length = 255;
    auto test_execution = [&]()
            {
                // participant specific callbacks
                auto participant_listener = make_shared<MockListener>();
                ASSERT_TRUE(participant_->add_statistics_listener(participant_listener,
                        EventKind::RTPS_SENT | EventKind::NETWORK_LATENCY | EventKind::RTPS_LOST));

                // writer callbacks through participant listener
                auto participant_writer_listener = make_shared<MockListener>();
                ASSERT_TRUE(participant_->add_statistics_listener(participant_writer_listener,
                        EventKind::DATA_COUNT | EventKind::RESENT_DATAS |
                        EventKind::PUBLICATION_THROUGHPUT | EventKind::SAMPLE_DATAS));

                // writer specific callbacks
                auto writer_listener = make_shared<MockListener>();
                ASSERT_TRUE(writer_->add_statistics_listener(writer_listener));

                // reader callbacks through participant listener
                auto participant_reader_listener = make_shared<MockListener>();
                ASSERT_TRUE(participant_->add_statistics_listener(participant_reader_listener,
                        EventKind::ACKNACK_COUNT | EventKind::HISTORY2HISTORY_LATENCY |
                        EventKind::SUBSCRIPTION_THROUGHPUT));

                // reader specific callbacks
                auto reader_listener = make_shared<MockListener>();
                ASSERT_TRUE(reader_->add_statistics_listener(reader_listener));

                // we must received the RTPS_SENT, RTPS_LOST and NETWORK_LATENCY notifications
                EXPECT_CALL(*participant_listener, on_rtps_sent)
                        .Times(AtLeast(1));
                EXPECT_CALL(*participant_listener, on_rtps_lost)
                        .Times(AtLeast(1));
                EXPECT_CALL(*participant_listener, on_network_latency)
                        .Times(AtLeast(1));

                // Check callbacks on data exchange, at least, we must received:
                // + RTPSWriter: PUBLICATION_THROUGHPUT, RESENT_DATAS,
                //               GAP_COUNT, DATA_COUNT, SAMPLE_DATAS & PHYSICAL_DATA
                //   optionally: NACKFRAG_COUNT
                EXPECT_CALL(*writer_listener, on_heartbeat_count)
                        .Times(AtLeast(1));
                EXPECT_CALL(*writer_listener, on_data_count)
                        .Times(AtLeast(1));
                EXPECT_CALL(*writer_listener, on_resent_count)
                        .Times(AtLeast(1));
                EXPECT_CALL(*writer_listener, on_sample_datas)
                        .Times(AtLeast(1));
                EXPECT_CALL(*writer_listener, on_publisher_throughput)
                        .Times(AtLeast(1));

                EXPECT_CALL(*participant_writer_listener, on_data_count)
                        .Times(AtLeast(1));
                EXPECT_CALL(*participant_writer_listener, on_resent_count)
                        .Times(AtLeast(1));
                EXPECT_CALL(*participant_writer_listener, on_sample_datas)
                        .Times(AtLeast(1));
                EXPECT_CALL(*participant_writer_listener, on_publisher_throughput)
                        .Times(AtLeast(1));

                // + RTPSReader: SUBSCRIPTION_THROUGHPUT,
                //               SAMPLE_DATAS & PHYSICAL_DATA
                //   optionally: ACKNACK_COUNT
                EXPECT_CALL(*reader_listener, on_acknack_count)
                        .Times(AtLeast(1));
                EXPECT_CALL(*reader_listener, on_history_latency)
                        .Times(AtLeast(1));
                EXPECT_CALL(*reader_listener, on_subscriber_throughput)
                        .Times(AtLeast(1));

                EXPECT_CALL(*participant_reader_listener, on_acknack_count)
                        .Times(AtLeast(1));
                EXPECT_CALL(*participant_reader_listener, on_history_latency)
                        .Times(AtLeast(1));
                EXPECT_CALL(*participant_reader_listener, on_subscriber_throughput)
                        .Times(AtLeast(1));

                // match writer and reader on a dummy topic
                match_endpoints(false, "string", "statisticsSmallTopic");

                // exchange data
                write_small_sample(length);

                // wait for reception
                EXPECT_TRUE(reader_->wait_for_unread_cache(dds::Duration_t(5, 0)));

                // receive the sample
                CacheChange_t* reader_change = reader_->next_untaken_cache();
                ASSERT_NE(nullptr, reader_change);

                // wait for acknowledgement
                EXPECT_TRUE(writer_->wait_for_all_acked(dds::Duration_t(5, 0)));

                EXPECT_TRUE(writer_->remove_statistics_listener(writer_listener));
                EXPECT_TRUE(reader_->remove_statistics_listener(reader_listener));

                EXPECT_TRUE(participant_->remove_statistics_listener(participant_listener,
                        EventKind::RTPS_SENT | EventKind::NETWORK_LATENCY | EventKind::RTPS_LOST));
                EXPECT_TRUE(participant_->remove_statistics_listener(participant_writer_listener,
                        EventKind::DATA_COUNT | EventKind::RESENT_DATAS |
                        EventKind::PUBLICATION_THROUGHPUT | EventKind::SAMPLE_DATAS));
                EXPECT_TRUE(participant_->remove_statistics_listener(participant_reader_listener,
                        EventKind::ACKNACK_COUNT | EventKind::HISTORY2HISTORY_LATENCY |
                        EventKind::SUBSCRIPTION_THROUGHPUT));
            };

    // Check that setting the mask after creating the endpoints work
    uint32_t enable_writers_mask =
            EventKind::HISTORY2HISTORY_LATENCY |
            EventKind::NETWORK_LATENCY |
            EventKind::PUBLICATION_THROUGHPUT |
            EventKind::SUBSCRIPTION_THROUGHPUT |
            EventKind::RTPS_SENT |
            EventKind::RTPS_LOST |
            EventKind::RESENT_DATAS |
            EventKind::HEARTBEAT_COUNT |
            EventKind::ACKNACK_COUNT |
            EventKind::DATA_COUNT |
            EventKind::SAMPLE_DATAS;
    create_endpoints(length, RELIABLE);
    participant_->set_enabled_statistics_writers_mask(enable_writers_mask);
    test_execution();

    // Check that creating the endpoints after setting the mask also works
    destroy_endpoints();
    create_endpoints(length, RELIABLE);
    samples_filtered = 0;
    test_execution();
}

/*
 * This test checks RTPSParticipant, RTPSWriter and RTPSReader statistics module related APIs.
 * - participant listeners management with late joiners
 * - HISTORY2HISTORY_LATENCY callbacks are performed
 * - DATA_COUNT callbacks with DATA_FRAGS are performed
 * - NACK_FRAG callbacks assessment
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_listener_callbacks_fragmented)
{
    using namespace ::testing;
    using namespace fastdds;
    using namespace fastdds::rtps;
    using namespace std;

    // payload size
    uint32_t length = 1048576;
    uint16_t fragment_size = 64000; // should fit in transport message size

    // make sure some messages are lost to assure the NACKFRAG callback
    set_transport_filter(
        DATA_FRAG,
        [](fastdds::rtps::CDRMessage_t& msg)-> bool
        {
            static uint32_t max_fragment = 0;
            static bool keep_filtering = true;

            uint32_t fragmentNum = 0;
            uint32_t old_pos = msg.pos;
            msg.pos += 20;
            fastdds::rtps::CDRMessage::readUInt32(&msg, &fragmentNum);
            msg.pos = old_pos;

            // generate losses only on the first burst
            if ( keep_filtering )
            {
                keep_filtering = max_fragment <= fragmentNum;
                max_fragment = fragmentNum;
                return fragmentNum % 2 == 0;
            }

            return false;
        });

    uint32_t enable_writers_mask =
            EventKind::HISTORY2HISTORY_LATENCY |
            EventKind::HEARTBEAT_COUNT |
            EventKind::ACKNACK_COUNT |
            EventKind::NACKFRAG_COUNT |
            EventKind::DATA_COUNT;

    // writer callbacks through participant listener
    auto participant_listener = make_shared<MockListener>();
    uint32_t mask = EventKind::DATA_COUNT | EventKind::HEARTBEAT_COUNT
            | EventKind::ACKNACK_COUNT | EventKind::NACKFRAG_COUNT | EventKind::HISTORY2HISTORY_LATENCY;
    ASSERT_TRUE(participant_->add_statistics_listener(participant_listener, mask));
    participant_->set_enabled_statistics_writers_mask(enable_writers_mask);

    EXPECT_CALL(*participant_listener, on_data_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*participant_listener, on_heartbeat_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*participant_listener, on_acknack_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*participant_listener, on_history_latency)
            .Times(AtLeast(1));
    EXPECT_CALL(*participant_listener, on_nackfrag_count)
            .Times(AtLeast(1));

    // Create the testing endpoints
    create_endpoints(length, RELIABLE);
    writer_->set_enabled_statistics_writers_mask(enable_writers_mask);
    reader_->set_enabled_statistics_writers_mask(enable_writers_mask);

    // match writer and reader on a dummy topic
    match_endpoints(false, "chunk", "statisticsLargeTopic");

    // exchange data
    write_large_sample(length, fragment_size);

    // wait for reception
    EXPECT_TRUE(reader_->wait_for_unread_cache(dds::Duration_t(10, 0)));

    // receive the sample
    CacheChange_t* reader_change = reader_->next_untaken_cache();
    ASSERT_NE(nullptr, reader_change);

    // wait for acknowledgement
    EXPECT_TRUE(writer_->wait_for_all_acked(dds::Duration_t(1, 0)));

    EXPECT_TRUE(participant_->remove_statistics_listener(participant_listener, mask));
}

/*
 * This test checks the behaviour of RTPSParticipant, RTPSWriter and RTPSReader statistics module
 * related APIs when their enabled statistics writers mask is set to 0.
 * - RTPS_SENT callbacks are not performed
 * - RTPS_LOST callbacks are not performed
 * - NETWORK_LATENCY callbacks are not performed
 * - HISTORY2HISTORY_LATENCY callbacks are not performed
 * - DATA_COUNT callbacks are not performed for DATA submessages
 * - RESENT_DATAS callbacks are not performed for DATA submessages demanded by the readers
 * - ACKNACK_COUNT callbacks are not performed
 * - HEARBEAT_COUNT callbacks are not performed
 * - SAMPLE_DATAS callbacks are not performed
 * - PUBLICATION_THROUGHPUT callbacks are not performed
 * - SUBSCRIPTION_THROUGHPUT callbacks are not performed
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_listener_callbacks_no_enabled_writers)
{
    using namespace ::testing;
    using namespace fastdds;
    using namespace fastdds::rtps;
    using namespace std;

    // make sure some messages are lost to assure the RESENT_DATAS callback
    set_transport_filter(
        DATA,
        [](fastdds::rtps::CDRMessage_t& msg)-> bool
        {
            static unsigned int samples_filtered = 0;
            uint32_t old_pos = msg.pos;

            // see RTPS DDS 9.4.5.3 Data Submessage
            EntityId_t readerID, writerID;
            SequenceNumber_t sn;

            msg.pos += 2; // flags
            msg.pos += 2; // octets to inline quos
            CDRMessage::readEntityId(&msg, &readerID);
            CDRMessage::readEntityId(&msg, &writerID);
            CDRMessage::readSequenceNumber(&msg, &sn);

            // restore buffer pos
            msg.pos = old_pos;

            // generate losses
            if ( samples_filtered < 10 // only a few times (mind the interfaces)
            && (writerID.value[3] & 0xC0) == 0      // only user endpoints
            && (sn == SequenceNumber_t{0, 1}))     // only first sample
            {
                ++samples_filtered;
                return true;
            }

            return false;
        });

    // create the testing endpoints
    uint16_t length = 255;
    create_endpoints(length, RELIABLE);

    // participant specific callbacks
    auto participant_listener = make_shared<MockListener>();
    ASSERT_TRUE(participant_->add_statistics_listener(participant_listener,
            EventKind::RTPS_SENT | EventKind::NETWORK_LATENCY | EventKind::RTPS_LOST));

    // writer callbacks through participant listener
    auto participant_writer_listener = make_shared<MockListener>();
    ASSERT_TRUE(participant_->add_statistics_listener(participant_writer_listener,
            EventKind::DATA_COUNT | EventKind::RESENT_DATAS |
            EventKind::PUBLICATION_THROUGHPUT | EventKind::SAMPLE_DATAS));

    // writer specific callbacks
    auto writer_listener = make_shared<MockListener>();
    ASSERT_TRUE(writer_->add_statistics_listener(writer_listener));

    // reader callbacks through participant listener
    auto participant_reader_listener = make_shared<MockListener>();
    ASSERT_TRUE(participant_->add_statistics_listener(participant_reader_listener,
            EventKind::ACKNACK_COUNT | EventKind::HISTORY2HISTORY_LATENCY |
            EventKind::SUBSCRIPTION_THROUGHPUT));

    // reader specific callbacks
    auto reader_listener = make_shared<MockListener>();
    ASSERT_TRUE(reader_->add_statistics_listener(reader_listener));

    // we must not receive the RTPS_SENT, RTPS_LOST and NETWORK_LATENCY notifications
    EXPECT_CALL(*participant_listener, on_rtps_sent)
            .Times(0);
    EXPECT_CALL(*participant_listener, on_rtps_lost)
            .Times(0);
    EXPECT_CALL(*participant_listener, on_network_latency)
            .Times(0);

    // Check callbacks on data exchange; we must not receive:
    // + RTPSWriter: PUBLICATION_THROUGHPUT, RESENT_DATAS,
    //               DATA_COUNT, SAMPLE_DATAS & PHYSICAL_DATA
    //   neither: NACKFRAG_COUNT
    EXPECT_CALL(*writer_listener, on_heartbeat_count)
            .Times(0);
    EXPECT_CALL(*writer_listener, on_data_count)
            .Times(0);
    EXPECT_CALL(*writer_listener, on_resent_count)
            .Times(0);
    EXPECT_CALL(*writer_listener, on_sample_datas)
            .Times(0);
    EXPECT_CALL(*writer_listener, on_publisher_throughput)
            .Times(0);

    EXPECT_CALL(*participant_writer_listener, on_data_count)
            .Times(0);
    EXPECT_CALL(*participant_writer_listener, on_resent_count)
            .Times(0);
    EXPECT_CALL(*participant_writer_listener, on_sample_datas)
            .Times(0);
    EXPECT_CALL(*participant_writer_listener, on_publisher_throughput)
            .Times(0);

    // + RTPSReader: SUBSCRIPTION_THROUGHPUT,
    //               SAMPLE_DATAS & PHYSICAL_DATA
    //   neither: ACKNACK_COUNT
    EXPECT_CALL(*reader_listener, on_acknack_count)
            .Times(0);
    EXPECT_CALL(*reader_listener, on_history_latency)
            .Times(0);
    EXPECT_CALL(*reader_listener, on_subscriber_throughput)
            .Times(0);

    EXPECT_CALL(*participant_reader_listener, on_acknack_count)
            .Times(0);
    EXPECT_CALL(*participant_reader_listener, on_history_latency)
            .Times(0);
    EXPECT_CALL(*participant_reader_listener, on_subscriber_throughput)
            .Times(0);

    // match writer and reader on a dummy topic
    match_endpoints(false, "string", "statisticsSmallTopic");

    // exchange data
    write_small_sample(length);

    // wait for reception
    EXPECT_TRUE(reader_->wait_for_unread_cache(dds::Duration_t(5, 0)));

    // receive the sample
    CacheChange_t* reader_change = reader_->next_untaken_cache();
    ASSERT_NE(nullptr, reader_change);

    // wait for acknowledgement
    EXPECT_TRUE(writer_->wait_for_all_acked(dds::Duration_t(5, 0)));

    EXPECT_TRUE(writer_->remove_statistics_listener(writer_listener));
    EXPECT_TRUE(reader_->remove_statistics_listener(reader_listener));

    EXPECT_TRUE(participant_->remove_statistics_listener(participant_listener,
            EventKind::RTPS_SENT | EventKind::NETWORK_LATENCY | EventKind::RTPS_LOST));
    EXPECT_TRUE(participant_->remove_statistics_listener(participant_writer_listener,
            EventKind::DATA_COUNT | EventKind::RESENT_DATAS |
            EventKind::PUBLICATION_THROUGHPUT | EventKind::SAMPLE_DATAS));
    EXPECT_TRUE(participant_->remove_statistics_listener(participant_reader_listener,
            EventKind::ACKNACK_COUNT | EventKind::HISTORY2HISTORY_LATENCY |
            EventKind::SUBSCRIPTION_THROUGHPUT));
}

/*
 * This test checks RTPSWriter GAP_COUNT statistics callback
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_listener_gap_callback)
{
    using namespace ::testing;
    using namespace fastdds;
    using namespace fastdds::rtps;
    using namespace std;

    uint32_t enable_writers_mask =
            EventKind::PUBLICATION_THROUGHPUT |
            EventKind::RESENT_DATAS |
            EventKind::HEARTBEAT_COUNT |
            EventKind::ACKNACK_COUNT |
            EventKind::NACKFRAG_COUNT |
            EventKind::GAP_COUNT |
            EventKind::DATA_COUNT |
            EventKind::SAMPLE_DATAS;

    // create the listeners and set expectations
    auto participant_writer_listener = make_shared<MockListener>();
    auto writer_listener = make_shared<MockListener>();

    // check callbacks on data exchange
    EXPECT_CALL(*writer_listener, on_gap_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*writer_listener, on_heartbeat_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*writer_listener, on_data_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*writer_listener, on_resent_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*writer_listener, on_sample_datas)
            .Times(AtLeast(1));
    EXPECT_CALL(*writer_listener, on_publisher_throughput)
            .Times(AtLeast(1));

    EXPECT_CALL(*participant_writer_listener, on_gap_count)
            .Times(AtLeast(1));

    // create the writer, reader is a late joiner
    uint16_t length = 255;
    create_writer(length, RELIABLE, TRANSIENT_LOCAL);
    writer_->set_enabled_statistics_writers_mask(enable_writers_mask);

    // writer callback through participant listener
    ASSERT_TRUE(participant_->add_statistics_listener(participant_writer_listener, EventKind::GAP_COUNT));

    // writer specific callbacks
    ASSERT_TRUE(writer_->add_statistics_listener(writer_listener));

    // add a sample to the writer history that will be delivered
    write_small_sample(length);
    // add a second sample and remove it to generate the gap
    write_small_sample(length);
    ASSERT_TRUE(writer_history_->remove_change(SequenceNumber_t{0, 2}));
    // add a third sample in order to force the gap
    write_small_sample(length);

    // create the late joiner as VOLATILE
    create_reader(length, RELIABLE, VOLATILE);

    // match writer and reader on a dummy topic
    match_endpoints(false, "string", "statisticsSmallTopic");

    // wait for reception
    EXPECT_TRUE(reader_->wait_for_unread_cache(dds::Duration_t(5, 0)));

    // receive the second sample
    CacheChange_t* reader_change = reader_->next_untaken_cache();
    ASSERT_NE(nullptr, reader_change);

    // wait for acknowledgement
    EXPECT_TRUE(writer_->wait_for_all_acked(dds::Duration_t(1, 0)));

    // release the listeners
    EXPECT_TRUE(writer_->remove_statistics_listener(writer_listener));
    EXPECT_TRUE(participant_->remove_statistics_listener(participant_writer_listener, EventKind::GAP_COUNT));
}

/*
 * This test checks the participant discovery callbacks
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_listener_discovery_callbacks)
{
    using namespace ::testing;
    using namespace fastdds;
    using namespace fastdds::rtps;
    using namespace std;

    uint32_t enable_writers_mask =
            EventKind::PDP_PACKETS |
            EventKind::EDP_PACKETS |
            EventKind::DISCOVERED_ENTITY;

    // create the listener and set expectations
    auto participant_listener = make_shared<MockListener>();
    ASSERT_TRUE(participant_->add_statistics_listener(participant_listener,
            EventKind::DISCOVERED_ENTITY | EventKind::PDP_PACKETS | EventKind::EDP_PACKETS));
    participant_->set_enabled_statistics_writers_mask(enable_writers_mask);

    // check callbacks on data exchange
    atomic_int callbacks(0);
    ON_CALL(*participant_listener, on_entity_discovery)
            .WillByDefault([&callbacks](const eprosima::fastdds::statistics::DiscoveryTime&)
            {
                ++callbacks;
            });
    EXPECT_CALL(*participant_listener, on_entity_discovery)
            .Times(AtLeast(5));
    EXPECT_CALL(*participant_listener, on_pdp_packets)
            .Times(AtLeast(1));
    EXPECT_CALL(*participant_listener, on_edp_packets)
            .Times(AtLeast(1));

    // create local endpoints
    uint16_t length = 255;
    create_endpoints(length);

    // register local endpoints
    match_endpoints(false, "string", "statisticsSmallTopic");

    // create remote endpoints and register them
    {
        RTPSStatisticsTestsImpl remote;
        remote.create_participant();
        remote.create_endpoints(length);
        remote.match_endpoints(false, "string", "statisticsSmallTopic");

        int loop = 0;
        while ( callbacks < 5 )
        {
            this_thread::sleep_for(chrono::milliseconds(100));
            if ( ++loop > 30 )
            {
                break;
            }
        }

        remote.remove_participant();
    }

    // release the listener
    EXPECT_TRUE(participant_->remove_statistics_listener(participant_listener, EventKind::DISCOVERED_ENTITY));
}

/*
 * This test checks improves coverity by asserting that every RESENT_DATAS callback is associated with a message
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_avoid_empty_resent_callbacks)
{
    using namespace ::testing;
    using namespace fastdds;
    using namespace fastdds::rtps;
    using namespace std;

    // The history must be cleared after the acknack is received
    // but before the response is processed
    atomic_bool acknack_sent(false);

    // filter out all user DATAs
    set_transport_filter(
        DATA,
        [](fastdds::rtps::CDRMessage_t& msg)-> bool
        {
            uint32_t old_pos = msg.pos;

            // see RTPS DDS 9.4.5.3 Data Submessage
            EntityId_t readerID, writerID;
            SequenceNumber_t sn;

            msg.pos += 2; // flags
            msg.pos += 2; // octets to inline quos
            CDRMessage::readEntityId(&msg, &readerID);
            CDRMessage::readEntityId(&msg, &writerID);
            CDRMessage::readSequenceNumber(&msg, &sn);

            // restore buffer pos
            msg.pos = old_pos;

            // filter user traffic
            return ((writerID.value[3] & 0xC0) == 0);
        });

    set_transport_filter(
        ACKNACK,
        [&acknack_sent](fastdds::rtps::CDRMessage_t& msg)-> bool
        {
            uint32_t old_pos = msg.pos;

            // see RTPS DDS 9.4.5.2 Acknack Submessage
            EntityId_t readerID, writerID;
            SequenceNumberSet_t snSet;

            CDRMessage::readEntityId(&msg, &readerID);
            CDRMessage::readEntityId(&msg, &writerID);
            snSet = CDRMessage::readSequenceNumberSet(&msg);

            // restore buffer pos
            msg.pos = old_pos;

            // The acknack has been sent
            acknack_sent = !snSet.empty();

            // we are not filtering
            return false;
        });

    // create the listeners and set expectations
    auto writer_listener = make_shared<MockListener>();

    // check callbacks on data exchange
    EXPECT_CALL(*writer_listener, on_heartbeat_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*writer_listener, on_data_count)
            .Times(AtLeast(1));
    EXPECT_CALL(*writer_listener, on_sample_datas)
            .Times(AtLeast(1));
    EXPECT_CALL(*writer_listener, on_publisher_throughput)
            .Times(AtLeast(1));
    EXPECT_CALL(*writer_listener, on_resent_count)
            .Times(0); // never called

    // create the writer, reader is a late joiner
    uint16_t length = 255;
    create_lazy_writer(length, RELIABLE, TRANSIENT_LOCAL);
    uint32_t enable_writers_mask =
            EventKind::HEARTBEAT_COUNT |
            EventKind::DATA_COUNT |
            EventKind::SAMPLE_DATAS |
            EventKind::PUBLICATION_THROUGHPUT |
            EventKind::RESENT_DATAS;
    writer_->set_enabled_statistics_writers_mask(enable_writers_mask);

    // writer specific callbacks
    ASSERT_TRUE(writer_->add_statistics_listener(writer_listener));

    // create the late joiner as VOLATILE
    create_reader(length, RELIABLE, VOLATILE);

    // match writer and reader on a dummy topic
    match_endpoints(false, "string", "statisticsSmallTopic");

    // add a sample to the writer history that cannot be delivered
    // due to the filter
    write_small_sample(length);

    // wait for the acknack to be sent before clearing the history
    while (!acknack_sent)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    // remove the sample from the writer
    ASSERT_TRUE(writer_history_->remove_change(SequenceNumber_t{ 0, 1 }));

    // give room to process the ACKNACK
    this_thread::sleep_for(chrono::milliseconds(400));

    // release the listeners
    EXPECT_TRUE(writer_->remove_statistics_listener(writer_listener));
}

/*
 * This test checks additional cases for RTPS_LOST when RTPS datagrams are received unordered
 */
TEST_F(RTPSStatisticsTests, statistics_rpts_unordered_datagrams)
{
    using namespace ::testing;
    using namespace fastdds;
    using namespace fastdds::rtps;
    using namespace std;

    constexpr uint16_t num_messages = 10;
    constexpr std::array<size_t, num_messages> message_order {
        2, 5, 1, 3, 4, 7, 8, 6, 9, 0
    };
    uint64_t lost_count_notified[] =
    {
        // seq received    description     notify
        // 2               first sequence  NO
        // 5               3 & 4 lost      2
        2,
        // 1               before first    NO
        // 3               3 recovered     1
        1,
        // 4               4 recovered     0
        0,
        // 7               6 lost          1
        1,
        // 8               in sequence     NO
        // 6               6 recovered     0
        0
        // 9               in sequence     NO
        // 0               before first    NO
    };

    // A filter to add the first `num_messages` user DATA_FRAG into `user_data`
    test_transport_descriptor_->test_transport_options->test_UDPv4Transport_DropLogLength = num_messages;
    set_transport_filter(
        DATA_FRAG,
        [](fastdds::rtps::CDRMessage_t& msg)-> bool
        {
            uint32_t old_pos = msg.pos;

            // see RTPS DDS 9.4.5.3 Data Submessage
            EntityId_t readerID, writerID;

            msg.pos += 2; // flags
            msg.pos += 2; // octets to inline quos
            CDRMessage::readEntityId(&msg, &readerID);
            CDRMessage::readEntityId(&msg, &writerID);

            // restore buffer pos
            msg.pos = old_pos;

            // Let non-user traffic pass
            return (writerID.value[3] & 0xC0) == 0;
        });

    uint16_t length = 100;
    create_endpoints(num_messages * length, BEST_EFFORT);
    match_endpoints(false, "string", "statisticsSmallTopic");

    // This will send `num_messages` DATA_FRAG nessages, which will be backed up on `user_data`
    for (uint16_t i = 0; i < num_messages; ++i)
    {
        write_large_sample(num_messages * length, length);
    }

    // create the listener and set expectations
    auto participant_listener = make_shared<MockListener>();
    ASSERT_TRUE(participant_->add_statistics_listener(participant_listener, EventKind::RTPS_LOST));
    participant_->set_enabled_statistics_writers_mask(EventKind::RTPS_LOST);

    std::vector<Entity2LocatorTraffic> lost_callback_data;
    auto callback_action = [&lost_callback_data](const Entity2LocatorTraffic& data) -> void
            {
                const Locator_t& loc = *(reinterpret_cast<const Locator_t*>(&data.dst_locator()));
                if (IPLocator::isLocal(loc))
                {
                    std::cout << "RTPS_LOST " << data.packet_count() << std::endl;
                    lost_callback_data.push_back(data);
                }
            };
    EXPECT_CALL(*participant_listener, on_rtps_lost).Times(AtLeast(1)).WillRepeatedly(callback_action);

    // Calculate destination where datagrams should be sent to
    const Locator_t& locator = *(reader_->getAttributes().unicastLocatorList.begin());
    auto locator_ip = IPLocator::getIPv4(locator);
    auto locator_port = IPLocator::getPhysicalPort(locator);
    asio::ip::address_v4::bytes_type asio_ip{ { locator_ip[0], locator_ip[1], locator_ip[2], locator_ip[3] } };
    asio::ip::udp::endpoint destination(asio::ip::address_v4(asio_ip), locator_port);

    // Prepare sending socket
    asio::error_code ec;
    asio::io_context ctx;
    asio::ip::udp::socket sender(ctx);
    sender.open(asio::ip::udp::v4());

    // Send messages in different order
    for (size_t idx : message_order)
    {
        const std::vector<octet>& msg =
                test_transport_descriptor_->test_transport_options->test_UDPv4Transport_DropLog[idx];
        EXPECT_EQ(msg.size(), sender.send_to(asio::buffer(msg.data(), msg.size()), destination, 0, ec)) << ec;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // release the listener
    EXPECT_TRUE(participant_->remove_statistics_listener(participant_listener, EventKind::RTPS_LOST));

    // Check reported callbacks
    EXPECT_EQ(sizeof(lost_count_notified) / sizeof(lost_count_notified[0]), lost_callback_data.size());
    for (size_t i = 0; i < lost_callback_data.size(); ++i)
    {
        EXPECT_EQ(lost_count_notified[i], lost_callback_data[i].packet_count());
    }

    // Last reported callback should be 0, as all packets have been sent
    const Entity2LocatorTraffic& last_lost_data = lost_callback_data.back();
    EXPECT_EQ(0u, last_lost_data.packet_count());
    EXPECT_EQ(0u, last_lost_data.byte_count());
    EXPECT_EQ(0, last_lost_data.byte_magnitude_order());
}

TEST_F(RTPSStatisticsTests, iconnections_queryable_get_entity_connections)
{
    ConnectionList conns_reader, conns_writer;
    create_endpoints(1024);

    // match writer and reader on a dummy topic
    match_endpoints(false, "string", "test_topic_name");

    auto participant_mock = static_cast<RTPSParticipantMock*>(participant_);
    auto part_impl = participant_mock->get_impl();

    part_impl->get_entity_connections(reader_->getGuid(), conns_reader);
    part_impl->get_entity_connections(writer_->getGuid(), conns_writer);

    ASSERT_EQ(1, conns_writer.size());
    ASSERT_EQ(1, conns_writer.size());
    ASSERT_EQ(conns_reader[0].guid(), statistics::to_statistics_type(writer_->getGuid()));
    ASSERT_EQ(conns_writer[0].guid(), statistics::to_statistics_type(reader_->getGuid()));

    for (auto& locator : conns_reader[0].announced_locators())
    {
        bool found = false;
        auto base_writer = static_cast<fastdds::rtps::BaseWriter*>(writer_);
        for (auto& writer_loc : base_writer->get_general_locator_selector().locator_selector)
        {
            //! Checking the address can be confusing since the writer_loc could be translated to
            //! 127.0.0.1
            if (statistics::to_statistics_type(writer_loc).port() == locator.port() &&
                    statistics::to_statistics_type(writer_loc).kind() == locator.kind())
            {
                found = true;
            }
        }

        ASSERT_TRUE(found);
    }
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    eprosima::fastdds::dds::Log::Flush();
    return ret;
}
