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
 * @file LatencyPublisher.h
 *
 */

#ifndef LATENCYPUBLISHER_H_
#define LATENCYPUBLISHER_H_

#include <asio.hpp>

#include "LatencyTestTypes.h"

#include <condition_variable>
#include <chrono>

//#include <fastrtps/types/DynamicTypeBuilderFactory.h>
//#include <fastrtps/types/DynamicDataFactory.h>
//#include <fastrtps/types/DynamicTypeBuilder.h>
//#include <fastrtps/types/DynamicTypeBuilderPtr.h>
//#include <fastrtps/types/TypeDescriptor.h>
//#include <fastrtps/types/MemberDescriptor.h>
//#include <fastrtps/types/DynamicType.h>
//#include <fastrtps/types/DynamicData.h>
//#include <fastrtps/types/DynamicPubSubType.h>

class TimeStats{
public:
    TimeStats() :nbytes(0), received(0), m_min(0), m_max(0), p50(0), p90(0), p99(0), p9999(0), mean(0), stdev(0){}
    ~TimeStats(){}
    uint64_t nbytes;
    unsigned int received;
    std::chrono::duration<double, std::micro>  m_min, m_max;
    double p50, p90, p99, p9999, mean, stdev;
};

class LatencyTestPublisher {

public:
    LatencyTestPublisher();
    virtual ~LatencyTestPublisher();

    eprosima::fastrtps::Participant* mp_participant;
    eprosima::fastrtps::Publisher* mp_datapub;
    eprosima::fastrtps::Publisher* mp_commandpub;
    eprosima::fastrtps::Subscriber* mp_datasub;
    eprosima::fastrtps::Subscriber* mp_commandsub;
    std::chrono::steady_clock::time_point t_start_, t_end_;
    std::chrono::duration<double, std::micro> t_overhead_;
    int n_subscribers;
    unsigned int n_samples;
    eprosima::fastrtps::SampleInfo_t m_sampleinfo;
    std::vector<std::chrono::duration<double, std::micro>> times_;
    std::vector<TimeStats> m_stats;
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
        const eprosima::fastrtps::rtps::PropertyPolicy& property_policy, bool large_data,
        const std::string& sXMLConfigFile, bool dynamic_types, int forced_domain);
    void run();
    void analyzeTimes(uint32_t datasize);
    bool test(uint32_t datasize);
    void printStat(TimeStats& TS);

    class DataPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:
        DataPubListener(LatencyTestPublisher* up) :mp_up(up), n_matched(0) {}
        ~DataPubListener() {}
        void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
            eprosima::fastrtps::rtps::MatchingInfo& info);
        LatencyTestPublisher* mp_up;
        int n_matched;
    } m_datapublistener;

    class DataSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:
        DataSubListener(LatencyTestPublisher* up) :mp_up(up), n_matched(0) {}
        ~DataSubListener() {}
        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
            eprosima::fastrtps::rtps::MatchingInfo& into);
        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
        LatencyTestPublisher* mp_up;
        int n_matched;
    } m_datasublistener;

    class CommandPubListener : public eprosima::fastrtps::PublisherListener
    {
    public:
        CommandPubListener(LatencyTestPublisher* up) :mp_up(up), n_matched(0) {}
        ~CommandPubListener() {}
        void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
            eprosima::fastrtps::rtps::MatchingInfo& info);
        LatencyTestPublisher* mp_up;
        int n_matched;
    } m_commandpublistener;

    class CommandSubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:
        CommandSubListener(LatencyTestPublisher* up) :mp_up(up), n_matched(0) {}
        ~CommandSubListener() {}
        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
            eprosima::fastrtps::rtps::MatchingInfo& into);
        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
        LatencyTestPublisher* mp_up;
        int n_matched;
    } m_commandsublistener;

    TestCommandDataType command_t;

    std::stringstream output_file_minimum;
    std::stringstream output_file_average;
    std::stringstream output_file_16;
    std::stringstream output_file_32;
    std::stringstream output_file_64;
    std::stringstream output_file_128;
    std::stringstream output_file_256;
    std::stringstream output_file_512;
    std::stringstream output_file_1024;
    std::stringstream output_file_2048;
    std::stringstream output_file_4096;
    std::stringstream output_file_8192;
    std::stringstream output_file_16384;
    std::stringstream output_file_64000;
    std::stringstream output_file_131072;
    std::string m_sXMLConfigFile;
    bool reliable_;
    //bool dynamic_data = false;
    int m_forcedDomain;
    // Static Types
    LatencyDataType latency_t;
    LatencyType* mp_latency_in;
    LatencyType* mp_latency_out;
    // Dynamic Types
    //eprosima::fastrtps::types::DynamicData* m_DynData_in;
    //eprosima::fastrtps::types::DynamicData* m_DynData_out;
    //eprosima::fastrtps::types::DynamicPubSubType m_DynType;
    //eprosima::fastrtps::types::DynamicType_ptr m_pDynType;
};


#endif /* LATENCYPUBLISHER_H_ */
