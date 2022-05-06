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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/dds/builtin/topic/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/AnyDataWriter.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>

#include "../../logging/mock/MockConsumer.h"

#include <fastdds/publisher/DataWriterImpl.hpp>

#include <mutex>
#include <condition_variable>

namespace eprosima {
namespace fastdds {
namespace dds {

using namespace eprosima::fastrtps::rtps;
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
        m_typeSize = 4u;
        setName("footype");
    }

    bool serialize(
            void* /*data*/,
            fastrtps::rtps::SerializedPayload_t* /*payload*/) override
    {
        return true;
    }

    bool deserialize(
            fastrtps::rtps::SerializedPayload_t* /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/) override
    {
        return std::function<uint32_t()>();
    }

    void* createData() override
    {
        return nullptr;
    }

    void deleteData(
            void* /*data*/) override
    {
    }

    bool getKey(
            void* /*data*/,
            fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
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
        m_typeSize = 4u;
        m_isGetKeyDefined = true;
        setName("instancefootype");
    }

    bool serialize(
            void* /*data*/,
            fastrtps::rtps::SerializedPayload_t* /*payload*/) override
    {
        return true;
    }

    bool deserialize(
            fastrtps::rtps::SerializedPayload_t* /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/) override
    {
        return std::function<uint32_t()>();
    }

    void* createData() override
    {
        return nullptr;
    }

    void deleteData(
            void* /*data*/) override
    {
    }

    bool getKey(
            void* /*data*/,
            fastrtps::rtps::InstanceHandle_t* ihandle,
            bool /*force_md5*/) override
    {
        ihandle->value[0] = 1;
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
        m_typeSize = 4u;
        setName("bounded_footype");
    }

    bool serialize(
            void* /*data*/,
            fastrtps::rtps::SerializedPayload_t* /*payload*/) override
    {
        return true;
    }

    bool deserialize(
            fastrtps::rtps::SerializedPayload_t* /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/) override
    {
        return std::function<uint32_t()>();
    }

    void* createData() override
    {
        return nullptr;
    }

    void deleteData(
            void* /*data*/) override
    {
    }

    bool getKey(
            void* /*data*/,
            fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
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

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

/*!
 * This test checks `DataWriter::get_guid` function works when the entity was created but not enabled.
 */
TEST(DataWriterTests, get_guid)
{
    class DiscoveryListener : public DomainParticipantListener
    {
    public:

        void on_publisher_discovery(
                DomainParticipant*,
                fastrtps::rtps::WriterDiscoveryInfo&& info)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER == info.status)
            {
                guid = info.info.guid();
                cv.notify_one();
            }
        }

        fastrtps::rtps::GUID_t guid;
        std::mutex mutex;
        std::condition_variable cv;
    }
    discovery_listener;

    DomainParticipantQos participant_qos = PARTICIPANT_QOS_DEFAULT;
    participant_qos.wire_protocol().builtin.discovery_config.ignoreParticipantFlags =
            static_cast<eprosima::fastrtps::rtps::ParticipantFilteringFlags_t>(
        eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_DIFFERENT_HOST |
        eprosima::fastrtps::rtps::ParticipantFilteringFlags_t::FILTER_DIFFERENT_PROCESS);

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

    fastrtps::rtps::GUID_t guid = datawriter->guid();

    participant->enable();

    factory_qos.entity_factory().autoenable_created_entities = true;
    DomainParticipantFactory::get_instance()->set_qos(factory_qos);

    {
        std::unique_lock<std::mutex> lock(discovery_listener.mutex);
        discovery_listener.cv.wait(lock, [&]()
                {
                    return fastrtps::rtps::GUID_t::unknown() != discovery_listener.guid;
                });
    }
    ASSERT_EQ(guid, discovery_listener.guid);

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(
                listener_participant) == ReturnCode_t::RETCODE_OK);
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

    ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    DataWriterQos wqos;
    datawriter->get_qos(wqos);

    ASSERT_EQ(qos, wqos);
    ASSERT_EQ(wqos.deadline().period, 260);

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
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
    qos.endpoint().history_memory_policy = fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
    datawriter = publisher->create_datawriter(topic, qos);
    ASSERT_NE(datawriter, nullptr);
    ASSERT_EQ(publisher->delete_datawriter(datawriter), ReturnCode_t::RETCODE_OK);

