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
 * @file VideoTestSubscriber.cpp
 *
 */
#include "VideoTestSubscriber.hpp"

#include "VideoTestSubscriber.hpp"

#include <cmath>
#include <fstream>
#include <numeric>
#include <thread>

#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <gtest/gtest.h>

using namespace eprosima;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds;

using std::cout;
using std::endl;

VideoTestSubscriber::VideoTestSubscriber()
    : mp_participant(nullptr)
    , mp_commandpub(nullptr)
    , mp_datasub(nullptr)
    , mp_commandsub(nullptr)
    , disc_count_(0)
    , comm_count_(0)
    , data_count_(0)
    , m_status(0)
    , n_received(0)
    , n_samples(0)
    , m_datasublistener(nullptr)
    , m_commandpublistener(nullptr)
    , m_commandsublistener(nullptr)
    , g_servertimestamp(0)
    , g_clienttimestamp(0)
    , g_framesDropped(0)
    , m_videoWidth(1024)
    , m_videoHeight(720)
    , m_videoFrameRate(30)
{
    m_datasublistener.mp_up = this;
    m_commandpublistener.mp_up = this;
    m_commandsublistener.mp_up = this;

    m_sExportPrefix = "";
    m_forcedDomain = -1;

    m_bRunning = true;
    source_id_ = 0;
}

VideoTestSubscriber::~VideoTestSubscriber()
{
    stop();

    if (mp_participant != nullptr)
    {
        if (mp_commandsub)
        {
            if (mp_commanhd_dr)
            {
                mp_commandsub->delete_datareader(mp_commanhd_dr);
            }
            mp_participant->delete_subscriber(mp_commandsub);
        }
        if (mp_datasub)
        {
            if (mp_data_dr)
            {
                mp_datasub->delete_datareader(mp_data_dr);
            }
            mp_participant->delete_subscriber(mp_datasub);
        }
        if (mp_commandpub)
        {
            if (mp_dw)
            {
                mp_commandpub->delete_datawriter(mp_dw);
            }
            mp_participant->delete_publisher(mp_commandpub);
        }
        if (mp_command_sub_topic)
        {
            mp_participant->delete_topic(mp_command_sub_topic);
        }
        DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
    }

    if (gmain_loop_ != nullptr)
    {
        std::unique_lock<std::mutex> lock(gst_mutex_);
        g_main_loop_quit(gmain_loop_);
        gmain_loop_ = nullptr;
    }

    gst_bin_remove_many(GST_BIN(pipeline), appsrc, videoconvert, sink, NULL);

    thread_.join();
}

