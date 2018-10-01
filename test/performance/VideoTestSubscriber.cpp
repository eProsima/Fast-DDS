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

#include "VideoTestSubscriber.h"
#include "fastrtps/log/Log.h"
#include "fastrtps/log/Colors.h"

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;


VideoTestSubscriber::VideoTestSubscriber():
    mp_participant(nullptr),
    mp_datapub(nullptr),
    mp_commandpub(nullptr),
    mp_datasub(nullptr),
    mp_commandsub(nullptr),
    disc_count_(0),
    comm_count_(0),
    data_count_(0),
    m_status(0),
    n_received(0),
    n_samples(0),
    m_datapublistener(nullptr),
    m_datasublistener(nullptr),
    m_commandpublistener(nullptr),
    m_commandsublistener(nullptr)
{
    m_datapublistener.mp_up = this;
    m_datasublistener.mp_up = this;
    m_commandpublistener.mp_up = this;
    m_commandsublistener.mp_up = this;

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

    if (pipeline)   gst_object_unref(GST_OBJECT(pipeline)), pipeline = nullptr;
    if (sink)   gst_object_unref(GST_OBJECT(sink)), sink = nullptr;
    if (videoconvert)   gst_object_unref(GST_OBJECT(videoconvert)), videoconvert = nullptr;
    if (appsrc)   gst_object_unref(GST_OBJECT(appsrc)), appsrc = nullptr;
    thread_.join();
}

