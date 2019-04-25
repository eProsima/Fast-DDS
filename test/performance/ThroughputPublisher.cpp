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
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/Domain.h>

#include <map>
#include <fstream>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

ThroughputPublisher::DataPubListener::DataPubListener(ThroughputPublisher& up):m_up(up){}
ThroughputPublisher::DataPubListener::~DataPubListener(){}

void ThroughputPublisher::DataPubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(m_up.dataMutex_);
    if (info.status == MATCHED_MATCHING)
    {
        //std::cout << C_RED << "DATA Pub Matched" << C_DEF << std::endl;
        ++m_up.data_disc_count_;
    }
    else
    {
        //std::cout << C_RED << "DATA PUBLISHER MATCHING REMOVAL" << C_DEF << std::endl;
        --m_up.data_disc_count_;
    }

    lock.unlock();
    m_up.data_disc_cond_.notify_one();
}

ThroughputPublisher::CommandSubListener::CommandSubListener(ThroughputPublisher& up):m_up(up){}
ThroughputPublisher::CommandSubListener::~CommandSubListener(){}
void ThroughputPublisher::CommandSubListener::onSubscriptionMatched(Subscriber* /*sub*/, MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(m_up.mutex_);
    if (info.status == MATCHED_MATCHING)
    {
        std::cout << C_RED << "COMMAND Sub Matched" << C_DEF << std::endl;
        ++m_up.disc_count_;
    }
    else
    {
        std::cout << C_RED << "COMMAND SUBSCRIBER MATCHING REMOVAL" << C_DEF << std::endl;
        --m_up.disc_count_;
    }

    lock.unlock();
    m_up.disc_cond_.notify_one();
}

ThroughputPublisher::CommandPubListener::CommandPubListener(ThroughputPublisher& up):m_up(up){}
ThroughputPublisher::CommandPubListener::~CommandPubListener(){}

void ThroughputPublisher::CommandPubListener::onPublicationMatched(Publisher* /*pub*/,
    MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(m_up.mutex_);
    if (info.status == MATCHED_MATCHING)
    {
        std::cout << C_RED << "COMMAND Pub Matched" << C_DEF << std::endl;
        ++m_up.disc_count_;
    }
    else
    {
        std::cout << C_RED << "COMMAND PUBLISHER MATCHING REMOVAL" << C_DEF << std::endl;
        --m_up.disc_count_;
    }

    lock.unlock();
    m_up.disc_cond_.notify_one();
}

ThroughputPublisher::ThroughputPublisher(bool reliable, uint32_t pid, bool hostname, bool export_csv,
        const std::string& export_prefix,
        const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
        const std::string& sXMLConfigFile, bool dynamic_types, int forced_domain)
    : disc_count_(0),
    data_disc_count_(0),
