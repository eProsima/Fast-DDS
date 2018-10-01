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
 * @file VideoPublisher.h
 *
 */

#ifndef VIDEOPUBLISHER_H_
#define VIDEOPUBLISHER_H_

#include <asio.hpp>

#include "VideoTestTypes.h"
#include <gstreamer-1.0/gst/app/gstappsrc.h>
#include <gstreamer-1.0/gst/app/gstappsink.h>
#include <gstreamer-1.0/gst/gst.h>

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

class VideoTestPublisher
{
    public:
        VideoTestPublisher();
        virtual ~VideoTestPublisher();

        eprosima::fastrtps::Participant* mp_participant;
        eprosima::fastrtps::Publisher* mp_datapub;
        eprosima::fastrtps::Publisher* mp_commandpub;
        eprosima::fastrtps::Subscriber* mp_datasub;
        eprosima::fastrtps::Subscriber* mp_commandsub;
        VideoType* mp_video_out;
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
        bool init(int n_sub, int n_sam, bool reliable, uint32_t pid, bool hostname,
                const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
                const eprosima::fastrtps::rtps::PropertyPolicy& property_policy, bool large_data,
                const std::string& sXMLConfigFile);
        void run();
        void analyzeTimes(uint32_t datasize);
        bool test(uint32_t datasize);
        void printStat(TimeStats& TS);

        class DataPubListener : public eprosima::fastrtps::PublisherListener
        {
            public:
                DataPubListener(VideoTestPublisher* up):mp_up(up),n_matched(0){}
                ~DataPubListener(){}
                void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
                        eprosima::fastrtps::rtps::MatchingInfo& info);
                VideoTestPublisher* mp_up;
                int n_matched;
        } m_datapublistener;

        class DataSubListener : public eprosima::fastrtps::SubscriberListener
        {
            public:
                DataSubListener(VideoTestPublisher* up):mp_up(up),n_matched(0){}
                ~DataSubListener(){}
                void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
                        eprosima::fastrtps::rtps::MatchingInfo& into);
                void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
                VideoTestPublisher* mp_up;
                int n_matched;
        } m_datasublistener;

        class CommandPubListener : public eprosima::fastrtps::PublisherListener
        {
            public:
                CommandPubListener(VideoTestPublisher* up):mp_up(up),n_matched(0){}
                ~CommandPubListener(){}
                void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
                        eprosima::fastrtps::rtps::MatchingInfo& info);
                VideoTestPublisher* mp_up;
                int n_matched;
        } m_commandpublistener;

        class CommandSubListener : public eprosima::fastrtps::SubscriberListener
        {
            public:
                CommandSubListener(VideoTestPublisher* up):mp_up(up),n_matched(0){}
                ~CommandSubListener(){}
                void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
                        eprosima::fastrtps::rtps::MatchingInfo& into);
                void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
                VideoTestPublisher* mp_up;
                int n_matched;
        } m_commandsublistener;

        VideoDataType latency_t;
        TestCommandDataType command_t;
        std::string m_sXMLConfigFile;
        bool reliable_;

        GstElement* pipeline;
        GstElement* filesrc;
        GstElement* sink;

    protected:

        void InitGStreamer();
        static GstFlowReturn new_sample(GstElement *sink, VideoTestPublisher *sub);

};


#endif /* VIDEOPUBLISHER_H_ */
