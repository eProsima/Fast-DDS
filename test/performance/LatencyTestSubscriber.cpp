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
 * @file LatencyTestSubscriber.cpp
 *
 */

#include "LatencyTestSubscriber.h"
#include "fastrtps/log/Log.h"
#include "fastrtps/log/Colors.h"

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;


uint32_t datassub[] = {12,28,60,124,252,508,1020,2044,4092,8188,16380};

std::vector<uint32_t> data_size_sub (datassub, datassub + sizeof(datassub) / sizeof(uint32_t) );

LatencyTestSubscriber::LatencyTestSubscriber():
    mp_participant(nullptr),
    mp_datapub(nullptr),
    mp_commandpub(nullptr),
    mp_datasub(nullptr),
    mp_commandsub(nullptr),
    mp_latency(nullptr),
    disc_count_(0),
    comm_count_(0),
    data_count_(0),
    m_status(0),
    n_received(0),
    n_samples(0),
    m_datapublistener(nullptr),
    m_datasublistener(nullptr),
    m_commandpublistener(nullptr),
    m_commandsublistener(nullptr),
    m_echo(true)
{
    m_datapublistener.mp_up = this;
    m_datasublistener.mp_up = this;
    m_commandpublistener.mp_up = this;
    m_commandsublistener.mp_up = this;


}

LatencyTestSubscriber::~LatencyTestSubscriber()
{
    Domain::removeParticipant(mp_participant);
}

bool LatencyTestSubscriber::init(bool echo, int nsam, bool reliable, uint32_t pid, bool hostname)
{
    m_echo = echo;
    n_samples = nsam;
    ParticipantAttributes PParam;
    PParam.rtps.defaultSendPort = 10042;
    PParam.rtps.builtin.domainId = pid % 230;
    PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.sendSocketBufferSize = 65536;
    PParam.rtps.listenSocketBufferSize = 2*65536;
    PParam.rtps.setName("Participant_sub");
    mp_participant = Domain::createParticipant(PParam);
    if(mp_participant == nullptr)
        return false;
    Domain::registerType(mp_participant,(TopicDataType*)&latency_t);
    Domain::registerType(mp_participant,(TopicDataType*)&command_t);

    // DATA PUBLISHER
    PublisherAttributes PubDataparam;
    PubDataparam.topic.topicDataType = "LatencyType";
    PubDataparam.topic.topicKind = NO_KEY;
    std::ostringstream pt;
    pt << "LatencyTest_";
    if(hostname)
        pt << asio::ip::host_name() << "_";
    pt << pid << "_SUB2PUB";
    PubDataparam.topic.topicName = pt.str();
    PubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    PubDataparam.topic.historyQos.depth = n_samples;
    PubDataparam.topic.resourceLimitsQos.max_samples = n_samples + 1;
    PubDataparam.topic.resourceLimitsQos.allocated_samples = n_samples + 1;
    PubDataparam.times.heartbeatPeriod.seconds = 0;
    PubDataparam.times.heartbeatPeriod.fraction = 4294967 * 100;
    if(!reliable)
        PubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    //PubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    Locator_t loc;
    loc.port = 15002;
    PubDataparam.unicastLocatorList.push_back(loc);
    mp_datapub = Domain::createPublisher(mp_participant,PubDataparam,(PublisherListener*)&this->m_datapublistener);
    if(mp_datapub == nullptr)
        return false;

    //DATA SUBSCRIBER
    SubscriberAttributes SubDataparam;
    SubDataparam.topic.topicDataType = "LatencyType";
    SubDataparam.topic.topicKind = NO_KEY;
    std::ostringstream st;
    st << "LatencyTest_";
    if(hostname)
        st << asio::ip::host_name() << "_";
    st << pid << "_PUB2SUB";
    SubDataparam.topic.topicName = st.str();
    SubDataparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    SubDataparam.topic.historyQos.depth = 1;
    if(reliable)
        SubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    loc.port = 15003;
    SubDataparam.unicastLocatorList.push_back(loc);
    //loc.set_IP4_address(239,255,0,2);
    //SubDataparam.multicastLocatorList.push_back(loc);
    mp_datasub = Domain::createSubscriber(mp_participant,SubDataparam,&this->m_datasublistener);
    if(mp_datasub == nullptr)
        return false;

    //COMMAND PUBLISHER
    PublisherAttributes PubCommandParam;
    PubCommandParam.topic.topicDataType = "TestCommandType";
    PubCommandParam.topic.topicKind = NO_KEY;
    std::ostringstream pct;
    pct << "LatencyTest_Command_";
    if(hostname)
        pct << asio::ip::host_name() << "_";
    pct << pid << "_SUB2PUB";
    PubCommandParam.topic.topicName = pct.str();
    PubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    PubCommandParam.topic.historyQos.depth = 100;
    PubCommandParam.topic.resourceLimitsQos.max_samples = 101;
    PubCommandParam.topic.resourceLimitsQos.allocated_samples = 101;
    PubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_commandpub = Domain::createPublisher(mp_participant,PubCommandParam,&this->m_commandpublistener);
    if(mp_commandpub == nullptr)
        return false;
    SubscriberAttributes SubCommandParam;
    SubCommandParam.topic.topicDataType = "TestCommandType";
    SubCommandParam.topic.topicKind = NO_KEY;
    std::ostringstream sct;
    sct << "LatencyTest_Command_";
    if(hostname)
        sct << asio::ip::host_name() << "_";
    sct << pid << "_PUB2SUB";
    SubCommandParam.topic.topicName = sct.str();
    SubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    SubCommandParam.topic.historyQos.depth = 100;
    SubCommandParam.topic.resourceLimitsQos.max_samples = 101;
    SubCommandParam.topic.resourceLimitsQos.allocated_samples = 101;
    SubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    SubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_commandsub = Domain::createSubscriber(mp_participant,SubCommandParam,&this->m_commandsublistener);
    if(mp_commandsub == nullptr)
        return false;
    return true;
}



