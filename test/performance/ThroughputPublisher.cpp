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
 * @file ThroughputPublisher.cxx
 *
 */

#include "ThroughputPublisher.h"

#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/Domain.h>

#include <map>
#include <fstream>



ThroughputPublisher::DataPubListener::DataPubListener(ThroughputPublisher& up):m_up(up){}
ThroughputPublisher::DataPubListener::~DataPubListener(){}

void ThroughputPublisher::DataPubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(m_up.mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        cout << C_RED << "DATA Pub Matched"<<C_DEF<<endl;
        ++m_up.disc_count_;
    }
    else
    {
        cout << C_RED << "DATA PUBLISHER MATCHING REMOVAL" << C_DEF<<endl;
        --m_up.disc_count_;
    }

    lock.unlock();
    m_up.disc_cond_.notify_one();
}

ThroughputPublisher::CommandSubListener::CommandSubListener(ThroughputPublisher& up):m_up(up){}
ThroughputPublisher::CommandSubListener::~CommandSubListener(){}
void ThroughputPublisher::CommandSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(m_up.mutex_);

    if(info.status == MATCHED_MATCHING)
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

ThroughputPublisher::CommandPubListener::CommandPubListener(ThroughputPublisher& up):m_up(up){}
ThroughputPublisher::CommandPubListener::~CommandPubListener(){}
void ThroughputPublisher::CommandPubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
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

ThroughputPublisher::ThroughputPublisher(bool reliable, uint32_t pid, bool hostname, bool export_csv): disc_count_(0),
#pragma warning(disable:4355)
    m_DataPubListener(*this), m_CommandSubListener(*this), m_CommandPubListener(*this),
    ready(true), m_export_csv(export_csv), reliable_(reliable)
{
    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = pid % 230;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_publisher");
    mp_par = Domain::createParticipant(PParam);
    if(mp_par == nullptr)
    {
        cout << "ERROR creating participant"<<endl;
        ready = false;
        return;
    }
    //REGISTER THE TYPES
    Domain::registerType(mp_par,(TopicDataType*)&latency_t);
    Domain::registerType(mp_par,(TopicDataType*)&throuputcommand_t);

    // Calculate overhead
    t_start_ = std::chrono::steady_clock::now();
    for(int i= 0; i < 1000; ++i)
        t_end_ = std::chrono::steady_clock::now();
    t_overhead_ = std::chrono::duration<double, std::micro>(t_end_ - t_start_) / 1001;
    cout << "Overhead " << t_overhead_.count() << " us"  << endl;

    //DATA PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicDataType = "ThroughputType";
    Wparam.topic.topicKind = NO_KEY;
    std::ostringstream pt;
    pt << "ThroughputTest_";
    if(hostname)
        pt << asio::ip::host_name() << "_";
    pt << pid << "_UP";
    Wparam.topic.topicName = pt.str();

    if(reliable)
    {
        //RELIABLE
        Wparam.times.heartbeatPeriod = TimeConv::MilliSeconds2Time_t(100);
        Wparam.times.nackSupressionDuration = TimeConv::MilliSeconds2Time_t(0);
        Wparam.times.nackResponseDelay = TimeConv::MilliSeconds2Time_t(0);
        Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        //BEST EFFORT:
        Wparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    }


    Wparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Wparam.topic.resourceLimitsQos.max_samples = 0;
    Wparam.topic.resourceLimitsQos.allocated_samples = 1000;

    mp_datapub = Domain::createPublisher(mp_par,Wparam,(PublisherListener*)&this->m_DataPubListener);

    //COMMAND
    SubscriberAttributes Rparam;
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.topic.resourceLimitsQos.max_samples = 20;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.topic.topicDataType = "ThroughputCommand";
    Rparam.topic.topicKind = NO_KEY;
    std::ostringstream sct;
    sct << "ThroughputTest_Command_";
    if(hostname)
        sct << asio::ip::host_name() << "_";
    sct << pid << "_SUB2PUB";
    Rparam.topic.topicName = sct.str();
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_commandsub = Domain::createSubscriber(mp_par,Rparam,(SubscriberListener*)&this->m_CommandSubListener);
    PublisherAttributes Wparam2;
    Wparam2.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam2.topic.historyQos.depth = 50;
    Wparam2.topic.resourceLimitsQos.max_samples = 50;
    Wparam2.topic.resourceLimitsQos.allocated_samples = 50;
    Wparam2.topic.topicDataType = "ThroughputCommand";
    Wparam2.topic.topicKind = NO_KEY;
    std::ostringstream pct;
    pct << "ThroughputTest_Command_";
    if(hostname)
        pct << asio::ip::host_name() << "_";
    pct << pid << "_PUB2SUB";
    Wparam2.topic.topicName = pct.str();
    Wparam2.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Wparam2.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_commandpub = Domain::createPublisher(mp_par,Wparam2,(PublisherListener*)&this->m_CommandPubListener);

    if(mp_datapub == nullptr || mp_commandsub == nullptr || mp_commandpub == nullptr)
        ready = false;
}