    // DataSharing automatic, bounded topic data type
    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_NE(datawriter, nullptr);
    ASSERT_EQ(publisher->delete_datawriter(datawriter), ReturnCode_t::RETCODE_OK);

    // DataSharing enabled, unbounded topic data type
    qos = DATAWRITER_QOS_DEFAULT;
    qos.endpoint().history_memory_policy = fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
    qos.data_sharing().on(".");
    datawriter = publisher->create_datawriter(topic, qos);
    ASSERT_EQ(datawriter, nullptr);

    // DataSharing enabled, bounded topic data type
    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_NE(datawriter, nullptr);
    ASSERT_EQ(publisher->delete_datawriter(datawriter), ReturnCode_t::RETCODE_OK);

    // DataSharing enabled, bounded topic data type, Dynamic memory policy
    qos = DATAWRITER_QOS_DEFAULT;
    qos.data_sharing().on(".");
    qos.endpoint().history_memory_policy = fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_EQ(datawriter, nullptr);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(bounded_topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);


    // DataSharing forced, bounded topic data type, security enabled
    static const char* certs_path = std::getenv("CERTS_PATH");
    if (certs_path == nullptr)
    {
        std::cout << "Cannot get enviroment variable CERTS_PATH" << std::endl;
        ASSERT_TRUE(false);
    }

#ifdef HAS_SECURITY
    fastrtps::rtps::PropertyPolicy security_property;
    security_property.properties().emplace_back(fastrtps::rtps::Property("dds.sec.auth.plugin",
            "builtin.PKI-DH"));
    security_property.properties().emplace_back(fastrtps::rtps::Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    security_property.properties().emplace_back(fastrtps::rtps::Property(
                "dds.sec.auth.builtin.PKI-DH.identity_certificate",
                "file://" + std::string(certs_path) + "/mainsubcert.pem"));
    security_property.properties().emplace_back(fastrtps::rtps::Property("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainsubkey.pem"));
    security_property.properties().emplace_back(fastrtps::rtps::Property("dds.sec.crypto.plugin",
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
    qos.endpoint().history_memory_policy = fastrtps::rtps::PREALLOCATED_MEMORY_MODE;


    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_EQ(datawriter, nullptr);

    ASSERT_EQ(participant->delete_topic(bounded_topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
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
    const ReturnCode_t unsupported_code = ReturnCode_t::RETCODE_UNSUPPORTED;

    DataWriterQos qos;
    qos = DATAWRITER_QOS_DEFAULT;
    qos.durability().kind = PERSISTENT_DURABILITY_QOS;
    EXPECT_EQ(unsupported_code, datawriter->set_qos(qos));

    qos = DATAWRITER_QOS_DEFAULT;
    qos.destination_order().kind = BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    EXPECT_EQ(unsupported_code, datawriter->set_qos(qos));

    qos = DATAWRITER_QOS_DEFAULT;
    qos.properties().properties().emplace_back("fastdds.unique_network_flows", "");
    EXPECT_EQ(unsupported_code, datawriter->set_qos(qos));

    /* Inconsistent QoS */
    const ReturnCode_t inconsistent_code = ReturnCode_t::RETCODE_INCONSISTENT_POLICY;

    qos = DATAWRITER_QOS_DEFAULT;
    qos.properties().properties().emplace_back("fastdds.push_mode", "false");
    qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos));

    qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    qos.reliable_writer_qos().times.heartbeatPeriod = eprosima::fastrtps::c_TimeInfinite;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos));

    qos = DATAWRITER_QOS_DEFAULT;
    qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    qos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;
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
    qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos));

    qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::DYNAMIC_REUSABLE_MEMORY_MODE;
    EXPECT_EQ(inconsistent_code, datawriter->set_qos(qos));

    qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->set_qos(qos));

    qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->set_qos(qos));

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
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
    ASSERT_TRUE(datawriter->write(&data, fastrtps::rtps::c_InstanceHandle_Unknown) ==
            ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(datawriter->write(&data, participant->get_instance_handle()) ==
            ReturnCode_t::RETCODE_PRECONDITION_NOT_MET);

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

void set_listener_test (
        DataWriter* writer,
        DataWriterListener* listener,
        StatusMask mask)
{
    ASSERT_EQ(writer->set_listener(listener, mask), ReturnCode_t::RETCODE_OK);
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

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
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
 * This test checks unregister_instance API
 */
TEST(DataWriterTests, UnregisterInstance)
{
    // Test parameters
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

    // 1. Calling unregister_instance in a disable writer returns RETCODE_NOT_ENABLED
    EXPECT_EQ(ReturnCode_t::RETCODE_NOT_ENABLED, datawriter->unregister_instance(&data, handle));

    // 2. Calling unregister_instance in a non keyed topic returns RETCODE_PRECONDITION_NOT MET
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, datawriter->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, datawriter->unregister_instance(&data, handle));

    // 3. Calling unregister_instance with an invalid sample returns RETCODE_BAD_PARAMETER
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->unregister_instance(nullptr, handle));

#if !defined(NDEBUG)
    // 4. Calling unregister_instance with an inconsistent handle returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, instance_datawriter->unregister_instance(&data,
            datawriter->get_instance_handle()));
