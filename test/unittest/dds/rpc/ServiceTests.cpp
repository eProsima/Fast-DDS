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
    ASSERT_EQ(participant->delete_contained_entities(), RETCODE_OK);
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
    requester_qos.writer_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_EQ(requester, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(participant->delete_contained_entities(), RETCODE_OK);
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
    requester_qos.reader_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_EQ(requester, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(participant->delete_contained_entities(), RETCODE_OK);
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
    ASSERT_EQ(participant->delete_contained_entities(), RETCODE_OK);
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
    replier_qos.writer_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_EQ(replier, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(participant->delete_contained_entities(), RETCODE_OK);
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
    replier_qos.reader_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_EQ(replier, nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(participant->delete_contained_entities(), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, ChangeEnabledState)
{
    Duration_t find_topic_timeout = Duration_t(5, 0);

    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);

    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);
    ASSERT_EQ(service->is_enabled(), true);

    Topic* request_topic = participant->find_topic("Service_Request", find_topic_timeout);
    Topic* reply_topic = participant->find_topic("Service_Reply", find_topic_timeout);
    ASSERT_NE(request_topic, nullptr);
    ASSERT_NE(reply_topic, nullptr);
    // Release resources
    ASSERT_EQ(participant->delete_topic(request_topic), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(reply_topic), RETCODE_OK);

    ReplierQos replier_qos;
    RequesterQos requester_qos;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_NE(replier, nullptr);
    // Service is enabled, so the replier is created in enabled state by default
    ASSERT_EQ(replier->is_enabled(), true);
    ASSERT_NE(replier->get_replier_writer(), nullptr);
    ASSERT_NE(replier->get_replier_reader(), nullptr);

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_NE(requester, nullptr);
    // Service is enabled, so the requester is created in enabled state by default
    ASSERT_EQ(requester->is_enabled(), true);
    ASSERT_NE(requester->get_requester_writer(), nullptr);
    ASSERT_NE(requester->get_requester_reader(), nullptr);

    // Now disable the service.
    // It should destroy Request/Reply topics and disable all its internal Requester/Replier entities
    ASSERT_EQ(service->close(), RETCODE_OK);
    ASSERT_EQ(service->is_enabled(), false);
    ASSERT_EQ(participant->find_topic("Service_Request", find_topic_timeout), nullptr);
    ASSERT_EQ(participant->find_topic("Service_Reply", find_topic_timeout), nullptr);

    // If requester and replier are disabled, they should not have any writer/reader
    ASSERT_EQ(replier->is_enabled(), false);
    ASSERT_EQ(replier->get_replier_writer(), nullptr);
    ASSERT_EQ(replier->get_replier_reader(), nullptr);
    ASSERT_EQ(requester->is_enabled(), false);
    ASSERT_EQ(requester->get_requester_writer(), nullptr);
    ASSERT_EQ(requester->get_requester_reader(), nullptr);

    // Create a new requester and replier.
    // Now Service is disabled, so they should be created in disabled state
    Replier* replier_2 = participant->create_service_replier(service, replier_qos);
    ASSERT_NE(replier_2, nullptr);
    ASSERT_EQ(replier_2->is_enabled(), false);
    ASSERT_EQ(replier_2->get_replier_writer(), nullptr);
    ASSERT_EQ(replier_2->get_replier_reader(), nullptr);

    Requester* requester_2 = participant->create_service_requester(service, requester_qos);
    ASSERT_NE(requester_2, nullptr);
    ASSERT_EQ(requester_2->is_enabled(), false);
    ASSERT_EQ(requester_2->get_requester_writer(), nullptr);
    ASSERT_EQ(requester_2->get_requester_reader(), nullptr);

    // Enable the service again.
    // It should create Request/Reply topics and enable all its internal Requester/Replier entities.
    // The requester and replier created while the service was disabled should be enabled now.
    ASSERT_EQ(service->enable(), RETCODE_OK);
    ASSERT_EQ(service->is_enabled(), true);
    request_topic = participant->find_topic("Service_Request", find_topic_timeout);
    reply_topic = participant->find_topic("Service_Reply", find_topic_timeout);
    ASSERT_NE(request_topic, nullptr);
    ASSERT_NE(reply_topic, nullptr);
    // Release resources
    ASSERT_EQ(participant->delete_topic(request_topic), RETCODE_OK);
    ASSERT_EQ(participant->delete_topic(reply_topic), RETCODE_OK);

    ASSERT_EQ(replier->is_enabled(), true);
    ASSERT_NE(replier->get_replier_writer(), nullptr);
    ASSERT_NE(replier->get_replier_reader(), nullptr);
    ASSERT_EQ(requester->is_enabled(), true);
    ASSERT_NE(requester->get_requester_writer(), nullptr);
    ASSERT_NE(requester->get_requester_reader(), nullptr);
    ASSERT_EQ(replier_2->is_enabled(), true);
    ASSERT_NE(replier_2->get_replier_writer(), nullptr);
    ASSERT_NE(replier_2->get_replier_reader(), nullptr);
    ASSERT_EQ(requester_2->is_enabled(), true);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(participant->delete_contained_entities(), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, EnableEntityOnDisabledService)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);

    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);
    ASSERT_EQ(service->is_enabled(), true);

    // Disable the service
    ASSERT_EQ(service->close(), RETCODE_OK);
    ASSERT_EQ(service->is_enabled(), false);

    // Create a requester and a replier, and try to enable them
    // They should not be enabled because the service is disabled
    ReplierQos replier_qos;
    RequesterQos requester_qos;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_NE(replier, nullptr);
    ASSERT_EQ(replier->is_enabled(), false);
    ASSERT_EQ(replier->enable(), RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(replier->is_enabled(), false);
    ASSERT_EQ(replier->get_replier_writer(), nullptr);
    ASSERT_EQ(replier->get_replier_reader(), nullptr);

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_NE(requester, nullptr);
    ASSERT_EQ(requester->is_enabled(), false);
    ASSERT_EQ(requester->enable(), RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(requester->is_enabled(), false);
    ASSERT_EQ(requester->get_requester_writer(), nullptr);
    ASSERT_EQ(requester->get_requester_reader(), nullptr);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(participant->delete_contained_entities(), RETCODE_OK);
    ASSERT_EQ(DomainParticipantFactory::get_instance()->delete_participant(participant), RETCODE_OK);
}

TEST(ServiceTests, SendOrTakeOnDisabledRequesterReplier)
{
    DomainParticipant* participant =
            DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(participant, nullptr);

    TypeSupport type(new TopicDataTypeMock());
    ServiceTypeSupport service_type(type, type);
    ASSERT_EQ(participant->register_service_type(service_type, "ServiceType"), RETCODE_OK);

    Service* service = participant->create_service("Service", "ServiceType");
    ASSERT_NE(service, nullptr);

    // Disable the service
    ASSERT_EQ(service->close(), RETCODE_OK);

    // Create a requester and a replier, and try to send/take on them
    // It should return RETCODE_PRECONDITION_NOT_MET because the service is disabled
    ReplierQos replier_qos;
    RequesterQos requester_qos;

    Replier* replier = participant->create_service_replier(service, replier_qos);
    ASSERT_NE(replier, nullptr);

    Requester* requester = participant->create_service_requester(service, requester_qos);
    ASSERT_NE(requester, nullptr);

    void* data = nullptr;
    RequestInfo info;

    ASSERT_EQ(requester->send_request(data, info), RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(replier->send_reply(data, info), RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(requester->take_reply(data, info), RETCODE_PRECONDITION_NOT_MET);
    ASSERT_EQ(replier->take_request(data, info), RETCODE_PRECONDITION_NOT_MET);

    ASSERT_EQ(participant->delete_service(service), RETCODE_OK);
    ASSERT_EQ(participant->delete_contained_entities(), RETCODE_OK);
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