void VideoTestSubscriber::init(
        int nsam,
        bool reliable,
        uint32_t pid,
        bool hostname,
        const eprosima::fastdds::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastdds::rtps::PropertyPolicy& property_policy,
        bool large_data,
        const std::string& sXMLConfigFile,
        bool export_csv,
        const std::string& export_prefix,
        int forced_domain,
        int video_width,
        int video_height,
        int frame_rate)
{
    large_data = true;
    m_sXMLConfigFile = sXMLConfigFile;
    n_samples = nsam;
    m_bReliable = reliable;
    m_bExportCsv = export_csv;
    m_sExportPrefix = export_prefix;
    m_forcedDomain = forced_domain;
    m_videoWidth = video_width;
    m_videoHeight = video_height;
    m_videoFrameRate = frame_rate;

    InitGStreamer();

    // Create Participant
    std::string participant_profile_name = "sub_participant_profile";
    DomainParticipantQos participant_qos;

    participant_qos.name("video_test_subscriber");

    participant_qos.properties(part_property_policy);

    if (m_sXMLConfigFile.length() > 0)
    {
        if (m_forcedDomain >= 0)
        {
            mp_participant = DomainParticipantFactory::get_instance()->create_participant_with_profile(m_forcedDomain,
                            participant_profile_name);
        }
        else
        {
            mp_participant = DomainParticipantFactory::get_instance()->create_participant_with_profile(
                participant_profile_name);
        }
    }
    else
    {
        if (m_forcedDomain >= 0)
        {
            mp_participant = DomainParticipantFactory::get_instance()->create_participant(
                m_forcedDomain, participant_qos);
        }
        else
        {
            mp_participant = DomainParticipantFactory::get_instance()->create_participant(
                pid % 230, participant_qos);
        }
    }

    ASSERT_NE(mp_participant, nullptr);

    // Register the type
    TypeSupport video_type;
    TypeSupport command_type;
    video_type.reset(new VideoDataType());
    command_type.reset(new TestCommandDataType());
    ASSERT_EQ(mp_participant->register_type(video_type), RETCODE_OK);
    ASSERT_EQ(mp_participant->register_type(command_type), RETCODE_OK);

    // Create Data Subscriber
    std::string profile_name = "subscriber_profile";

    if (m_sXMLConfigFile.length() > 0)
    {
        mp_datasub = mp_participant->create_subscriber_with_profile(profile_name);
    }
    else
    {
        mp_datasub = mp_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    }
    ASSERT_NE(mp_datasub, nullptr);
    ASSERT_TRUE(mp_datasub->is_enabled());

    // Create topic
    std::ostringstream video_topic_name;
    video_topic_name << "VideoTest_";
    if (hostname)
    {
        video_topic_name << asio::ip::host_name() << "_";
    }
    video_topic_name << pid << "_PUB2SUB";
    mp_video_topic = mp_participant->create_topic(video_topic_name.str(),
                    "VideoType", TOPIC_QOS_DEFAULT);
    ASSERT_NE(mp_video_topic, nullptr);
    ASSERT_TRUE(mp_video_topic->is_enabled());

    // Create data DataReader
    if (m_bReliable)
    {
        datareader_qos_data.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        datareader_qos_data.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    }
    datareader_qos_data.properties(property_policy);

    if (large_data)
    {
        datareader_qos_data.endpoint().history_memory_policy =
                eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    }

    mp_data_dr = mp_datasub->create_datareader(mp_video_topic, datareader_qos_data, &this->m_datasublistener);
    ASSERT_NE(mp_data_dr, nullptr);
    ASSERT_TRUE(mp_data_dr->is_enabled());


    // Create Command Publisher
    mp_commandpub = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    // Create topic
    std::ostringstream pub_cmd_topic_name;
    pub_cmd_topic_name << "VideoTest_Command_";
    if (hostname)
    {
        pub_cmd_topic_name << asio::ip::host_name() << "_";
    }
    pub_cmd_topic_name << pid << "_SUB2PUB";

    mp_command_pub_topic = mp_participant->create_topic(pub_cmd_topic_name.str(),
                    "TestCommandType", TOPIC_QOS_DEFAULT);
    ASSERT_NE(mp_command_pub_topic, nullptr);
    ASSERT_TRUE(mp_command_pub_topic->is_enabled());

    // Create DataWriter
    datawriter_qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    datawriter_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datawriter_qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_dw = mp_commandpub->create_datawriter(mp_command_pub_topic, datawriter_qos,
                    &this->m_commandpublistener);
    ASSERT_NE(mp_dw, nullptr);
    ASSERT_TRUE(mp_dw->is_enabled());


    // Create Command Subscriber
    mp_commandsub = mp_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(mp_commandsub, nullptr);
    ASSERT_TRUE(mp_commandsub->is_enabled());

    // Create topic
    std::ostringstream sub_cmd_topic_name;
    sub_cmd_topic_name << "VideoTest_Command_";
    if (hostname)
    {
        sub_cmd_topic_name << asio::ip::host_name() << "_";
    }
    sub_cmd_topic_name << pid << "_PUB2SUB";
    mp_command_sub_topic = mp_participant->create_topic(sub_cmd_topic_name.str(),
                    "TestCommandType", TOPIC_QOS_DEFAULT);
    ASSERT_NE(mp_command_sub_topic, nullptr);
    ASSERT_TRUE(mp_command_sub_topic->is_enabled());

    datareader_qos_cmd.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    datareader_qos_cmd.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos_cmd.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;

    mp_commanhd_dr = mp_commandsub->create_datareader(mp_command_sub_topic, datareader_qos_cmd,
                    &this->m_commandsublistener);
    ASSERT_NE(mp_commanhd_dr, nullptr);
    ASSERT_TRUE(mp_commanhd_dr->is_enabled());
}