#endif // NDEBUG

    // 5. Calling unregister_instance with a key not yet registered returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, instance_datawriter->unregister_instance(&data, handle));

    // 6. Calling unregister_instance with a valid key returns RETCODE_OK
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->write(&data, c_InstanceHandle_Unknown));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->unregister_instance(&data, handle));

    // 7. Calling unregister_instance with a valid InstanceHandle also returns RETCODE_OK
    data.message("HelloWorld_1");
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->write(&data, c_InstanceHandle_Unknown));
    instance_type.get_key(&data, &handle);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->unregister_instance(&data, handle));

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

    // 1. Calling dispose in a disable writer returns RETCODE_NOT_ENABLED
    EXPECT_EQ(ReturnCode_t::RETCODE_NOT_ENABLED, datawriter->dispose(&data, handle));

    // 2. Calling dispose in a non keyed topic returns RETCODE_PRECONDITION_NOT MET
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, datawriter->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, datawriter->dispose(&data, handle));

    // 3. Calling dispose with an invalid sample returns RETCODE_BAD_PARAMETER
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->dispose(nullptr, handle));

#if !defined(NDEBUG)
    // 4. Calling dispose with an inconsistent handle returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, instance_datawriter->dispose(&data,
            datawriter->get_instance_handle()));
