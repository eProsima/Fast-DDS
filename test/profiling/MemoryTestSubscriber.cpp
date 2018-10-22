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
 * @file MemoryTestSubscriber.cpp
 *
 */

#include "MemoryTestSubscriber.h"
#include "fastrtps/log/Log.h"
#include "fastrtps/log/Colors.h"

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

MemoryTestSubscriber::MemoryTestSubscriber():
    mp_participant(nullptr),
    mp_commandpub(nullptr),
    mp_datasub(nullptr),
    mp_commandsub(nullptr),
    mp_memory(nullptr),
    disc_count_(0),
    comm_count_(0),
    data_count_(0),
    m_status(0),
    n_received(0),
    n_samples(0),
    m_datasublistener(nullptr),
    m_commandpublistener(nullptr),
    m_commandsublistener(nullptr),
    m_echo(true)
{
    m_datasublistener.mp_up = this;
    m_commandpublistener.mp_up = this;
    m_commandsublistener.mp_up = this;


}

MemoryTestSubscriber::~MemoryTestSubscriber()
{
    Domain::removeParticipant(mp_participant);
}

bool MemoryTestSubscriber::init(bool echo, int nsam, bool reliable, uint32_t pid, bool hostname,
        const PropertyPolicy& part_property_policy, const PropertyPolicy& property_policy,
        const std::string& sXMLConfigFile, uint32_t data_size)
{
    m_sXMLConfigFile = sXMLConfigFile;
    m_echo = echo;
    n_samples = nsam;
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

        Domain::registerType(mp_participant, (TopicDataType*)&memory_t);
        Domain::registerType(mp_participant, (TopicDataType*)&command_t);

        // Create Data Subscriber
        std::string profile_name = "subscriber_profile";
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

        Domain::registerType(mp_participant, (TopicDataType*)&memory_t);
        Domain::registerType(mp_participant, (TopicDataType*)&command_t);


        Locator_t loc;
        //DATA SUBSCRIBER
        SubscriberAttributes SubDataparam;
        SubDataparam.topic.topicDataType = "MemoryType";
        SubDataparam.topic.topicKind = NO_KEY;
        std::ostringstream st;
        st << "MemoryTest_";
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
        if (m_data_size > 60000)
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
        pct << "MemoryTest_Command_";
        if (hostname)
            pct << asio::ip::host_name() << "_";
        pct << pid << "_SUB2PUB";
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
        sct << pid << "_PUB2SUB";
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

void MemoryTestSubscriber::DataSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        logInfo(MemoryTest,"Data Sub Matched ");
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void MemoryTestSubscriber::CommandPubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        logInfo(MemoryTest, "Command Pub Matched ");
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void MemoryTestSubscriber::CommandSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        logInfo(MemoryTest, "Command Sub Matched ");
        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void MemoryTestSubscriber::CommandSubListener::onNewDataMessage(Subscriber* subscriber)
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

void MemoryTestSubscriber::DataSubListener::onNewDataMessage(Subscriber* subscriber)
{
    subscriber->takeNextData((void*)mp_up->mp_memory,&mp_up->m_sampleinfo);
    //	cout << "R: "<< mp_up->mp_memory->seqnum << "|"<<mp_up->m_echo<<std::flush;
    //	//	eClock::my_sleep(50);
    //		cout << "NSAMPLES: " << (uint32_t)mp_up->n_samples<< endl;
    ++mp_up->n_received;
    if (mp_up->m_echo)
    {
        std::cout << "Receied data: " << mp_up->mp_memory->seqnum << "(" << mp_up->n_received << ")" << std::endl;
    }
}


void MemoryTestSubscriber::run()
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 3 Matchings (Comd Pub+Sub and publisher or subscriber)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&](){
        return disc_count_ != 3;
    });
    disc_lock.unlock();

    cout << C_B_MAGENTA << "DISCOVERY COMPLETE "<<C_DEF<<endl;

    test(m_data_size);
}

bool MemoryTestSubscriber::test(uint32_t datasize)
{
    cout << "Preparing test with data size: " << datasize + 4 << endl;
    mp_memory = new MemoryType(datasize);

    std::unique_lock<std::mutex> lock(mutex_);
    if (comm_count_ == 0) comm_cond_.wait(lock);
    --comm_count_;
    lock.unlock();

    m_status = 0;
    n_received = 0;
    TestCommandType command;
    command.m_command = BEGIN;
    cout << "Testing with data size: " << datasize + 4 << endl;
    mp_commandpub->write(&command);

    lock.lock();
    data_cond_.wait(lock, [&]()
    {
        return data_count_ > 0;
    });
    --data_count_;
    lock.unlock();

    cout << "TEST OF SIZE: " << datasize + 4 << " ENDS" << endl;
    eClock::my_sleep(50);
    //cout << "REMOVED: "<< removed<<endl;
    delete(mp_memory);
    if (m_status == -1)
    {
        return false;
    }
    return true;
}
