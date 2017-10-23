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
 * @file ThroughputSubscriber.cxx
 *
 */

#include "ThroughputSubscriber.h"

#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/Domain.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#include <vector>

int writecalls= 0;


ThroughputSubscriber::DataSubListener::DataSubListener(ThroughputSubscriber& up):
    m_up(up),lastseqnum(0),saved_lastseqnum(0),lostsamples(0),saved_lostsamples(0),first(true),throughputin(nullptr)
{
}

ThroughputSubscriber::DataSubListener::~DataSubListener()
{
}

void ThroughputSubscriber::DataSubListener::reset()
{
    lastseqnum = 0;
    first = true;
    lostsamples=0;
}

void ThroughputSubscriber::DataSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& match_info)
{
    std::unique_lock<std::mutex> lock(m_up.mutex_);

    if(match_info.status == MATCHED_MATCHING)
    {
        cout << C_RED << "DATA Sub Matched"<<C_DEF<<endl;
        ++m_up.disc_count_;
    }
    else
    {
        cout << C_RED << "DATA SUBSCRIBER MATCHING REMOVAL" << C_DEF<<endl;
        --m_up.disc_count_;
    }

    lock.unlock();
    m_up.disc_cond_.notify_one();
}
void ThroughputSubscriber::DataSubListener::onNewDataMessage(Subscriber* subscriber)
{
    //	cout << "NEW DATA MSG: "<< throughputin->seqnum << endl;
    while(subscriber->takeNextData((void*)throughputin, &info))
    {
        //myfile << throughputin.seqnum << ",";
        if(info.sampleKind == ALIVE)
        {
            //cout << "R:"<<throughputin->seqnum<<std::flush;
            if((lastseqnum+1)<throughputin->seqnum)
            {
                lostsamples+=throughputin->seqnum-lastseqnum-1;
                //	myfile << "***** lostsamples: "<< lastseqnum << "|"<< lostsamples<< "*****";
            }
            lastseqnum = throughputin->seqnum;
        }
        else
        {
            cout << "NOT ALIVE DATA RECEIVED"<<endl;
        }
    }
    //	cout << ";O|"<<std::flush;
}

void ThroughputSubscriber::DataSubListener::saveNumbers()
{
    saved_lastseqnum = lastseqnum;
    saved_lostsamples = lostsamples;
}



ThroughputSubscriber::CommandSubListener::CommandSubListener(ThroughputSubscriber& up):m_up(up){}
ThroughputSubscriber::CommandSubListener::~CommandSubListener(){}
void ThroughputSubscriber::CommandSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& match_info)
{
    std::unique_lock<std::mutex> lock(m_up.mutex_);

    if(match_info.status == MATCHED_MATCHING)
    {
        cout << C_RED << "COMMAND Sub Matched"<<C_DEF<<endl;
        ++m_up.disc_count_;
    }
    else
    {
        cout << C_RED << "COMMAND SUBSCRIBER MATCHING REMOVAL" << C_DEF<<endl;
        --m_up.disc_count_;
    }

    lock.unlock();
    m_up.disc_cond_.notify_one();
}
void ThroughputSubscriber::CommandSubListener::onNewDataMessage(Subscriber* subscriber)
{
    //cout << "Command Received: ";
    if(subscriber->takeNextData((void*)&m_commandin,&info))
    {
        //cout << "RECEIVED COMMAND: "<< m_commandin.m_command << endl;
        switch(m_commandin.m_command)
        {
            default: break;
            case (DEFAULT): break;
            case (BEGIN):{
                             break;
                         }
            case (READY_TO_START):{
                                      cout << "Command: READY_TO_START" << endl;
                                      m_up.m_datasize = m_commandin.m_size;
                                      m_up.m_demand = m_commandin.m_demand;
                                      //cout << "Ready to start data size: " << m_datasize << " and demand; "<<m_demand << endl;
                                      m_up.m_DataSubListener.throughputin = new ThroughputType((uint16_t)m_up.m_datasize);
                                      ThroughputCommandType command(BEGIN);
                                      eClock::my_sleep(50);
                                      m_up.m_DataSubListener.reset();
                                      //cout << "SEND COMMAND: "<< command.m_command << endl;
                                      //cout << "writecall "<< ++writecalls << endl;
                                      m_up.mp_commandpubli->write(&command);
                                      break;
                                  }
            case (TEST_STARTS):{
                                   m_up.t_start_ = std::chrono::steady_clock::now();
                                   cout << "Command: TEST_STARTS" << endl;
                                   break;
                               }
            case (TEST_ENDS):{
                                 m_up.t_end_ = std::chrono::steady_clock::now();
                                 m_up.m_DataSubListener.saveNumbers();
                                 cout << "Command: TEST_ENDS" << endl;
                                 std::unique_lock<std::mutex> lock(m_up.mutex_);
                                 m_up.stop_count_ = 1;
                                 lock.unlock();
                                 m_up.stop_cond_.notify_one();
                                 break;
                             }
            case (ALL_STOPS):
                             {
                                 std::unique_lock<std::mutex> lock(m_up.mutex_);
                                 m_up.stop_count_ = 2;
                                 lock.unlock();
                                 m_up.stop_cond_.notify_one();
                                 cout << "Command: ALL_STOPS" << endl;
                             }
        }
    }
    else
    {
        cout << "Error reading command"<<endl;
    }
}

ThroughputSubscriber::CommandPubListener::CommandPubListener(ThroughputSubscriber& up):m_up(up){}
ThroughputSubscriber::CommandPubListener::~CommandPubListener(){}
void ThroughputSubscriber::CommandPubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(m_up.mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        cout << C_RED << "COMMAND Pub Matched"<<C_DEF<<endl;
        ++m_up.disc_count_;
    }
    else
    {
        cout << C_RED << "COMMAND PUBLISHER MATCHING REMOVAL" << C_DEF<<endl;
        --m_up.disc_count_;
    }

    lock.unlock();
    m_up.disc_cond_.notify_one();
}