bool VideoTestSubscriber::init(int nsam, bool reliable, uint32_t pid, bool hostname,
        const PropertyPolicy& part_property_policy, const PropertyPolicy& property_policy, bool large_data,
        const std::string& sXMLConfigFile)
{
    //ARCE:
    large_data = true;
    //ARCE:

    m_sXMLConfigFile = sXMLConfigFile;
    n_samples = nsam;

    InitGStreamer();

    if (m_sXMLConfigFile.length() > 0)
    {
        // Create RTPSParticipant
        std::string participant_profile_name = "participant_profile";
        mp_participant = Domain::createParticipant(participant_profile_name);
        if (mp_participant == nullptr)
        {
            return false;
        }

        Domain::registerType(mp_participant, (TopicDataType*)&video_t);
        Domain::registerType(mp_participant, (TopicDataType*)&command_t);

        // Create Sending Publisher
        std::string profile_name = "publisher_profile";
        mp_datapub = Domain::createPublisher(mp_participant, profile_name, (PublisherListener*)&this->m_datapublistener);
        if (mp_datapub == nullptr)
        {
            return false;
        }
        std::cout << "Publisher created" << std::endl;

        // Create Echo Subscriber
        profile_name = "subscriber_profile";
        mp_datasub = Domain::createSubscriber(mp_participant, profile_name, &this->m_datasublistener);
        if (mp_datasub == nullptr)
        {
            return false;
        }
        std::cout << "Echo Subscriber created" << std::endl;

        // Create Command Publisher
        profile_name = "publisher_cmd_profile";
        mp_commandpub = Domain::createPublisher(mp_participant, profile_name, (PublisherListener*)&this->m_commandpublistener);
        if (mp_commandpub == nullptr)
        {
            return false;
        }
        std::cout << "Publisher created" << std::endl;

        profile_name = "subscriber_cmd_profile";
        mp_commandsub = Domain::createSubscriber(mp_participant, profile_name, &this->m_commandsublistener);
        if (mp_commandsub == nullptr)
        {
            return false;
        }
    }
    else
    {
        ParticipantAttributes PParam;
        PParam.rtps.defaultSendPort = 10042;
        PParam.rtps.builtin.domainId = pid % 230;
        PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
        PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
        PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
        PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
        PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
        PParam.rtps.sendSocketBufferSize = 65536;
        PParam.rtps.listenSocketBufferSize = 2 * 65536;
        PParam.rtps.setName("Participant_sub");
        PParam.rtps.properties = part_property_policy;
        mp_participant = Domain::createParticipant(PParam);
        if (mp_participant == nullptr)
        {
            return false;
        }

        Domain::registerType(mp_participant, (TopicDataType*)&video_t);
        Domain::registerType(mp_participant, (TopicDataType*)&command_t);

        // DATA PUBLISHER
        PublisherAttributes PubDataparam;
        PubDataparam.topic.topicDataType = "VideoType";
        PubDataparam.topic.topicKind = NO_KEY;
        std::ostringstream pt;
        pt << "VideoTest_";
        if (hostname)
            pt << asio::ip::host_name() << "_";
        pt << pid << "_SUB2PUB";
        PubDataparam.topic.topicName = pt.str();
        PubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
        PubDataparam.topic.historyQos.depth = n_samples;
        PubDataparam.topic.resourceLimitsQos.max_samples = n_samples + 1;
        PubDataparam.topic.resourceLimitsQos.allocated_samples = n_samples + 1;
        PubDataparam.times.heartbeatPeriod.seconds = 0;
        PubDataparam.times.heartbeatPeriod.fraction = 4294967 * 100;
        if (!reliable)
            PubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
        //PubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

        Locator_t loc;
        loc.port = 15002;
        PubDataparam.unicastLocatorList.push_back(loc);
        PubDataparam.properties = property_policy;
        if (large_data)
        {
            PubDataparam.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
            PubDataparam.qos.m_publishMode.kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
        }
        mp_datapub = Domain::createPublisher(mp_participant, PubDataparam, (PublisherListener*)&this->m_datapublistener);
        if (mp_datapub == nullptr)
            return false;

        //DATA SUBSCRIBER
        SubscriberAttributes SubDataparam;
        SubDataparam.topic.topicDataType = "VideoType";
        SubDataparam.topic.topicKind = NO_KEY;
        std::ostringstream st;
        st << "VideoTest_";
        if (hostname)
            st << asio::ip::host_name() << "_";
        st << pid << "_PUB2SUB";
        SubDataparam.topic.topicName = st.str();
        SubDataparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
        SubDataparam.topic.historyQos.depth = 1;
        if (reliable)
            SubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        loc.port = 15003;
        SubDataparam.unicastLocatorList.push_back(loc);
        SubDataparam.properties = property_policy;
        //loc.set_IP4_address(239,255,0,2);
        //SubDataparam.multicastLocatorList.push_back(loc);
        if (large_data)
        {
            SubDataparam.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        }
        mp_datasub = Domain::createSubscriber(mp_participant, SubDataparam, &this->m_datasublistener);
        if (mp_datasub == nullptr)
            return false;

        //COMMAND PUBLISHER
        PublisherAttributes PubCommandParam;
        PubCommandParam.topic.topicDataType = "TestCommandType";
        PubCommandParam.topic.topicKind = NO_KEY;
        std::ostringstream pct;
        pct << "VideoTest_Command_";
        if (hostname)
            pct << asio::ip::host_name() << "_";
        pct << pid << "_SUB2PUB";
        PubCommandParam.topic.topicName = pct.str();
        PubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
        PubCommandParam.topic.historyQos.depth = 100;
        PubCommandParam.topic.resourceLimitsQos.max_samples = 101;
        PubCommandParam.topic.resourceLimitsQos.allocated_samples = 101;
        PubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        mp_commandpub = Domain::createPublisher(mp_participant, PubCommandParam, &this->m_commandpublistener);
        if (mp_commandpub == nullptr)
            return false;
        SubscriberAttributes SubCommandParam;
        SubCommandParam.topic.topicDataType = "TestCommandType";
        SubCommandParam.topic.topicKind = NO_KEY;
        std::ostringstream sct;
        sct << "VideoTest_Command_";
        if (hostname)
            sct << asio::ip::host_name() << "_";
        sct << pid << "_PUB2SUB";
        SubCommandParam.topic.topicName = sct.str();
        SubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
        SubCommandParam.topic.historyQos.depth = 100;
        SubCommandParam.topic.resourceLimitsQos.max_samples = 101;
        SubCommandParam.topic.resourceLimitsQos.allocated_samples = 101;
        SubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        SubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        mp_commandsub = Domain::createSubscriber(mp_participant, SubCommandParam, &this->m_commandsublistener);
        if (mp_commandsub == nullptr)
            return false;
    }
    return true;
}



void VideoTestSubscriber::DataPubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        logInfo(VideoTest,"Data Pub Matched ");
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
        mp_up->comm_cond_.notify_one();
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void VideoTestSubscriber::DataSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        logInfo(VideoTest,"Data Sub Matched ");
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
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
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
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
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
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
        cout << "RCOMMAND: "<< command.m_command << endl;
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
            mp_up->mutex_.lock();
            ++mp_up->data_count_;
            mp_up->mutex_.unlock();
            mp_up->comm_cond_.notify_one();
            mp_up->data_cond_.notify_one();
        }
        else if(command.m_command == STOP_ERROR)
        {
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
    //cout << "SAMPLE INFO: "<< mp_up->m_sampleinfo.writerGUID << mp_up->m_sampleinfo.sampleKind << endl;
}

