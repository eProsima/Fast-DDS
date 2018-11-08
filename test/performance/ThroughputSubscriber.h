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
 * @file ThroughputSubscriber.h
 *
 */

#ifndef THROUGHPUTSUBSCRIBER_H_
#define THROUGHPUTSUBSCRIBER_H_

#include <asio.hpp>

#include "ThroughputTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/rtps/attributes/PropertyPolicy.h>
//#include <fastrtps/types/DynamicTypeBuilderFactory.h>
//#include <fastrtps/types/DynamicDataFactory.h>
//#include <fastrtps/types/DynamicTypeBuilder.h>
//#include <fastrtps/types/DynamicTypeBuilderPtr.h>
//#include <fastrtps/types/TypeDescriptor.h>
//#include <fastrtps/types/MemberDescriptor.h>
//#include <fastrtps/types/DynamicType.h>
//#include <fastrtps/types/DynamicData.h>
//#include <fastrtps/types/DynamicPubSubType.h>

#include <condition_variable>
#include <chrono>

#include <fstream>
#include <iostream>



class ThroughputSubscriber
{
public:

    ThroughputSubscriber(bool reliable, uint32_t pid, bool hostname,
        const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
        const std::string& sXMLConfigFile, bool dynamic_types, int forced_domain);
    virtual ~ThroughputSubscriber();
    eprosima::fastrtps::Participant* mp_par;
    eprosima::fastrtps::Subscriber* mp_datasub;
    eprosima::fastrtps::Publisher* mp_commandpubli;
    eprosima::fastrtps::Subscriber* mp_commandsub;
    std::chrono::steady_clock::time_point t_start_, t_end_;
    std::chrono::duration<double, std::micro> t_overhead_;
    std::mutex mutex_;
    uint32_t disc_count_;
    std::condition_variable disc_cond_;
    std::mutex dataMutex_;
    uint32_t data_disc_count_;
    std::condition_variable data_disc_cond_;
    //! 0 - Continuing test, 1 - End of a test, 2 - Finish application
    int stop_count_;
    std::condition_variable stop_cond_;
    class DataSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:

        DataSubListener(ThroughputSubscriber& up);
        virtual ~DataSubListener();
        ThroughputSubscriber& m_up;
        void reset();
        uint32_t lastseqnum, saved_lastseqnum;
        uint32_t lostsamples, saved_lostsamples;
        bool first;
        eprosima::fastrtps::SampleInfo_t info;
        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
            eprosima::fastrtps::rtps::MatchingInfo& info);
        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
        void saveNumbers();
        std::ofstream myfile;
    }m_DataSubListener;

    class CommandSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:

        CommandSubListener(ThroughputSubscriber& up);
        virtual ~CommandSubListener();
        ThroughputSubscriber& m_up;
        ThroughputCommandType m_commandin;
        eprosima::fastrtps::SampleInfo_t info;
        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
            eprosima::fastrtps::rtps::MatchingInfo& info);
        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
        void saveNumbers();

    private:

        CommandSubListener& operator=(const CommandSubListener&);
    }m_CommandSubListener;

    class CommandPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:

        CommandPubListener(ThroughputSubscriber& up);
        virtual ~CommandPubListener();
        ThroughputSubscriber& m_up;
        void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
            eprosima::fastrtps::rtps::MatchingInfo& info);

    private:

        CommandPubListener& operator=(const CommandPubListener&);
    }m_CommandPubListener;

    bool ready;
    uint32_t m_datasize;
    uint32_t m_demand;
    void run();
    ThroughputCommandDataType throuputcommand_t;
    std::string m_sXMLConfigFile;
    //bool dynamic_data = false;
    int m_forced_domain;

    // Static Data
    ThroughputDataType throughput_t;
    ThroughputType* throughputin;
    // Dynamic Data
    //eprosima::fastrtps::types::DynamicData* m_DynData;
    //eprosima::fastrtps::types::DynamicPubSubType m_DynType;
    //eprosima::fastrtps::types::DynamicType_ptr m_pDynType;
    //eprosima::fastrtps::SubscriberAttributes subAttr;
};

#endif /* THROUGHPUTSUBSCRIBER_H_ */
