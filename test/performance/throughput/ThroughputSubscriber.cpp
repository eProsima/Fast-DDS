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
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/Domain.h>
#include <vector>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;


// *******************************************************************************************
// ************************************ DATA SUB LISTENER ************************************
// *******************************************************************************************
ThroughputSubscriber::DataSubListener::DataSubListener(ThroughputSubscriber& up)
    : m_up(up)
    , lastseqnum(0)
    , saved_lastseqnum(0)
    , lostsamples(0)
    , saved_lostsamples(0)
    , first(true)
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

void ThroughputSubscriber::DataSubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& match_info)
{
    std::unique_lock<std::mutex> lock(m_up.dataMutex_);

    if (match_info.status == MATCHED_MATCHING)
    {
        std::cout << C_RED << "DATA Sub Matched" << C_DEF << std::endl;
        ++m_up.data_disc_count_;
    }
    else
    {
        std::cout << C_RED << "DATA SUBSCRIBER MATCHING REMOVAL" << C_DEF << std::endl;
        --m_up.data_disc_count_;
    }

    lock.unlock();
    m_up.data_disc_cond_.notify_one();
}

void ThroughputSubscriber::DataSubListener::onNewDataMessage(Subscriber* subscriber)
{
    if (m_up.dynamic_data)
    {
        while (subscriber->takeNextData((void*)m_up.m_DynData, &info))
        {
            if (info.sampleKind == ALIVE)
            {
                if ((lastseqnum + 1) < m_up.m_DynData->get_uint32_value(0))
                {
                    lostsamples += m_up.m_DynData->get_uint32_value(0) - lastseqnum - 1;
                }
                lastseqnum = m_up.m_DynData->get_uint32_value(0);
            }
            else
            {
                std::cout << "NOT ALIVE DATA RECEIVED" << std::endl;
            }
        }
    }
    else
    {
        if (m_up.throughputin != nullptr)
        {
            while (subscriber->takeNextData((void*)m_up.throughputin, &info))
            {
                if (info.sampleKind == ALIVE)
                {
                    if ((lastseqnum + 1) < m_up.throughputin->seqnum)
                    {
                        lostsamples += m_up.throughputin->seqnum - lastseqnum - 1;
                    }
                    lastseqnum = m_up.throughputin->seqnum;
                }
                else
                {
                    std::cout << "NOT ALIVE DATA RECEIVED" << std::endl;
                }
            }
        }
        else
        {
            std::cout << "DATA MESSAGE RECEIVED BEFORE COMMAND READY_TO_START" << std::endl;
        }
    }
}

void ThroughputSubscriber::DataSubListener::saveNumbers()
{
    saved_lastseqnum = lastseqnum;
    saved_lostsamples = lostsamples;
}

// *******************************************************************************************
// *********************************** COMMAND SUB LISTENER **********************************
// *******************************************************************************************
ThroughputSubscriber::CommandSubListener::CommandSubListener(ThroughputSubscriber& up)
    :m_up(up)
{
}

ThroughputSubscriber::CommandSubListener::~CommandSubListener()
{
}