void VideoTestSubscriber::DataSubListener::onNewDataMessage(Subscriber* subscriber)
{
    VideoType videoData;
    eprosima::fastrtps::SampleInfo_t info;
    subscriber->takeNextData((void*)&videoData, &info);
    {
        //std::cout << "NEW SAMPLE RECEIVED: " << videoData.seqnum << std::endl;
        mp_up->push_video_packet(videoData);
    }
}


void VideoTestSubscriber::run()
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    while(disc_count_ != 4) disc_cond_.wait(disc_lock);
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

    std::cout << "GstSink END!" << std::endl;
}

bool VideoTestSubscriber::test()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (comm_count_ == 0) comm_cond_.wait(lock);
    --comm_count_;
    lock.unlock();

    m_status = 0;
    n_received = 0;
    TestCommandType command;
    command.m_command = BEGIN;
    mp_commandpub->write(&command);

    //lock.lock();
    //data_cond_.wait(lock, [&]()
    //{
    //    return data_count_ > 0;
    //});
    //--data_count_;
    //lock.unlock();

    cout << "TEST FINISHED" << endl;
    eClock::my_sleep(50);
    size_t removed;
    this->mp_datapub->removeAllChange(&removed);

    if (m_status == -1)
    {
        return false;
    }
    return true;
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
            g_signal_connect(appsrc, "need-data", G_CALLBACK(start_feed_cb), this);
            g_signal_connect(appsrc, "enough-data", G_CALLBACK(stop_feed_cb), this);

            videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
            ok = (videoconvert != nullptr);
            if (ok)
            {
                sink = gst_element_factory_make("autovideosink", "sink"); //
                ok = (sink != nullptr);
                if (ok)
                {
                    GstCaps *caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420",
                        "width", G_TYPE_INT, 480, "height", G_TYPE_INT, 320, NULL);
                    gst_app_src_set_caps(GST_APP_SRC(appsrc), caps);
                    gst_caps_unref(caps);

                    gst_bin_add_many(GST_BIN(pipeline), appsrc, /*decodebin, */videoconvert, sink, NULL);

                    caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420",
                        "width", G_TYPE_INT, 480, "height", G_TYPE_INT, 320, NULL);
                    ok = gst_element_link_filtered(appsrc, videoconvert, caps) == TRUE;
                    gst_caps_unref(caps);

                    ok = gst_element_link_many(videoconvert, sink, NULL) == TRUE;
                    if (ok)
                    {
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
        static bool feeding = false;
        int counter = 0;
        while (!sub->hasData() && counter < WAIT_AFTER_LAST_FEED_MS)
        {
            Sleep(1); if (feeding) ++counter;
        }
        if (counter == WAIT_AFTER_LAST_FEED_MS)
        {
            sub->stop();
            return FALSE;
        }
        feeding = true;

        GstBuffer *buffer;
        GstFlowReturn ret;
        GstMapInfo map;
        //gint16 *raw;
        gint num_samples = 0;
        //gint size = 0;
        gsize chunk_size = sub->currentSize();

        // Create a new empty buffer
        buffer = gst_buffer_new_and_alloc(chunk_size);

        // Set its timestamp and duration
        //GST_BUFFER_TIMESTAMP(buffer) = gst_util_uint64_scale(sink->num_samples, GST_SECOND, SAMPLE_RATE);
        //GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(num_samples, GST_SECOND, SAMPLE_RATE);
        gst_buffer_map(buffer, &map, GST_MAP_WRITE);
        guint8* pos = map.data;
        while (chunk_size && sub->hasData())
        {
            VideoType& vpacket = sub->pop_video_packet();
            gsize size = vpacket.data.size();
            memmove(pos, vpacket.data.data(), size);
            pos += size;
            chunk_size -= size;
            ++num_samples;
        }

        gst_buffer_unmap(buffer, &map);
        //sub->num_samples_ += num_samples;

        // Push the buffer into the appsrc
        g_signal_emit_by_name(sub->appsrc, "push-buffer", buffer, &ret);

        // Free the buffer now that we are done with it
        gst_buffer_unref(buffer);

        if (ret != GST_FLOW_OK)
        {
            // We got some error, stop sending data
            return FALSE;
        }
    }
    return TRUE;
}

void VideoTestSubscriber::stop()
{
    m_bRunning = false;
    gst_element_set_state(pipeline, GST_STATE_NULL);
}