#endif // NDEBUG

    // 5. Calling dispose with a key not yet registered returns RETCODE_PRECONDITION_NOT_MET
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, instance_datawriter->dispose(&data, handle));

    // 6. Calling dispose with a valid key returns RETCODE_OK
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->write(&data, c_InstanceHandle_Unknown));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->dispose(&data, handle));

    // 7. Calling dispose with a valid InstanceHandle also returns RETCODE_OK
    data.message("HelloWorld_1");
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->write(&data, c_InstanceHandle_Unknown));
    instance_type.get_key(&data, &handle);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->dispose(&data, handle));

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

    // 1. Check nullptr on key_holder
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, datawriter->get_key_value(nullptr, wrong_handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(nullptr, wrong_handle));

    // 2. Check HANDLE_NIL
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, datawriter->get_key_value(&data, HANDLE_NIL));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, HANDLE_NIL));

    // 3. Check type should have keys, and writer should be enabled
    EXPECT_EQ(ReturnCode_t::RETCODE_ILLEGAL_OPERATION, datawriter->get_key_value(&data, wrong_handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_NOT_ENABLED, instance_datawriter->get_key_value(&data, wrong_handle));

    ASSERT_EQ(ReturnCode_t::RETCODE_OK, datawriter->enable());
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->enable());

    // 4. Calling get_key_value with a key not yet registered returns RETCODE_BAD_PARAMETER
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, wrong_handle));

    // 5. Calling get_key_value on a registered instance should work.
    valid_handle = instance_datawriter->register_instance(&valid_data);
    EXPECT_NE(HANDLE_NIL, valid_handle);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));

    // 6. Calling get_key_value on an unregistered instance should return RETCODE_BAD_PARAMETER.
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->unregister_instance(&valid_data, valid_handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->get_key_value(&data, valid_handle));

    // 7. Calling get_key_value with a valid instance should work
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->write(&valid_data, c_InstanceHandle_Unknown));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));

    // 8. Calling get_key_value on a disposed instance should work.
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->dispose(&valid_data, valid_handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->get_key_value(&data, valid_handle));
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

    LoanableTypeSupport()
        : TopicDataType()
    {
        m_typeSize = 4u + sizeof(LoanableType);
        setName("LoanableType");
    }

    bool serialize(
            void* /*data*/,
            fastrtps::rtps::SerializedPayload_t* /*payload*/) override
    {
        return true;
    }

    bool deserialize(
            fastrtps::rtps::SerializedPayload_t* /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/) override
    {
        return [this]()
               {
                   return m_typeSize;
               };
    }

    void* createData() override
    {
        return nullptr;
    }

    void deleteData(
            void* /*data*/) override
    {
    }

    bool getKey(
            void* /*data*/,
            fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

    bool is_bounded() const override
    {
        return true;
    }

    bool is_plain() const override
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
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->loan_sample(sample, InitKind::NO_LOAN_INITIALIZATION));
    EXPECT_NE(nullptr, sample);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->discard_loan(sample));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, datawriter->discard_loan(sample));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->loan_sample(sample, InitKind::ZERO_LOAN_INITIALIZATION));
    ASSERT_NE(nullptr, sample);
    EXPECT_EQ(0u, static_cast<LoanableType*>(sample)->index);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->discard_loan(sample));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, datawriter->discard_loan(sample));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->loan_sample(sample, InitKind::CONSTRUCTED_LOAN_INITIALIZATION));
    ASSERT_NE(nullptr, sample);
    EXPECT_EQ(LoanableType::initialization_value(), static_cast<LoanableType*>(sample)->index);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->discard_loan(sample));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, datawriter->discard_loan(sample));

    // Resource limits:
    // Depth has been configured to 1, so pool will allow up to depth + 1 loans.
    // We will check that the 3rd unreturned loan returns OUT_OF_RESOURCES.
    void* sample_2 = nullptr;
    void* sample_3 = nullptr;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->loan_sample(sample));
    EXPECT_NE(nullptr, sample);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->loan_sample(sample_2));
    EXPECT_NE(nullptr, sample_2);
    EXPECT_EQ(ReturnCode_t::RETCODE_OUT_OF_RESOURCES, datawriter->loan_sample(sample_3));
    EXPECT_EQ(nullptr, sample_3);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->discard_loan(sample_2));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->discard_loan(sample));

    // Write samples, both loaned and not
    LoanableType data;
    fastrtps::rtps::InstanceHandle_t handle;
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->loan_sample(sample));
    EXPECT_NE(nullptr, sample);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->loan_sample(sample_2));
    EXPECT_NE(nullptr, sample_2);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->write(sample, handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, datawriter->discard_loan(sample));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->write(sample_2, handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, datawriter->discard_loan(sample_2));

    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->write(&data, handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->loan_sample(sample));
    EXPECT_NE(nullptr, sample);
    EXPECT_EQ(ReturnCode_t::RETCODE_OUT_OF_RESOURCES, datawriter->write(&data, handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->discard_loan(sample));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->write(&data, handle));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->write(&data, handle));

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
}

class LoanableTypeSupportTesting : public LoanableTypeSupport
{
public:

    bool is_plain_result = true;
    bool construct_sample_result = true;

    bool is_plain() const override
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
    auto original_type_size = type_support->m_typeSize;

    // Check for illegal operation
    type_support->is_plain_result = false;
    EXPECT_EQ(ReturnCode_t::RETCODE_ILLEGAL_OPERATION, datawriter->loan_sample(sample));
    type_support->is_plain_result = true;
    type_support->m_typeSize = 0;
    EXPECT_EQ(ReturnCode_t::RETCODE_ILLEGAL_OPERATION, datawriter->loan_sample(sample));
    type_support->m_typeSize = original_type_size;

    // Check for not enabled
    EXPECT_EQ(ReturnCode_t::RETCODE_NOT_ENABLED, datawriter->loan_sample(sample));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->enable());

    // Check for constructor support
    type_support->construct_sample_result = false;
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED,
            datawriter->loan_sample(sample, InitKind::CONSTRUCTED_LOAN_INITIALIZATION));
    type_support->construct_sample_result = true;

    // Check preconditions on delete_datawriter
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->loan_sample(sample));
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, publisher->delete_datawriter(datawriter));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, datawriter->discard_loan(sample));

    ASSERT_TRUE(publisher->delete_datawriter(datawriter) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_topic(topic) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(participant->delete_publisher(publisher) == ReturnCode_t::RETCODE_OK);
    ASSERT_TRUE(DomainParticipantFactory::get_instance()->delete_participant(participant) == ReturnCode_t::RETCODE_OK);
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
        return &history_;
    }

};

