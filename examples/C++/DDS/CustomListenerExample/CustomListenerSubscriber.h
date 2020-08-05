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
 * @file CustomListenerSubscriber.h
 *
 */

#ifndef CUSTOMLISTENERSUBSCRIBER_H_
#define CUSTOMLISTENERSUBSCRIBER_H_

#include "HelloWorldPubSubTypes.h"
#include "CustomListeners.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>

class CustomListenerSubscriber
{
public:

    CustomListenerSubscriber();

    virtual ~CustomListenerSubscriber();

    //!Initialize the subscriber
    bool init(bool use_dr);

    //!RUN the subscriber
    void run();

    //!Run the subscriber until number samples have been received.
    void run(
            uint32_t number);

private:


    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataReader* reader_;

    eprosima::fastdds::dds::TypeSupport type_;

    eprosima::fastdds::dds::StatusMask dr_mask_;

    eprosima::fastdds::dds::StatusMask p_mask_;

    CustomDataReaderListener dr_listener_;

    CustomDomainParticipantListener p_listener_;
};

#endif /* CUSTOMLISTENERSUBSCRIBER_H_ */