ThroughputSubscriber::~ThroughputSubscriber(){Domain::stopAll();}

ThroughputSubscriber::ThroughputSubscriber(bool reliable, uint32_t pid, bool hostname) : disc_count_(0), stop_count_(0),
#pragma warning(disable:4355)
    m_DataSubListener(*this),m_CommandSubListener(*this),m_CommandPubListener(*this),
    ready(true),m_datasize(0),m_demand(0)
{
    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = pid % 230;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_subscriber");
    mp_par = Domain::createParticipant(PParam);
    if(mp_par == nullptr)
    {
        cout << "ERROR"<<endl;
        ready = false;
        return;
    }
    //REGISTER THE TYPES
    Domain::registerType(mp_par,(TopicDataType*)&throughput_t);
    Domain::registerType(mp_par,(TopicDataType*)&throuputcommand_t);


    t_start_ = std::chrono::steady_clock::now();
    for(int i = 0; i < 1000; ++i)
        t_end_ = std::chrono::steady_clock::now();
    t_overhead_ = std::chrono::duration<double, std::micro>(t_end_ - t_start_) / 1001;
    cout << "Overhead " << t_overhead_.count() << endl;

    //SUBSCRIBER DATA
    SubscriberAttributes Sparam;
    Sparam.topic.topicDataType = "ThroughputType";
    Sparam.topic.topicKind = NO_KEY;
    std::ostringstream st;
    st << "ThroughputTest_";
    if(hostname)
        st << asio::ip::host_name() << "_";
    st << pid << "_UP";
    Sparam.topic.topicName = st.str();

    if(reliable)
    {
        //RELIABLE
        Sparam.times.heartbeatResponseDelay = TimeConv::MilliSeconds2Time_t(0);
        Sparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        //BEST EFFORT
        Sparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    }

    Sparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Sparam.topic.resourceLimitsQos.max_samples = 100000;
    Sparam.topic.resourceLimitsQos.allocated_samples = 10000;

    Locator_t loc;
    loc.port = 10110;
    Sparam.unicastLocatorList.push_back(loc);
    mp_datasub = Domain::createSubscriber(mp_par,Sparam,(SubscriberListener*)&this->m_DataSubListener);
    //COMMAND
    PublisherAttributes Wparam;
    //Wparam.historyMaxSize = 20;
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 50;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 50;
    Wparam.topic.topicDataType = "ThroughputCommand";
    Wparam.topic.topicKind = NO_KEY;
    std::ostringstream pct;
    pct << "ThroughputTest_Command_";
    if(hostname)
        pct << asio::ip::host_name() << "_";
    pct << pid << "_SUB2PUB";
    Wparam.topic.topicName = pct.str();
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_commandpubli = Domain::createPublisher(mp_par,Wparam,(PublisherListener*)&this->m_CommandPubListener);
    SubscriberAttributes Rparam;
    Rparam.topic.topicDataType = "ThroughputCommand";
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicName = "ThroughputCommandP2S";
    std::ostringstream sct;
    sct << "ThroughputTest_Command_";
    if(hostname)
        sct << asio::ip::host_name() << "_";
    sct << pid << "_PUB2SUB";
    Rparam.topic.topicName = sct.str();
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.topic.historyQos.depth = 20;
    Rparam.topic.resourceLimitsQos.max_samples = 20;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    loc.port = 7556;
    Rparam.unicastLocatorList.push_back(loc);
    mp_commandsub = Domain::createSubscriber(mp_par,Rparam,(SubscriberListener*)&this->m_CommandSubListener);

    if(mp_datasub == nullptr || mp_commandsub == nullptr || mp_commandpubli == nullptr)
        ready = false;

    eClock::my_sleep(1000);
}

void ThroughputSubscriber::run()
{
    if(!ready)
        return;
    cout << "Waiting for discovery"<<endl;
    std::unique_lock<std::mutex> lock(mutex_);
    while(disc_count_ != 3) disc_cond_.wait(lock);
    cout << "Discovery complete"<<endl;

    while (stop_count_ != 2)
    {
        stop_cond_.wait(lock);

        if (stop_count_ == 1)
        {
            cout << "Waiting clean state" << endl;
            while (!mp_datasub->isInCleanState())
            {
                eClock::my_sleep(50);
            }
            cout << "Sending results" << endl;
            ThroughputCommandType comm;
            comm.m_command = TEST_RESULTS;
            comm.m_demand = m_demand;
            comm.m_size = m_datasize + 4 + 4;
            comm.m_lastrecsample = m_DataSubListener.saved_lastseqnum;
            comm.m_lostsamples = m_DataSubListener.saved_lostsamples;

            auto total_time_count = (std::chrono::duration<double, std::micro>(t_end_ - t_start_) - t_overhead_).count();
            if(total_time_count < std::numeric_limits<uint64_t>::min()){
                comm.m_totaltime = std::numeric_limits<uint64_t>::min();
            }
            else if(total_time_count > std::numeric_limits<uint64_t>::max()){
                comm.m_totaltime = std::numeric_limits<uint64_t>::max();
            }
            else{
                comm.m_totaltime = static_cast<uint64_t>(total_time_count);
            }

            cout << "Last Received Sample: " << comm.m_lastrecsample << endl;
            cout << "Lost Samples: " << comm.m_lostsamples << endl;
            cout << "Test of size " << comm.m_size << " and demand " << comm.m_demand << " ends." << endl;
            mp_commandpubli->write(&comm);

            stop_count_ = 0;
        }
    }
    return;

}
