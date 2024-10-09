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

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/builtin/topic/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/publisher/DataWriterImpl.hpp>

#include "../../common/CustomPayloadPool.hpp"
#include "../../logging/mock/MockConsumer.h"

namespace eprosima {
namespace fastdds {
namespace dds {

using ::testing::_;

class FooType
{
public:

    FooType()
    {
    }

    ~FooType()
    {
    }

    inline std::string& message()
    {
        return message_;
    }

    inline void message(
            const std::string& message)
    {
        message_ = message;
    }

    bool isKeyDefined()
    {
        return false;
    }

private:

    std::string message_;
};

class TopicDataTypeMock : public TopicDataType
{
public:

    typedef FooType type;

    TopicDataTypeMock()
        : TopicDataType()
    {
        max_serialized_type_size = 4u;
        set_name("footype");
    }

    bool serialize(
            const void* const /*data*/,
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return true;
    }

    bool deserialize(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    uint32_t calculate_serialized_size(
            const void* const /*data*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return 0;
    }

    void* create_data() override
    {
        return nullptr;
    }

    void delete_data(
            void* /*data*/) override
    {
    }

    bool compute_key(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool compute_key(
            const void* const /*data*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

};

class InstanceFooType
{
public:

    InstanceFooType()
    {
    }

    ~InstanceFooType()
    {
    }

    inline std::string& message()
    {
        return message_;
    }

    inline void message(
            const std::string& message)
    {
        message_ = message;
    }

    bool isKeyDefined()
    {
        return true;
    }

private:

    std::string message_;
};

class InstanceTopicDataTypeMock : public TopicDataType
{
public:

    typedef FooType type;

    InstanceTopicDataTypeMock()
        : TopicDataType()
    {
        max_serialized_type_size = 4u;
        is_compute_key_provided = true;
        set_name("instancefootype");
    }

    bool serialize(
            const void* const /*data*/,
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return true;
    }

    bool deserialize(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    uint32_t calculate_serialized_size(
            const void* const /*data*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return 0;
    }

    void* create_data() override
    {
        return nullptr;
    }

    void delete_data(
            void* /*data*/) override
    {
    }

    bool compute_key(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool compute_key(
            const void* const /*data*/,
            fastdds::rtps::InstanceHandle_t& ihandle,
            bool /*force_md5*/) override
    {
        ihandle.value[0] = 1;
        return true;
    }

};

class BoundedTopicDataTypeMock : public TopicDataType
{
public:

    typedef FooType type;

    BoundedTopicDataTypeMock()
        : TopicDataType()
    {
        max_serialized_type_size = 4u;
        set_name("bounded_footype");
    }

    bool serialize(
            const void* const /*data*/,
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return true;
    }

    bool deserialize(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    uint32_t calculate_serialized_size(
            const void* const /*data*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return 0;
    }

    void* create_data() override
    {
        return nullptr;
    }

    void delete_data(
            void* /*data*/) override
    {
    }

    bool compute_key(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool compute_key(
            const void* const /*data*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    inline bool is_bounded() const override
    {
        return true;
    }

};

TEST(DataWriterTests, get_type)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);

    ASSERT_EQ(type, datawriter->get_type());

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

/*!
 * This test checks `DataWriter::get_guid` function works when the entity was created but not enabled.
 */
TEST(DataWriterTests, get_guid)
{
    class DiscoveryListener : public DomainParticipantListener
    {
    public:

        void on_data_writer_discovery(
                DomainParticipant*,
                fastdds::rtps::WriterDiscoveryStatus reason,
                const fastdds::dds::PublicationBuiltinTopicData& info,
                bool& /*should_be_ignored*/) override
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER == reason)
            {
                guid = info.guid;
                cv.notify_one();
            }
        }

        fastdds::rtps::GUID_t guid;
        std::mutex mutex;
        std::condition_variable cv;

    private:

        using DomainParticipantListener::on_data_writer_discovery;
    }
    discovery_listener;

    DomainParticipantQos participant_qos = PARTICIPANT_QOS_DEFAULT;
    participant_qos.wire_protocol().builtin.discovery_config.ignoreParticipantFlags =
            static_cast<eprosima::fastdds::rtps::ParticipantFilteringFlags>(
        eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST |
        eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS);

    DomainParticipant* listener_participant =
            DomainParticipantFactory::get_instance()->create_participant(0, participant_qos,
                    &discovery_listener,
                    StatusMask::none());

    DomainParticipantFactoryQos factory_qos;
    DomainParticipantFactory::get_instance()->get_qos(factory_qos);
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, participant_qos);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);

    fastdds::rtps::GUID_t guid = datawriter->guid();

    participant->enable();

    factory_qos.entity_factory().autoenable_created_entities = true;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    {
        std::unique_lock<std::mutex> lock(discovery_listener.mutex);
        discovery_listener.cv.wait(lock, [&]()
                {
                    return fastdds::rtps::GUID_t::unknown() != discovery_listener.guid;
                });
    }
    ASSERT_EQ(guid, discovery_listener.guid);

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(
                listener_participant) == RETCODE_OK);
}

TEST(DataWriterTests, ChangeDataWriterQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);

    DataWriterQos qos;
    datawriter->get_qos(qos);
    ASSERT_EQ(qos, DATAWRITER_QOS_DEFAULT);

    qos.deadline().period = 260;

    ASSERT_TRUE(datawriter->set_qos(qos) == RETCODE_OK);
    DataWriterQos wqos;
    datawriter->get_qos(wqos);

    ASSERT_EQ(qos, wqos);
    ASSERT_EQ(wqos.deadline().period, 260);

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

TEST(DataWriterTests, ChangeImmutableDataWriterQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;
    pub_qos.entity_factory().autoenable_created_entities = false;
    Publisher* publisher = participant->create_publisher(pub_qos);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);

    ASSERT_FALSE(datawriter->is_enabled());

    DataWriterQos qos;
    datawriter->get_qos(qos);
    ASSERT_EQ(qos, DATAWRITER_QOS_DEFAULT);

    qos.reliable_writer_qos().disable_positive_acks.enabled = true;

    ASSERT_TRUE(datawriter->set_qos(qos) == RETCODE_OK);
    DataWriterQos wqos;
    datawriter->get_qos(wqos);

    ASSERT_EQ(qos, wqos);
    ASSERT_TRUE(wqos.reliable_writer_qos().disable_positive_acks.enabled);

    ASSERT_TRUE(datawriter->enable() == RETCODE_OK);
    ASSERT_TRUE(datawriter->is_enabled());

    qos.reliable_writer_qos().disable_positive_acks.enabled = false;
    ASSERT_FALSE(qos == wqos);
    ASSERT_TRUE(datawriter->set_qos(qos) == RETCODE_IMMUTABLE_POLICY);

    DataWriterQos wqos2;
    datawriter->get_qos(wqos2);
    ASSERT_EQ(wqos, wqos2);
    ASSERT_TRUE(wqos2.reliable_writer_qos().disable_positive_acks.enabled);

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

TEST(DataWriterTests, ForcedDataSharing)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    TypeSupport bounded_type(new BoundedTopicDataTypeMock());
    bounded_type.register_type(participant);

    Topic* bounded_topic =
            participant->create_topic("bounded_footopic", bounded_type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(bounded_topic, nullptr);

    DataWriterQos qos;
    DataWriter* datawriter = nullptr;

    // DataSharing automatic, unbounded topic data type
    qos = DATAWRITER_QOS_DEFAULT;
    qos.endpoint().history_memory_policy = fastdds::rtps::PREALLOCATED_MEMORY_MODE;
    datawriter = publisher->create_datawriter(topic, qos);
    ASSERT_NE(datawriter, nullptr);
    ASSERT_EQ(publisher->delete_datawriter(datawriter), RETCODE_OK);

    // DataSharing automatic, bounded topic data type
    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_NE(datawriter, nullptr);
    ASSERT_EQ(publisher->delete_datawriter(datawriter), RETCODE_OK);

    // DataSharing enabled, unbounded topic data type
    qos = DATAWRITER_QOS_DEFAULT;
    qos.endpoint().history_memory_policy = fastdds::rtps::PREALLOCATED_MEMORY_MODE;
    qos.data_sharing().on(".");
    datawriter = publisher->create_datawriter(topic, qos);
    ASSERT_EQ(datawriter, nullptr);

    // DataSharing enabled, bounded topic data type
    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_NE(datawriter, nullptr);
    ASSERT_EQ(publisher->delete_datawriter(datawriter), RETCODE_OK);

    // DataSharing enabled, bounded topic data type, Dynamic memory policy
    qos = DATAWRITER_QOS_DEFAULT;
    qos.data_sharing().on(".");
    qos.endpoint().history_memory_policy = fastdds::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_EQ(datawriter, nullptr);

    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(bounded_topic), RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);


    // DataSharing forced, bounded topic data type, security enabled
    static const char* certs_path = std::getenv("CERTS_PATH");
    if (certs_path == nullptr)
    {
        std::cout << "Cannot get enviroment variable CERTS_PATH" << std::endl;
        ASSERT_TRUE(false);
    }

#ifdef HAS_SECURITY
    fastdds::rtps::PropertyPolicy security_property;
    security_property.properties().emplace_back(fastdds::rtps::Property("dds.sec.auth.plugin",
            "builtin.PKI-DH"));
    security_property.properties().emplace_back(fastdds::rtps::Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    security_property.properties().emplace_back(fastdds::rtps::Property(
                "dds.sec.auth.builtin.PKI-DH.identity_certificate",
                "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    security_property.properties().emplace_back(fastdds::rtps::Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    security_property.properties().emplace_back(fastdds::rtps::Property("dds.sec.crypto.plugin",
            "builtin.AES-GCM-GMAC"));
    security_property.properties().emplace_back("rtps.participant.rtps_protection_kind", "ENCRYPT");

    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.properties() = security_property;

    participant =
            DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    ASSERT_NE(participant, nullptr);

    publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    bounded_type.register_type(participant);

    bounded_topic = participant->create_topic("bounded_footopic", bounded_type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(bounded_topic, nullptr);

    qos = DATAWRITER_QOS_DEFAULT;
    qos.data_sharing().on(".");
    qos.endpoint().history_memory_policy = fastdds::rtps::PREALLOCATED_MEMORY_MODE;


    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_EQ(datawriter, nullptr);

    ASSERT_EQ(participant->delete_topic(bounded_topic), RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
#endif // HAS_SECURITY
}

TEST(DataWriterTests, InvalidQos)
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.entity_factory().autoenable_created_entities = false;
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);

    /* Unsupported QoS */
    const ReturnCode_t unsupported_code = RETCODE_UNSUPPORTED;

    DataWriterQos qos;

    qos = DATAWRITER_QOS_DEFAULT;
    qos.destination_order().kind = BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    EXPECT_EQ(unsupported_code, datawriter->set_qos(qos));

    qos = DATAWRITER_QOS_DEFAULT;
    qos.properties().properties().emplace_back("fastdds.unique_network_flows", "");
    EXPECT_EQ(unsupported_code, datawriter->set_qos(qos));

    /* Inconsistent QoS */
    const ReturnCode_t inconsistent_code = RETCODE_INCONSISTENT_POLICY;

    qos = DATAWRITER_QOS_DEFAULT;
    qos.properties().properties().emplace_back("fastdds.push_mode", "false");
    qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos));

    qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    qos.reliable_writer_qos().times.heartbeat_period = eprosima::fastdds::dds::c_TimeInfinite;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos));

