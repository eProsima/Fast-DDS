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

namespace eprosima {
namespace fastdds {
namespace dds {

TEST(PublisherTests, ChangePublisherQos)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    PublisherQos qos;
    ASSERT_TRUE(publisher->get_qos(qos) == ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory.autoenable_created_entities = false;

    ASSERT_TRUE(publisher->set_qos(qos) == ReturnCode_t::RETCODE_OK);
    PublisherQos pqos;
    ASSERT_TRUE(publisher->get_qos(pqos) == ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory.autoenable_created_entities, false);

}

TEST(PublisherTests, ChangePSMPublisherQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0);
    ::dds::pub::Publisher publisher = ::dds::pub::Publisher(participant);

    ::dds::pub::qos::PublisherQos qos = publisher.qos();
    ASSERT_EQ(qos, PUBLISHER_QOS_DEFAULT);

    qos.entity_factory.autoenable_created_entities = false;
    ASSERT_NO_THROW(publisher.qos(qos));
    ::dds::pub::qos::PublisherQos pqos = publisher.qos();

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory.autoenable_created_entities, false);
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
