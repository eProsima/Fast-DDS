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
#include <fastdds/dds/log/Log.hpp>
#include "fastrtps/log/Colors.h"
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <numeric>
#include <cmath>
#include <fstream>

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

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

    Domain::removeParticipant(mp_participant);

    if (gmain_loop_ != nullptr)
    {
        std::unique_lock<std::mutex> lock(gst_mutex_);
        g_main_loop_quit(gmain_loop_);
        gmain_loop_ = nullptr;
    }

    gst_bin_remove_many(GST_BIN(pipeline), appsrc, videoconvert, sink, NULL);
    //if (pipeline)   gst_object_unref(GST_OBJECT(pipeline)), pipeline = nullptr;
    //if (sink)   gst_object_unref(GST_OBJECT(sink)), sink = nullptr;
    //if (videoconvert)   gst_object_unref(GST_OBJECT(videoconvert)), videoconvert = nullptr;
    //if (appsrc)   gst_object_unref(GST_OBJECT(appsrc)), appsrc = nullptr;
    thread_.join();
}

bool VideoTestSubscriber::init(int nsam, bool reliable, uint32_t pid, bool hostname,
        const PropertyPolicy& part_property_policy, const PropertyPolicy& property_policy, bool large_data,
        const std::string& sXMLConfigFile, bool export_csv, const std::string& export_prefix,
        int forced_domain, int video_width, int video_height, int frame_rate)
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

    // Create RTPSParticipant
    std::string participant_profile_name = "sub_participant_profile";
    ParticipantAttributes PParam;

    if (m_forcedDomain >= 0)
    {
        PParam.domainId = m_forcedDomain;
    }
    else
    {
        PParam.domainId = pid % 230;
    }
    PParam.rtps.setName("video_test_subscriber");
    PParam.rtps.properties = part_property_policy;

    if (m_sXMLConfigFile.length() > 0)
    {
        if (m_forcedDomain >= 0)
        {
            ParticipantAttributes participant_att;
            if (eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK ==
                eprosima::fastrtps::xmlparser::XMLProfileManager::fillParticipantAttributes(participant_profile_name,
                    participant_att))
            {
                participant_att.domainId = m_forcedDomain;
                mp_participant = Domain::createParticipant(participant_att);
            }
        }
        else
        {
            mp_participant = Domain::createParticipant(participant_profile_name);
        }
    }
    else
    {
        mp_participant = Domain::createParticipant(PParam);
    }

    if (mp_participant == nullptr)
    {
        return false;
    }

    Domain::registerType(mp_participant, (TopicDataType*)&video_t);
    Domain::registerType(mp_participant, (TopicDataType*)&command_t);

    // Create Data subscriber
    std::string profile_name = "subscriber_profile";
    SubscriberAttributes SubDataparam;

    if (reliable)
    {
        SubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    }
    SubDataparam.properties = property_policy;
    if (large_data)
    {
        SubDataparam.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    }

    if (m_sXMLConfigFile.length() > 0)
    {
        eprosima::fastrtps::xmlparser::XMLProfileManager::fillSubscriberAttributes(profile_name, SubDataparam);
    }

    SubDataparam.topic.topicDataType = "VideoType";
    SubDataparam.topic.topicKind = NO_KEY;
    std::ostringstream st;
    st << "VideoTest_";
    if (hostname)
    {
        st << asio::ip::host_name() << "_";
    }
    st << pid << "_PUB2SUB";
    SubDataparam.topic.topicName = st.str();

    mp_datasub = Domain::createSubscriber(mp_participant, SubDataparam, &this->m_datasublistener);
    if (mp_datasub == nullptr)
    {
        std::cout << "Cannot create data subscriber" << std::endl;
        return false;
    }

    // Create Command Publisher
    PublisherAttributes PubCommandParam;
    PubCommandParam.topic.topicDataType = "TestCommandType";
    PubCommandParam.topic.topicKind = NO_KEY;
    std::ostringstream pct;
    pct << "VideoTest_Command_";
    if (hostname)
    {
        pct << asio::ip::host_name() << "_";
    }
    pct << pid << "_SUB2PUB";
    PubCommandParam.topic.topicName = pct.str();
    PubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    PubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    PubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    mp_commandpub = Domain::createPublisher(mp_participant, PubCommandParam, &this->m_commandpublistener);
    if (mp_commandpub == nullptr)
    {
        return false;
    }

    SubscriberAttributes SubCommandParam;
    SubCommandParam.topic.topicDataType = "TestCommandType";
    SubCommandParam.topic.topicKind = NO_KEY;
    std::ostringstream sct;
    sct << "VideoTest_Command_";
    if (hostname)
    {
        sct << asio::ip::host_name() << "_";
    }
    sct << pid << "_PUB2SUB";
    SubCommandParam.topic.topicName = sct.str();
    SubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    SubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    SubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    mp_commandsub = Domain::createSubscriber(mp_participant, SubCommandParam, &this->m_commandsublistener);
    if (mp_commandsub == nullptr)
    {
        return false;
    }
    return true;
}