    qos = DATAWRITER_QOS_DEFAULT;
    qos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;
    qos.liveliness().announcement_period = 20;
    qos.liveliness().lease_duration = 10;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos));

    qos.liveliness().kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos));

    qos = DATAWRITER_QOS_DEFAULT;
    qos.data_sharing().on("/tmp");
    qos.endpoint().history_memory_policy = eprosima::fastdds::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos));

    qos.endpoint().history_memory_policy = eprosima::fastdds::rtps::DYNAMIC_REUSABLE_MEMORY_MODE;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos));

    qos.endpoint().history_memory_policy = eprosima::fastdds::rtps::PREALLOCATED_MEMORY_MODE;
    EXPECT_EQ(RETCODE_OK, datawriter->set_qos(qos));

    qos.endpoint().history_memory_policy = eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    EXPECT_EQ(RETCODE_OK, datawriter->set_qos(qos));

    qos = DATAWRITER_QOS_DEFAULT;
    qos.history().kind = KEEP_LAST_HISTORY_QOS;
    qos.history().depth = 0;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos)); // KEEP LAST 0 is inconsistent
    qos.history().depth = 2;
    EXPECT_EQ(RETCODE_OK, datawriter->set_qos(qos)); // KEEP LAST 2 is OK
    // KEEP LAST 2000 but max_samples_per_instance default (400) is inconsistent but right now it only shows a warning
    // This test will fail whenever we enforce the consistency between depth and max_samples_per_instance.
    qos.history().depth = 2000;
    EXPECT_EQ(RETCODE_OK, datawriter->set_qos(qos));

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

TEST(DataWriterTests, PersistentDurabilityIsAValidQoS)
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.entity_factory().autoenable_created_entities = false;
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);

    DataWriterQos qos;
    qos = DATAWRITER_QOS_DEFAULT;
    qos.durability().kind = PERSISTENT_DURABILITY_QOS;
    // PERSISTENT DataWriter behaves as TRANSIENT
    EXPECT_EQ(RETCODE_OK, datawriter->set_qos(qos));

    // Cleanup
    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
}

//TODO: Activate the test once PSM API for DataWriter is in place
//TEST(DataWriterTests, DISABLED_ChangePSMDataWriterQos)
//{
//    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
//    ::dds::pub::Publisher publisher = ::dds::pub::Publisher(participant);

//    ::dds::pub::AnyDataWriter datawriter = ::dds::pub::AnyDataWriter();

//    ::dds::pub::qos::DataWriterQos qos = datawriter.qos();
//        ASSERT_EQ(qos, DATAWRITER_QOS_DEFAULT);

//        qos.deadline().period = 540;

//        ASSERT_NO_THROW(datawriter.qos(qos));
//        ::dds::pub::qos::DataWriterQos wqos = datawriter.qos();

//        ASSERT_EQ(qos, wqos);
//        ASSERT_EQ(wqos.deadline().period, 540);
//}

TEST(DataWriterTests, Write)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);

    FooType data;
    data.message("HelloWorld");
    ASSERT_FALSE(datawriter->write(nullptr, HANDLE_NIL) == RETCODE_OK);
    ASSERT_TRUE(datawriter->write(&data, HANDLE_NIL) == RETCODE_OK);
    ASSERT_TRUE(datawriter->write(&data, participant->get_instance_handle()) ==
            RETCODE_PRECONDITION_NOT_MET);

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

TEST(DataWriterTests, WriteWithTimestamp)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);

    eprosima::fastdds::dds::Time_t ts{ 0, 1 };

    FooType data;
    data.message("HelloWorld");

    // 1. Calling write with nullptr data returns RETCODE_BAD_PARAMETER
    ASSERT_EQ(RETCODE_BAD_PARAMETER, datawriter->write_w_timestamp(nullptr, HANDLE_NIL, ts));
    // 2. Calling write with an invalid timestamps returns RETCODE_BAD_PARAMETER
    EXPECT_EQ(RETCODE_BAD_PARAMETER,
            datawriter->write_w_timestamp(&data, HANDLE_NIL, fastdds::dds::c_TimeInfinite));
    EXPECT_EQ(RETCODE_BAD_PARAMETER,
            datawriter->write_w_timestamp(&data, HANDLE_NIL, fastdds::dds::c_TimeInvalid));
    // 3. Calling write with a wrong instance handle returns RETCODE_PRECONDITION_NOT_MET
    ASSERT_EQ(RETCODE_PRECONDITION_NOT_MET,
            datawriter->write_w_timestamp(&data, participant->get_instance_handle(), ts));
    // 4. Correct case
    ASSERT_EQ(RETCODE_OK, datawriter->write_w_timestamp(&data, HANDLE_NIL, ts));

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

