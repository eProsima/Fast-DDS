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


MemoryTestPublisher::MemoryTestPublisher()
    : mp_participant(nullptr)
    , mp_datapub(nullptr)
    , mp_commandpub(nullptr)
    , mp_commandsub(nullptr)
    , n_subscribers(0)
    , n_samples(0)
    , disc_count_(0)
    , comm_count_(0)
    , data_count_(0)
    , m_status(0)
    , n_received(0)
    , m_datapublistener(nullptr)
    , m_commandpublistener(nullptr)
    , m_commandsublistener(nullptr)
    , m_data_size(0)
    , dynamic_data(false)
    , mp_memory(nullptr)
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
        const PropertyPolicy& property_policy, const std::string& sXMLConfigFile,
        uint32_t data_size, bool dynamic_types)
{
    m_sXMLConfigFile = sXMLConfigFile;
    n_samples = n_sam;
    n_subscribers = n_sub;
    n_export_csv = export_csv;
    m_exportPrefix = export_prefix;
    reliable_ = reliable;
    m_data_size = data_size;
    dynamic_data = dynamic_types;

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
        struct_type_builder->set_name("MemoryType");

        m_pDynType = struct_type_builder->build();
        m_DynType.SetDynamicType(m_pDynType);
    }

    // Create RTPSParticipant
    std::string participant_profile_name = "participant_profile";
    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = pid % 230;
    PParam.rtps.properties = part_property_policy;
    PParam.rtps.setName("Participant_pub");

    if (m_sXMLConfigFile.length() > 0)
    {
        mp_participant = Domain::createParticipant(participant_profile_name);
    }
    else
    {
        mp_participant = Domain::createParticipant(PParam);
    }

    if (mp_participant == nullptr)
    {
        return false;
    }

    // Register the type
    if (dynamic_data)
    {
        Domain::registerType(mp_participant, &m_DynType);
    }
    else
    {
        Domain::registerType(mp_participant, (TopicDataType*)&memory_t);
    }
    Domain::registerType(mp_participant, (TopicDataType*)&command_t);

    // Create Sending Publisher
    std::string profile_name = "publisher_profile";
    PublisherAttributes PubDataparam;
    PubDataparam.topic.topicDataType = "MemoryType";
    PubDataparam.topic.topicKind = NO_KEY;
    std::ostringstream pt;
    pt << "MemoryTest_";
    if (hostname)
        pt << asio::ip::host_name() << "_";
    pt << pid << "_PUB2SUB";
    PubDataparam.topic.topicName = pt.str();
    if (!reliable)
    {
        PubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    }
    PubDataparam.properties = property_policy;
    if (m_data_size > 60000)
    {
        PubDataparam.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        PubDataparam.qos.m_publishMode.kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
    }

    if (m_sXMLConfigFile.length() > 0)
    {
        mp_datapub = Domain::createPublisher(mp_participant, profile_name, (PublisherListener*)&this->m_datapublistener);
    }
    else
    {
        mp_datapub = Domain::createPublisher(mp_participant, PubDataparam, (PublisherListener*)&this->m_datapublistener);
    }

    if (mp_datapub == nullptr)
    {
        return false;
    }
    std::cout << "Publisher created" << std::endl;

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
    PubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    PubCommandParam.qos.m_publishMode.kind = eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE;

    mp_commandpub = Domain::createPublisher(mp_participant, PubCommandParam, &this->m_commandpublistener);

    if (mp_commandpub == nullptr)
    {
        return false;
    }

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
    {
        return false;
    }

    if (dynamic_data)
    {
        DynamicTypeBuilderFactory::delete_instance();
        pubAttr = mp_datapub->getAttributes();
        Domain::removePublisher(mp_datapub);
        Domain::unregisterType(mp_participant, "MemoryType"); // Unregister as we will register it later with correct size
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
        cout << C_MAGENTA << "Data Pub Unmatched "<<C_DEF<<endl;
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
        cout << C_MAGENTA << "Command Pub unmatched "<<C_DEF<<endl;
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
        cout << C_MAGENTA << "Command Sub unmatched "<<C_DEF<<endl;
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

void MemoryTestPublisher::run(uint32_t test_time)
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 3 Matchings (Comm pub+sub and publisher or subscriber)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&](){
        return disc_count_ >= (n_subscribers * 3);
    });
    disc_lock.unlock();

    test(test_time, m_data_size);
    eClock::my_sleep(100);

    cout << "REMOVING PUBLISHER"<<endl;
    Domain::removePublisher(this->mp_commandpub);
    cout << "REMOVING SUBSCRIBER"<<endl;
    Domain::removeSubscriber(mp_commandsub);
}

