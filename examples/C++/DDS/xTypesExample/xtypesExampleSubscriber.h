// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file xtypesExampleSubscriber.h
 *
 */

#ifndef xtypesExampleSUBSCRIBER_H_
#define xtypesExampleSUBSCRIBER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>

#include "GenericTopicDataType.h"

#define TYPE_NAME_SUB "xTypesExampleType"
#define TOPIC_NAME "xtypesExampleTopic"
#define IDL_FILE_NAME "xtypesExample.idl"

class xtypesExampleSubscriber
{
public:

    xtypesExampleSubscriber();

    virtual ~xtypesExampleSubscriber();

    //! Initialize subscriber
    bool init();

    //! Run subscriber
    void run();

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataReader* reader_;

    // DataReader Listener class to handle callbacks of matching and new data received
    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        SubListener()
            : type(new GenericTopicDataType(IDL_FILE_NAME, TYPE_NAME_SUB))
        {
        }

        ~SubListener() override
        {
        }

        // Callback called when a new data has been received
        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        // Callback called when a subscriber has matched or unmatched
        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        // Type is on Listener and not on Subscriber so it can be used in on_data_available
        eprosima::fastdds::dds::TypeSupport type;
    }
    listener_;
};

#endif /* xtypesExampleSUBSCRIBER_H_ */
