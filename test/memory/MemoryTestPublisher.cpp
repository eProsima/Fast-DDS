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
 * @file MemoryPublisher.cpp
 *
 */

#include "MemoryTestPublisher.h"
#include "fastrtps/log/Log.h"
#include "fastrtps/log/Colors.h"
#include <numeric>
#include <cmath>
#include <fstream>
#include <inttypes.h>

#ifndef _WIN32
#define localtime_s(X, Y) localtime_r(Y, X)
#endif

#define TIME_LIMIT_US 10000

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;


MemoryTestPublisher::MemoryTestPublisher():
    mp_participant(nullptr),
    mp_datapub(nullptr),
    mp_commandpub(nullptr),
    mp_commandsub(nullptr),
    mp_memory(nullptr),
    n_subscribers(0),
    n_samples(0),
    disc_count_(0),
    comm_count_(0),
    data_count_(0),
    m_status(0),
    n_received(0),
    m_datapublistener(nullptr),
    m_commandpublistener(nullptr),
    m_commandsublistener(nullptr),
    m_data_size(0)
{
    m_datapublistener.mp_up = this;
    m_commandpublistener.mp_up = this;
    m_commandsublistener.mp_up = this;
    m_exportPrefix = "";
}

MemoryTestPublisher::~MemoryTestPublisher()
{
    Domain::removeParticipant(mp_participant);
}


bool MemoryTestPublisher::init(int n_sub, int n_sam, bool reliable, uint32_t pid, bool hostname, bool export_csv,
        const std::string& export_prefix, const PropertyPolicy& part_property_policy,
        const PropertyPolicy& property_policy, const std::string& sXMLConfigFile, uint32_t data_size)
{
    m_sXMLConfigFile = sXMLConfigFile;
    n_samples = n_sam;
    n_subscribers = n_sub;
    n_export_csv = export_csv;
    m_exportPrefix = export_prefix;
    reliable_ = reliable;
    m_data_size = data_size;

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
        Domain::registerType(mp_participant, (TopicDataType*)&memory_t);
        Domain::registerType(mp_participant, (TopicDataType*)&command_t);

        // Create Sending Publisher
        std::string profile_name = "publisher_profile";
        mp_datapub = Domain::createPublisher(mp_participant, profile_name, (PublisherListener*)&this->m_datapublistener);
        if (mp_datapub == nullptr)
        {
            return false;
        }
        std::cout << "Publisher created" << std::endl;

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

        Domain::registerType(mp_participant, (TopicDataType*)&memory_t);
        Domain::registerType(mp_participant, (TopicDataType*)&command_t);

        // DATA PUBLISHER
        PublisherAttributes PubDataparam;
        PubDataparam.topic.topicDataType = "MemoryType";
        PubDataparam.topic.topicKind = NO_KEY;
        std::ostringstream pt;
        pt << "MemoryTest_";
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
        if (m_data_size > 60000)
        {
            PubDataparam.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
            PubDataparam.qos.m_publishMode.kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
        }
        mp_datapub = Domain::createPublisher(mp_participant, PubDataparam, (PublisherListener*)&this->m_datapublistener);
        if (mp_datapub == nullptr)
            return false;

        //COMMAND PUBLISHER
        PublisherAttributes PubCommandParam;
        PubCommandParam.topic.topicDataType = "TestCommandType";
        PubCommandParam.topic.topicKind = NO_KEY;
        std::ostringstream pct;
        pct << "MemoryTest_Command_";
        if (hostname)
            pct << asio::ip::host_name() << "_";
        pct << pid << "_PUB2SUB";
        PubCommandParam.topic.topicName = pct.str();
        PubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
        PubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        mp_commandpub = Domain::createPublisher(mp_participant, PubCommandParam, &this->m_commandpublistener);
        if (mp_commandpub == nullptr)
            return false;
        SubscriberAttributes SubCommandParam;
        SubCommandParam.topic.topicDataType = "TestCommandType";
        SubCommandParam.topic.topicKind = NO_KEY;
        std::ostringstream sct;
        sct << "MemoryTest_Command_";
        if (hostname)
            sct << asio::ip::host_name() << "_";
        sct << pid << "_SUB2PUB";
        SubCommandParam.topic.topicName = sct.str();
        SubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
        SubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        SubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        mp_commandsub = Domain::createSubscriber(mp_participant, SubCommandParam, &this->m_commandsublistener);
        if (mp_commandsub == nullptr)
            return false;
    }

    return true;
}

void MemoryTestPublisher::DataPubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
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

void MemoryTestPublisher::CommandPubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
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

void MemoryTestPublisher::CommandSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
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

void MemoryTestPublisher::CommandSubListener::onNewDataMessage(Subscriber* subscriber)
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

void MemoryTestPublisher::run()
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 3 Matchings (Comm pub+sub and publisher or subscriber)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&](){
        return disc_count_ != (n_subscribers * 3);
    });
    disc_lock.unlock();

    cout << C_B_MAGENTA << "DISCOVERY COMPLETE "<<C_DEF<<endl;

    test(m_data_size);
    eClock::my_sleep(100);

    cout << "REMOVING PUBLISHER"<<endl;
    Domain::removePublisher(this->mp_commandpub);
    cout << "REMOVING SUBSCRIBER"<<endl;
    Domain::removeSubscriber(mp_commandsub);

    std::string str_reliable = "besteffort";
    if(reliable_)
        str_reliable = "reliable";
}

bool MemoryTestPublisher::test(uint32_t datasize)
{
    //cout << "Beginning test of size: "<<datasize+4 <<endl;
    m_status = 0;
    n_received = 0;
    mp_memory = new MemoryType(datasize);

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

    for(unsigned int count = 1; count <= n_samples; ++count)
    {
        mp_memory->seqnum = count;

        mp_datapub->write((void*)mp_memory);

        /*
        lock.lock();
        data_cond_.wait_for(lock, std::chrono::milliseconds(10), [&]() { return data_count_ > 0; });

        if(data_count_ > 0)
        {
            --data_count_;
        }
        lock.unlock();
        */
    }

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

    delete(mp_memory);

    return true;
}