void set_listener_test (
        DataWriter* writer,
        DataWriterListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(writer->set_listener(listener, mask), RETCODE_OK);
    ASSERT_EQ(writer->get_status_mask(), mask);
}

class CustomListener : public DataWriterListener
{

};

TEST(DataWriterTests, SetListener)
{
    CustomListener listener;

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT, &listener);
    ASSERT_NE(datawriter, nullptr);
    ASSERT_EQ(datawriter->get_status_mask(), StatusMask::all());

    std::vector<std::tuple<DataWriter*, DataWriterListener*, StatusMask>> testing_cases{
        //statuses, one by one
        { datawriter, &listener, StatusMask::liveliness_lost() },
        { datawriter, &listener, StatusMask::offered_deadline_missed() },
        { datawriter, &listener, StatusMask::offered_incompatible_qos() },
        { datawriter, &listener, StatusMask::publication_matched() },
        //all except one
        { datawriter, &listener, StatusMask::all() >> StatusMask::liveliness_lost() },
        { datawriter, &listener, StatusMask::all() >> StatusMask::offered_deadline_missed() },
        { datawriter, &listener, StatusMask::all() >> StatusMask::offered_incompatible_qos() },
        { datawriter, &listener, StatusMask::all() >> StatusMask::publication_matched() },
        //all and none
        { datawriter, &listener, StatusMask::all() },
        { datawriter, &listener, StatusMask::none() }
    };

    for (auto testing_case : testing_cases)
    {
        set_listener_test(std::get<0>(testing_case),
                std::get<1>(testing_case),
                std::get<2>(testing_case));
    }

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

TEST(DataWriterTests, TerminateWithoutDestroyingWriter)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(datawriter, nullptr);
}

/**
 * Create two disabled data writer objects, one for a non-keyed topic, and another one for a keyed topic.
 *
 * @param [out] datawriter           Pointer to the data writer created for the non-keyed topic.
 * @param [out] instance_datawriter  Pointer to the data writer created for the keyed topic.
 * @param [out] keyed_type_support   Optionally written with the type support of the keyed topic.
 */
static void create_writers_for_instance_test(
        DataWriter*& datawriter,
        DataWriter*& instance_datawriter,
        TypeSupport* keyed_type_support = nullptr)
{
    // Create participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);

    // Create publisher
    PublisherQos pqos = PUBLISHER_QOS_DEFAULT;
    pqos.entity_factory().autoenable_created_entities = false;
    Publisher* publisher = participant->create_publisher(pqos);
    ASSERT_NE(nullptr, publisher);

    // Register types and topics
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    TypeSupport instance_type(new InstanceTopicDataTypeMock());
    instance_type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);
    Topic* instance_topic = participant->create_topic("instancefootopic", instance_type.get_type_name(),
                    TOPIC_QOS_DEFAULT);
    ASSERT_NE(instance_topic, nullptr);

    // Create disabled DataWriters
    datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(nullptr, datawriter);
    instance_datawriter = publisher->create_datawriter(instance_topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(nullptr, instance_datawriter);

    if (nullptr != keyed_type_support)
    {
        *keyed_type_support = instance_type;
    }
}

/**
 * This test checks register_instance API
 */
TEST(DataWriterTests, RegisterInstance)
{
    // Test parameters
    InstanceFooType data;
    data.message("HelloWorld");

    // Create disabled DataWriters
    DataWriter* datawriter;
    DataWriter* instance_datawriter;
    create_writers_for_instance_test(datawriter, instance_datawriter);

    // 1. Calling register_instance in a disable writer returns HANDLE_NIL
    EXPECT_EQ(HANDLE_NIL, datawriter->register_instance(&data));
    EXPECT_EQ(HANDLE_NIL, instance_datawriter->register_instance(&data));

    // 2. Calling register_instance in a non keyed topic returns HANDLE_NIL
    ASSERT_EQ(RETCODE_OK, datawriter->enable());
    EXPECT_EQ(HANDLE_NIL, datawriter->register_instance(&data));

    // 3. Calling register_instance with an invalid sample returns HANDLE_NIL
    ASSERT_EQ(RETCODE_OK, instance_datawriter->enable());
    EXPECT_EQ(HANDLE_NIL, instance_datawriter->register_instance(nullptr));

    // 4. Calling register_instance with a valid key returns a valid handle
    EXPECT_NE(HANDLE_NIL, instance_datawriter->register_instance(&data));
}

/**
 * This test checks register_instance_w_timestamp API
 */
TEST(DataWriterTests, RegisterInstanceWithTimestamp)
{
    // Test parameters
    InstanceFooType data;
    data.message("HelloWorld");

    // Create disabled DataWriters
    DataWriter* datawriter;
    DataWriter* instance_datawriter;
    create_writers_for_instance_test(datawriter, instance_datawriter);

    eprosima::fastdds::dds::Time_t ts{ 0, 1 };

    // 1. Calling register_instance_w_timestamp in a disable writer returns HANDLE_NIL
    EXPECT_EQ(HANDLE_NIL, datawriter->register_instance_w_timestamp(&data, ts));
    EXPECT_EQ(HANDLE_NIL, instance_datawriter->register_instance_w_timestamp(&data, ts));

    // 2. Calling register_instance_w_timestamp in a non keyed topic returns HANDLE_NIL
    ASSERT_EQ(RETCODE_OK, datawriter->enable());
    EXPECT_EQ(HANDLE_NIL, datawriter->register_instance_w_timestamp(&data, ts));

    // 3. Calling register_instance with an invalid sample returns HANDLE_NIL
    ASSERT_EQ(RETCODE_OK, instance_datawriter->enable());
    EXPECT_EQ(HANDLE_NIL, instance_datawriter->register_instance_w_timestamp(nullptr, ts));

    // 4. Calling register_instance with an invalid timestamps returns HANDLE_NIL
    EXPECT_EQ(HANDLE_NIL, instance_datawriter->register_instance_w_timestamp(&data, fastdds::dds::c_TimeInfinite));
    EXPECT_EQ(HANDLE_NIL, instance_datawriter->register_instance_w_timestamp(&data, fastdds::dds::c_TimeInvalid));

    // 5. Calling register_instance with a valid key returns a valid handle
    EXPECT_NE(HANDLE_NIL, instance_datawriter->register_instance_w_timestamp(&data, ts));
}

/**
 * This test checks unregister_instance API
 */
TEST(DataWriterTests, UnregisterInstance)
{
    // Test parameters
    InstanceHandle_t handle;
    InstanceFooType data;
    data.message("HelloWorld");

    // Create disabled DataWriters
    TypeSupport instance_type;
    DataWriter* datawriter;
    DataWriter* instance_datawriter;
    create_writers_for_instance_test(datawriter, instance_datawriter, &instance_type);

    // 1. Calling unregister_instance in a disable writer returns RETCODE_NOT_ENABLED
    EXPECT_EQ(RETCODE_NOT_ENABLED, datawriter->unregister_instance(&data, handle));

    // 2. Calling unregister_instance in a non keyed topic returns RETCODE_PRECONDITION_NOT MET
    ASSERT_EQ(RETCODE_OK, datawriter->enable());
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, datawriter->unregister_instance(&data, handle));

    // 3. Calling unregister_instance with an invalid sample returns RETCODE_BAD_PARAMETER
    ASSERT_EQ(RETCODE_OK, instance_datawriter->enable());
    EXPECT_EQ(RETCODE_BAD_PARAMETER, instance_datawriter->unregister_instance(nullptr, handle));

#if !defined(NDEBUG)
    // 4. Calling unregister_instance with an inconsistent handle returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, instance_datawriter->unregister_instance(&data,
            datawriter->get_instance_handle()));
