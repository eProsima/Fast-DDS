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

/**
 * @file MemoryTestSubscriber.h
 *
 */

#ifndef MEMORYTESTSUBSCRIBER_H_
#define MEMORYTESTSUBSCRIBER_H_

#include <asio.hpp>
#include <condition_variable>
#include "MemoryTestTypes.h"
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicPubSubType.h>

class MemoryTestSubscriber
{
public:
    MemoryTestSubscriber();
    virtual ~MemoryTestSubscriber();

    eprosima::fastrtps::Participant* mp_participant;
    eprosima::fastrtps::Publisher* mp_commandpub;
    eprosima::fastrtps::Subscriber* mp_datasub;
    eprosima::fastrtps::Subscriber* mp_commandsub;
    eprosima::fastrtps::SampleInfo_t m_sampleinfo;
    std::mutex mutex_;
    int disc_count_;
    std::condition_variable disc_cond_;
    int comm_count_;
    std::condition_variable comm_cond_;
    int data_count_;
    std::condition_variable data_cond_;
    int m_status;
    int n_received;
    int n_samples;
    bool init(bool echo, int nsam, bool reliable, uint32_t pid, bool hostname,
        const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
        const std::string& sXMLConfigFile, uint32_t data_size, bool dynamic_types);

    void run();
    bool test(uint32_t datasize);

    class DataSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:
        DataSubListener(MemoryTestSubscriber* up) :mp_up(up) {}
        ~DataSubListener() {}
        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
            eprosima::fastrtps::rtps::MatchingInfo& into);
        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
        MemoryTestSubscriber* mp_up;
    } m_datasublistener;

    class CommandPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:
        CommandPubListener(MemoryTestSubscriber* up) :mp_up(up) {}
        ~CommandPubListener() {}
        void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
            eprosima::fastrtps::rtps::MatchingInfo& info);
        MemoryTestSubscriber* mp_up;
    } m_commandpublistener;

    class CommandSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:
        CommandSubListener(MemoryTestSubscriber* up) :mp_up(up) {}
        ~CommandSubListener() {}
        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
            eprosima::fastrtps::rtps::MatchingInfo& into);
        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
        MemoryTestSubscriber* mp_up;
    } m_commandsublistener;

    bool m_echo;
    TestCommandDataType command_t;
    std::string m_sXMLConfigFile;
    uint32_t m_data_size;
    bool dynamic_data = false;
    // Static Data
    MemoryType* mp_memory;
    MemoryDataType memory_t;
    // Dynamic Data
    eprosima::fastrtps::types::DynamicData* m_DynData;
    eprosima::fastrtps::types::DynamicPubSubType m_DynType;
    eprosima::fastrtps::types::DynamicType_ptr m_pDynType;
    eprosima::fastrtps::SubscriberAttributes subAttr;
};

#endif /* MEMORYTESTSUBSCRIBER_H_ */