void LatencyTestSubscriber::DataPubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        logInfo(LatencyTest,"Data Pub Matched ");
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void LatencyTestSubscriber::DataSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        logInfo(LatencyTest,"Data Sub Matched ");
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}



void LatencyTestSubscriber::CommandPubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        logInfo(LatencyTest, "Command Pub Matched ");
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void LatencyTestSubscriber::CommandSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        logInfo(LatencyTest, "Command Sub Matched ");
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void LatencyTestSubscriber::CommandSubListener::onNewDataMessage(Subscriber* subscriber)
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
            mp_up->data_cond_.notify_one();
        }
        else if(command.m_command == STOP_ERROR)
        {
            mp_up->m_status = -1;
            mp_up->mutex_.lock();
            ++mp_up->data_count_;
            mp_up->mutex_.unlock();
            mp_up->data_cond_.notify_one();
        }
        else if(command.m_command == DEFAULT)
        {
            std::cout << "Something is wrong" << std::endl;
        }
    }
    //cout << "SAMPLE INFO: "<< mp_up->m_sampleinfo.writerGUID << mp_up->m_sampleinfo.sampleKind << endl;
}

void LatencyTestSubscriber::DataSubListener::onNewDataMessage(Subscriber* subscriber)
{
    subscriber->takeNextData((void*)mp_up->mp_latency,&mp_up->m_sampleinfo);
    //	cout << "R: "<< mp_up->mp_latency->seqnum << "|"<<mp_up->m_echo<<std::flush;
    //	//	eClock::my_sleep(50);
    //		cout << "NSAMPLES: " << (uint32_t)mp_up->n_samples<< endl;
    if(mp_up->m_echo)
        mp_up->mp_datapub->write((void*)mp_up->mp_latency);
}


void LatencyTestSubscriber::run()
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    while(disc_count_ != 4) disc_cond_.wait(disc_lock);
    disc_lock.unlock();

    cout << C_B_MAGENTA << "DISCOVERY COMPLETE "<<C_DEF<<endl;

    for(std::vector<uint32_t>::iterator ndata = data_size_sub.begin();ndata!=data_size_sub.end();++ndata)
    {
        if(!this->test(*ndata))
            break;
    }
}

bool LatencyTestSubscriber::test(uint32_t datasize)
{
    cout << "Preparing test with data size: " << datasize+4<<endl;
    mp_latency = new LatencyType((uint16_t)datasize);

    std::unique_lock<std::mutex> lock(mutex_);
    if(comm_count_ == 0) comm_cond_.wait(lock);
    --comm_count_;
    lock.unlock();

    m_status = 0;
    n_received = 0;
    TestCommandType command;
    command.m_command = BEGIN;
    cout << "Testing with data size: " << datasize+4<<endl;
    mp_commandpub->write(&command);

    lock.lock();
    if(data_count_ == 0) data_cond_.wait(lock);
    --data_count_;
    lock.unlock();

    cout << "TEST OF SIZE: "<< datasize +4 << " ENDS"<<endl;
    eClock::my_sleep(50);
    size_t removed;
    this->mp_datapub->removeAllChange(&removed);
    //cout << "REMOVED: "<< removed<<endl;
    delete(mp_latency);
    if(m_status == -1)
        return false;
    return true;
}