#endif // NDEBUG

    // 5. Calling unregister_instance with a key not yet registered returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, instance_datawriter->unregister_instance(&data, handle));

    // 6. Calling unregister_instance with a valid key returns RETCODE_OK
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write(&data, HANDLE_NIL));
    EXPECT_EQ(RETCODE_OK, instance_datawriter->unregister_instance(&data, handle));

    // 7. Calling unregister_instance with a valid InstanceHandle also returns RETCODE_OK
    data.message("HelloWorld_1");
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write(&data, HANDLE_NIL));
    instance_type->compute_key(&data, handle);
    EXPECT_EQ(RETCODE_OK, instance_datawriter->unregister_instance(&data, handle));

    // TODO(jlbueno) There are other possible errors sending the unregister message: RETCODE_OUT_OF_RESOURCES,
    // RETCODE_ERROR, and RETCODE_TIMEOUT (only if HAVE_STRICT_REALTIME has been defined).
}

/**
 * This test checks unregister_instance_w_timestamp API
 */
TEST(DataWriterTests, UnregisterInstanceWithTimestamp)
{
    // Test parameters
    InstanceHandle_t handle;
    InstanceFooType data;
    data.message("HelloWorld");

    // Create disabled DataWriters
    TypeSupport instance_type;
    DataWriter* datawriter;
    DataWriter* instance_datawriter;
    create_writers_for_instance_test(datawriter, instance_datawriter, &instance_type);

    eprosima::fastdds::dds::Time_t ts{ 0, 1 };

    // 1. Calling unregister_instance in a disable writer returns RETCODE_NOT_ENABLED
    EXPECT_EQ(RETCODE_NOT_ENABLED, datawriter->unregister_instance_w_timestamp(&data, handle, ts));

    // 2. Calling unregister_instance in a non keyed topic returns RETCODE_PRECONDITION_NOT MET
    ASSERT_EQ(RETCODE_OK, datawriter->enable());
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET,
            datawriter->unregister_instance_w_timestamp(&data, handle, ts));

    // 3. Calling unregister_instance with an invalid sample returns RETCODE_BAD_PARAMETER
    ASSERT_EQ(RETCODE_OK, instance_datawriter->enable());
    EXPECT_EQ(RETCODE_BAD_PARAMETER,
            instance_datawriter->unregister_instance_w_timestamp(nullptr, handle, ts));

#if !defined(NDEBUG)
    // 4. Calling unregister_instance with an inconsistent handle returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, instance_datawriter->unregister_instance_w_timestamp(&data,
            datawriter->get_instance_handle(), ts));
#endif // NDEBUG

    // 5. Calling unregister_instance with a key not yet registered returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET,
            instance_datawriter->unregister_instance_w_timestamp(&data, handle, ts));

    // 6. Calling unregister_instance with a valid key returns RETCODE_OK
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write_w_timestamp(&data, HANDLE_NIL, ts));
    EXPECT_EQ(RETCODE_OK, instance_datawriter->unregister_instance_w_timestamp(&data, handle, ts));

    // 7. Calling unregister_instance with a valid InstanceHandle also returns RETCODE_OK
    data.message("HelloWorld_1");
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write_w_timestamp(&data, HANDLE_NIL, ts));
    instance_type.compute_key(&data, handle);
    EXPECT_EQ(RETCODE_OK, instance_datawriter->unregister_instance_w_timestamp(&data, handle, ts));

    // 8. Check invalid timestamps
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write_w_timestamp(&data, HANDLE_NIL, ts));
    ts = eprosima::fastdds::dds::c_TimeInfinite;
    EXPECT_EQ(RETCODE_BAD_PARAMETER,
            instance_datawriter->unregister_instance_w_timestamp(&data, handle, ts));
    ts = eprosima::fastdds::dds::c_TimeInvalid;
    EXPECT_EQ(RETCODE_BAD_PARAMETER,
            instance_datawriter->unregister_instance_w_timestamp(&data, handle, ts));

    // TODO(jlbueno) There are other possible errors sending the unregister message: RETCODE_OUT_OF_RESOURCES,
    // RETCODE_ERROR, and RETCODE_TIMEOUT (only if HAVE_STRICT_REALTIME has been defined).
}

/**
 * This test checks dispose API
 */
TEST(DataWriterTests, Dispose)
{
    // Test parameters
    InstanceHandle_t handle;
    InstanceFooType data;
    data.message("HelloWorld");

    // Create disabled DataWriters
    TypeSupport instance_type;
    DataWriter* datawriter;
    DataWriter* instance_datawriter;
    create_writers_for_instance_test(datawriter, instance_datawriter, &instance_type);

    // 1. Calling dispose in a disable writer returns RETCODE_NOT_ENABLED
    EXPECT_EQ(RETCODE_NOT_ENABLED, datawriter->dispose(&data, handle));

    // 2. Calling dispose in a non keyed topic returns RETCODE_PRECONDITION_NOT MET
    ASSERT_EQ(RETCODE_OK, datawriter->enable());
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, datawriter->dispose(&data, handle));

    // 3. Calling dispose with an invalid sample returns RETCODE_BAD_PARAMETER
    ASSERT_EQ(RETCODE_OK, instance_datawriter->enable());
    EXPECT_EQ(RETCODE_BAD_PARAMETER, instance_datawriter->dispose(nullptr, handle));

#if !defined(NDEBUG)
    // 4. Calling dispose with an inconsistent handle returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, instance_datawriter->dispose(&data,
            datawriter->get_instance_handle()));
#endif // NDEBUG

    // 5. Calling dispose with a key not yet registered returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, instance_datawriter->dispose(&data, handle));

    // 6. Calling dispose with a valid key returns RETCODE_OK
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write(&data, HANDLE_NIL));
    EXPECT_EQ(RETCODE_OK, instance_datawriter->dispose(&data, handle));

    // 7. Calling dispose with a valid InstanceHandle also returns RETCODE_OK
    data.message("HelloWorld_1");
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write(&data, HANDLE_NIL));
    instance_type.compute_key(&data, handle);
    EXPECT_EQ(RETCODE_OK, instance_datawriter->dispose(&data, handle));

    // TODO(jlbueno) There are other possible errors sending the dispose message: RETCODE_OUT_OF_RESOURCES,
    // RETCODE_ERROR, and RETCODE_TIMEOUT (only if HAVE_STRICT_REALTIME has been defined).
}

/**
 * This test checks dispose_w_timestamp API
 */
TEST(DataWriterTests, DisposeWithTimestamp)
{
    // Test parameters
    InstanceHandle_t handle;
    InstanceFooType data;
    data.message("HelloWorld");

    // Create disabled DataWriters
    TypeSupport instance_type;
    DataWriter* datawriter;
    DataWriter* instance_datawriter;
    create_writers_for_instance_test(datawriter, instance_datawriter, &instance_type);

    eprosima::fastdds::dds::Time_t ts{ 0, 1 };

    // 1. Calling dispose in a disable writer returns RETCODE_NOT_ENABLED
    EXPECT_EQ(RETCODE_NOT_ENABLED, datawriter->dispose_w_timestamp(&data, handle, ts));

    // 2. Calling dispose in a non keyed topic returns RETCODE_PRECONDITION_NOT MET
    ASSERT_EQ(RETCODE_OK, datawriter->enable());
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, datawriter->dispose_w_timestamp(&data, handle, ts));

    // 3. Calling dispose with an invalid sample returns RETCODE_BAD_PARAMETER
    ASSERT_EQ(RETCODE_OK, instance_datawriter->enable());
    EXPECT_EQ(RETCODE_BAD_PARAMETER, instance_datawriter->dispose_w_timestamp(nullptr, handle, ts));

#if !defined(NDEBUG)
    // 4. Calling dispose with an inconsistent handle returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, instance_datawriter->dispose_w_timestamp(&data,
            datawriter->get_instance_handle(), ts));