bool MemoryTestPublisher::test(uint32_t test_time, uint32_t datasize)
{
    //cout << "Beginning test of size: "<<datasize+4 <<endl;
    m_status = 0;
    n_received = 0;

    if (dynamic_data)
    {
        // Create basic builders
        DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

        // Add members to the struct.
        struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
        struct_type_builder->add_member(1, "data",
            DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                DynamicTypeBuilderFactory::get_instance()->create_byte_type(), datasize
            ));
        struct_type_builder->set_name("MemoryType");

        m_pDynType = struct_type_builder->build();
        m_DynType.CleanDynamicType();
        m_DynType.SetDynamicType(m_pDynType);

        Domain::registerType(mp_participant, &m_DynType);

        mp_datapub = Domain::createPublisher(mp_participant, pubAttr, &m_datapublistener);

        m_DynData = DynamicDataFactory::get_instance()->create_data(m_pDynType);

        MemberId id;
        DynamicData *my_data = m_DynData->loan_value(m_DynData->GetMemberIdAtIndex(1));
        for (uint32_t i = 0; i < datasize; ++i)
        {
            my_data->insert_sequence_data(id);
            my_data->set_byte_value(0, id);
        }
        m_DynData->return_loaned_value(my_data);
    }
    else
    {
        mp_memory = new MemoryType(datasize);
    }
    std::chrono::duration<double, std::micro> test_time_us = std::chrono::seconds(test_time);
    auto t_end_ = std::chrono::steady_clock::now();

    // Finally Data matching
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&](){
        return disc_count_ >= (n_subscribers * 3);
    });
    disc_lock.unlock();
    cout << C_B_MAGENTA << "DISCOVERY COMPLETE "<<C_DEF<<endl;

    TestCommandType command;
    command.m_command = READY;
    mp_commandpub->write(&command);

    //cout << "WAITING FOR COMMAND RESPONSES "<<endl;;
    std::unique_lock<std::mutex> lock(mutex_);
    comm_cond_.wait(lock, [&](){
        return comm_count_ == n_subscribers;
    });
    --comm_count_;
    lock.unlock();
    //cout << endl;
    //BEGIN THE TEST:

    auto t_start_ = std::chrono::steady_clock::now();

    while (std::chrono::duration<double, std::micro>(t_end_ - t_start_) < test_time_us)
    {
        for(unsigned int count = 1; count <= n_samples; ++count)
        {
            if (dynamic_data)
            {
                m_DynData->set_uint32_value(count, 0);
                mp_datapub->write((void*)m_DynData);
            }
            else
            {
                mp_memory->seqnum = count;
                mp_datapub->write((void*)mp_memory);
            }
        }
        t_end_ = std::chrono::steady_clock::now();
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

    if (dynamic_data)
    {
        DynamicTypeBuilderFactory::delete_instance();
        DynamicDataFactory::get_instance()->delete_data(m_DynData);
        pubAttr = mp_datapub->getAttributes();
        Domain::removePublisher(mp_datapub);
        Domain::unregisterType(mp_participant, "MemoryType");
    }
    else
    {
        delete(mp_memory);
    }

    return true;
}