void VideoTestSubscriber::DataSubListener::on_subscription_matched(
        DataReader* /*datareader*/,
        const SubscriptionMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);
    if (info.current_count_change > 0)
    {
        EPROSIMA_LOG_INFO(VideoTest, "Data Sub Matched ");
        std::cout << "Data Sub Matched " << std::endl;
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
        mp_up->m_status = 0;
        mp_up->comm_cond_.notify_one();
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void VideoTestSubscriber::CommandPubListener::on_publication_matched(
        DataWriter* /*datawriter*/,
        const PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if (info.current_count_change > 0)
    {
        EPROSIMA_LOG_INFO(VideoTest, "Command Pub Matched ");
        std::cout << "Command Pub Matched " << std::endl;
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
        mp_up->m_status = 0;
        mp_up->comm_cond_.notify_one();
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void VideoTestSubscriber::CommandSubListener::on_subscription_matched(
        DataReader* /*datareader*/,
        const SubscriptionMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);
    if (info.current_count_change > 0)
    {
        EPROSIMA_LOG_INFO(VideoTest, "Command Sub Matched ");
        std::cout << "Command Sub Matched " << std::endl;
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
        mp_up->m_status = 0;
        mp_up->comm_cond_.notify_one();
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void VideoTestSubscriber::CommandSubListener::on_data_available(
        DataReader* datareader)
{
    SampleInfo info;
    TestCommandType command;
    if (RETCODE_OK == datareader->take_next_sample((void*)&command, &info))
    {
        if (info.valid_data)
        {
            if (command.m_command == READY)
            {
                cout << "Publisher has new test ready..." << endl;
                mp_up->mutex_.lock();
                ++mp_up->comm_count_;
                mp_up->mutex_.unlock();
                mp_up->comm_cond_.notify_one();
            }
            else if (command.m_command == STOP)
            {
                cout << "Publisher has stopped the test" << endl;
                mp_up->mutex_.lock();
                ++mp_up->data_count_;
                mp_up->mutex_.unlock();
                mp_up->comm_cond_.notify_one();
                mp_up->data_cond_.notify_one();
            }
            else if (command.m_command == STOP_ERROR)
            {
                cout << "Publisher has canceled the test" << endl;
                mp_up->m_status = -1;
                mp_up->mutex_.lock();
                ++mp_up->data_count_;
                mp_up->mutex_.unlock();
                mp_up->comm_cond_.notify_one();
                mp_up->data_cond_.notify_one();
            }
            else if (command.m_command == DEFAULT)
            {
                std::cout << "Something is wrong" << std::endl;
            }
        }
    }
}

void VideoTestSubscriber::DataSubListener::on_data_available(
        DataReader* datareader)
{
    VideoType videoData;
    SampleInfo info;
    datareader->take_next_sample((void*)&videoData, &info);
    {
        mp_up->push_video_packet(videoData);
    }
}

void VideoTestSubscriber::run()
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&]()
            {
                return disc_count_ >= 3;
            });
    disc_lock.unlock();

    cout << C_B_MAGENTA << "DISCOVERY COMPLETE " << C_DEF << endl;

    this->test();
}

void VideoTestSubscriber::gst_run(
        VideoTestSubscriber* sub)
{
    if (!sub)
    {
        printf("VideoTestSubscriber::gst_run -> BAD PARAMETERS!\n");
        return;
    }

    // Create a GLib Main Loop and set it to run
    sub->gmain_loop_ = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(sub->gmain_loop_);
}

bool VideoTestSubscriber::test()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (comm_count_ == 0)
    {
        comm_cond_.wait(lock);
    }
    --comm_count_;
    lock.unlock();

    cout << "TEST STARTED" << endl;

    t_start_ = std::chrono::steady_clock::now();
    t_drop_start_ = t_start_;
    m_status = 0;
    n_received = 0;
    samples_.clear();
    drops_.clear();
    avgs_.clear();
    TestCommandType command;
    command.m_command = BEGIN;
    mp_dw->write(&command);

    lock.lock();
    data_cond_.wait(lock, [&]()
            {
                return data_count_ > 0;
            });
    --data_count_;
    lock.unlock();

    cout << "TEST FINISHED" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    analyzeTimes();
    if (m_stats.size() > 0)
    {
        printStat(m_stats.back());
    }

    if (m_status == -1)
    {
        return false;
    }
    return true;
}

