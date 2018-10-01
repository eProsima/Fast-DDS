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
#include "VideoTestTypes.h"
#include <gstreamer-1.0/gst/app/gstappsrc.h>
#include <gstreamer-1.0/gst/app/gstappsink.h>
#include <gstreamer-1.0/gst/gst.h>

class VideoTestSubscriber
{
    public:
        VideoTestSubscriber();
        virtual ~VideoTestSubscriber();

        eprosima::fastrtps::Participant* mp_participant;
        eprosima::fastrtps::Publisher* mp_datapub;
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
                const std::string& sXMLConfigFile);

        void run();
        bool test();

        class DataPubListener : public eprosima::fastrtps::PublisherListener
        {
            public:
                DataPubListener(VideoTestSubscriber* up):mp_up(up){}
                ~DataPubListener(){}
                void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
                        eprosima::fastrtps::rtps::MatchingInfo& info);
                VideoTestSubscriber* mp_up;
        } m_datapublistener;

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

        std::thread thread_;
        std::deque<VideoType> packet_deque_;
        std::mutex deque_mutex_;
        std::mutex gst_mutex_;

protected:

    static void start_feed_cb(GstElement* source, guint size, VideoTestSubscriber* sub);
    static void stop_feed_cb(GstElement* source, VideoTestSubscriber* sub);
    static gboolean push_data_cb(VideoTestSubscriber* sub);

    void InitGStreamer();
    void stop();

    void push_video_packet(VideoType& packet)
    {
        std::unique_lock<std::mutex> lock(deque_mutex_);
        packet_deque_.push_back(packet);
    }

    uint32_t currentSize()
    {
        uint32_t size = 0;
        std::unique_lock<std::mutex> lock(deque_mutex_);
        for (auto& vpacket : packet_deque_)
        {
            size += (uint32_t) vpacket.data.size();
        }
        return size;
    }

    VideoType pop_video_packet()
    {
        std::unique_lock<std::mutex> lock(deque_mutex_);
        VideoType vpacket;
        if (!packet_deque_.empty())
        {
            vpacket = packet_deque_[0];
            packet_deque_.pop_front();
        }
        return vpacket;
    }

    bool hasData()
    {
        std::unique_lock<std::mutex> lock(deque_mutex_);
        return !packet_deque_.empty();
    }

    static void gst_run(VideoTestSubscriber* sub);
};

#endif /* VIDEOTESTSUBSCRIBER_H_ */
