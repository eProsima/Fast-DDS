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
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/AnyDataWriter.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>

#include "../../logging/mock/MockConsumer.h"

namespace eprosima {
namespace fastdds {
namespace dds {

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

    inline bool is_bounded() const
    {
        return true;
    }
};

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

TEST(DataWriterTests, FocedDataSharing)
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

    Topic* bounded_topic = participant->create_topic("bounded_footopic", bounded_type.get_type_name(), TOPIC_QOS_DEFAULT);
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
    qos.data_sharing().on("path");
    datawriter = publisher->create_datawriter(topic, qos);
    ASSERT_EQ(datawriter, nullptr);

    // DataSharing enabled, bounded topic data type
    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_NE(datawriter, nullptr);
    ASSERT_EQ(publisher->delete_datawriter(datawriter), ReturnCode_t::RETCODE_OK);

    // DataSharing enabled, bounded topic data type, Dynamic memory policy
    qos = DATAWRITER_QOS_DEFAULT;
    qos.data_sharing().on("path");
    qos.endpoint().history_memory_policy = fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_EQ(datawriter, nullptr);

    ASSERT_EQ(participant->delete_topic(topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(bounded_topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);


    // DataSharing forced, bounded topic data type, security enabled
    static const char* certs_path = std::getenv("CERTS_PATH");

    fastrtps::rtps::PropertyPolicy security_property;
    security_property.properties().emplace_back(fastrtps::rtps::Property("dds.sec.auth.plugin",
            "builtin.PKI-DH"));
    security_property.properties().emplace_back(fastrtps::rtps::Property("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem"));
    security_property.properties().emplace_back(fastrtps::rtps::Property("dds.sec.auth.builtin.PKI-DH.identity_certificate",
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
    qos.data_sharing().on("path");
    qos.endpoint().history_memory_policy = fastrtps::rtps::PREALLOCATED_MEMORY_MODE;


    datawriter = publisher->create_datawriter(bounded_topic, qos);
    ASSERT_EQ(datawriter, nullptr);

    ASSERT_EQ(participant->delete_topic(bounded_topic), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(participant->delete_publisher(publisher), ReturnCode_t::RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), ReturnCode_t::RETCODE_OK);
}

TEST(DataWriterTests, InvalidQos)
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.entity_factory().autoenable_created_entities = false;
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, pqos);
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
    ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_UNSUPPORTED);

    qos = DATAWRITER_QOS_DEFAULT;
    qos.destination_order().kind = BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_UNSUPPORTED);

    qos = DATAWRITER_QOS_DEFAULT;
    qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    qos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;
    ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_INCONSISTENT_POLICY);

    qos = DATAWRITER_QOS_DEFAULT;
    qos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;
    qos.liveliness().announcement_period = 20;
    qos.liveliness().lease_duration = 10;
    ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_INCONSISTENT_POLICY);

    qos.liveliness().kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_INCONSISTENT_POLICY);

    qos = DATAWRITER_QOS_DEFAULT;
    qos.data_sharing().on("/tmp");
    qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::DYNAMIC_RESERVE_MEMORY_MODE;
    ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_INCONSISTENT_POLICY);

    qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::DYNAMIC_REUSABLE_MEMORY_MODE;
    ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_INCONSISTENT_POLICY);

    qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_MEMORY_MODE;
    ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_OK);

    qos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    ASSERT_TRUE(datawriter->set_qos(qos) == ReturnCode_t::RETCODE_OK);

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
 * 1. get_publication_matched_status
 * 2. get_matched_subscription_data
 * 3. write_w_timestamp
 * 4. register_instance_w_timestamp
 * 5. unregister_instance_w_timestamp
 * 6. get_matched_subscriptions
 * 7. get_key_value
 * 8. lookup_instance
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

    PublicationMatchedStatus status;
    EXPECT_EQ(
        ReturnCode_t::RETCODE_UNSUPPORTED,
        data_writer->get_publication_matched_status(status));

    builtin::SubscriptionBuiltinTopicData subscription_data;
    fastrtps::rtps::InstanceHandle_t subscription_handle;
    EXPECT_EQ(
        ReturnCode_t::RETCODE_UNSUPPORTED,
        data_writer->get_matched_subscription_data(subscription_data, subscription_handle));

    {
        InstanceHandle_t handle;
        fastrtps::rtps::Time_t timestamp;
        EXPECT_EQ(
            ReturnCode_t::RETCODE_UNSUPPORTED,
            data_writer->write_w_timestamp(nullptr /* data */, handle, timestamp));
    }

    {
        fastrtps::rtps::Time_t timestamp;
        EXPECT_EQ(
            HANDLE_NIL,
            data_writer->register_instance_w_timestamp(nullptr /* instance */, timestamp));
    }

    {
        InstanceHandle_t handle;
        fastrtps::rtps::Time_t timestamp;
        EXPECT_EQ(
            ReturnCode_t::RETCODE_UNSUPPORTED,
            data_writer->unregister_instance_w_timestamp(nullptr /* instance */, handle, timestamp));
    }


    std::vector<fastrtps::rtps::InstanceHandle_t*> subscription_handles;
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, data_writer->get_matched_subscriptions(subscription_handles));

    fastrtps::rtps::InstanceHandle_t key_handle;
    EXPECT_EQ(ReturnCode_t::RETCODE_UNSUPPORTED, data_writer->get_key_value(nullptr /* key_holder */, key_handle));

    EXPECT_EQ(HANDLE_NIL, data_writer->lookup_instance(nullptr /* instance */));

    // Expected logWarnings: register_instance_w_timestamp, lookup_instance
    HELPER_WaitForEntries(2);

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
