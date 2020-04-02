// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <dds/sub/Subscriber.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

TEST(SubscriberTests, ChangeSubscriberQos)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    SubscriberQos qos;
    ASSERT_EQ(subscriber->get_qos(qos), ReturnCode_t::RETCODE_OK);

    ASSERT_EQ(qos, SUBSCRIBER_QOS_DEFAULT);

    qos.entity_factory.autoenable_created_entities = false;

    ASSERT_EQ(subscriber->set_qos(qos), ReturnCode_t::RETCODE_OK);
    SubscriberQos pqos;
    ASSERT_EQ(subscriber->get_qos(pqos), ReturnCode_t::RETCODE_OK);

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory.autoenable_created_entities, false);

}

TEST(SubscriberTests, ChangePSMSubscriberQos)
{
    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(0);
    ::dds::sub::Subscriber subscriber = ::dds::sub::Subscriber(participant);

    ::dds::sub::qos::SubscriberQos qos = subscriber.qos();
    ASSERT_EQ(qos, SUBSCRIBER_QOS_DEFAULT);

    qos.entity_factory.autoenable_created_entities = false;
    ASSERT_NO_THROW(subscriber.qos(qos));
    ::dds::sub::qos::SubscriberQos pqos = subscriber.qos();

    ASSERT_TRUE(qos == pqos);
    ASSERT_EQ(pqos.entity_factory.autoenable_created_entities, false);
}

TEST(SubscriberTests, ChangeSubscriberListener)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    ASSERT_EQ(subscriber->get_listener(), nullptr);

    SubscriberListener listener;

    ASSERT_EQ(subscriber->set_listener(&listener, StatusMask::publication_matched()),
            ReturnCode_t::RETCODE_OK);
    SubscriberListener* plistener = subscriber->get_listener();

    ASSERT_EQ(plistener, &listener);
    ASSERT_EQ(subscriber->get_status_mask(), StatusMask::publication_matched());
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
