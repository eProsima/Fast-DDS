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
 * @file VideoPublisher.cpp
 *
 */

#include "VideoTestPublisher.hpp"

#include <cstdint>
#include <fstream>
#include <thread>

#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <gtest/gtest.h>

#define TIME_LIMIT_US 10000

using namespace eprosima;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;

using std::cout;
using std::endl;

VideoTestPublisher::VideoTestPublisher()
    : mp_participant(nullptr)
    , mp_datapub(nullptr)
    , mp_commandpub(nullptr)
    , mp_commandsub(nullptr)
    , mp_video_out(nullptr)
    , n_subscribers(0)
    , n_samples(0)
    , disc_count_(0)
    , comm_count_(0)
    , timer_on_(false)
    , m_status(0)
    , n_received(0)
    , m_datapublistener(nullptr)
    , m_commandpublistener(nullptr)
    , m_commandsublistener(nullptr)
    , m_dropRate(0)
    , m_sendSleepTime(0)
    , m_forcedDomain(-1)
    , m_videoWidth(1024)
    , m_videoHeight(720)
    , m_videoFrameRate(30)
{
    m_datapublistener.mp_up = this;
    m_commandpublistener.mp_up = this;
    m_commandsublistener.mp_up = this;
}

VideoTestPublisher::~VideoTestPublisher()
{
    if (sink)
    {
        gst_object_unref(GST_OBJECT(sink));
        sink = nullptr;
    }

    if (videorate)
    {
        gst_object_unref(GST_OBJECT(videorate));
        videorate = nullptr;
    }

    if (filesrc)
    {
        gst_object_unref(GST_OBJECT(filesrc));
        filesrc = nullptr;
    }

    if (pipeline)
    {
        gst_object_unref(GST_OBJECT(pipeline));
        pipeline = nullptr;
    }

    if (mp_participant != nullptr)
    {
        if (mp_commandsub)
        {
            if (mp_dr)
            {
                mp_commandsub->delete_datareader(mp_dr);
            }
            mp_participant->delete_subscriber(mp_commandsub);
        }
        if (mp_datapub)
        {
            if (mp_data_dw)
            {
                mp_datapub->delete_datawriter(mp_data_dw);
            }
            mp_participant->delete_publisher(mp_datapub);
        }
        if (mp_commandpub)
        {
            if (mp_command_dw)
            {
                mp_commandpub->delete_datawriter(mp_command_dw);
            }
            mp_participant->delete_publisher(mp_commandpub);
        }
        if (mp_command_sub_topic)
        {
            mp_participant->delete_topic(mp_command_sub_topic);
        }
        if (mp_video_topic)
        {
            mp_participant->delete_topic(mp_video_topic);
        }
        if (mp_command_pub_topic)
        {
            mp_participant->delete_topic(mp_command_pub_topic);
        }
        DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
    }
}

