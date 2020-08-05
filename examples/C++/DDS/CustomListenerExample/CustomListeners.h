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

/**
 * @file CustomListeners.h
 *
 */

#ifndef CUSTOMLISTENERS_H_
#define CUSTOMLISTENERS_H_

#include "HelloWorldPubSubTypes.h"

#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <fastdds/dds/publisher/PublisherListener.hpp>

#include <fastdds/dds/topic/TopicListener.hpp>

#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastrtps/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <fastrtps/subscriber/SampleInfo.h>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>


class CustomDataWriterListener : public eprosima::fastdds::dds::DataWriterListener
{

public:

    CustomDataWriterListener()
    : eprosima::fastdds::dds::DataWriterListener()
    {
    }

    virtual ~CustomDataWriterListener()
    {
    }

    void on_publication_matched(
        eprosima::fastdds::dds::DataWriter* writer,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

    int matched_ = 0;

    bool firstConnected_  = false;
};

class CustomDataReaderListener : public eprosima::fastdds::dds::DataReaderListener
{

    public:

        CustomDataReaderListener()
            : matched_(0)
            , samples_(0)
        {
        }

        ~CustomDataReaderListener() override
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        HelloWorld hello_;

        int matched_;

        uint32_t samples_;
 };


class CustomDomainParticipantListener : public eprosima::fastdds::dds::DomainParticipantListener 
{
    public:
        CustomDomainParticipantListener();

        ~CustomDomainParticipantListener() override;

        void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;


        void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        HelloWorld hello_;

        int matched_ = 0;

        bool firstConnected_  = false;

        uint32_t samples_ = 0;
};

#endif /* CUSTOMLISTENERS_H_ */
