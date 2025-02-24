// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/rpc/Replier.hpp>
#include <fastdds/dds/rpc/Requester.hpp>
#include <fastdds/dds/rpc/Service.hpp>
#include <fastdds/dds/rpc/ServiceTypeSupport.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

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

    TopicDataTypeMock()
        : TopicDataType()
    {
        max_serialized_type_size = 4u;
        set_name("footype");
    }

    bool serialize(
            const void* const /*data*/,
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            fastdds::dds::DataRepresentationId_t /*data_representation*/) override
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
            fastdds::dds::DataRepresentationId_t /*data_representation*/) override
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

private:

    using TopicDataType::calculate_serialized_size;
    using TopicDataType::serialize;
};

} // namespace dds

namespace dds {
namespace rpc {

TEST(ServiceTests, CreateRequesterValidParams)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    RequesterQos requester_qos;
    requester_qos.service_name = "Service";
    requester_qos.request_type = "ServiceType_Request";
    requester_qos.reply_type = "ServiceType_Reply";
    requester_qos.request_topic_name = "Service_Request";
    requester_qos.reply_topic_name = "Service_Reply";
    requester_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    requester_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_NE(requester, nullptr);
    ASSERT_NE(requester->get_requester_writer(), nullptr);
    ASSERT_NE(requester->get_requester_reader(), nullptr);

    EXPECT_EQ(requester->get_service_name(), "Service");
    EXPECT_EQ(requester->get_requester_writer()->get_type(), type);
    EXPECT_EQ(requester->get_requester_writer()->get_qos(), requester_qos.writer_qos);
    EXPECT_EQ(requester->get_requester_writer()->get_topic()->get_name(), "Service_Request");
    EXPECT_EQ(requester->get_requester_reader()->type(), type);
    EXPECT_EQ(requester->get_requester_reader()->get_qos(), requester_qos.reader_qos);
    EXPECT_EQ(requester->get_requester_reader()->get_topicdescription()->get_name(), "Service_ReplyFiltered");