ThroughputPublisher::~ThroughputPublisher()
{
    Domain::removeParticipant(mp_par);
}

void ThroughputPublisher::run(uint32_t test_time, uint32_t recovery_time_ms, int demand, int msg_size)
{
    if (!ready)
    {

        return;
    }
    if (demand == 0 || msg_size == 0)
    {
        if (!this->loadDemandsPayload())
            return;
    }
    else
    {
        payload = msg_size;
        m_demand_payload[msg_size - 8].push_back(demand);
    }
    cout << "Waiting for discovery" << endl;
    std::unique_lock<std::mutex> disc_lock(mutex_);
    while(disc_count_ != 3) disc_cond_.wait(disc_lock);
    disc_lock.unlock();
    cout << "Discovery complete" << endl;

    ThroughputCommandType command;
    SampleInfo_t info;
    printResultTitle();
    for (auto sit = m_demand_payload.begin(); sit != m_demand_payload.end(); ++sit)
    {
        for (auto dit = sit->second.begin(); dit != sit->second.end(); ++dit)
        {
            eClock::my_sleep(100);
            //cout << "Starting test with demand: " << *dit << endl;
            command.m_command = READY_TO_START;
            command.m_size = sit->first;
            command.m_demand = *dit;
            //cout << "SEND COMMAND "<< command.m_command << endl;
            mp_commandpub->write((void*)&command);
            command.m_command = DEFAULT;
            mp_commandsub->waitForUnreadMessage();
            mp_commandsub->takeNextData((void*)&command, &info);
            //cout << "RECI COMMAND "<< command.m_command << endl;
            //cout << "Received command of type: "<< command << endl;
            if (command.m_command == BEGIN)
            {
                if (!test(test_time, recovery_time_ms, *dit, sit->first))
                {
                    command.m_command = ALL_STOPS;
                    //	cout << "SEND COMMAND "<< command.m_command << endl;
                    mp_commandpub->write((void*)&command);
                    return;
                }
            }

            if ((dit + 1) != sit->second.end())
                output_file << ",";
        }

        sit++;
        if (sit != m_demand_payload.end())
            output_file << ",";
        sit--;
    }
    command.m_command = ALL_STOPS;
    //	cout << "SEND COMMAND "<< command.m_command << endl;
    mp_commandpub->write((void*)&command);
    mp_commandpub->wait_for_all_acked(Time_t(20, 0));

    if (m_export_csv) {
        std::ofstream outFile;
        std::string str_reliable = "besteffort";
        if(reliable_)
            str_reliable = "reliable";
        outFile.open("perf_ThroughputTest_" + std::to_string(payload) + "B_" + str_reliable + ".csv");
        outFile << output_file.str();
        outFile.close();
    }
}