void VideoTestPublisher::init(
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
        int videoWidth,
        int videoHeight,
        int videoFrameRate)
{
    large_data = true;
    m_testTime = test_time;
    m_dropRate = drop_rate;
    m_sendSleepTime = max_sleep_time;
    m_sXMLConfigFile = sXMLConfigFile;
    n_samples = n_sam;
    n_subscribers = n_sub;
    reliable_ = reliable;
    m_forcedDomain = forced_domain;
    m_videoWidth = videoWidth;
    m_videoHeight = videoHeight;
    m_videoFrameRate = videoFrameRate;

    // GSTREAMER PIPELINE INITIALIZATION.
    InitGStreamer();

    // Create Participant
    std::string participant_profile_name = "pub_participant_profile";
    DomainParticipantQos participant_qos;

    participant_qos.properties(part_property_policy);

    participant_qos.name("video_test_publisher");

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
    TypeSupport type_video;
    TypeSupport type_command;
    type_video.reset(new VideoDataType());
    type_command.reset(new TestCommandDataType());
    ASSERT_EQ(mp_participant->register_type(type_video), RETCODE_OK);
    ASSERT_EQ(mp_participant->register_type(type_command), RETCODE_OK);

    // Create Data Publisher
    std::string profile_name = "publisher_profile";

    if (m_sXMLConfigFile.length() > 0)
    {
        mp_datapub = mp_participant->create_publisher_with_profile(profile_name);
    }
    else
    {
        mp_datapub = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    }
    ASSERT_NE(mp_datapub, nullptr);
    ASSERT_TRUE(mp_datapub->is_enabled());

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

    // Create Data DataWriter
    if (reliable_)
    {
        datawriter_qos_data.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        datawriter_qos_data.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    }

    datawriter_qos_data.properties(property_policy);

    if (large_data)
    {
        datawriter_qos_data.endpoint().history_memory_policy =
                eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        datawriter_qos_data.publish_mode().kind = PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE;
    }
    datawriter_qos_data.reliable_writer_qos().times.heartbeat_period.seconds = 0;
    datawriter_qos_data.reliable_writer_qos().times.heartbeat_period.nanosec = 100000000;

    mp_data_dw = mp_datapub->create_datawriter(mp_video_topic, datawriter_qos_data, &this->m_datapublistener);
    ASSERT_NE(mp_data_dw, nullptr);
    ASSERT_TRUE(mp_data_dw->is_enabled());


    // Create Command Publisher
    mp_commandpub = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    // Create topic
    std::ostringstream command_topic_name;
    command_topic_name << "VideoTest_Command_";
    if (hostname)
    {
        command_topic_name << asio::ip::host_name() << "_";
    }
    command_topic_name << pid << "_PUB2SUB";

    mp_command_pub_topic = mp_participant->create_topic(command_topic_name.str(),
                    "TestCommandType", TOPIC_QOS_DEFAULT);
    ASSERT_NE(mp_command_pub_topic, nullptr);
    ASSERT_TRUE(mp_command_pub_topic->is_enabled());

    //Create Command DataWriter
    datawriter_qos_cmd.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    datawriter_qos_cmd.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datawriter_qos_cmd.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    datawriter_qos_cmd.publish_mode().kind = PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE;

    mp_command_dw = mp_commandpub->create_datawriter(mp_command_pub_topic, datawriter_qos_cmd,
                    &this->m_commandpublistener);
    ASSERT_NE(mp_command_dw, nullptr);
    ASSERT_TRUE(mp_command_dw->is_enabled());


    // Create subscriber
    mp_commandsub = mp_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(mp_commandsub, nullptr);
    ASSERT_TRUE(mp_commandsub->is_enabled());

    // Create topic
    std::ostringstream sub_topic_name;
    sub_topic_name << "VideoTest_Command_";
    if (hostname)
    {
        sub_topic_name << asio::ip::host_name() << "_";
    }
    sub_topic_name << pid << "_SUB2PUB";

    mp_command_sub_topic = mp_participant->create_topic(sub_topic_name.str(),
                    "TestCommandType", eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    ASSERT_NE(mp_command_sub_topic, nullptr);
    ASSERT_TRUE(mp_command_sub_topic->is_enabled());

    // Create DataReader
    datareader_qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
    datareader_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    datareader_qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_dr = mp_commandsub->create_datareader(mp_command_sub_topic, datareader_qos, &this->m_commandsublistener);
    ASSERT_NE(mp_dr, nullptr);
    ASSERT_TRUE(mp_dr->is_enabled());

}

void VideoTestPublisher::DataPubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter* /*datawriter*/,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if (info.current_count_change > 0)
    {
        cout << C_MAGENTA << "Data Pub Matched " << C_DEF << endl;

        n_matched++;
        if (n_matched > mp_up->n_subscribers)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            mp_up->m_status = -1;
        }

        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void VideoTestPublisher::CommandPubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter* /*datawriter*/,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if (info.current_count_change > 0)
    {
        cout << C_MAGENTA << "Command Pub Matched " << C_DEF << endl;

        n_matched++;
        if (n_matched > mp_up->n_subscribers)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            mp_up->m_status = -1;
        }

        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void VideoTestPublisher::CommandSubListener::on_subscription_matched(
        eprosima::fastdds::dds::DataReader* /*datareader*/,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);
    if (info.current_count_change > 0)
    {
        cout << C_MAGENTA << "Command Sub Matched " << C_DEF << endl;

        n_matched++;
        if (n_matched > mp_up->n_subscribers)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            mp_up->m_status = -1;
        }

        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void VideoTestPublisher::CommandSubListener::on_data_available(
        eprosima::fastdds::dds::DataReader* datareader)
{
    ASSERT_NE(datareader, nullptr);

    TestCommandType command;
    eprosima::fastdds::dds::SampleInfo info;
    if (RETCODE_OK == datareader->take_next_sample((void*)&command, &info))
    {
        if (info.valid_data)
        {
            if (command.m_command == BEGIN)
            {
                mp_up->mutex_.lock();
                ++mp_up->comm_count_;
                mp_up->mutex_.unlock();
                mp_up->comm_cond_.notify_one();
            }
        }
    }
    else
    {
        cout << "Problem reading" << endl;
    }
}

void VideoTestPublisher::run()
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&]()
            {
                return disc_count_ >= (n_subscribers * 3);
            });
    disc_lock.unlock();

    cout << C_B_MAGENTA << "DISCOVERY COMPLETE " << C_DEF << endl;

    this->test(0);
}