void ThroughputSubscriber::CommandSubListener::onSubscriptionMatched(
    Subscriber* /*sub*/,
    MatchingInfo& match_info)
{
    std::unique_lock<std::mutex> lock(m_up.mutex_);
    if (match_info.status == MATCHED_MATCHING)
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

void ThroughputSubscriber::CommandSubListener::onNewDataMessage(Subscriber*)
{
}

// *******************************************************************************************
// ********************************** THROUGHPUT SUBSCRIBER **********************************
// *******************************************************************************************
void ThroughputSubscriber::processMessage()
{
    if (mp_commandsub->takeNextData((void*)&m_CommandSubListener.m_commandin, &m_CommandSubListener.info))
    {
        switch (m_CommandSubListener.m_commandin.m_command)
        {
            default: break;
            case (DEFAULT): break;
            case (BEGIN):
            {
                break;
            }
            case (READY_TO_START):
            {
                std::cout << "Command: READY_TO_START" << std::endl;
                m_datasize = m_CommandSubListener.m_commandin.m_size;
                m_demand = m_CommandSubListener.m_commandin.m_demand;
                if (dynamic_data)
                {
                    // Create basic builders
                    DynamicTypeBuilder_ptr struct_type_builder(
                        DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

                    // Add members to the struct.
                    struct_type_builder->add_member(
                        0,
                        "seqnum",
                        DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
                    struct_type_builder->add_member(
                        1,
                        "data",
                        DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                            DynamicTypeBuilderFactory::get_instance()->create_byte_type(),
                            m_datasize
                        ));
                    struct_type_builder->set_name("ThroughputType");

                    m_pDynType = struct_type_builder->build();
                    m_DynType.CleanDynamicType();
                    m_DynType.SetDynamicType(m_pDynType);

                    Domain::registerType(mp_par, &m_DynType);

                    m_DynData = DynamicDataFactory::get_instance()->create_data(m_pDynType);
                }
                else
                {
                    delete(throughput_t);
                    delete(throughputin);

                    throughput_t = new ThroughputDataType(m_datasize);
                    Domain::registerType(mp_par, throughput_t);
                    throughputin = new ThroughputType((uint16_t)m_datasize);
                }

                mp_datasub = Domain::createSubscriber(mp_par, subAttr, &m_DataSubListener);

                ThroughputCommandType command(BEGIN);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                m_DataSubListener.reset();
                mp_commandpubli->write(&command);

                std::cout << "Waiting for data discovery" << std::endl;
                std::unique_lock<std::mutex> data_disc_lock(dataMutex_);
                data_disc_cond_.wait(data_disc_lock, [&]()
                {
                    return data_disc_count_ > 0;
                });
                data_disc_lock.unlock();
                std::cout << "Discovery data complete" << std::endl;
                break;
            }
            case (TEST_STARTS):
            {
                t_start_ = std::chrono::steady_clock::now();
                std::cout << "Command: TEST_STARTS" << std::endl;
                break;
            }
            case (TEST_ENDS):
            {
                t_end_ = std::chrono::steady_clock::now();
                m_DataSubListener.saveNumbers();
                std::cout << "Command: TEST_ENDS" << std::endl;
                std::unique_lock<std::mutex> lock(mutex_);
                stop_count_ = 1;
                lock.unlock();
                if (dynamic_data)
                {
                    DynamicTypeBuilderFactory::delete_instance();
                    DynamicDataFactory::get_instance()->delete_data(m_DynData);
                }
                else
                {
                    delete(throughputin);
                    throughputin = nullptr;
                }
                subAttr = mp_datasub->getAttributes();
                break;
            }
            case (ALL_STOPS):
            {
                std::unique_lock<std::mutex> lock(mutex_);
                stop_count_ = 2;
                lock.unlock();
                std::cout << "Command: ALL_STOPS" << std::endl;
            }
        }
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

// *******************************************************************************************
// *********************************** COMMAND PUB LISTENER **********************************
// *******************************************************************************************
ThroughputSubscriber::CommandPubListener::CommandPubListener(ThroughputSubscriber& up)
    :m_up(up)
{
}

ThroughputSubscriber::CommandPubListener::~CommandPubListener()
{
}

void ThroughputSubscriber::CommandPubListener::onPublicationMatched(
        Publisher* /*pub*/,
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

ThroughputSubscriber::~ThroughputSubscriber()
{
    Domain::stopAll();
}

ThroughputSubscriber::ThroughputSubscriber(
        bool reliable,
        uint32_t pid,
        bool hostname,
        const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
        const std::string& sXMLConfigFile,
        bool dynamic_types,
        int forced_domain)
    : disc_count_(0)
    , data_disc_count_(0)
    , stop_count_(0)
#pragma warning(disable:4355)
    , m_DataSubListener(*this)
    , m_CommandSubListener(*this)
    , m_CommandPubListener(*this)
    , ready(true)
    , m_datasize(0)
    , m_demand(0)
    , m_sXMLConfigFile(sXMLConfigFile)
    , dynamic_data(dynamic_types)
    , m_forced_domain(forced_domain)
    , throughputin(nullptr)
{
    // Dummy type registration
    if (dynamic_data)
    {
        // Create basic builders
        DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

        // Add members to the struct.
        struct_type_builder->add_member(
            0,
            "seqnum",
            DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
        struct_type_builder->add_member(
            1,
            "data",
            DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                DynamicTypeBuilderFactory::get_instance()->create_byte_type(),
                LENGTH_UNLIMITED
            ));
        struct_type_builder->set_name("ThroughputType");

        m_pDynType = struct_type_builder->build();
        m_DynType.SetDynamicType(m_pDynType);
    }

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
    PParam.rtps.setName("Participant_subscriber");
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

    //REGISTER THE COMMAND TYPE
    throughput_t = nullptr;
    Domain::registerType(mp_par, (TopicDataType*)&throuputcommand_t);

    std::string profile_name = "subscriber_profile";
    SubscriberAttributes Sparam;
    Sparam.topic.topicDataType = "ThroughputType";
    Sparam.topic.topicKind = NO_KEY;
    std::ostringstream st;
    st << "ThroughputTest_";
    if (hostname)
    {
        st << asio::ip::host_name() << "_";
    }
    st << pid << "_UP";
    Sparam.topic.topicName = st.str();
    if (reliable)
    {
        //RELIABLE
        Sparam.times.heartbeatResponseDelay = TimeConv::MilliSeconds2Time_t(0).to_duration_t();
        Sparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        //BEST EFFORT
        Sparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    }
    Sparam.properties = property_policy;

    if (m_sXMLConfigFile.length() > 0)
    {
        if (xmlparser::XMLP_ret::XML_ERROR
                == xmlparser::XMLProfileManager::fillSubscriberAttributes(profile_name, subAttr))
        {
            std::cout << "Cannot read subscriber profile " << profile_name << std::endl;
        }
    }
    else
    {
        subAttr = Sparam;
    }
    mp_datasub = nullptr;

    //COMMAND
    PublisherAttributes Wparam;
    Wparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Wparam.topic.topicDataType = "ThroughputCommand";
    Wparam.topic.topicKind = NO_KEY;
    std::ostringstream pct;
    pct << "ThroughputTest_Command_";
    if (hostname)
    {
        pct << asio::ip::host_name() << "_";
    }
    pct << pid << "_SUB2PUB";
    Wparam.topic.topicName = pct.str();
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Wparam.qos.m_publishMode.kind = SYNCHRONOUS_PUBLISH_MODE;

    mp_commandpubli = Domain::createPublisher(mp_par, Wparam, (PublisherListener*)&this->m_CommandPubListener);

    SubscriberAttributes Rparam;
    Rparam.topic.topicDataType = "ThroughputCommand";
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicName = "ThroughputCommandP2S";
    std::ostringstream sct;
    sct << "ThroughputTest_Command_";
    if (hostname)
    {
        sct << asio::ip::host_name() << "_";
    }
    sct << pid << "_PUB2SUB";
    Rparam.topic.topicName = sct.str();
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;

    mp_commandsub = Domain::createSubscriber(mp_par, Rparam, (SubscriberListener*)&this->m_CommandSubListener);

    t_start_ = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        t_end_ = std::chrono::steady_clock::now();
    }
    t_overhead_ = std::chrono::duration<double, std::micro>(t_end_ - t_start_) / 1001;
    std::cout << "Overhead " << t_overhead_.count() << std::endl;

    if (mp_commandsub == nullptr || mp_commandpubli == nullptr)
    {
        ready = false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void ThroughputSubscriber::run()
{
    if (!ready)
    {
        return;
    }
    std::cout << "Waiting for command discovery" << std::endl;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        disc_cond_.wait(lock, [&](){
            return disc_count_ >= 2;
        });
    }
    std::cout << "Discovery command complete" << std::endl;

    do
    {
        processMessage();

        if (stop_count_ == 1)
        {
            std::cout << "Waiting clean state" << std::endl;
            while (!mp_datasub->isInCleanState())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            std::cout << "Sending results" << std::endl;
            ThroughputCommandType comm;
            comm.m_command = TEST_RESULTS;
            comm.m_demand = m_demand;
            comm.m_size = m_datasize + 4 + 4;
            comm.m_lastrecsample = m_DataSubListener.saved_lastseqnum;
            comm.m_lostsamples = m_DataSubListener.saved_lostsamples;

            auto total_time_count =
                (std::chrono::duration<double, std::micro>(t_end_ - t_start_) - t_overhead_).count();

            if (total_time_count < std::numeric_limits<uint64_t>::min())
            {
                comm.m_totaltime = std::numeric_limits<uint64_t>::min();
            }
            else if (total_time_count > std::numeric_limits<uint64_t>::max())
            {
                comm.m_totaltime = std::numeric_limits<uint64_t>::max();
            }
            else
            {
                comm.m_totaltime = static_cast<uint64_t>(total_time_count);
            }

            std::cout << "Last Received Sample: " << comm.m_lastrecsample << std::endl;
            std::cout << "Lost Samples: " << comm.m_lostsamples << std::endl;
            std::cout << "Samples per second: "
                << (double)(comm.m_lastrecsample - comm.m_lostsamples) * 1000000 / comm.m_totaltime
                << std::endl;
            std::cout << "Test of size " << comm.m_size << " and demand " << comm.m_demand << " ends." << std::endl;
            mp_commandpubli->write(&comm);

            stop_count_ = 0;

            Domain::removeSubscriber(mp_datasub);
            mp_datasub = nullptr;
            Domain::unregisterType(mp_par, "ThroughputType");

            if (!dynamic_data)
            {
                delete throughput_t;
                throughput_t = nullptr;
            }
        }
    } while (stop_count_ != 2);

    return;
}