#pragma warning(disable:4355)
    m_DataPubListener(*this),
    m_CommandSubListener(*this),
    m_CommandPubListener(*this),
    ready(true),
    m_export_csv(export_csv),
    reliable_(reliable),
    m_sXMLConfigFile(sXMLConfigFile),
    dynamic_data(dynamic_types),
    m_forced_domain(forced_domain)
{
    if (dynamic_data) // Dummy type registration
    {
        // Create basic builders
        DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

        // Add members to the struct.
        struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
        struct_type_builder->add_member(1, "data",
            DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                DynamicTypeBuilderFactory::get_instance()->create_byte_type(), LENGTH_UNLIMITED
            ));
        struct_type_builder->set_name("ThroughputType");

        m_pDynType = struct_type_builder->build();
        m_DynType.SetDynamicType(m_pDynType);
    }

    m_sExportPrefix = export_prefix;

    // Create RTPSParticipant
    std::string participant_profile_name = "participant_profile";
    ParticipantAttributes PParam;

    if (m_forced_domain >= 0)
    {
        PParam.rtps.builtin.domainId = m_forced_domain;
    }
    else
    {
        PParam.rtps.builtin.domainId = pid % 230;
    }
    PParam.rtps.setName("Participant_publisher");
    PParam.rtps.properties = part_property_policy;

    if (m_sXMLConfigFile.length() > 0)
    {
        if (m_forced_domain >= 0)
        {
            ParticipantAttributes participant_att;
            if (eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK ==
                eprosima::fastrtps::xmlparser::XMLProfileManager::fillParticipantAttributes(participant_profile_name,
                    participant_att))
            {
                participant_att.rtps.builtin.domainId = m_forced_domain;
                mp_par = Domain::createParticipant(participant_att);
            }
        }
        else
        {
            mp_par = Domain::createParticipant(participant_profile_name);
        }
    }
    else
    {
        mp_par = Domain::createParticipant(PParam);
    }

    if (mp_par == nullptr)
    {
        std::cout << "ERROR creating participant" << std::endl;
        ready = false;
        return;
    }

    //REGISTER THE TYPES
    if (dynamic_data)
    {
        Domain::registerType(mp_par, &m_DynType);
    }
    else
    {
        Domain::registerType(mp_par, (TopicDataType*)&latency_t);
    }
    Domain::registerType(mp_par, (TopicDataType*)&throuputcommand_t);

    // Create Sending Publisher
    std::string profile_name = "publisher_profile";
    PublisherAttributes Wparam;
    Wparam.topic.topicDataType = "ThroughputType";
    Wparam.topic.topicKind = NO_KEY;
    std::ostringstream pt;
    pt << "ThroughputTest_";
    if (hostname)
    {
        pt << asio::ip::host_name() << "_";
    }
    pt << pid << "_UP";
    Wparam.topic.topicName = pt.str();
    if (reliable)
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
    Wparam.properties = property_policy;

    if (m_sXMLConfigFile.length() > 0)
    {
        mp_datapub = Domain::createPublisher(mp_par, profile_name, (PublisherListener*)&this->m_DataPubListener);
    }
    else
    {
        mp_datapub = Domain::createPublisher(mp_par, Wparam, (PublisherListener*)&this->m_DataPubListener);
    }

    if (mp_datapub == nullptr)
    {
        std::cout << "ERROR creating publisher" << std::endl;
        ready = false;
        return;
    }

    // COMMAND SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.topic.topicDataType = "ThroughputCommand";
    Rparam.topic.topicKind = NO_KEY;
    std::ostringstream sct;
    sct << "ThroughputTest_Command_";
    if (hostname)
    {
        sct << asio::ip::host_name() << "_";
    }
    sct << pid << "_SUB2PUB";
    Rparam.topic.topicName = sct.str();
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    mp_commandsub = Domain::createSubscriber(mp_par, Rparam, (SubscriberListener*)&this->m_CommandSubListener);

    PublisherAttributes Wparam2;
    Wparam2.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Wparam2.topic.topicDataType = "ThroughputCommand";
    Wparam2.topic.topicKind = NO_KEY;
    std::ostringstream pct;
    pct << "ThroughputTest_Command_";
    if (hostname)
    {
        pct << asio::ip::host_name() << "_";
    }
    pct << pid << "_PUB2SUB";
    Wparam2.topic.topicName = pct.str();
    Wparam2.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Wparam2.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Wparam2.qos.m_publishMode.kind = SYNCHRONOUS_PUBLISH_MODE;

    mp_commandpub = Domain::createPublisher(mp_par, Wparam2, (PublisherListener*)&this->m_CommandPubListener);

    // Calculate overhead
    t_start_ = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        t_end_ = std::chrono::steady_clock::now();
    }
    t_overhead_ = std::chrono::duration<double, std::micro>(t_end_ - t_start_) / 1001;
    std::cout << "Overhead " << t_overhead_.count() << " us"  << std::endl;

    if (mp_datapub == nullptr || mp_commandsub == nullptr || mp_commandpub == nullptr)
    {
        ready = false;
    }

    if (dynamic_data)
    {
        DynamicTypeBuilderFactory::delete_instance();
        pubAttr = mp_datapub->getAttributes();
        Domain::removePublisher(mp_datapub);
        Domain::unregisterType(mp_par, "ThroughputType"); // Unregister as we will register it later with correct size
    }
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
        {
            return;
        }
    }
    else
    {
        payload = msg_size;
        m_demand_payload[msg_size - 8].push_back(demand);
    }

    std::cout << "Waiting for command discovery" << std::endl;
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&]()
    {
        return disc_count_ == 2;
    });
    disc_lock.unlock();
    std::cout << "Discovery command complete" << std::endl;

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
            {
                output_file << ",";
            }
        }

        sit++;
        if (sit != m_demand_payload.end())
        {
            output_file << ",";
        }
        sit--;
    }
    command.m_command = ALL_STOPS;
    //	cout << "SEND COMMAND "<< command.m_command << endl;
    mp_commandpub->write((void*)&command);
    mp_commandpub->wait_for_all_acked(Time_t(20, 0));

    if (m_export_csv)
    {
        std::ofstream outFile;
        if (m_sExportPrefix.length() == 0)
        {
            std::string str_reliable = "besteffort";
            if (reliable_)
            {
                str_reliable = "reliable";
            }
            outFile.open("perf_ThroughputTest_" + std::to_string(payload) + "B_" + str_reliable + "_all_.csv");
        }
        else
        {
            outFile.open(m_sExportPrefix + std::to_string(payload) + "_all_.csv");
        }

        outFile << output_file.str();
        outFile.close();
    }
}