#endif // NDEBUG

    // 5. Calling dispose with a key not yet registered returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, instance_datawriter->dispose_w_timestamp(&data, handle, ts));

    // 6. Calling dispose with a valid key returns RETCODE_OK
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write_w_timestamp(&data, HANDLE_NIL, ts));
    EXPECT_EQ(RETCODE_OK, instance_datawriter->dispose_w_timestamp(&data, handle, ts));

    // 7. Calling dispose with a valid InstanceHandle also returns RETCODE_OK
    data.message("HelloWorld_1");
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write_w_timestamp(&data, HANDLE_NIL, ts));
    instance_type.compute_key(&data, handle);
    EXPECT_EQ(RETCODE_OK, instance_datawriter->dispose_w_timestamp(&data, handle, ts));

    // 8. Check invalid timestamps
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write_w_timestamp(&data, HANDLE_NIL, ts));
    ts = eprosima::fastdds::dds::c_TimeInfinite;
    EXPECT_EQ(RETCODE_BAD_PARAMETER, instance_datawriter->dispose_w_timestamp(&data, handle, ts));
    ts = eprosima::fastdds::dds::c_TimeInvalid;
    EXPECT_EQ(RETCODE_BAD_PARAMETER, instance_datawriter->dispose_w_timestamp(&data, handle, ts));

    // TODO(jlbueno) There are other possible errors sending the dispose message: RETCODE_OUT_OF_RESOURCES,
    // RETCODE_ERROR, and RETCODE_TIMEOUT (only if HAVE_STRICT_REALTIME has been defined).
}

/**
 * This test checks get_key_value API
 */
TEST(DataWriterTests, GetKeyValue)
{
    // Test parameters
    InstanceFooType data;
    InstanceHandle_t wrong_handle;
    wrong_handle.value[0] = 0xee;
    InstanceHandle_t valid_handle;
    InstanceFooType valid_data;
    valid_data.message("HelloWorld");

    // Create disabled DataWriters
    DataWriter* datawriter;
    DataWriter* instance_datawriter;
    create_writers_for_instance_test(datawriter, instance_datawriter);

    // 1. Check nullptr on key_holder
    EXPECT_EQ(RETCODE_BAD_PARAMETER, datawriter->get_key_value(nullptr, wrong_handle));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(nullptr, wrong_handle));

    // 2. Check HANDLE_NIL
    EXPECT_EQ(RETCODE_BAD_PARAMETER, datawriter->get_key_value(&data, HANDLE_NIL));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, HANDLE_NIL));

    // 3. Check type should have keys, and writer should be enabled
    EXPECT_EQ(RETCODE_ILLEGAL_OPERATION, datawriter->get_key_value(&data, wrong_handle));
    EXPECT_EQ(RETCODE_NOT_ENABLED, instance_datawriter->get_key_value(&data, wrong_handle));

    ASSERT_EQ(RETCODE_OK, datawriter->enable());
    ASSERT_EQ(RETCODE_OK, instance_datawriter->enable());

    // 4. Calling get_key_value with a key not yet registered returns RETCODE_BAD_PARAMETER
    EXPECT_EQ(RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, wrong_handle));

    // 5. Calling get_key_value on a registered instance should work.
    valid_handle = instance_datawriter->register_instance(&valid_data);
    EXPECT_NE(HANDLE_NIL, valid_handle);
    EXPECT_EQ(RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));

    // 6. Calling get_key_value on an unregistered instance should return RETCODE_BAD_PARAMETER.
    ASSERT_EQ(RETCODE_OK, instance_datawriter->unregister_instance(&valid_data, valid_handle));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, valid_handle));

    // 7. Calling get_key_value with a valid instance should work
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write(&valid_data, HANDLE_NIL));
    EXPECT_EQ(RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));

    // 8. Calling get_key_value on a disposed instance should work.
    ASSERT_EQ(RETCODE_OK, instance_datawriter->dispose(&valid_data, valid_handle));
    EXPECT_EQ(RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));
}

struct LoanableType
{
    static constexpr uint32_t initialization_value()
    {
        return 27u;
    }

    uint32_t index = initialization_value();
};

class LoanableTypeSupport : public TopicDataType
{
public:

    typedef LoanableType type;
    using TopicDataType::is_plain;

    LoanableTypeSupport()
        : TopicDataType()
    {
        max_serialized_type_size = 4u + sizeof(LoanableType);
        set_name("LoanableType");
    }

    bool serialize(
            const void* const /*data*/,
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return true;
    }

    bool deserialize(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    uint32_t calculate_serialized_size(
            const void* const /*data*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return max_serialized_type_size;
    }

    void* create_data() override
    {
        return nullptr;
    }

    void delete_data(
            void* /*data*/) override
    {
    }

    bool compute_key(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool compute_key(
            const void* const /*data*/,
            fastdds::rtps::InstanceHandle_t& /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool is_bounded() const override
    {
        return true;
    }

    bool is_plain(
            DataRepresentationId_t) const override
    {
        return true;
    }

    bool construct_sample(
            void* sample) const override
    {
        new (sample) LoanableType();
        return true;
    }

};

TEST(DataWriterTests, LoanPositiveTests)
{
    using InitKind = DataWriter::LoanInitializationKind;

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new LoanableTypeSupport());
    type.register_type(participant);

    Topic* topic = participant->create_topic("loanable_topic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriterQos wqos;
    wqos.history().depth = 1;

    DataWriter* datawriter = publisher->create_datawriter(topic, wqos);
    ASSERT_NE(datawriter, nullptr);
    ASSERT_EQ(datawriter->get_status_mask(), StatusMask::all());

    void* sample = nullptr;

    // Loan and discard (check different initialization schemes)
    EXPECT_EQ(RETCODE_OK, datawriter->loan_sample(sample, InitKind::NO_LOAN_INITIALIZATION));
    EXPECT_NE(nullptr, sample);
    EXPECT_EQ(RETCODE_OK, datawriter->discard_loan(sample));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, datawriter->discard_loan(sample));

    EXPECT_EQ(RETCODE_OK, datawriter->loan_sample(sample, InitKind::ZERO_LOAN_INITIALIZATION));
    ASSERT_NE(nullptr, sample);
    EXPECT_EQ(0u, static_cast<LoanableType*>(sample)->index);
    EXPECT_EQ(RETCODE_OK, datawriter->discard_loan(sample));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, datawriter->discard_loan(sample));

    EXPECT_EQ(RETCODE_OK, datawriter->loan_sample(sample, InitKind::CONSTRUCTED_LOAN_INITIALIZATION));
    ASSERT_NE(nullptr, sample);
    EXPECT_EQ(LoanableType::initialization_value(), static_cast<LoanableType*>(sample)->index);
    EXPECT_EQ(RETCODE_OK, datawriter->discard_loan(sample));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, datawriter->discard_loan(sample));

    // Resource limits:
    // Depth has been configured to 1, so pool will allow up to depth + 1 loans.
    // We will check that the 3rd unreturned loan returns OUT_OF_RESOURCES.
    void* sample_2 = nullptr;
    void* sample_3 = nullptr;
    EXPECT_EQ(RETCODE_OK, datawriter->loan_sample(sample));
    EXPECT_NE(nullptr, sample);
    EXPECT_EQ(RETCODE_OK, datawriter->loan_sample(sample_2));
    EXPECT_NE(nullptr, sample_2);
    EXPECT_EQ(RETCODE_OUT_OF_RESOURCES, datawriter->loan_sample(sample_3));
    EXPECT_EQ(nullptr, sample_3);
    EXPECT_EQ(RETCODE_OK, datawriter->discard_loan(sample_2));
    EXPECT_EQ(RETCODE_OK, datawriter->discard_loan(sample));

