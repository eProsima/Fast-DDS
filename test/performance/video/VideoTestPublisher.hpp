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
#include <condition_variable>
#include <chrono>

#include "VideoTestTypes.hpp"
#include <gstreamer-1.0/gst/app/gstappsrc.h>
#include <gstreamer-1.0/gst/app/gstappsink.h>
#include <gstreamer-1.0/gst/gst.h>


class VideoTestPublisher
{
public:

    VideoTestPublisher();
    virtual ~VideoTestPublisher();

    eprosima::fastdds::dds::DomainParticipant* mp_participant;
    eprosima::fastdds::dds::Publisher* mp_datapub;
    eprosima::fastdds::dds::Publisher* mp_commandpub;
    eprosima::fastdds::dds::Subscriber* mp_commandsub;
    eprosima::fastdds::dds::Topic* mp_video_topic;
    eprosima::fastdds::dds::Topic* mp_command_pub_topic;
    eprosima::fastdds::dds::Topic* mp_command_sub_topic;
    VideoType* mp_video_out;
    std::chrono::steady_clock::time_point t_start_;
    int n_subscribers;
    unsigned int n_samples;
    std::mutex mutex_;
    int disc_count_;
    std::condition_variable disc_cond_;
    int comm_count_;
    std::condition_variable comm_cond_;
    bool timer_on_;
    std::chrono::steady_clock::time_point send_start_;
    std::condition_variable timer_cond_;
    int m_status;
    unsigned int n_received;
    void init(
            int n_sub,
            int n_sam,
            bool reliable,
            uint32_t pid,
            bool hostname,
            const eprosima::fastdds::rtps::PropertyPolicy& part_property_policy,
            const eprosima::fastdds::rtps::PropertyPolicy& property_policy,
            bool large_data,
            const std::string& sXMLConfigFile,
            int test_time,
            int drop_rate,
            int max_sleep_time,
            int forced_domain,
            int video_width,
            int video_height,
            int frame_rate);
    void run();
    bool test(
            uint32_t datasize);

    class DataPubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        DataPubListener(
                VideoTestPublisher* up)
            : mp_up(up)
            , n_matched(0)
        {
        }

        ~DataPubListener()
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* /*datawriter*/,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;
        VideoTestPublisher* mp_up;
        int n_matched;
    }
    m_datapublistener;

    class CommandPubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        CommandPubListener(
                VideoTestPublisher* up)
            : mp_up(up)
            , n_matched(0)
        {
        }

        ~CommandPubListener()
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* /*datawriter*/,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;
        VideoTestPublisher* mp_up;
        int n_matched;
    }
    m_commandpublistener;

    class CommandSubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        CommandSubListener(
                VideoTestPublisher* up)
            : mp_up(up)
            , n_matched(0)
        {
        }

        ~CommandSubListener()
        {
        }

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* /*datareader*/,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;
        void on_data_available(
                eprosima::fastdds::dds::DataReader* datareader) override;
        VideoTestPublisher* mp_up;
        int n_matched;
    }
    m_commandsublistener;

    std::string m_sXMLConfigFile;
    bool reliable_;

    GstElement* pipeline;
    GstElement* filesrc;
    GstElement* videorate;
    GstElement* sink;
    int m_testTime;
    int m_dropRate;
    int m_sendSleepTime;
    int m_forcedDomain;
    int m_videoWidth;
    int m_videoHeight;
    int m_videoFrameRate;

protected:

    void InitGStreamer();
    static GstFlowReturn new_sample(
            GstElement* sink,
            VideoTestPublisher* sub);
    eprosima::fastdds::dds::DataReaderQos datareader_qos;
    eprosima::fastdds::dds::DataWriterQos datawriter_qos_cmd;
    eprosima::fastdds::dds::DataWriterQos datawriter_qos_data;
    eprosima::fastdds::dds::DataWriter* mp_data_dw;
    eprosima::fastdds::dds::DataReader* mp_dr;
    eprosima::fastdds::dds::DataWriter* mp_command_dw;

};


#endif /* VIDEOPUBLISHER_H_ */
