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

#include <cmath>
#include <cstdint>
#include <fstream>
#include <numeric>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

#ifndef _WIN32
#define localtime_s(X, Y) localtime_r(Y, X)
#endif // ifndef _WIN32

#define TIME_LIMIT_US 10000

using namespace eprosima::fastdds::dds;

MemoryTestPublisher::MemoryTestPublisher()
{
    m_datapublistener.up_ = this;
    m_commandpublistener.up_ = this;
    m_commandsublistener.up_ = this;
}

MemoryTestPublisher::~MemoryTestPublisher()
{
    participant_->delete_contained_entities();
}

bool MemoryTestPublisher::init(
        int n_sub,
        int n_sam,
        bool reliable,
        uint32_t pid,
        bool hostname,
        bool export_csv,
        const std::string& export_prefix,
        const eprosima::fastdds::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastdds::rtps::PropertyPolicy& property_policy,
        const std::string& sXMLConfigFile,
        uint32_t data_size,
        bool dynamic_types)
{
    m_sXMLConfigFile = sXMLConfigFile;
    n_samples = n_sam;
    n_subscribers = n_sub;
    n_export_csv = export_csv;
    m_exportPrefix = export_prefix;
    reliable_ = reliable;
    m_data_size = data_size;
    dynamic_data_ = dynamic_types;

    if (dynamic_data_) // Dummy type registration
    {
        DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};

        // Create basic builders
        TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
        type_descriptor->kind(TK_STRUCTURE);
        type_descriptor->name("MemoryType");
        DynamicTypeBuilder::_ref_type struct_type_builder(factory->create_type(type_descriptor));

        // Add members to the struct.
        MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
        member_descriptor->type(factory->get_primitive_type(eprosima::fastdds::dds::TK_UINT32));
        member_descriptor->name("seqnum");
        struct_type_builder->add_member(member_descriptor);
        member_descriptor = traits<MemberDescriptor>::make_shared();
        member_descriptor->type(factory->create_sequence_type(factory->get_primitive_type(eprosima::fastdds::dds::
                        TK_BYTE), static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
        member_descriptor->name("data");
        struct_type_builder->add_member(member_descriptor);

        m_pDynType = struct_type_builder->build();
    }

    // Create RTPSParticipant
    std::string participant_profile_name = "participant_profile";
    DomainParticipantQos participant_qos;
    participant_qos.name("MemoryTest_Participant_Publisher");
    participant_qos.properties(part_property_policy);

    if (m_sXMLConfigFile.length() > 0)
    {
        participant_ = DomainParticipantFactory::get_instance()->create_participant_with_profile(pid % 230,
                        participant_profile_name);
    }
    else
    {
        participant_ = DomainParticipantFactory::get_instance()->create_participant(pid % 230, participant_qos);
    }

    if (nullptr == participant_)
    {
        return false;
    }

    // Register the type
    if (dynamic_data_)
    {
        dynamic_type_support_ = new DynamicPubSubType(m_pDynType);
        data_type_ = TypeSupport{dynamic_type_support_};
    }
    else
    {
        data_type_ = TypeSupport{new MemoryDataType()};
    }
    participant_->register_type(data_type_);
    participant_->register_type(command_type_);

    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (nullptr == publisher_)
    {
        return false;
    }

    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (nullptr == subscriber_)
    {
        return false;
    }

    std::ostringstream st;
    st << "MemoryTest_";
    if (hostname)
    {
        st << asio::ip::host_name() << "_";
    }
    st << pid << "_PUB2SUB";
    data_topic_ = participant_->create_topic(st.str(), "MemoryType", TOPIC_QOS_DEFAULT);

    if (nullptr == data_topic_)
    {
        return false;
    }

    // Create Sending Publisher
    std::string profile_name = "publisher_profile";
    DataWriterQos writer_qos;
    if (!reliable)
    {
        writer_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    }
    writer_qos.properties(property_policy);

    if (m_data_size > 60000)
    {
        writer_qos.endpoint().history_memory_policy =
                eprosima::fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        writer_qos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    }

    if (m_sXMLConfigFile.length() > 0)
    {
        data_writer_ = publisher_->create_datawriter_with_profile(data_topic_, profile_name, &this->m_datapublistener);
    }
    else
    {
        data_writer_ = publisher_->create_datawriter(data_topic_, writer_qos, &this->m_datapublistener);
    }

    if (nullptr == data_writer_)
    {
        return false;
    }

    std::cout << "Publisher created" << std::endl;

    //COMMAND PUBLISHER
    std::ostringstream pct;
    pct << "MemoryTest_Command_";
    if (hostname)
    {
        pct << asio::ip::host_name() << "_";
    }
    pct << pid << "_PUB2SUB";
    command_pub_topic_ = participant_->create_topic(pct.str(), "TestCommandType", TOPIC_QOS_DEFAULT);

    if (nullptr == command_pub_topic_)
    {
        return false;
    }

    DataWriterQos command_writer_qos;
    command_writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    command_writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    command_writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    command_writer_ = publisher_->create_datawriter(command_pub_topic_, command_writer_qos,
                    &this->m_commandpublistener);

    if (nullptr == command_writer_)
    {
        return false;
    }

    std::ostringstream sct;
    sct << "MemoryTest_Command_";
    if (hostname)
    {
        sct << asio::ip::host_name() << "_";
    }
    sct << pid << "_SUB2PUB";
    command_sub_topic_ = participant_->create_topic(sct.str(), "TestCommandType", TOPIC_QOS_DEFAULT);

    if (nullptr == command_sub_topic_)
    {
        return false;
    }

    DataReaderQos reader_qos;
    reader_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    reader_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    command_reader_ = subscriber_->create_datareader(command_sub_topic_, reader_qos, &this->m_commandsublistener);

    if (nullptr == command_reader_)
    {
        return false;
    }

    return true;
}

void MemoryTestPublisher::DataPubListener::on_publication_matched(
        DataWriter*,
        const PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(up_->mutex_);

    if (0 < info.current_count_change)
    {
        std::cout << C_MAGENTA << "Data Pub Matched " << C_DEF << std::endl;

        n_matched = info.total_count;
        if (n_matched > up_->n_subscribers)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            up_->m_status = -1;
        }

    }
    else
    {
        std::cout << C_MAGENTA << "Data Pub Unmatched " << C_DEF << std::endl;
    }
    up_->disc_count_ += info.current_count_change;

    lock.unlock();
    up_->disc_cond_.notify_one();
}

