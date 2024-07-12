// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file HelloWorldSubscriber.h
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_DDS_DYNAMIC_HELLO_WORLD_EXAMPLE__HELLOWORLDSUBSCRIBER_H
#define FASTDDS_EXAMPLES_CPP_DDS_DYNAMIC_HELLO_WORLD_EXAMPLE__HELLOWORLDSUBSCRIBER_H

#include <atomic>
#include <condition_variable>
#include <map>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

class HelloWorldSubscriber
{
public:

    HelloWorldSubscriber();

    virtual ~HelloWorldSubscriber();

    //!Initialize the subscriber
    bool init();

    //!RUN the subscriber
    void run();

    //!Run the subscriber until number samples have been received.
    void run(
            uint32_t number);

    //! Initialize all required entities for data transmission
    void initialize_entities();

private:

    eprosima::fastdds::dds::DomainParticipant* mp_participant;

    eprosima::fastdds::dds::Subscriber* mp_subscriber;

    std::map<eprosima::fastdds::dds::DataReader*, eprosima::fastdds::dds::Topic*> topics_;

    std::map<eprosima::fastdds::dds::DataReader*, eprosima::fastdds::dds::DynamicType::_ref_type> readers_;

    std::map<eprosima::fastdds::dds::DataReader*, eprosima::fastdds::dds::DynamicData::_ref_type> datas_;

    eprosima::fastdds::dds::DataReaderQos qos_;

public:

    class SubListener
        :  public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:

        SubListener(
                HelloWorldSubscriber* sub)
            : n_matched(0)
            , n_samples(0)
            , subscriber_(sub)
        {
        }

        ~SubListener() override
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        int n_matched;

        uint32_t n_samples;

        std::mutex types_mx_;

        std::condition_variable types_cv_;

        eprosima::fastdds::dds::DynamicType::_ref_type received_type_;

        std::atomic<bool> reception_flag_{false};

        HelloWorldSubscriber* subscriber_;

    }
    m_listener;

};

#endif // FASTDDS_EXAMPLES_CPP_DDS_DYNAMIC_HELLO_WORLD_EXAMPLE__HELLOWORLDSUBSCRIBER_H