bool ThroughputPublisher::test(uint32_t test_time, uint32_t recovery_time_ms, uint32_t demand, uint32_t size)
{
    if (dynamic_data)
    {
        // Create basic builders
        DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

        // Add members to the struct.
        struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
        struct_type_builder->add_member(1, "data",
            DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                DynamicTypeBuilderFactory::get_instance()->create_byte_type(), size
            ));
        struct_type_builder->set_name("ThroughputType");

        m_pDynType = struct_type_builder->build();
        m_DynType.CleanDynamicType();
        m_DynType.SetDynamicType(m_pDynType);

        Domain::registerType(mp_par, &m_DynType);
        mp_datapub = Domain::createPublisher(mp_par, pubAttr, &m_DataPubListener);
        m_DynData = DynamicDataFactory::get_instance()->create_data(m_pDynType);

        MemberId id;
        DynamicData *my_data = m_DynData->loan_value(m_DynData->GetMemberIdAtIndex(1));
        for (uint32_t i = 0; i < size; ++i)
        {
            my_data->insert_sequence_data(id);
            my_data->set_byte_value(0, id);
        }
        m_DynData->return_loaned_value(my_data);
    }
    else
    {
        latency = new ThroughputType((uint16_t)size);
    }

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

    //std::cout << "Waiting for data discovery" << std::endl;
    std::unique_lock<std::mutex> data_disc_lock(dataMutex_);
    data_disc_cond_.wait(data_disc_lock, [&]()
    {
        return data_disc_count_ > 0;
    });
    data_disc_lock.unlock();
    //std::cout << "Discovery data complete" << std::endl;

    t_start_ = std::chrono::steady_clock::now();
    while (std::chrono::duration<double, std::micro>(t_end_ - t_start_) < test_time_us)
    {
        for (uint32_t sample = 0; sample < demand; sample++)
        {
            //cout << sample << "*"<<std::flush;
            if (dynamic_data)
            {
                m_DynData->set_uint32_value(m_DynData->get_uint32_value(0) + 1, 0);
                mp_datapub->write((void*)m_DynData);
            }
            else
            {
                latency->seqnum++;
                mp_datapub->write((void*)latency);
            }
        }
        t_end_ = std::chrono::steady_clock::now();
        samples += demand;
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

    if (dynamic_data)
    {
        DynamicTypeBuilderFactory::delete_instance();
        DynamicDataFactory::get_instance()->delete_data(m_DynData);
        pubAttr = mp_datapub->getAttributes();
        Domain::removePublisher(mp_datapub);
        Domain::unregisterType(mp_par, "ThroughputType");
    }
    else
    {
        delete(latency);
    }

    mp_commandsub->waitForUnreadMessage();
    if (mp_commandsub->takeNextData((void*)&command, &info))
    {
        //cout << "RECI COMMAND "<< command.m_command << endl;
        if (command.m_command == TEST_RESULTS)
        {
            //cout << "Received results from subscriber"<<endl;
            TroughputResults result;
            result.demand = demand;
            result.payload_size = size + 4 + 4;
            result.publisher.send_samples = samples;
            result.publisher.totaltime_us = std::chrono::duration<double, std::micro>(t_end_ - t_start_) - timewait_us;
            result.subscriber.recv_samples = command.m_lastrecsample - command.m_lostsamples;
            result.subscriber.totaltime_us = std::chrono::microseconds(command.m_totaltime);
            result.subscriber.lost_samples = command.m_lostsamples;
            result.compute();
            m_timeStats.push_back(result);

            output_file << "\"" << result.subscriber.MBitssec << "\"";
            if (m_export_csv)
            {
                std::ofstream outFile;
                std::string str_reliable = "besteffort";
                if (reliable_)
                {
                    str_reliable = "reliable";
                }

                std::string fileName = "";
                if (m_sExportPrefix.length() > 0)
                {
                    fileName = m_sExportPrefix + std::to_string(result.payload_size) + "B_" + str_reliable + "_" +
                        std::to_string(result.demand) + "demand.csv";
                }
                else
                {
                    fileName = "perf_ThroughputTest_" + std::to_string(result.payload_size) +
                        "B_" + str_reliable + "_" + std::to_string(result.demand) + "demand.csv";
                }
                outFile.open(fileName);
                outFile << "\"" << result.payload_size << " bytes; demand " << result.demand << " (" + str_reliable + ")\"" << std::endl;
                outFile << "\"" << result.subscriber.MBitssec << "\"";
                outFile.close();
            }

            printResults(result);
            mp_commandpub->removeAllChange(&aux);
            return true;
        }
        else
        {
            std::cout << "The test expected results, stopping" << std::endl;
        }
    }
    else
        std::cout << "PROBLEM READING RESULTS;" << std::endl;

    return false;
}