/**
 * This test checks instance wait_for_acknowledgements API
 */
TEST(DataWriterTests, InstanceWaitForAcknowledgement)
{
    // Test parameters
    Duration_t max_wait(2, 0);
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
    EXPECT_EQ(ReturnCode_t::RETCODE_NOT_ENABLED, datawriter->wait_for_acknowledgments(&data, handle, max_wait));

    // 2. Calling wait_for_acknowledgments in a non keyed topic returns RETCODE_PRECONDITION_NOT MET
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, datawriter->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, datawriter->wait_for_acknowledgments(&data, handle,
            max_wait));

    // 3. Calling wait_for_acknowledgments with an invalid sample returns RETCODE_BAD_PARAMETER
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->enable());
    EXPECT_EQ(ReturnCode_t::RETCODE_BAD_PARAMETER, instance_datawriter->wait_for_acknowledgments(nullptr, handle,
            max_wait));

#if !defined(NDEBUG)
    // 4. Calling wait_for_acknowledgments with an inconsistent handle returns RETCODE_BAD_PARAMETER
    EXPECT_EQ(ReturnCode_t::RETCODE_PRECONDITION_NOT_MET, instance_datawriter->wait_for_acknowledgments(&data,
            datawriter->get_instance_handle(), max_wait));
#endif // NDEBUG

    // Access PublisherHistory
    DataWriterTest* instance_datawriter_test = static_cast<DataWriterTest*>(instance_datawriter);
    ASSERT_NE(nullptr, instance_datawriter_test);
    DataWriterImpl* datawriter_impl = instance_datawriter_test->get_impl();
    ASSERT_NE(nullptr, datawriter_impl);
    DataWriterImplTest* datawriter_impl_test = static_cast<DataWriterImplTest*>(datawriter_impl);
    ASSERT_NE(nullptr, datawriter_impl_test);
    auto history = datawriter_impl_test->get_history();

    // 5. Calling wait_for_acknowledgments in a keyed topic with c_InstanceHandle_Unknown returns
    // RETCODE_OK
    EXPECT_CALL(*history, wait_for_acknowledgement_last_change(_, _, _)).WillOnce(testing::Return(true));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->write(&data, c_InstanceHandle_Unknown));
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->wait_for_acknowledgments(&data, handle,
            max_wait));

    // 6. Calling wait_for_acknowledgments in a keyed topic with a known handle returns RETCODE_OK (no matched readers)
    // Expectations
    EXPECT_CALL(*history, wait_for_acknowledgement_last_change(_, _, _)).WillOnce(testing::Return(true));

    instance_type.get_key(&data, &handle);
    EXPECT_EQ(ReturnCode_t::RETCODE_OK, instance_datawriter->wait_for_acknowledgments(&data, handle, max_wait));

    // 7. Calling wait_for_acknowledgments in a keyed topic with a known handle timeouts if some reader has not
    // acknowledged before max_wait time (mock) returns RETCODE_TIMEOUT
    // Expectations
    EXPECT_CALL(*history, wait_for_acknowledgement_last_change(_, _, _)).WillOnce(testing::Return(false));
    EXPECT_EQ(ReturnCode_t::RETCODE_TIMEOUT, instance_datawriter->wait_for_acknowledgments(&data, handle, max_wait));
}

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
 * ReturnCode_t::RETCODE_UNSUPPORTED. The following methods are checked:
 * 1. get_matched_subscription_data
 * 2. get_matched_subscriptions
 * 3. lookup_instance
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

    builtin::SubscriptionBuiltinTopicData subscription_data;
    fastrtps::rtps::InstanceHandle_t subscription_handle;
    EXPECT_EQ(
        ReturnCode_t::RETCODE_UNSUPPORTED,
        data_writer->get_matched_subscription_data(subscription_data, subscription_handle));

    std::vector<InstanceHandle_t> subscription_handles;
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, data_writer->get_matched_subscriptions(subscription_handles));

    EXPECT_EQ(HANDLE_NIL, data_writer->lookup_instance(nullptr /* instance */));

    // Expected logWarnings: lookup_instance
    HELPER_WaitForEntries(1);

    ASSERT_EQ(publisher->delete_datawriter(data_writer), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
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
