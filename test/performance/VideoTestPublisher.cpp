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

#include "VideoTestPublisher.h"
#include "fastrtps/log/Log.h"
#include "fastrtps/log/Colors.h"
#include <numeric>
#include <cmath>
#include <fstream>
#include <inttypes.h>

#define TIME_LIMIT_US 10000

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

VideoTestPublisher::VideoTestPublisher():
    mp_participant(nullptr),
    mp_datapub(nullptr),
    mp_commandpub(nullptr),
    mp_datasub(nullptr),
    mp_commandsub(nullptr),
    mp_video_out(nullptr),
    t_overhead_(0.0),
    n_subscribers(0),
    n_samples(0),
    disc_count_(0),
    comm_count_(0),
    data_count_(0),
    m_status(0),
    n_received(0),
    m_datapublistener(nullptr),
    m_datasublistener(nullptr),
    m_commandpublistener(nullptr),
    m_commandsublistener(nullptr)
{
    m_datapublistener.mp_up = this;
    m_datasublistener.mp_up = this;
    m_commandpublistener.mp_up = this;
    m_commandsublistener.mp_up = this;
}

VideoTestPublisher::~VideoTestPublisher()
{
    if (sink)   gst_object_unref(GST_OBJECT(sink)), sink = nullptr;
    if (filesrc)   gst_object_unref(GST_OBJECT(filesrc)), filesrc = nullptr;
    if (pipeline)   gst_object_unref(GST_OBJECT(pipeline)), pipeline = nullptr;

    Domain::removeParticipant(mp_participant);
}


bool VideoTestPublisher::init(int n_sub, int n_sam, bool reliable, uint32_t pid, bool hostname,
        const PropertyPolicy& part_property_policy, const PropertyPolicy& property_policy, bool large_data,
        const std::string& sXMLConfigFile)
{
    //ARCE:
    large_data = true;
    //ARCE:

    m_sXMLConfigFile = sXMLConfigFile;
    n_samples = n_sam;
    n_subscribers = n_sub;
    reliable_ = reliable;

    // GSTREAMER PIPELINE INITIALIZATION.
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

        // Register the type
        Domain::registerType(mp_participant, (TopicDataType*)&latency_t);
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
        PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
        PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
        PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
        PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
        PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
        PParam.rtps.properties = part_property_policy;
        PParam.rtps.sendSocketBufferSize = 65536;
        PParam.rtps.listenSocketBufferSize = 2 * 65536;
        PParam.rtps.setName("Participant_pub");
        mp_participant = Domain::createParticipant(PParam);
        if (mp_participant == nullptr)
        {
            return false;
        }

        Domain::registerType(mp_participant, (TopicDataType*)&latency_t);
        Domain::registerType(mp_participant, (TopicDataType*)&command_t);

        // DATA PUBLISHER
        PublisherAttributes PubDataparam;
        PubDataparam.topic.topicDataType = "VideoType";
        PubDataparam.topic.topicKind = NO_KEY;
        std::ostringstream pt;
        pt << "VideoTest_";
        if (hostname)
            pt << asio::ip::host_name() << "_";
        pt << pid << "_PUB2SUB";
        PubDataparam.topic.topicName = pt.str();
        PubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
        PubDataparam.topic.historyQos.depth = n_samples;
        PubDataparam.topic.resourceLimitsQos.max_samples = n_samples + 1;
        PubDataparam.topic.resourceLimitsQos.allocated_samples = n_samples + 1;
        PubDataparam.times.heartbeatPeriod.seconds = 0;
        PubDataparam.times.heartbeatPeriod.fraction = 4294967 * 100;
        if (!reliable)
            PubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
        Locator_t loc;
        loc.port = 15000;
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
        st << pid << "_SUB2PUB";
        SubDataparam.topic.topicName = st.str();
        SubDataparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
        SubDataparam.topic.historyQos.depth = 1;
        if (reliable)
            SubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        loc.port = 15001;
        SubDataparam.unicastLocatorList.push_back(loc);
        SubDataparam.properties = property_policy;
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
        pct << pid << "_PUB2SUB";
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
        sct << pid << "_SUB2PUB";
        SubCommandParam.topic.topicName = sct.str();
        SubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
        SubCommandParam.topic.historyQos.depth = 100;
        SubCommandParam.topic.resourceLimitsQos.allocated_samples = 101;
        SubCommandParam.topic.resourceLimitsQos.allocated_samples = 101;
        SubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        SubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        mp_commandsub = Domain::createSubscriber(mp_participant, SubCommandParam, &this->m_commandsublistener);
        if (mp_commandsub == nullptr)
            return false;
    }

    // Calculate overhead
    t_start_ = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i)
        t_end_ = std::chrono::steady_clock::now();
    t_overhead_ = std::chrono::duration<double, std::micro>(t_end_ - t_start_) / 1001;
    cout << "Overhead " << t_overhead_.count() << " ns" << endl;

    return true;
}