void VideoTestSubscriber::DataSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);
    if(info.status == MATCHED_MATCHING)
    {
        logInfo(VideoTest,"Data Sub Matched ");
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

void VideoTestSubscriber::CommandPubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        logInfo(VideoTest, "Command Pub Matched ");
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

void VideoTestSubscriber::CommandSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);
    if(info.status == MATCHED_MATCHING)
    {
        logInfo(VideoTest, "Command Sub Matched ");
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

void VideoTestSubscriber::CommandSubListener::onNewDataMessage(Subscriber* subscriber)
{
    TestCommandType command;
    if(subscriber->takeNextData(&command,&mp_up->m_sampleinfo))
    {
        //cout << "RCOMMAND: "<< command.m_command << endl;
        if(command.m_command == READY)
        {
            cout << "Publisher has new test ready..."<<endl;
            mp_up->mutex_.lock();
            ++mp_up->comm_count_;
            mp_up->mutex_.unlock();
            mp_up->comm_cond_.notify_one();
        }
        else if(command.m_command == STOP)
        {
            cout << "Publisher has stopped the test" << endl;
            mp_up->mutex_.lock();
            ++mp_up->data_count_;
            mp_up->mutex_.unlock();
            mp_up->comm_cond_.notify_one();
            mp_up->data_cond_.notify_one();
        }
        else if(command.m_command == STOP_ERROR)
        {
            cout << "Publisher has canceled the test" << endl;
            mp_up->m_status = -1;
            mp_up->mutex_.lock();
            ++mp_up->data_count_;
            mp_up->mutex_.unlock();
            mp_up->comm_cond_.notify_one();
            mp_up->data_cond_.notify_one();
        }
        else if(command.m_command == DEFAULT)
        {
            std::cout << "Something is wrong" << std::endl;
        }
    }
}

void VideoTestSubscriber::DataSubListener::onNewDataMessage(Subscriber* subscriber)
{
    VideoType videoData;
    eprosima::fastrtps::SampleInfo_t info;
    subscriber->takeNextData((void*)&videoData, &info);
    {
        mp_up->push_video_packet(videoData);
    }
}


void VideoTestSubscriber::run()
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&]() { return disc_count_ >= 3; });
    disc_lock.unlock();

    cout << C_B_MAGENTA << "DISCOVERY COMPLETE "<<C_DEF<<endl;

    this->test();
}

void VideoTestSubscriber::gst_run(VideoTestSubscriber* sub)
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
    mp_commandpub->write(&command);

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

void VideoTestSubscriber::fps_stats_cb(GstElement* /*source*/, gdouble /*fps*/, gdouble /*droprate*/,
    gdouble avgfps, VideoTestSubscriber* sub)
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
            GstCaps *caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420",
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

void VideoTestSubscriber::start_feed_cb(GstElement* /*source*/, guint /*size*/, VideoTestSubscriber* sub)
{
    if (sub->source_id_ == 0)
    {
        sub->source_id_ = g_idle_add((GSourceFunc)push_data_cb, sub);
    }
}

void VideoTestSubscriber::stop_feed_cb(GstElement* /*source*/, VideoTestSubscriber* sub)
{
    if (sub->source_id_ != 0)
    {
        g_source_remove(sub->source_id_);
        sub->source_id_ = 0;
    }
}

#define WAIT_AFTER_LAST_FEED_MS 2000
gboolean VideoTestSubscriber::push_data_cb(VideoTestSubscriber* sub)
{
    std::unique_lock<std::mutex> lock(sub->gst_mutex_);
    if (sub->m_bRunning)
    {
        GstBuffer *buffer;
        GstFlowReturn ret;
        GstMapInfo map;
        //gint16 *raw;
        gint num_samples = 0;
        //gint size = 0;

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

void VideoTestSubscriber::message_cb(GstBus* /*bus*/, GstMessage* message, gpointer /*user_data*/)
{
    GError* err = nullptr;
    gchar* debug_info = nullptr;
    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_ERROR:
        {
            gst_message_parse_error(message, &err, &debug_info);

            printf("# GST INTERNAL # Error received from element: %s; message: %s\n", GST_OBJECT_NAME(message->src), err->message);
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

            printf("# GST INTERNAL # Warning received from element: %s; message: %s\n", GST_OBJECT_NAME(message->src), err->message);
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

            printf("# GST INTERNAL # Info received from element: %s; message: %s\n", GST_OBJECT_NAME(message->src), err->message);
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


void VideoTestSubscriber::printStat(TimeStats& TS)
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

    output_file_csv << "Samples, Avg stdev, Avg Mean, min Avg, Avg 50 %%, Avg 90 %%, Avg 99 %%, \
        Avg 99.99%%, Avg max, Drop stdev, Drop Mean, min Drop, Drop 50 %%, Drop 90 %%, Drop 99 %%, \
        Drop 99.99%%, Drop max" << std::endl;

    output_mean_csv << "Avg Mean" << std::endl;

    printf("Statistics for video test \n");
    printf("    Samples,  Avg stdev,   Avg Mean,    min Avg,    Avg 50%%,    Avg 90%%,    Avg 99%%,   Avg 99.99%%,    Avg max\n");
    printf("-----------,-----------,-----------,-----------,-----------,-----------,-----------,-------------,-----------\n");
    printf("%11u,%11.2f,%11.2f,%11.2f,%11.2f,%11.2f,%11.2f,%13.2f,%11.2f \n\n\n",
        TS.received, TS.pAvgStdev, TS.pAvgMean, TS.m_minAvg, TS.pAvg50, TS.pAvg90, TS.pAvg99, TS.pAvg9999, TS.m_maxAvg);

    printf("    Samples, FameDrop stdev, FameDrop Mean, min FameDrop,  FameDrop 50%%,  FameDrop 90%%,  FameDrop 99%%, FameDrop 99.99%%,  FameDrop max\n");
    printf("-----------,---------------,--------------,-------------,--------------,--------------,--------------,----------------,--------------\n");
    printf("%11u,%15.2f,%14.2f,%13.2f,%14.2f,%14.2f,%14.2f,%16.2f,%14.2f \n",
        TS.received, TS.pDropStdev, TS.pDropMean, TS.m_minDrop, TS.pDrop50, TS.pDrop90, TS.pDrop99, TS.pDrop9999, TS.m_maxDrop);

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