void VideoTestSubscriber::fps_stats_cb(
        GstElement* /*source*/,
        gdouble /*fps*/,
        gdouble /*droprate*/,
        gdouble avgfps,
        VideoTestSubscriber* sub)
{
    std::unique_lock<std::mutex> lock(sub->stats_mutex_);

    sub->t_end_ = std::chrono::steady_clock::now();
    sub->samples_.push_back(sub->t_end_ - sub->t_start_);

    sub->t_drop_end_ = sub->t_end_;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(sub->t_drop_end_ - sub->t_drop_start_).count() >= 1000)
    {
        sub->t_drop_start_ = sub->t_drop_end_;
        sub->g_servertimestamp = sub->g_clienttimestamp;
        sub->drops_.push_back(static_cast<double> (sub->g_framesDropped));
    }

    sub->avgs_.push_back(avgfps);
}

void VideoTestSubscriber::InitGStreamer()
{
    bool ok = true;
    std::string id_ = "tst";
    std::stringstream default_name;
    default_name << "pipeline_" << id_.c_str();
    pipeline = gst_element_factory_make("pipeline", default_name.str().c_str());
    ok = (pipeline != nullptr);
    if (ok)
    {
        appsrc = gst_element_factory_make("appsrc", "source");
        ok = (appsrc != nullptr);
        if (ok)
        {
            g_object_set(appsrc, "is-live", TRUE, NULL);
            g_object_set(appsrc, "do-timestamp", TRUE, NULL);
            g_signal_connect(appsrc, "need-data", G_CALLBACK(start_feed_cb), this);
            g_signal_connect(appsrc, "enough-data", G_CALLBACK(stop_feed_cb), this);
            GstCaps* caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420",
                            "width", G_TYPE_INT, m_videoWidth, "height", G_TYPE_INT, m_videoHeight,
                            "framerate", GST_TYPE_FRACTION, m_videoFrameRate, 1, NULL);
            gst_app_src_set_caps(GST_APP_SRC(appsrc), caps);
            gst_caps_unref(caps);

            videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
            ok = (videoconvert != nullptr);
            if (ok)
            {
                g_object_set(videoconvert, "qos", TRUE, NULL);

                sink = gst_element_factory_make("fpsdisplaysink", "sink"); //fpsdisplaysink autovideosink
                ok = (sink != nullptr);
                if (ok)
                {
                    g_object_set(sink, "sync", FALSE, NULL);
                    g_object_set(sink, "text-overlay", FALSE, NULL);
                    g_object_set(sink, "signal-fps-measurements", TRUE, NULL);
                    g_signal_connect(G_OBJECT(sink), "fps-measurements", (GCallback)fps_stats_cb, this);

                    gst_bin_add_many(GST_BIN(pipeline), appsrc, videoconvert, sink, NULL);
                    ok = gst_element_link_many(appsrc, videoconvert, sink, NULL) == TRUE;
                    if (ok)
                    {
                        GstBus* bus = gst_element_get_bus(pipeline);
                        gst_bus_add_signal_watch(bus);
                        g_signal_connect(G_OBJECT(bus), "message", (GCallback)message_cb, this);
                        gst_object_unref(bus);

                        gst_element_set_state(pipeline, GST_STATE_PLAYING);
                        thread_ = std::thread(VideoTestSubscriber::gst_run, this);
                    }
                }
            }
        }
    }
}

void VideoTestSubscriber::start_feed_cb(
        GstElement* /*source*/,
        guint /*size*/,
        VideoTestSubscriber* sub)
{
    if (sub->source_id_ == 0)
    {
        sub->source_id_ = g_idle_add((GSourceFunc)push_data_cb, sub);
    }
}

void VideoTestSubscriber::stop_feed_cb(
        GstElement* /*source*/,
        VideoTestSubscriber* sub)
{
    if (sub->source_id_ != 0)
    {
        g_source_remove(sub->source_id_);
        sub->source_id_ = 0;
    }
}