void VideoTestPublisher::DataPubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        cout << C_MAGENTA << "Data Pub Matched "<<C_DEF<<endl;

        n_matched++;
        if(n_matched > mp_up->n_subscribers)
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

void VideoTestPublisher::DataSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        cout << C_MAGENTA << "Data Sub Matched "<<C_DEF<<endl;

        n_matched++;
        if(n_matched > mp_up->n_subscribers)
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

void VideoTestPublisher::CommandPubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        cout << C_MAGENTA << "Command Pub Matched "<<C_DEF<<endl;

        n_matched++;
        if(n_matched > mp_up->n_subscribers)
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

void VideoTestPublisher::CommandSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        cout << C_MAGENTA << "Command Sub Matched "<<C_DEF<<endl;

        n_matched++;
        if(n_matched > mp_up->n_subscribers)
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

void VideoTestPublisher::CommandSubListener::onNewDataMessage(Subscriber* subscriber)
{
    TestCommandType command;
    SampleInfo_t info;
    //	cout << "COMMAND RECEIVED"<<endl;
    if(subscriber->takeNextData((void*)&command,&info))
    {
        if(info.sampleKind == ALIVE)
        {
            //cout << "ALIVE "<<command.m_command<<endl;
            if(command.m_command == BEGIN)
            {
                //	cout << "POSTING"<<endl;
                mp_up->mutex_.lock();
                ++mp_up->comm_count_;
                mp_up->mutex_.unlock();
                mp_up->comm_cond_.notify_one();
            }
        }
    }
    else
        cout<< "Problem reading"<<endl;
}

void VideoTestPublisher::DataSubListener::onNewDataMessage(Subscriber* /*subscriber*/)
{
}

void VideoTestPublisher::run()
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    while (disc_count_ != (n_subscribers * 4))
    {
        disc_cond_.wait(disc_lock);
    }
    disc_lock.unlock();

    cout << C_B_MAGENTA << "DISCOVERY COMPLETE "<<C_DEF<<endl;

    mp_video_out = new VideoType();
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    std::chrono::steady_clock::time_point send_start_ = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point send_end = send_start_;
    do
    {
        send_end = std::chrono::steady_clock::now();
    }
    while (std::chrono::duration<double, std::deci>(send_end - send_start_).count() < 50); //ARCE

    cout << "REMOVING PUBLISHER"<<endl;
    Domain::removePublisher(this->mp_commandpub);
    cout << "REMOVING SUBSCRIBER"<<endl;
    Domain::removeSubscriber(mp_commandsub);

    std::string str_reliable = "besteffort";
    if(reliable_)
        str_reliable = "reliable";
}

bool VideoTestPublisher::test(uint32_t datasize)
{
    //cout << "Beginning test of size: "<<datasize+4 <<endl;
    m_status = 0;
    n_received = 0;
    mp_video_out = new VideoType(datasize);
    times_.clear();
    TestCommandType command;
    command.m_command = READY;
    mp_commandpub->write(&command);

    //cout << "WAITING FOR COMMAND RESPONSES "<<endl;;
    std::unique_lock<std::mutex> lock(mutex_);
    while(comm_count_ != n_subscribers) comm_cond_.wait(lock);
    --comm_count_;
    lock.unlock();
    //cout << endl;
    //BEGIN THE TEST:

    //for(unsigned int count = 1; count <= n_samples; ++count)
    //{
    //    mp_video_in->seqnum = 0;
    //    mp_video_out->seqnum = count;

    //    t_start_ = std::chrono::steady_clock::now();
    //    mp_datapub->write((void*)mp_video_out);

    //    lock.lock();
    //    data_cond_.wait_for(lock, std::chrono::seconds(1), [&]() { return data_count_ > 0; });

    //    if(data_count_ > 0)
    //    {
    //        --data_count_;
    //    }
    //    lock.unlock();
    //}

    command.m_command = STOP;
    mp_commandpub->write(&command);

    if(m_status !=0)
    {
        cout << "Error in test "<<endl;
        return false;
    }
    //TEST FINISHED:
    size_t removed=0;
    mp_datapub->removeAllChange(&removed);
    //cout << "   REMOVED: "<< removed<<endl;
    analyzeTimes(datasize);
    printStat(m_stats.back());

    delete(mp_video_out);

    return true;
}

