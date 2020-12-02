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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include "deadlinepayloadSubscriber.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

deadlinepayloadSubscriber::deadlinepayloadSubscriber()
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
    , mp_reader(nullptr)
    , mp_topic(nullptr)
    , myType(new HelloMsgPubSubType())
    , m_listener()
{
}

deadlinepayloadSubscriber::~deadlinepayloadSubscriber()
{
    if (mp_reader != nullptr)
    {
        mp_subscriber->delete_datareader(mp_reader);
    }
    if (mp_topic != nullptr)
    {
        mp_participant->delete_topic(mp_topic);
    }
    if (mp_subscriber != nullptr)
    {
        mp_participant->delete_subscriber(mp_subscriber);
    }
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
}

bool deadlinepayloadSubscriber::init(
        double deadline_ms)
{
    // Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name("Participant_subscriber"); //You can put the name you want

    mp_participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //Register the type
    myType.register_type(mp_participant);

    // Create Subscriber
    mp_subscriber = mp_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

    // Create topic
    mp_topic = mp_participant->create_topic("deadlinepayloadPubSubTopic", myType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (mp_topic == nullptr)
    {
        return false;
    }

    // Create datareader
    DataReaderQos rqos;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.deadline().period = deadline_ms * 1e-3;
    rqos.history().depth = 5;

    mp_reader = mp_subscriber->create_datareader(mp_topic, rqos, &m_listener);

    if (mp_reader == nullptr)
    {
        return false;
    }

    return true;
}

void deadlinepayloadSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void deadlinepayloadSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    HelloMsg st;
    if (reader->take_next_sample(&st, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
        {
            std::cout << "Message with key " << st.deadlinekey() << " received" << std::endl;
        }
    }
}

void deadlinepayloadSubscriber::SubListener::on_requested_deadline_missed(
        DataReader* reader,
        const RequestedDeadlineMissedStatus& status)
{
    (void)reader;
    std::cout << "Deadline missed for instance: " << status.last_instance_handle << std::endl;
}

void deadlinepayloadSubscriber::run()
{
    std::cout << "Subscriber running. Press Enter to stop the Subscriber. " << std::endl;
    std::cin.ignore();
}