    // Write samples, both loaned and not
    LoanableType data;
    fastdds::rtps::InstanceHandle_t handle;
    EXPECT_EQ(RETCODE_OK, datawriter->loan_sample(sample));
    EXPECT_NE(nullptr, sample);
    EXPECT_EQ(RETCODE_OK, datawriter->loan_sample(sample_2));
    EXPECT_NE(nullptr, sample_2);
    EXPECT_EQ(RETCODE_OK, datawriter->write(sample, handle));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, datawriter->discard_loan(sample));
    EXPECT_EQ(RETCODE_OK, datawriter->write(sample_2, handle));
    EXPECT_EQ(RETCODE_BAD_PARAMETER, datawriter->discard_loan(sample_2));

    EXPECT_EQ(RETCODE_OK, datawriter->write(&data, handle));
    EXPECT_EQ(RETCODE_OK, datawriter->loan_sample(sample));
    EXPECT_NE(nullptr, sample);
    EXPECT_EQ(RETCODE_OUT_OF_RESOURCES, datawriter->write(&data, handle));
    EXPECT_EQ(RETCODE_OK, datawriter->discard_loan(sample));
    EXPECT_EQ(RETCODE_OK, datawriter->write(&data, handle));
    EXPECT_EQ(RETCODE_OK, datawriter->write(&data, handle));

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

class LoanableTypeSupportTesting : public LoanableTypeSupport
{
public:

    using LoanableTypeSupport::is_plain;

    bool is_plain_result = true;
    bool construct_sample_result = true;

    bool is_plain(
            DataRepresentationId_t) const override
    {
        return is_plain_result;
    }

    bool construct_sample(
            void* sample) const override
    {
        new (sample) LoanableType();
        return construct_sample_result;
    }

};

TEST(DataWriterTests, LoanNegativeTests)
{
    using InitKind = DataWriter::LoanInitializationKind;

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    PublisherQos pqos = PUBLISHER_QOS_DEFAULT;
    pqos.entity_factory().autoenable_created_entities = false;
    Publisher* publisher = participant->create_publisher(pqos);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new LoanableTypeSupportTesting());
    type.register_type(participant);
    LoanableTypeSupportTesting* type_support = static_cast<LoanableTypeSupportTesting*>(type.get());

    Topic* topic = participant->create_topic("loanable_topic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriterQos wqos;
    wqos.history().depth = 1;

    DataWriter* datawriter = publisher->create_datawriter(topic, wqos);
    ASSERT_NE(datawriter, nullptr);
    ASSERT_EQ(datawriter->get_status_mask(), StatusMask::all());

    void* sample = nullptr;
    auto original_type_size = type_support->max_serialized_type_size;

    // Check for illegal operation
    type_support->is_plain_result = false;
    EXPECT_EQ(RETCODE_ILLEGAL_OPERATION, datawriter->loan_sample(sample));
    type_support->is_plain_result = true;
    type_support->max_serialized_type_size = 0;
    EXPECT_EQ(RETCODE_ILLEGAL_OPERATION, datawriter->loan_sample(sample));
    type_support->max_serialized_type_size = original_type_size;

    // Check for not enabled
    EXPECT_EQ(RETCODE_NOT_ENABLED, datawriter->loan_sample(sample));
    EXPECT_EQ(RETCODE_OK, datawriter->enable());

    // Check for constructor support
    type_support->construct_sample_result = false;
    EXPECT_EQ(RETCODE_UNSUPPORTED,
            datawriter->loan_sample(sample, InitKind::CONSTRUCTED_LOAN_INITIALIZATION));
    type_support->construct_sample_result = true;

    // Check preconditions on delete_datawriter
    EXPECT_EQ(RETCODE_OK, datawriter->loan_sample(sample));
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, publisher->delete_datawriter(datawriter));
    EXPECT_EQ(RETCODE_OK, datawriter->discard_loan(sample));

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == RETCODE_OK);
}

class DataWriterTest : public DataWriter
{
public:

    DataWriterImpl* get_impl() const
    {
        return impl_;
    }

};

class DataWriterImplTest : public DataWriterImpl
{
public:

    DataWriterHistory* get_history()
    {
        return history_.get();
    }

};

/**
 * This test checks instance wait_for_acknowledgements API
 */
#ifndef __QNXNTO__
TEST(DataWriterTests, InstanceWaitForAcknowledgement)
{
    // Test parameters
    dds::Duration_t max_wait(2, 0);
    InstanceHandle_t handle;
    InstanceFooType data;
    data.message("HelloWorld");

    // Create participant
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);

    // Create publisher
    PublisherQos pqos = PUBLISHER_QOS_DEFAULT;
    pqos.entity_factory().autoenable_created_entities = false;
    Publisher* publisher = participant->create_publisher(pqos);
    ASSERT_NE(nullptr, publisher);

    // Register types and topics
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);
    TypeSupport instance_type(new InstanceTopicDataTypeMock());
    instance_type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);
    Topic* instance_topic = participant->create_topic("instancefootopic", instance_type.get_type_name(),
                    TOPIC_QOS_DEFAULT);
    ASSERT_NE(instance_topic, nullptr);

    // Create disabled DataWriters
    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(nullptr, datawriter);
    DataWriter* instance_datawriter = publisher->create_datawriter(instance_topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(nullptr, instance_datawriter);

    // 1. Calling wait_for_acknowledgments in a disable writer returns RETCODE_NOT_ENABLED
    EXPECT_EQ(RETCODE_NOT_ENABLED, datawriter->wait_for_acknowledgments(&data, handle, max_wait));

    // 2. Calling wait_for_acknowledgments in a non keyed topic returns RETCODE_PRECONDITION_NOT MET
    ASSERT_EQ(RETCODE_OK, datawriter->enable());
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, datawriter->wait_for_acknowledgments(&data, handle,
            max_wait));

    // 3. Calling wait_for_acknowledgments with an invalid sample returns RETCODE_BAD_PARAMETER
    ASSERT_EQ(RETCODE_OK, instance_datawriter->enable());
    EXPECT_EQ(RETCODE_BAD_PARAMETER, instance_datawriter->wait_for_acknowledgments(nullptr, handle,
            max_wait));

#if !defined(NDEBUG)
    // 4. Calling wait_for_acknowledgments with an inconsistent handle returns RETCODE_BAD_PARAMETER
    EXPECT_EQ(RETCODE_PRECONDITION_NOT_MET, instance_datawriter->wait_for_acknowledgments(&data,
            datawriter->get_instance_handle(), max_wait));
#endif // NDEBUG

    // Access DataWriterHistory
    DataWriterTest* instance_datawriter_test = static_cast<DataWriterTest*>(instance_datawriter);
    ASSERT_NE(nullptr, instance_datawriter_test);
    DataWriterImpl* datawriter_impl = instance_datawriter_test->get_impl();
    ASSERT_NE(nullptr, datawriter_impl);
    DataWriterImplTest* datawriter_impl_test = static_cast<DataWriterImplTest*>(datawriter_impl);
    ASSERT_NE(nullptr, datawriter_impl_test);
    auto history = datawriter_impl_test->get_history();

    // 5. Calling wait_for_acknowledgments in a keyed topic with HANDLE_NIL returns
    // RETCODE_OK
    EXPECT_CALL(*history, wait_for_acknowledgement_last_change(_, _, _)).WillOnce(testing::Return(true));
    ASSERT_EQ(RETCODE_OK, instance_datawriter->write(&data, HANDLE_NIL));
    EXPECT_EQ(RETCODE_OK, instance_datawriter->wait_for_acknowledgments(&data, handle,
            max_wait));

    // 6. Calling wait_for_acknowledgments in a keyed topic with a known handle returns RETCODE_OK (no matched readers)
    // Expectations
    EXPECT_CALL(*history, wait_for_acknowledgement_last_change(_, _, _)).WillOnce(testing::Return(true));

    instance_type.compute_key(&data, handle);
    EXPECT_EQ(RETCODE_OK, instance_datawriter->wait_for_acknowledgments(&data, handle, max_wait));

    // 7. Calling wait_for_acknowledgments in a keyed topic with a known handle timeouts if some reader has not
    // acknowledged before max_wait time (mock) returns RETCODE_TIMEOUT
    // Expectations
    EXPECT_CALL(*history, wait_for_acknowledgement_last_change(_, _, _)).WillOnce(testing::Return(false));
    EXPECT_EQ(RETCODE_TIMEOUT, instance_datawriter->wait_for_acknowledgments(&data, handle, max_wait));
}
#endif // __QNXNTO__

