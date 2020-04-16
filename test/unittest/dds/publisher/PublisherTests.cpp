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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/qos/PublisherQos.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {


TEST(PublisherTests, GetPublisherParticipant)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    ASSERT_EQ(publisher->get_participant(), participant);
}

TEST(PublisherTests, GetPSMPublisherParticipant)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::pub::Publisher publisher = ::dds::pub::Publisher(participant, PUBLISHER_QOS_DEFAULT);

    ASSERT_EQ(publisher.participant().delegate().get(), participant.delegate().get());

}

TEST(PublisherTests, ChangeDefaultDataWriterQos)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    DataWriterQos qos;
    publisher->get_default_datawriter_qos(qos);
    ASSERT_EQ(qos, DATAWRITER_QOS_DEFAULT);

    qos.deadline().period = 260;

    ASSERT_TRUE(publisher->set_default_datawriter_qos(qos) == ReturnCode_t::RETCODE_OK);
    DataWriterQos wqos;
    publisher->get_default_datawriter_qos(wqos);

    ASSERT_TRUE(qos == wqos);
    ASSERT_EQ(wqos.deadline().period, 260);
}


TEST(PublisherTests, ChangePSMDefaultDataWriterQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);
    ::dds::pub::Publisher publisher = ::dds::pub::Publisher(participant);

    ::dds::pub::qos::DataWriterQos qos = publisher.default_datawriter_qos();
    ASSERT_EQ(qos, DATAWRITER_QOS_DEFAULT);

    qos.deadline().period = 540;

    ASSERT_NO_THROW(publisher.default_datawriter_qos(qos));
    ::dds::pub::qos::DataWriterQos wqos = publisher.default_datawriter_qos();

    ASSERT_TRUE(qos == wqos);
    ASSERT_EQ(wqos.deadline().period, 540);
}

TEST(PublisherTests, ChangePublisherQos)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    PublisherQos qos;
    publisher->get_qos(qos);

    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;

    publisher->set_qos(qos);
    PublisherQos pqos;
    publisher->get_qos(pqos);

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);

}

TEST(PublisherTests, ChangePSMPublisherQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0);
    ::dds::pub::Publisher publisher = ::dds::pub::Publisher(participant);

    ::dds::pub::qos::PublisherQos qos = publisher.qos();
    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory().autoenable_created_entities = false;
    publisher.qos(qos);
    ::dds::pub::qos::PublisherQos pqos = publisher.qos();

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory().autoenable_created_entities, false);
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