#define WAIT_AFTER_LAST_FEED_MS 2000
gboolean VideoTestSubscriber::push_data_cb(
        VideoTestSubscriber* sub)
{
    std::unique_lock<std::mutex> lock(sub->gst_mutex_);
    if (sub->m_bRunning)
    {
        GstBuffer* buffer;
        GstFlowReturn ret;
        GstMapInfo map;
        gint num_samples = 0;

        const VideoType& vpacket = sub->pop_video_packet();

        // Create a new empty buffer
        gsize size = vpacket.data.size();
        buffer = gst_buffer_new_and_alloc(size);

        {
            std::unique_lock<std::mutex> stats_lock(sub->stats_mutex_);
            sub->g_framesDropped = static_cast<gint64>((vpacket.timestamp - sub->g_servertimestamp) / 33333333);

            //std::cout << "TIMESTAMP : " << std::to_string(sub->g_servertimestamp) << " _ " << std::to_string(vpacket.timestamp) << std::endl;
            sub->g_servertimestamp += vpacket.duration;
            sub->g_clienttimestamp = vpacket.timestamp;

            // Set its timestamp and duration
            //GST_BUFFER_TIMESTAMP(buffer) = gst_util_uint64_scale(sink->num_samples, GST_SECOND, SAMPLE_RATE);
            //GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(num_samples, GST_SECOND, SAMPLE_RATE);
            //GST_BUFFER_PTS(buffer) = sub->g_servertimestamp;
            //GST_BUFFER_DURATION(buffer) = vpacket.duration;
        }


        gst_buffer_map(buffer, &map, GST_MAP_WRITE);
        memmove(map.data, vpacket.data.data(), size);
        ++num_samples;

        gst_buffer_unmap(buffer, &map);
        //sub->num_samples_ += num_samples;

        // Push the buffer into the appsrc
        g_signal_emit_by_name(sub->appsrc, "push-buffer", buffer, &ret);

        // Free the buffer now that we are done with it
        gst_buffer_unref(buffer);

        if (ret != GST_FLOW_OK)
        {
            // We got some error, stop sending data
            std::cout << "Error on received frame" << std::endl;
            return FALSE;
        }
    }
    return TRUE;
}

void VideoTestSubscriber::stop()
{
    m_bRunning = false;
    deque_cond_.notify_one();
    gst_element_set_state(pipeline, GST_STATE_NULL);
}

void VideoTestSubscriber::message_cb(
        GstBus* /*bus*/,
        GstMessage* message,
        gpointer /*user_data*/)
{
    GError* err = nullptr;
    gchar* debug_info = nullptr;
    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_ERROR:
        {
            gst_message_parse_error(message, &err, &debug_info);

            printf("# GST INTERNAL # Error received from element: %s; message: %s\n", GST_OBJECT_NAME(
                        message->src), err->message);
            printf("# GST INTERNAL # Debugging information: %s\n", debug_info ? debug_info : "none");

            g_clear_error(&err);
            if (err)
            {
                g_error_free(err);
            }
            if (debug_info)
            {
                g_free(debug_info);
            }

            //if (loop) g_main_loop_quit(loop);
        }
        break;
        case GST_MESSAGE_WARNING:
        {
            gst_message_parse_warning(message, &err, &debug_info);

            printf("# GST INTERNAL # Warning received from element: %s; message: %s\n", GST_OBJECT_NAME(
                        message->src), err->message);
            printf("# GST INTERNAL # Debugging information: %s\n", debug_info ? debug_info : "none");

            g_clear_error(&err);
            if (err)
            {
                g_error_free(err);
            }
            if (debug_info)
            {
                g_free(debug_info);
            }
        }
        break;
        case GST_MESSAGE_INFO:
        {
            gst_message_parse_info(message, &err, &debug_info);

            printf("# GST INTERNAL # Info received from element: %s; message: %s\n", GST_OBJECT_NAME(
                        message->src), err->message);
            printf("# GST INTERNAL # Debugging information: %s\n", debug_info ? debug_info : "none");

            g_clear_error(&err);
            g_error_free(err);
            g_free(debug_info);
        }
        break;
        case GST_MESSAGE_EOS:
        {
            printf("# GST INTERNAL # Got EOS\n");
            //if (loop) g_main_loop_quit(loop);
        }
        break;
        default:
            break;
    }
}