    ASSERT_EQ(participant->delete_service_requester("Service", requester), RETCODE_OK);
    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateRequesterInvalidServiceName)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    RequesterQos requester_qos;
    requester_qos.service_name = "InvalidService";
    requester_qos.request_type = "ServiceType_Request";
    requester_qos.reply_type = "ServiceType_Reply";
    requester_qos.request_topic_name = "Service_Request";
    requester_qos.reply_topic_name = "Service_Reply";
    requester_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    requester_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_EQ(requester, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateRequesterInvalidRequestType)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    RequesterQos requester_qos;
    requester_qos.service_name = "Service";
    requester_qos.request_type = "InvalidType";
    requester_qos.reply_type = "ServiceType_Reply";
    requester_qos.request_topic_name = "Service_Request";
    requester_qos.reply_topic_name = "Service_Reply";
    requester_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    requester_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_EQ(requester, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateRequesterInvalidReplyType)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    RequesterQos requester_qos;
    requester_qos.service_name = "Service";
    requester_qos.request_type = "ServiceType_Request";
    requester_qos.reply_type = "InvalidType";
    requester_qos.request_topic_name = "Service_Request";
    requester_qos.reply_topic_name = "Service_Reply";
    requester_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    requester_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_EQ(requester, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateRequesterInvalidRequestTopicName)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    RequesterQos requester_qos;
    requester_qos.service_name = "Service";
    requester_qos.request_type = "ServiceType_Request";
    requester_qos.reply_type = "ServiceType_Reply";
    requester_qos.request_topic_name = "InvalidRequestTopic";
    requester_qos.reply_topic_name = "Service_Reply";
    requester_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    requester_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_EQ(requester, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateRequesterInvalidReplyTopicName)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    RequesterQos requester_qos;
    requester_qos.service_name = "Service";
    requester_qos.request_type = "ServiceType_Request";
    requester_qos.reply_type = "ServiceType_Reply";
    requester_qos.request_topic_name = "Service_Request";
    requester_qos.reply_topic_name = "InvalidReplyTopic";
    requester_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    requester_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_EQ(requester, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateRequesterInvalidWriterQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    RequesterQos requester_qos;
    requester_qos.service_name = "Service";
    requester_qos.request_type = "ServiceType_Request";
    requester_qos.reply_type = "ServiceType_Reply";
    requester_qos.request_topic_name = "Service_Request";
    requester_qos.reply_topic_name = "Service_Reply";
    requester_qos.writer_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    requester_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_EQ(requester, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateRequesterInvalidReaderQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    RequesterQos requester_qos;
    requester_qos.service_name = "Service";
    requester_qos.request_type = "ServiceType_Request";
    requester_qos.reply_type = "ServiceType_Reply";
    requester_qos.request_topic_name = "Service_Request";
    requester_qos.reply_topic_name = "Service_Reply";
    requester_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    requester_qos.reader_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_EQ(requester, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateReplierValidParams)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    ReplierQos replier_qos;
    replier_qos.service_name = "Service";
    replier_qos.request_type = "ServiceType_Request";
    replier_qos.reply_type = "ServiceType_Reply";
    replier_qos.request_topic_name = "Service_Request";
    replier_qos.reply_topic_name = "Service_Reply";
    replier_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    replier_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_NE(replier, nullptr);
    ASSERT_NE(replier->get_replier_writer(), nullptr);
    ASSERT_NE(replier->get_replier_reader(), nullptr);

    EXPECT_EQ(replier->get_service_name(), "Service");
    EXPECT_EQ(replier->get_replier_writer()->get_type(), type);
    EXPECT_EQ(replier->get_replier_writer()->get_qos(), replier_qos.writer_qos);
    EXPECT_EQ(replier->get_replier_writer()->get_topic()->get_name(), "Service_Reply");
    EXPECT_EQ(replier->get_replier_reader()->type(), type);
    EXPECT_EQ(replier->get_replier_reader()->get_qos(), replier_qos.reader_qos);
    EXPECT_EQ(replier->get_replier_reader()->get_topicdescription()->get_name(), "Service_Request");

    ASSERT_EQ(participant->delete_service_replier("Service", replier), RETCODE_OK);
    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateReplierInvalidServiceName)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    ReplierQos replier_qos;
    replier_qos.service_name = "InvalidService";
    replier_qos.request_type = "ServiceType_Request";
    replier_qos.reply_type = "ServiceType_Reply";
    replier_qos.request_topic_name = "Service_Request";
    replier_qos.reply_topic_name = "Service_Reply";
    replier_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    replier_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_EQ(replier, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateReplierInvalidRequestType)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    ReplierQos replier_qos;
    replier_qos.service_name = "Service";
    replier_qos.request_type = "InvalidType";
    replier_qos.reply_type = "ServiceType_Reply";
    replier_qos.request_topic_name = "Service_Request";
    replier_qos.reply_topic_name = "Service_Reply";
    replier_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    replier_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_EQ(replier, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateReplierInvalidReplyType)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    ReplierQos replier_qos;
    replier_qos.service_name = "Service";
    replier_qos.request_type = "ServiceType_Request";
    replier_qos.reply_type = "InvalidType";
    replier_qos.request_topic_name = "Service_Request";
    replier_qos.reply_topic_name = "Service_Reply";
    replier_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    replier_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_EQ(replier, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateReplierInvalidRequestTopicName)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    ReplierQos replier_qos;
    replier_qos.service_name = "Service";
    replier_qos.request_type = "ServiceType_Request";
    replier_qos.reply_type = "ServiceType_Reply";
    replier_qos.request_topic_name = "InvalidRequestTopic";
    replier_qos.reply_topic_name = "Service_Reply";
    replier_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    replier_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_EQ(replier, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateReplierInvalidReplyTopicName)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    ReplierQos replier_qos;
    replier_qos.service_name = "Service";
    replier_qos.request_type = "ServiceType_Request";
    replier_qos.reply_type = "ServiceType_Reply";
    replier_qos.request_topic_name = "Service_Request";
    replier_qos.reply_topic_name = "InvalidReplyTopic";
    replier_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    replier_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_EQ(replier, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateReplierInvalidWriterQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    ReplierQos replier_qos;
    replier_qos.service_name = "Service";
    replier_qos.request_type = "ServiceType_Request";
    replier_qos.reply_type = "ServiceType_Reply";
    replier_qos.request_topic_name = "Service_Request";
    replier_qos.reply_topic_name = "Service_Reply";
    replier_qos.writer_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    replier_qos.reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_EQ(replier, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, CreateReplierInvalidReaderQos)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);
    
    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    ReplierQos replier_qos;
    replier_qos.service_name = "Service";
    replier_qos.request_type = "ServiceType_Request";
    replier_qos.reply_type = "ServiceType_Reply";
    replier_qos.request_topic_name = "Service_Request";
    replier_qos.reply_topic_name = "Service_Reply";
    replier_qos.writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    replier_qos.reader_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_EQ(replier, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

} // namespace rpc
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