bool VideoTestPublisher::test(
        uint32_t datasize)
{
    m_status = 0;
    n_received = 0;
    // Create video sample with size <datasize>
    mp_video_out = new VideoType(datasize);

    // Send READY command
    TestCommandType command;
    command.m_command = READY;
    mp_command_dw->write(&command);

    std::unique_lock<std::mutex> lock(mutex_);
    // Wait for all the subscribers
    comm_cond_.wait(lock, [&]()
            {
                return comm_count_ >= n_subscribers;
            });
    --comm_count_;

    // BEGIN THE TEST
    // --------------------------------------------------------------
    // Get new sample
    mp_video_out = new VideoType();
    // Start "playing" on the pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Get start time
    send_start_ = std::chrono::steady_clock::now();
    // Wait until timer returns "finished"
    timer_cond_.wait(lock, [&]()
            {
                return timer_on_;
            });

    // Paused the sending
    gst_element_set_state(pipeline, GST_STATE_PAUSED);

    // Send STOP command to subscriber
    command.m_command = STOP;
    mp_command_dw->write(&command);

    // Wait until all subscribers unmatch
    disc_cond_.wait(lock, [&]()
            {
                return disc_count_ == 0;
            });

    if (m_status != 0)
    {
        cout << "Error in test " << endl;
        return false;
    }
    // --------------------------------------------------------------
    //TEST FINISHED

    // Clean up
    size_t removed = 0;
    mp_data_dw->clear_history(&removed);
    delete(mp_video_out);

    return true;
}

void VideoTestPublisher::InitGStreamer()
{
    bool ok = true;
    std::string id_ = "tst";
    std::stringstream default_name;
    default_name << "pipeline_" << id_.c_str();
    pipeline = gst_element_factory_make("pipeline", default_name.str().c_str());
    ok = (pipeline != nullptr);
    if (ok)
    {
        filesrc = gst_element_factory_make("videotestsrc", "source");
        ok = (filesrc != nullptr);
        if (ok)
        {
            g_object_set(filesrc, "is-live", TRUE, NULL);

            videorate = gst_element_factory_make("videorate", "rate"); //autovideosink
            ok = (videorate != nullptr);
            if (ok)
            {
                sink = gst_element_factory_make("appsink", "sink"); //autovideosink appsink fpsdisplaysink
                ok = (sink != nullptr);
                if (ok)
                {
                    g_object_set(sink, "emit-signals", TRUE, NULL);
                    g_signal_connect(sink, "new-sample", G_CALLBACK(new_sample), this);

                    /*std::string sFramerate = std::to_string(m_videoFrameRate) + "/1";*/
                    GstCaps* caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420",
                                    "width", G_TYPE_INT, m_videoWidth, "height", G_TYPE_INT, m_videoHeight,
                                    "framerate", GST_TYPE_FRACTION, m_videoFrameRate, 1, NULL);

                    // Link the camera source and colorspace filter using capabilities specified
                    gst_bin_add_many(GST_BIN(pipeline), filesrc, videorate, sink, NULL);
                    ok = gst_element_link_filtered(filesrc, videorate, caps) == TRUE;
                    ok = gst_element_link_filtered(videorate, sink, caps) == TRUE;
                    gst_caps_unref(caps);
                }
            }
        }
    }
}

/* The appsink has received a buffer */
GstFlowReturn VideoTestPublisher::new_sample(
        GstElement* sink,
        VideoTestPublisher* sub)
{
    GstFlowReturn returned_value = GST_FLOW_ERROR;
    if (sub->mp_video_out != nullptr)
    {
        if (sub->m_sendSleepTime != 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % sub->m_sendSleepTime));
        }

        GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
        if (sample)
        {
            GstBuffer* buffer = gst_sample_get_buffer(sample);
            if (buffer != nullptr)
            {
                GstClockTime timestamp = GST_BUFFER_PTS(buffer);
                GstClockTime duration = GST_BUFFER_DURATION(buffer);
                GstMapInfo map;
                if (gst_buffer_map(buffer, &map, GST_MAP_READ))
                {
                    sub->mp_video_out->seqnum++;
                    sub->mp_video_out->duration = duration;
                    sub->mp_video_out->timestamp = timestamp;

                    //std::cout << "NEW SAMPLE " << timestamp << std::endl;

                    sub->mp_video_out->data.assign(map.data, map.data + map.size);
                    sub->t_start_ = std::chrono::steady_clock::now();

                    if (rand() % 100 > sub->m_dropRate)
                    {
                        if (RETCODE_OK != sub->mp_data_dw->write((void*)sub->mp_video_out))
                        {
                            std::cout << "VideoPublication::run -> Cannot write video" << std::endl;
                        }
                    }
                    gst_buffer_unmap(buffer, &map);
                }
            }
            else
            {
                std::cout << "VideoPublication::run -> Buffer is nullptr" << std::endl;
            }

            gst_sample_unref(sample);
            returned_value =  GST_FLOW_OK;
        }
        else
        {
            std::cout << "VideoPublication::run -> Sample is nullptr" << std::endl;
        }

        returned_value =  GST_FLOW_ERROR;
    }
    else
    {
        std::cout << "VideoPublication::run -> Sample is nullptr" << std::endl;
    }

    returned_value =  GST_FLOW_OK;

    std::chrono::steady_clock::time_point send_end = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lock(sub->mutex_);
    if (std::chrono::duration<double, std::ratio<1, 1>>(send_end - sub->send_start_).count() >= sub->m_testTime)
    {
        sub->timer_on_ = true;
        sub->timer_cond_.notify_one();
    }

    return returned_value;
}