void VideoTestSubscriber::analyzeTimes()
{
    if (samples_.size() > 0)
    {
        for (uint32_t i = 0; i < drops_.size(); ++i)
        {
            drops_[i] -= drops_[0];
        }

        if (drops_.size() > 1)
        {
            drops_.pop_back();
        }

        TimeStats TS;
        TS.received = static_cast<uint32_t>(samples_.size());
        {
            if (avgs_.size() > 0)
            {
                // AVG
                TS.m_minAvg = *std::min_element(avgs_.begin(), avgs_.end());
                TS.m_maxAvg = *std::max_element(avgs_.begin(), avgs_.end());

                TS.pAvgMean = std::accumulate(avgs_.begin(), avgs_.end(), double(0)) / avgs_.size();
                double auxstdev = 0;
                for (std::vector<double>::iterator tit = avgs_.begin(); tit != avgs_.end(); ++tit)
                {
                    auxstdev += pow(((*tit) - TS.pAvgMean), 2);
                }
                auxstdev = sqrt(auxstdev / avgs_.size());
                TS.pAvgStdev = auxstdev;
                //TS.pAvgStdev = static_cast<double>(round(auxstdev));

                std::sort(avgs_.begin(), avgs_.end());
                size_t elem = 0;

                elem = static_cast<size_t>(avgs_.size() * 0.5);
                if (elem > 0 && elem <= avgs_.size())
                {
                    TS.pAvg50 = avgs_.at(--elem);
                }
                else
                {
                    if (avgs_.size() == 1)
                    {
                        TS.pAvg50 = *avgs_.begin();
                    }
                    else
                    {
                        TS.pAvg50 = NAN;
                    }
                }

                elem = static_cast<size_t>(avgs_.size() * 0.9);
                if (elem > 0 && elem <= avgs_.size())
                {
                    TS.pAvg90 = avgs_.at(--elem);
                }
                else
                {
                    if (avgs_.size() == 1)
                    {
                        TS.pAvg90 = *avgs_.begin();
                    }
                    else
                    {
                        TS.pAvg90 = NAN;
                    }
                }

                elem = static_cast<size_t>(avgs_.size() * 0.99);
                if (elem > 0 && elem <= avgs_.size())
                {
                    TS.pAvg99 = avgs_.at(--elem);
                }
                else
                {
                    if (avgs_.size() == 1)
                    {
                        TS.pAvg99 = *avgs_.begin();
                    }
                    else
                    {
                        TS.pAvg99 = NAN;
                    }
                }

                elem = static_cast<size_t>(avgs_.size() * 0.9999);
                if (elem > 0 && elem <= avgs_.size())
                {
                    TS.pAvg9999 = avgs_.at(--elem);
                }
                else
                {
                    if (avgs_.size() == 1)
                    {
                        TS.pAvg9999 = *avgs_.begin();
                    }
                    else
                    {
                        TS.pAvg9999 = NAN;
                    }
                }
            }
        }
        {
            if (drops_.size() > 0)
            {
                // DROP
                TS.m_minDrop = *std::min_element(drops_.begin(), drops_.end());
                TS.m_maxDrop = *std::max_element(drops_.begin(), drops_.end());

                TS.pDropMean = std::accumulate(drops_.begin(), drops_.end(), double(0)) / drops_.size();
                double auxstdev = 0;
                for (std::vector<double>::iterator tit = drops_.begin(); tit != drops_.end(); ++tit)
                {
                    auxstdev += pow(((*tit) - TS.pDropMean), 2);
                }
                auxstdev = sqrt(auxstdev / drops_.size());
                //TS.pDropStdev = static_cast<double>(round(auxstdev));
                TS.pDropStdev = auxstdev;

                std::sort(drops_.begin(), drops_.end());
                size_t elem = 0;

                elem = static_cast<size_t>(drops_.size() * 0.5);
                if (elem > 0 && elem <= drops_.size())
                {
                    TS.pDrop50 = drops_.at(--elem);
                }
                else
                {
                    if (drops_.size() == 1)
                    {
                        TS.pDrop50 = *drops_.begin();
                    }
                    else
                    {
                        TS.pDrop50 = NAN;
                    }
                }

                elem = static_cast<size_t>(drops_.size() * 0.9);
                if (elem > 0 && elem <= drops_.size())
                {
                    TS.pDrop90 = drops_.at(--elem);
                }
                else
                {
                    if (drops_.size() == 1)
                    {
                        TS.pDrop90 = *drops_.begin();
                    }
                    else
                    {
                        TS.pDrop90 = NAN;
                    }
                }

                elem = static_cast<size_t>(drops_.size() * 0.99);
                if (elem > 0 && elem <= drops_.size())
                {
                    TS.pDrop99 = drops_.at(--elem);
                }
                else
                {
                    if (drops_.size() == 1)
                    {
                        TS.pDrop99 = *drops_.begin();
                    }
                    else
                    {
                        TS.pDrop99 = NAN;
                    }
                }

                elem = static_cast<size_t>(drops_.size() * 0.9999);
                if (elem > 0 && elem <= drops_.size())
                {
                    TS.pDrop9999 = drops_.at(--elem);
                }
                else
                {
                    if (drops_.size() == 1)
                    {
                        TS.pDrop9999 = *drops_.begin();
                    }
                    else
                    {
                        TS.pDrop9999 = NAN;
                    }
                }
            }
        }

        m_stats.push_back(TS);
    }
}