bool ThroughputPublisher::test(uint32_t test_time, uint32_t recovery_time_ms, uint32_t demand, uint32_t size)
{
    ThroughputType latency((uint16_t)size);
    t_end_ = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::micro> timewait_us(0);
    std::chrono::duration<double, std::micro> test_time_us = std::chrono::seconds(test_time);
    uint32_t samples = 0;
    size_t aux;
    ThroughputCommandType command;
    SampleInfo_t info;
    command.m_command = TEST_STARTS;
    //cout << "SEND COMMAND "<< command.m_command << endl;
    mp_commandpub->write((void*)&command);

    t_start_ = std::chrono::steady_clock::now();
    while(std::chrono::duration<double, std::micro>(t_end_ - t_start_) < test_time_us)
    {
        for(uint32_t sample = 0; sample < demand;sample++)
        {
            latency.seqnum++;
            mp_datapub->write((void*)&latency);
            //cout << sample << "*"<<std::flush;
        }
        t_end_ = std::chrono::steady_clock::now();
        samples+=demand;
        //cout << "samples sent: "<<samples<< endl;
        eClock::my_sleep(recovery_time_ms);
        timewait_us += t_overhead_;
    }
    command.m_command = TEST_ENDS;
    //cout << "SEND COMMAND "<< command.m_command << endl;
    eClock::my_sleep(100);
    mp_commandpub->write((void*)&command);
    eClock::my_sleep(100);
    mp_datapub->removeAllChange();
    mp_commandsub->waitForUnreadMessage();
    if(mp_commandsub->takeNextData((void*)&command,&info))
    {
        //cout << "RECI COMMAND "<< command.m_command << endl;
        if(command.m_command == TEST_RESULTS)
        {
            //cout << "Received results from subscriber"<<endl;
            TroughputResults result;
            result.demand = demand;
            result.payload_size = size+4+4;
            result.publisher.send_samples = samples;
            result.publisher.totaltime_us = std::chrono::duration<double, std::micro>(t_end_ - t_start_) - timewait_us;
            result.subscriber.recv_samples = command.m_lastrecsample-command.m_lostsamples;
            result.subscriber.totaltime_us = std::chrono::microseconds(command.m_totaltime);
            result.subscriber.lost_samples = command.m_lostsamples;
            result.compute();
            m_timeStats.push_back(result);

            output_file << "\"" << result.subscriber.MBitssec << "\"";
            if (m_export_csv) {
                std::ofstream outFile;
                std::string str_reliable = "besteffort";
                if(reliable_)
                    str_reliable = "reliable";
                std::string fileName = "perf_ThroughputTest_" +
                    std::to_string(result.payload_size) + "B_" + str_reliable + "_" +
                    std::to_string(result.demand) + "demand"
                    ".csv";
                outFile.open(fileName);
                outFile << "\"" << result.payload_size << " bytes; demand " << result.demand << " (" + str_reliable + ")\"" << endl;
                outFile << "\"" << result.subscriber.MBitssec << "\"";
                outFile.close();
            }

            printResults(result);
            mp_commandpub->removeAllChange(&aux);
            return true;
        }
        else
        {
            cout << "The test expected results, stopping"<<endl;
        }
    }
    else
        cout << "PROBLEM READING RESULTS;"<<endl;

    return false;

}

bool ThroughputPublisher::loadDemandsPayload()
{
    std::ifstream fi(m_file_name);

    cout << "Reading File: " << m_file_name << endl;
    std::string DELIM = ";";
    if(!fi.is_open())
    {
        std::cout << "Could not open file: " << m_file_name << " , closing." << std::endl;
        return false;
    }

    std::string line;
    size_t start;
    size_t end;
    bool first = true;
    bool more = true;
    while(std::getline(fi,line))
    {
        //	cout << "READING LINE: "<< line<<endl;
        start = 0;
        end = line.find(DELIM);
        first = true;
        uint32_t demand;
        more = true;
        while(more)
        {
            //	cout << "SUBSTR: "<< line.substr(start,end-start) << endl;
            std::istringstream iss(line.substr(start,end-start));
            if(first)
            {
                iss >> payload;
                if(payload<8)
                {
                    cout << "Minimum payload is 16 bytes"<<endl;
                    return false;
                }
                payload -=8;
                first = false;
            }
            else
            {
                iss >> demand;
                m_demand_payload[payload].push_back(demand);
            }
            start = end+DELIM.length();
            end = line.find(DELIM,start);
            if(end == std::string::npos)
            {
                more = false;
                std::istringstream n_iss(line.substr(start,end-start));
                if(n_iss >> demand)
                    m_demand_payload[payload].push_back(demand);
            }
        }
    }
    fi.close();

    payload += 8;

    //////////////////////////////
    /*
       char date_buffer[9];
       char time_buffer[7];
       time_t t = time(0);   // get time now
       struct tm * now = localtime(&t);
       strftime(date_buffer, 9, "%Y%m%d", now);
       strftime(time_buffer, 7, "%H%M%S", now);
       *//*
            output_file_name << "perf_ThroughputTest.csv";
            for (std::vector<uint32_t>::iterator it = data_size_pub.begin(); it != data_size_pub.end(); ++it) {
            output_file << "\"" << n_samples << " samples of " << *it + 4 << " bytes (us)\"";
            if (it != data_size_pub.end() - 1)
            output_file << ",";
            }
            output_file << std::endl;
    //////////////////////////////*/

    cout << "Performing test with this payloads/demands:"<<endl;
    for(auto sit=m_demand_payload.begin();sit!=m_demand_payload.end();++sit)
    {
        printf("Payload: %6d; Demands: ",sit->first+8);
        for(auto dit=sit->second.begin();dit!=sit->second.end();++dit)
        {
            printf("%6d, ",*dit);
            output_file << "\"" << sit->first + 8 << " bytes; demand " << *dit << " (MBits/sec)\"";
            if ((dit+1) != sit->second.end())
                output_file << ",";
        }
        printf("\n");

        sit++;
        if (sit != m_demand_payload.end())
            output_file << ",";
        sit--;
    }

    output_file << std::endl;

    return true;
}