void VideoTestPublisher::analyzeTimes(uint32_t datasize)
{
    TimeStats TS;
    TS.nbytes = datasize+4;
    TS.received = n_received;
    TS.m_min = *std::min_element(times_.begin(), times_.end());
    TS.m_max = *std::max_element(times_.begin(), times_.end());
    TS.mean = std::accumulate(times_.begin(), times_.end(), std::chrono::duration<double, std::micro>(0)).count() / times_.size();

    double auxstdev=0;
    for(std::vector<std::chrono::duration<double, std::micro>>::iterator tit = times_.begin(); tit != times_.end(); ++tit)
    {
        auxstdev += pow(((*tit).count() - TS.mean), 2);
    }
    auxstdev = sqrt(auxstdev / times_.size());
    TS.stdev = static_cast<double>(round(auxstdev));

    std::sort(times_.begin(), times_.end());
    size_t elem = 0;

    elem = static_cast<size_t>(times_.size() * 0.5);
    if(elem > 0 && elem <= times_.size())
        TS.p50 = times_.at(--elem).count();
    else
        TS.p50 = NAN;

    elem = static_cast<size_t>(times_.size() * 0.9);
    if(elem > 0 && elem <= times_.size())
        TS.p90 = times_.at(--elem).count();
    else
        TS.p90 = NAN;

    elem = static_cast<size_t>(times_.size() * 0.99);
    if(elem > 0 && elem <= times_.size())
        TS.p99 = times_.at(--elem).count();
    else
        TS.p99 = NAN;

    elem = static_cast<size_t>(times_.size() * 0.9999);
    if(elem > 0 && elem <= times_.size())
        TS.p9999 = times_.at(--elem).count();
    else
        TS.p9999 = NAN;

    m_stats.push_back(TS);
}


void VideoTestPublisher::printStat(TimeStats& TS)
{
#ifdef _WIN32
    printf("%8I64u,%8u,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
        TS.nbytes, TS.received, TS.stdev, TS.mean,
        TS.m_min.count(),
        TS.p50, TS.p90, TS.p99, TS.p9999,
        TS.m_max.count());
#else
    printf("%8" PRIu64 ",%8u,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
        TS.nbytes, TS.received, TS.stdev, TS.mean,
        TS.m_min.count(),
        TS.p50, TS.p90, TS.p99, TS.p9999,
        TS.m_max.count());
#endif
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
            sink = gst_element_factory_make("appsink", "sink"); //autovideosink
            ok = (sink != nullptr);
            if (ok)
            {
                g_object_set(sink, "emit-signals", TRUE, NULL); // "caps", audio_caps,
                g_signal_connect(sink, "new-sample", G_CALLBACK(new_sample), this);
                //gst_caps_unref(audio_caps);

                GstCaps *caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420",
                    "width", G_TYPE_INT, 480, "height", G_TYPE_INT, 320, NULL);

                // Link the camera source and colorspace filter using capabilities specified
                gst_bin_add_many(GST_BIN(pipeline), filesrc, sink, NULL);
                ok = gst_element_link_filtered(filesrc, sink, caps) == TRUE;
                gst_caps_unref(caps);
                if (ok)
                {
                }
            }
        }
    }
}

/* The appsink has received a buffer */
GstFlowReturn VideoTestPublisher::new_sample(GstElement *sink, VideoTestPublisher *sub)
{
    if (sub->mp_video_out != nullptr)
    {
        GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
        if (sample)
        {
            GstBuffer* buffer = gst_sample_get_buffer(sample);
            if (buffer != nullptr)
            {
                GstMapInfo map;
                if (gst_buffer_map(buffer, &map, GST_MAP_READ))
                {
                    sub->mp_video_out->seqnum++;

                    //std::cout << "NEW SAMPLE " << sub->mp_video_out->seqnum << std::endl;

                    sub->mp_video_out->data.assign(map.data, map.data + map.size);
                    sub->t_start_ = std::chrono::steady_clock::now();
                    sub->mp_datapub->write((void*)sub->mp_video_out);
                    gst_buffer_unmap(buffer, &map);
                }
            }
            else
            {
                std::cout << "VideoPublication::run -> Buffer is nullptr" << std::endl;
            }
            gst_sample_unref(sample);
            return GST_FLOW_OK;
        }
        return GST_FLOW_ERROR;
    }
    return GST_FLOW_OK;
}
