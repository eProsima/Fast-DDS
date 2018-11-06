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
 * @file MemoryPublisher.h
 *
 */

#ifndef MEMORYPUBLISHER_H_
#define MEMORYPUBLISHER_H_

#include <asio.hpp>

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

#include <condition_variable>
#include <chrono>

class MemoryTestPublisher {
public:
    MemoryTestPublisher();
    virtual ~MemoryTestPublisher();

    eprosima::fastrtps::Participant* mp_participant;
    eprosima::fastrtps::Publisher* mp_datapub;
    eprosima::fastrtps::Publisher* mp_commandpub;
    eprosima::fastrtps::Subscriber* mp_commandsub;
    int n_subscribers;
    unsigned int n_samples;
    eprosima::fastrtps::SampleInfo_t m_sampleinfo;
    std::mutex mutex_;
    int disc_count_;
    std::condition_variable disc_cond_;
    int comm_count_;
    std::condition_variable comm_cond_;
    int data_count_;
    std::condition_variable data_cond_;
    int m_status;
    unsigned int n_received;
    bool n_export_csv;
    std::string m_exportPrefix;
    bool init(int n_sub, int n_sam, bool reliable, uint32_t pid, bool hostname, bool export_csv,
        const std::string& export_prefix,
        const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
        const std::string& sXMLConfigFile, uint32_t data_size, bool dynamic_types);
    void run(uint32_t test_time);
    bool test(uint32_t test_time, uint32_t datasize);

    class DataPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:
        DataPubListener(MemoryTestPublisher* up) :mp_up(up), n_matched(0) {}
        ~DataPubListener() {}
        void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
            eprosima::fastrtps::rtps::MatchingInfo& info);
        MemoryTestPublisher* mp_up;
        int n_matched;
    } m_datapublistener;

    class CommandPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:
        CommandPubListener(MemoryTestPublisher* up) :mp_up(up), n_matched(0) {}
        ~CommandPubListener() {}
        void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
            eprosima::fastrtps::rtps::MatchingInfo& info);
        MemoryTestPublisher* mp_up;
        int n_matched;
    } m_commandpublistener;

    class CommandSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:
        CommandSubListener(MemoryTestPublisher* up) :mp_up(up), n_matched(0) {}
        ~CommandSubListener() {}
        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
            eprosima::fastrtps::rtps::MatchingInfo& into);
        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
        MemoryTestPublisher* mp_up;
        int n_matched;
    } m_commandsublistener;

    TestCommandDataType command_t;
    std::string m_sXMLConfigFile;
    bool reliable_;
    uint32_t m_data_size;
    bool dynamic_data = false;
    // Static Data
    MemoryType* mp_memory;
    MemoryDataType memory_t;
    // Dynamic Data
    eprosima::fastrtps::types::DynamicData* m_DynData;
    eprosima::fastrtps::types::DynamicPubSubType m_DynType;
    eprosima::fastrtps::types::DynamicType_ptr m_pDynType;
    eprosima::fastrtps::PublisherAttributes pubAttr;

};


#endif /* MEMORYPUBLISHER_H_ */