void MemoryTestPublisher::CommandPubListener::on_publication_matched(
        DataWriter*,
        const PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(up_->mutex_);

    if (0 < info.current_count_change)
    {
        std::cout << C_MAGENTA << "Command Pub Matched " << C_DEF << std::endl;

        n_matched = info.total_count;
        if (n_matched > up_->n_subscribers)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            up_->m_status = -1;
        }
    }
    else
    {
        std::cout << C_MAGENTA << "Command Pub unmatched " << C_DEF << std::endl;
    }
    up_->disc_count_ += info.current_count_change;

    lock.unlock();
    up_->disc_cond_.notify_one();
}

void MemoryTestPublisher::CommandSubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(up_->mutex_);

    if (0 < info.current_count_change)
    {
        std::cout << C_MAGENTA << "Command Sub Matched " << C_DEF << std::endl;

        n_matched = info.total_count;
        if (n_matched > up_->n_subscribers)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            up_->m_status = -1;
        }
    }
    else
    {
        std::cout << C_MAGENTA << "Command Sub unmatched " << C_DEF << std::endl;
    }
    up_->disc_count_ += info.current_count_change;

    lock.unlock();
    up_->disc_cond_.notify_one();
}

void MemoryTestPublisher::CommandSubListener::on_data_available(
        DataReader* reader)
{
    TestCommandType command;
    SampleInfo info;

    if (RETCODE_OK == reader->take_next_sample((void*)&command, &info))
    {
        if (ALIVE_INSTANCE_STATE == info.instance_state)
        {
            if (BEGIN == command.m_command)
            {
                up_->mutex_.lock();
                ++up_->comm_count_;
                up_->mutex_.unlock();
                up_->comm_cond_.notify_one();
            }
        }
    }
    else
    {
        std::cout << "Problem reading" << std::endl;
    }
}

void MemoryTestPublisher::run(
        uint32_t test_time)
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 3 Matchings (Comm pub+sub and publisher or subscriber)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&]()
            {
                std::cout << "m = " << disc_count_ << " - su = " << n_subscribers << std::endl;
                return disc_count_ >= (n_subscribers * 3);
            });
    disc_lock.unlock();

    test(test_time, m_data_size);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

bool MemoryTestPublisher::test(
        uint32_t test_time,
        uint32_t datasize)
{
    m_status = 0;
    n_received = 0;

    if (dynamic_data_)
    {
        // Create basic builders
        m_DynData = DynamicDataFactory::get_instance()->create_data(m_pDynType);

        DynamicData::_ref_type my_data = m_DynData->loan_value(m_DynData->get_member_id_at_index(1));
        for (uint32_t i = 0; i < datasize; ++i)
        {
            my_data->set_byte_value(i, 0);
        }
        m_DynData->return_loaned_value(my_data);
    }
    else
    {
        memory_ = new MemoryType(datasize);
    }
    std::chrono::duration<double, std::micro> test_time_us = std::chrono::seconds(test_time);
    auto t_end_ = std::chrono::steady_clock::now();

    // Finally Data matching
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&]()
            {
                return disc_count_ >= (n_subscribers * 3);
            });
    disc_lock.unlock();
    std::cout << C_B_MAGENTA << "DISCOVERY COMPLETE " << C_DEF << std::endl;

    TestCommandType command;
    command.m_command = READY;
    command_writer_->write(&command);

    std::unique_lock<std::mutex> lock(mutex_);
    comm_cond_.wait(lock, [&]()
            {
                return comm_count_ == n_subscribers;
            });
    --comm_count_;
    lock.unlock();
    //BEGIN THE TEST:

    auto t_start_ = std::chrono::steady_clock::now();

    while (std::chrono::duration<double, std::micro>(t_end_ - t_start_) < test_time_us)
    {
        for (unsigned int count = 1; count <= n_samples; ++count)
        {
            if (dynamic_data_)
            {
                m_DynData->set_uint32_value(0, count);
                data_writer_->write(&m_DynData);
            }
            else
            {
                memory_->seqnum = count;
                data_writer_->write((void*)memory_);
            }
        }
        t_end_ = std::chrono::steady_clock::now();
    }

    command.m_command = STOP;
    command_writer_->write(&command);

    if (m_status != 0)
    {
        std::cout << "Error in test " << std::endl;
        return false;
    }
    //TEST FINISHED:
    size_t removed {0};
    data_writer_->clear_history(&removed);

    if (dynamic_data_)
    {
        DynamicDataFactory::get_instance()->delete_data(m_DynData);
    }
    else
    {
        delete(memory_);
    }

    return true;
}
