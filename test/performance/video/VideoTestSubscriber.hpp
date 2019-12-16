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
 * @file VideoTestSubscriber.h
 *
 */

#ifndef VIDEOTESTSUBSCRIBER_H_
#define VIDEOTESTSUBSCRIBER_H_

#include <asio.hpp>
#include <condition_variable>
#include "VideoTestTypes.hpp"
#include <gstreamer-1.0/gst/app/gstappsrc.h>
#include <gstreamer-1.0/gst/app/gstappsink.h>
#include <gstreamer-1.0/gst/gst.h>

class TimeStats
{
public:
    TimeStats() : received(0), m_minAvg(0), m_maxAvg(0), pDrop50(0), pDrop90(0), pDrop99(0), pDrop9999(0), pDropMean(0),
        pDropStdev(0), pAvg50(0), pAvg90(0), pAvg99(0), pAvg9999(0), pAvgMean(0), pAvgStdev(0)
    {
    }
    ~TimeStats()
    {
    }

    unsigned int received;
    double  m_minDrop, m_maxDrop, m_minAvg, m_maxAvg;
    double pDrop50, pDrop90, pDrop99, pDrop9999, pDropMean, pDropStdev;
    double pAvg50, pAvg90, pAvg99, pAvg9999, pAvgMean, pAvgStdev;

};

class VideoTestSubscriber
{
    public:
        VideoTestSubscriber();
        virtual ~VideoTestSubscriber();

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
        bool init(int nsam, bool reliable, uint32_t pid, bool hostname,
                const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
                const eprosima::fastrtps::rtps::PropertyPolicy& property_policy, bool large_data,
                const std::string& sXMLConfigFile, bool export_csv, const std::string& export_file,
                int forced_domain, int video_width, int video_height, int frame_rate);

        void run();
        bool test();

        class DataSubListener : public eprosima::fastrtps::SubscriberListener
        {
            public:
                DataSubListener(VideoTestSubscriber* up):mp_up(up){}
                ~DataSubListener(){}
                void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
                        eprosima::fastrtps::rtps::MatchingInfo& into);
                void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
                VideoTestSubscriber* mp_up;
        } m_datasublistener;

        class CommandPubListener : public eprosima::fastrtps::PublisherListener
        {
            public:
                CommandPubListener(VideoTestSubscriber* up):mp_up(up){}
                ~CommandPubListener(){}
                void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
                        eprosima::fastrtps::rtps::MatchingInfo& info);
                VideoTestSubscriber* mp_up;
        } m_commandpublistener;

        class CommandSubListener : public eprosima::fastrtps::SubscriberListener
        {
            public:
                CommandSubListener(VideoTestSubscriber* up):mp_up(up){}
                ~CommandSubListener(){}
                void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
                        eprosima::fastrtps::rtps::MatchingInfo& into);
                void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);
                VideoTestSubscriber* mp_up;
        } m_commandsublistener;

        VideoDataType video_t;
        TestCommandDataType command_t;
        std::string m_sXMLConfigFile;
        bool m_bRunning;

        GstElement* pipeline;
        GstElement* appsrc;
        GstElement* sink;
        GstElement* videoconvert;
        guint source_id_;      // To control the GSource
        GMainLoop* gmain_loop_; // GLib's Main Loop
        guint64 g_servertimestamp, g_clienttimestamp;
        gint64 g_framesDropped;
        bool m_bReliable;
        bool m_bExportCsv;
        std::string m_sExportPrefix;
        int m_forcedDomain;
        int m_videoWidth;
        int m_videoHeight;
        int m_videoFrameRate;

        std::thread thread_;
        std::deque<VideoType> packet_deque_;
        std::mutex stats_mutex_;
        std::mutex deque_mutex_;
        std::condition_variable deque_cond_;
        std::mutex gst_mutex_;

        std::chrono::steady_clock::time_point t_start_, t_end_;
        std::chrono::steady_clock::time_point t_drop_start_, t_drop_end_;
        std::vector<std::chrono::duration<double, std::micro>> samples_;
        std::vector<double> drops_;
        std::vector<double> avgs_;
        std::vector<TimeStats> m_stats;

protected:

    static void start_feed_cb(GstElement* source, guint size, VideoTestSubscriber* sub);
    static void stop_feed_cb(GstElement* source, VideoTestSubscriber* sub);
    static gboolean push_data_cb(VideoTestSubscriber* sub);
    static void fps_stats_cb(GstElement* source, gdouble fps, gdouble droprate, gdouble avgfps, VideoTestSubscriber* sub);
    void InitGStreamer();
    void stop();
    void analyzeTimes();
    void printStat(TimeStats& TS);

    void push_video_packet(VideoType& packet)
    {
        std::unique_lock<std::mutex> lock(deque_mutex_);
        packet_deque_.push_back(std::move(packet));
        deque_cond_.notify_one();
    }

    VideoType pop_video_packet()
    {
        std::unique_lock<std::mutex> lock(deque_mutex_);
        deque_cond_.wait(lock, [&]() { return !m_bRunning || !packet_deque_.empty(); });
        VideoType vpacket;
        if(!packet_deque_.empty())
        {
            vpacket = std::move(packet_deque_[0]);
            packet_deque_.pop_front();
        }
        return vpacket;
    }

    static void gst_run(VideoTestSubscriber* sub);
    static void message_cb(GstBus* bus, GstMessage* message, gpointer user_data);
};

#endif /* VIDEOTESTSUBSCRIBER_H_ */