bool ThroughputPublisher::loadDemandsPayload()
{
    std::ifstream fi(m_file_name);

    std::cout << "Reading File: " << m_file_name << std::endl;
    std::string DELIM = ";";
    if (!fi.is_open())
    {
        std::cout << "Could not open file: " << m_file_name << " , closing." << std::endl;
        return false;
    }

    std::string line;
    size_t start;
    size_t end;
    bool first = true;
    bool more = true;
    while (std::getline(fi, line))
    {
        //	cout << "READING LINE: "<< line<<endl;
        start = 0;
        end = line.find(DELIM);
        first = true;
        uint32_t demand;
        more = true;
        while (more)
        {
            //	cout << "SUBSTR: "<< line.substr(start,end-start) << endl;
            std::istringstream iss(line.substr(start, end - start));
            if (first)
            {
                iss >> payload;
                if (payload < 8)
                {
                    std::cout << "Minimum payload is 16 bytes" << std::endl;
                    return false;
                }
                payload -= 8;
                first = false;
            }
            else
            {
                iss >> demand;
                m_demand_payload[payload].push_back(demand);
            }

            start = end + DELIM.length();
            end = line.find(DELIM, start);
            if (end == std::string::npos)
            {
                more = false;
                std::istringstream n_iss(line.substr(start, end - start));
                if (n_iss >> demand)
                {
                    m_demand_payload[payload].push_back(demand);
                }
            }
        }
    }
    fi.close();

    payload += 8;

    std::cout << "Performing test with this payloads/demands:" << std::endl;
    for (auto sit = m_demand_payload.begin(); sit != m_demand_payload.end(); ++sit)
    {
        printf("Payload: %6d; Demands: ", sit->first + 8);
        for (auto dit = sit->second.begin(); dit != sit->second.end(); ++dit)
        {
            printf("%6d, ", *dit);
            output_file << "\"" << sit->first + 8 << " bytes; demand " << *dit << " (MBits/sec)\"";
            if ((dit + 1) != sit->second.end())
            {
                output_file << ",";
            }
        }
        printf("\n");

        sit++;
        if (sit != m_demand_payload.end())
        {
            output_file << ",";
        }
        sit--;
    }

    output_file << std::endl;

    return true;
}
