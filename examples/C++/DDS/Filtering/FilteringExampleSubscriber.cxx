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

#include "FilteringExampleSubscriber.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

FilteringExampleSubscriber::FilteringExampleSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , myType(new FilteringExamplePubSubType())
{
}

FilteringExampleSubscriber::~FilteringExampleSubscriber()
{
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool FilteringExampleSubscriber::init(
        int type)
{
    // Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name("Participant_subscriber"); //You can put the name you want

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    //Register the type
    myType.register_type(participant_);

    // Create Subscriber
    SubscriberQos sqos;
    if (type == 1)
    {
        sqos.partition().push_back("Fast_Partition");
    }
    else //2 = slow
    {
        sqos.partition().push_back("Slow_Partition");
    }

    subscriber_ = participant_->create_subscriber(sqos);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    //Create Topic
    topic_ = participant_->create_topic("FilteringExamplePubSubTopic", myType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    //Create DataReader
    reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &m_listener);

    if (reader_ == nullptr)
    {
        return false;
    }
    return true;
}

void FilteringExampleSubscriber::SubListener::on_subscription_matched(
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

void FilteringExampleSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    FilteringExample st;
    if (reader->take_next_sample(&st, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
        {
            ++n_msg;
            std::cout << "Sample received, count=" << n_msg << std::endl;
            std::cout << " sampleNumber=" << st.sampleNumber() << std::endl;
        }
    }
}

void FilteringExampleSubscriber::run()
{
    std::cout << "Waiting for Data, press Enter to stop the Subscriber. " << std::endl;
    std::cin.ignore();
    std::cout << "Shutting down the Subscriber." << std::endl;
}