class DataWriterUnsupportedTests : public ::testing::Test
{
public:

    DataWriterUnsupportedTests()
    {
        Reset();
    }

    ~DataWriterUnsupportedTests()
    {
        Log::Reset();
        Log::KillThread();
    }

    void Reset()
    {
        Log::ClearConsumers();
        // Only listen for logWarnings generated by the tested class
        mockConsumer = new MockConsumer("DATA_WRITER");
        Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
        Log::SetVerbosity(Log::Warning);
    }

    MockConsumer* mockConsumer;

    const uint32_t AsyncTries = 5;
    const uint32_t AsyncWaitMs = 25;

    void HELPER_WaitForEntries(
            uint32_t amount)
    {
        size_t entries = 0;
        for (uint32_t i = 0; i != AsyncTries; i++)
        {
            entries = mockConsumer->ConsumedEntries().size();
            if (entries == amount)
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(AsyncWaitMs));
        }

        ASSERT_EQ(amount, mockConsumer->ConsumedEntries().size());

    }

};


/*
 * This test checks that the DataWriter methods defined in the standard not yet implemented in FastDDS return
 * RETCODE_UNSUPPORTED. The following methods are checked:
 * 1. lookup_instance
 */
TEST_F(DataWriterUnsupportedTests, UnsupportedDataWriterMethods)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    DataWriter* data_writer = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    EXPECT_EQ(HANDLE_NIL, data_writer->lookup_instance(nullptr /* instance */));

    // Expected logWarnings: lookup_instance
    HELPER_WaitForEntries(1);

    ASSERT_EQ(publisher->delete_datawriter(data_writer), RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

/*
 * This test checks the allocation consistency when NOT using instances.
 * If the topic is keyed,
 * max_samples should be greater or equal than max_instances * max_samples_per_instance.
 * If that condition is not met, the endpoint creation should fail.
 * If not keyed (not using instances), the only property that is used is max_samples,
 * thus, should not fail with the previously mentioned configuration.
 * The following method is checked:
 * 1. Publisher::create_datawriter
 * 2. DataWriter::set_qos
 */
TEST(DataWriterTests, InstancePolicyAllocationConsistencyNotKeyed)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Next QoS config checks the default qos configuration,
    // create_datawriter() should NOT return nullptr.
    DataWriterQos qos = DATAWRITER_QOS_DEFAULT;

    DataWriter* data_writer1 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(data_writer1, nullptr);

    // Below an ampliation of the last comprobation, for which it is proved the case of < 0 (-1),
    // which also means infinite value, and does not make any change.
    // Updated to check negative values (Redmine ticket #20722)
    qos.resource_limits().max_samples = -1;
    qos.resource_limits().max_instances = -1;
    qos.resource_limits().max_samples_per_instance = -1;

    DataWriter* data_writer2 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(data_writer2, nullptr);

    // Next QoS config checks that if user sets max_samples < ( max_instances * max_samples_per_instance ) ,
    // create_datawriter() should NOT return nullptr.
    // By not using instances, instance allocation consistency is not checked.
    qos.resource_limits().max_samples = 4999;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataWriter* data_writer3 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(data_writer3, nullptr);

    // Next QoS config checks that if user sets max_samples > ( max_instances * max_samples_per_instance ) ,
    // create_datawriter() should NOT return nullptr.
    // By not using instances, instance allocation consistency is not checked.
    qos.resource_limits().max_samples = 5001;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataWriter* data_writer4 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(data_writer4, nullptr);

    // Next QoS config checks that if user sets max_samples infinite
    // and ( max_instances * max_samples_per_instance ) finite,
    // create_datawriter() should NOT return nullptr.
    // By not using instances, instance allocation consistency is not checked.
    qos.resource_limits().max_samples = 0;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataWriter* data_writer5 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(data_writer5, nullptr);

    // Next QoS config checks the default qos configuration,
    // set_qos() should return RETCODE_OK = 0
    DataWriterQos qos2 = DATAWRITER_QOS_DEFAULT;
    DataWriter* default_data_writer1 = publisher->create_datawriter(topic, qos2);
    ASSERT_NE(default_data_writer1, nullptr);

    ASSERT_EQ(RETCODE_OK, default_data_writer1->set_qos(qos2));

    // Below an ampliation of the last comprobation, for which it is proved the case of < 0 (-1),
    // which also means infinite value.
    // By not using instances, instance allocation consistency is not checked.
    // Updated to check negative values (Redmine ticket #20722)
    qos2.resource_limits().max_samples = -1;
    qos2.resource_limits().max_instances = -1;
    qos2.resource_limits().max_samples_per_instance = -1;

    ASSERT_EQ(RETCODE_OK, default_data_writer1->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples < ( max_instances * max_samples_per_instance ) ,
    // set_qos() should return RETCODE_OK = 0
    // By not using instances, instance allocation consistency is not checked.
    qos2.resource_limits().max_samples = 4999;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_writer1->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples > ( max_instances * max_samples_per_instance ) ,
    // set_qos() should return RETCODE_OK = 0
    // By not using instances, instance allocation consistency is not checked.
    qos2.resource_limits().max_samples = 5001;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_writer1->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples infinite
    // and ( max_instances * max_samples_per_instance ) finite,
    // set_qos() should return RETCODE_OK = 0
    // By not using instances, instance allocation consistency is not checked.
    qos2.resource_limits().max_samples = 0;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_writer1->set_qos(qos2));
}

/*
 * This test checks the allocation consistency when USING instances.
 * If the topic is keyed,
 * max_samples should be greater or equal than max_instances * max_samples_per_instance.
 * If that condition is not met, the endpoint creation should fail.
 * If not keyed (not using instances), the only property that is used is max_samples,
 * thus, should not fail with the previously mentioned configuration.
 * The following method is checked:
 * 1. Publisher::create_datawriter
 * 2. DataWriter::set_qos
 */