void VideoTestSubscriber::printStat(
        TimeStats& TS)
{
    std::ofstream outFile;
    std::ofstream outMeanFile;
    std::stringstream output_file_csv;
    std::stringstream output_mean_csv;
    std::string str_reliable = "besteffort";
    if (m_bReliable)
    {
        str_reliable = "reliable";
    }

    output_file_csv <<
        "Samples, Avg stdev, Avg Mean, min Avg, Avg 50 %%, Avg 90 %%, Avg 99 %%, \
        Avg 99.99%%, Avg max, Drop stdev, Drop Mean, min Drop, Drop 50 %%, Drop 90 %%, Drop 99 %%, \
        Drop 99.99%%, Drop max" << std::endl;

    output_mean_csv << "Avg Mean" << std::endl;

    printf("Statistics for video test \n");
    printf(
        "    Samples,  Avg stdev,   Avg Mean,    min Avg,    Avg 50%%,    Avg 90%%,    Avg 99%%,   Avg 99.99%%,    Avg max\n");
    printf(
        "-----------,-----------,-----------,-----------,-----------,-----------,-----------,-------------,-----------\n");
    printf("%11u,%11.2f,%11.2f,%11.2f,%11.2f,%11.2f,%11.2f,%13.2f,%11.2f \n\n\n",
            TS.received, TS.pAvgStdev, TS.pAvgMean, TS.m_minAvg, TS.pAvg50, TS.pAvg90, TS.pAvg99, TS.pAvg9999,
            TS.m_maxAvg);

    printf(
        "    Samples, FameDrop stdev, FameDrop Mean, min FameDrop,  FameDrop 50%%,  FameDrop 90%%,  FameDrop 99%%, FameDrop 99.99%%,  FameDrop max\n");
    printf(
        "-----------,---------------,--------------,-------------,--------------,--------------,--------------,----------------,--------------\n");
    printf("%11u,%15.2f,%14.2f,%13.2f,%14.2f,%14.2f,%14.2f,%16.2f,%14.2f \n",
            TS.received, TS.pDropStdev, TS.pDropMean, TS.m_minDrop, TS.pDrop50, TS.pDrop90, TS.pDrop99, TS.pDrop9999,
            TS.m_maxDrop);

    output_file_csv << TS.received << "," << TS.pAvgStdev << "," << TS.pAvgMean << "," << TS.m_minAvg << "," <<
        TS.pAvg50 << "," << TS.pAvg90 << "," << TS.pAvg99 << "," << TS.pAvg9999 << "," << TS.m_maxAvg << "," <<
        TS.pDropStdev << "," << TS.pDropMean << "," << TS.m_minDrop << "," << TS.pDrop50 << "," << TS.pDrop90 <<
        "," << TS.pDrop99 << "," << TS.pDrop9999 << "," << TS.m_maxDrop << "," << std::endl;

    output_mean_csv << TS.pAvgMean << "," << std::endl;

    if (m_bExportCsv)
    {
        if (m_sExportPrefix.length() > 0)
        {
            outFile.open(m_sExportPrefix + ".csv");
            outMeanFile.open(m_sExportPrefix + "_Mean.csv");
            outMeanFile << output_mean_csv.str();
        }
        else
        {
            outFile.open("perf_VideoTest_" + str_reliable + ".csv");
        }

        outFile << output_file_csv.str();
    }
}
