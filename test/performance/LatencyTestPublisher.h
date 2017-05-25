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

        Participant* mp_participant;
        Publisher* mp_datapub;
        Publisher* mp_commandpub;
        Subscriber* mp_datasub;
        Subscriber* mp_commandsub;
        LatencyType* mp_latency_in;
        LatencyType* mp_latency_out;
        std::chrono::steady_clock::time_point t_start_, t_end_;
        std::chrono::duration<double, std::micro> t_overhead_;
        int n_subscribers;
        unsigned int n_samples;
        SampleInfo_t m_sampleinfo;
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
        bool init(int n_sub, int n_sam, bool reliable, uint32_t pid, bool hostname, bool export_csv);
        void run();
        void analizeTimes(uint32_t datasize);
        bool test(uint32_t datasize);
        void printStat(TimeStats& TS);
        class DataPubListener : public PublisherListener
    {
        public:
            DataPubListener(LatencyTestPublisher* up):mp_up(up),n_matched(0){}
            ~DataPubListener(){}
            void onPublicationMatched(Publisher* pub,MatchingInfo& info);
            LatencyTestPublisher* mp_up;
            int n_matched;
    }m_datapublistener;
        class DataSubListener : public SubscriberListener
    {
        public:
            DataSubListener(LatencyTestPublisher* up):mp_up(up),n_matched(0){}
            ~DataSubListener(){}
            void onSubscriptionMatched(Subscriber* sub,MatchingInfo& into);
            void onNewDataMessage(Subscriber* sub);
            LatencyTestPublisher* mp_up;
            int n_matched;
    }m_datasublistener;
        class CommandPubListener : public PublisherListener
    {
        public:
            CommandPubListener(LatencyTestPublisher* up):mp_up(up),n_matched(0){}
            ~CommandPubListener(){}
            void onPublicationMatched(Publisher* pub,MatchingInfo& info);
            LatencyTestPublisher* mp_up;
            int n_matched;
    }m_commandpublistener;
        class CommandSubListener : public SubscriberListener
    {
        public:
            CommandSubListener(LatencyTestPublisher* up):mp_up(up),n_matched(0){}
            ~CommandSubListener(){}
            void onSubscriptionMatched(Subscriber* sub,MatchingInfo& into);
            void onNewDataMessage(Subscriber* sub);
            LatencyTestPublisher* mp_up;
            int n_matched;
    }m_commandsublistener;

        LatencyDataType latency_t;
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

        bool reliable_;
};


#endif /* LATENCYPUBLISHER_H_ */