TEST(DataWriterTests, InstancePolicyAllocationConsistencyKeyed)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    // This test pretends to use topic with instances, so the following flag is set.
    type.get()->is_compute_key_provided = true;

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Next QoS config checks the default qos configuration,
    // create_datawriter() should not return nullptr.
    DataWriterQos qos = DATAWRITER_QOS_DEFAULT;

    DataWriter* data_writer1 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(data_writer1, nullptr);

    // Below an ampliation of the last comprobation, for which it is proved the case of < 0 (-1),
    // which also means infinite value.
    // Updated to check negative values (Redmine ticket #20722)
    qos.resource_limits().max_samples = -1;
    qos.resource_limits().max_instances = -1;
    qos.resource_limits().max_samples_per_instance = -1;

    DataWriter* data_writer2 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(data_writer2, nullptr);

    // Next QoS config checks that if user sets max_samples < ( max_instances * max_samples_per_instance ) ,
    // create_datawriter() should return nullptr.
    qos.resource_limits().max_samples = 4999;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataWriter* data_writer3 = publisher->create_datawriter(topic, qos);
    ASSERT_EQ(data_writer3, nullptr);

    // Next QoS config checks that if user sets max_samples > ( max_instances * max_samples_per_instance ) ,
    // create_datawriter() should not return nullptr.
    qos.resource_limits().max_samples = 5001;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataWriter* data_writer4 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(data_writer4, nullptr);

    // Next QoS config checks that if user sets max_samples = ( max_instances * max_samples_per_instance ) ,
    // create_datawriter() should not return nullptr.
    qos.resource_limits().max_samples = 5000;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataWriter* data_writer5 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(data_writer5, nullptr);

    // Next QoS config checks that if user sets max_samples infinite
    // and ( max_instances * max_samples_per_instance ) finite,
    // create_datawriter() should not return nullptr.
    qos.resource_limits().max_samples = 0;
    qos.resource_limits().max_instances = 10;
    qos.resource_limits().max_samples_per_instance = 500;

    DataWriter* data_writer6 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(data_writer6, nullptr);

    // Next QoS config checks the default qos configuration,
    // set_qos() should return RETCODE_OK = 0, as the by default values are already infinite.
    DataWriterQos qos2 = DATAWRITER_QOS_DEFAULT;
    DataWriter* default_data_writer1 = publisher->create_datawriter(topic, qos2);
    ASSERT_NE(default_data_writer1, nullptr);

    ASSERT_EQ(RETCODE_OK, default_data_writer1->set_qos(qos2));

    // Below an ampliation of the last comprobation, for which it is proved the case of < 0 (-1),
    // which also means infinite value.
    // Updated to check negative values (Redmine ticket #20722)
    qos2.resource_limits().max_samples = -1;
    qos2.resource_limits().max_instances = -1;
    qos2.resource_limits().max_samples_per_instance = -1;

    ASSERT_EQ(RETCODE_OK, default_data_writer1->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples < ( max_instances * max_samples_per_instance ) ,
    // set_qos() should return a value != 0 (not OK)
    qos2.resource_limits().max_samples = 4999;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_NE(RETCODE_OK, default_data_writer1->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples > ( max_instances * max_samples_per_instance ) ,
    // set_qos() should return RETCODE_OK = 0.
    qos2.resource_limits().max_samples = 5001;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_writer1->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples = ( max_instances * max_samples_per_instance ) ,
    // set_qos() should return RETCODE_OK = 0.
    qos2.resource_limits().max_samples = 5000;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_writer1->set_qos(qos2));

    // Next QoS config checks that if user sets max_samples infinite
    // and ( max_instances * max_samples_per_instance ) finite,
    // set_qos() should return RETCODE_OK = 0.
    qos2.resource_limits().max_samples = 0;
    qos2.resource_limits().max_instances = 10;
    qos2.resource_limits().max_samples_per_instance = 500;

    ASSERT_EQ(RETCODE_OK, default_data_writer1->set_qos(qos2));
}


/*
 * This test checks the proper behavior of the custom payload pool DataReader overload.
 */
TEST(DataWriterTests, CustomPoolCreation)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(subscriber, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Next QoS config checks the default qos configuration,
    // create_datareader() should not return nullptr.
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;

    std::shared_ptr<CustomPayloadPool> payload_pool = std::make_shared<CustomPayloadPool>();

    DataReader* data_reader = subscriber->create_datareader(topic, reader_qos);

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;

    DataWriter* data_writer = publisher->create_datawriter(topic, writer_qos, nullptr, StatusMask::all(), payload_pool);

    ASSERT_NE(data_writer, nullptr);
    ASSERT_NE(data_reader, nullptr);

    FooType data;

    data_writer->write(&data, HANDLE_NIL);

    ASSERT_EQ(payload_pool->requested_payload_count, 1u);

    participant->delete_contained_entities();

    DomainParticipantFactory::get_instance()->delete_participant(participant);
}

TEST(DataWriterTests, history_depth_max_samples_per_instance_warning)
{

    /* Setup log so it may catch the expected warning */
    Log::ClearConsumers();
    MockConsumer* mockConsumer = new MockConsumer("RTPS_QOS_CHECK");
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(mockConsumer));
    Log::SetVerbosity(Log::Warning);

    /* Create a participant, topic, and a publisher */
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0,
                    PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    Topic* topic = participant->create_topic("footopic", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    /* Create a datawriter with the QoS that should generate a warning */
    DataWriterQos qos;
    qos.history().depth = 10;
    qos.resource_limits().max_samples_per_instance = 5;
    DataWriter* datawriter_1 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(datawriter_1, nullptr);

    /* Check that the config generated a warning */
    auto wait_for_log_entries =
            [&mockConsumer](const uint32_t amount, const uint32_t retries, const uint32_t wait_ms) -> size_t
            {
                size_t entries = 0;
                for (uint32_t i = 0; i < retries; i++)
                {
                    entries = mockConsumer->ConsumedEntries().size();
                    if (entries >= amount)
                    {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
                }
                return entries;
            };

    const size_t expected_entries = 1;
    const uint32_t retries = 4;
    const uint32_t wait_ms = 25;
    ASSERT_EQ(wait_for_log_entries(expected_entries, retries, wait_ms), expected_entries);

    /* Check that the datawriter can send data */
    FooType data;
    ASSERT_EQ(RETCODE_OK, datawriter_1->write(&data, HANDLE_NIL));

    /* Check that a correctly initialized writer does not produce any warning */
    qos.history().depth = 10;
    qos.resource_limits().max_samples_per_instance = 10;
    DataWriter* datawriter_2 = publisher->create_datawriter(topic, qos);
    ASSERT_NE(datawriter_2, nullptr);
    ASSERT_EQ(wait_for_log_entries(expected_entries, retries, wait_ms), expected_entries);

    /* Tear down */
    participant->delete_contained_entities();
    DomainParticipantFactory::get_instance()->delete_participant(participant);
    Log::KillThread();
}

class DataRepresentationTestsTypeSupport : public LoanableTypeSupport
{
public:

    bool is_bounded() const override
    {
        return true;
    }

    MOCK_CONST_METHOD1(custom_is_plain_with_rep, bool(DataRepresentationId_t data_representation_id));

    bool is_plain(
            DataRepresentationId_t data_representation_id) const override
    {
        return custom_is_plain_with_rep(data_representation_id);
    }

};

TEST(DataWriterTests, data_type_is_plain_data_representation)
{
    /* Create a participant, topic, and a publisher */
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0,
                    PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    DataRepresentationTestsTypeSupport* type = new DataRepresentationTestsTypeSupport();
    TypeSupport ts (type);
    ts.register_type(participant);

    Topic* topic = participant->create_topic("plain_topic", "LoanableType", TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(publisher, nullptr);

    /* Define default data representation (XCDR1) QoS to force "is_plain" call */
    DataWriterQos qos_xcdr = DATAWRITER_QOS_DEFAULT;
    qos_xcdr.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    /* Expect the "is_plain" method called with default data representation (XCDR1) */
    EXPECT_CALL(*type, custom_is_plain_with_rep(DataRepresentationId_t::XCDR_DATA_REPRESENTATION)).Times(
        testing::AtLeast(1)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*type, custom_is_plain_with_rep(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION)).Times(0);

    /* Create a datawriter will trigger the "is_plain" call */
    DataWriter* datawriter_xcdr = publisher->create_datawriter(topic, qos_xcdr);
    ASSERT_NE(datawriter_xcdr, nullptr);

    testing::Mock::VerifyAndClearExpectations(&type);

    /* Define XCDR2 data representation QoS to force "is_plain" call */
    DataWriterQos qos_xcdr2 = DATAWRITER_QOS_DEFAULT;
    qos_xcdr2.endpoint().history_memory_policy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    qos_xcdr2.representation().m_value.clear();
    qos_xcdr2.representation().m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    /* Expect the "is_plain" method called with XCDR2 data representation */
    EXPECT_CALL(*type, custom_is_plain_with_rep(DataRepresentationId_t::XCDR_DATA_REPRESENTATION)).Times(0);
    EXPECT_CALL(*type, custom_is_plain_with_rep(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION)).Times(
        testing::AtLeast(1)).WillRepeatedly(testing::Return(true));

    /* Create a datawriter will trigger the "is_plain" call */
    DataWriter* datawriter_xcdr2 = publisher->create_datawriter(topic, qos_xcdr2);
    ASSERT_NE(datawriter_xcdr2, nullptr);

    testing::Mock::VerifyAndClearExpectations(&type);

    /* Tear down */
    participant->delete_contained_entities();
    DomainParticipantFactory::get_instance()->delete_participant(participant);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
