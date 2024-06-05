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

using namespace eprosima::fastdds::dds;

MemoryTestSubscriber::MemoryTestSubscriber()
{
    m_datasublistener.up_ = this;
    m_commandpublistener.up_ = this;
    m_commandsublistener.up_ = this;
}

MemoryTestSubscriber::~MemoryTestSubscriber()
{
    participant_->delete_contained_entities();
}

bool MemoryTestSubscriber::init(
        bool echo,
        int nsam,
        bool reliable,
        uint32_t pid,
        bool hostname,
        const eprosima::fastdds::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastdds::rtps::PropertyPolicy& property_policy,
        const std::string& sXMLConfigFile,
        uint32_t data_size,
        bool dynamic_types)
{
    m_sXMLConfigFile = sXMLConfigFile;
    m_echo = echo;
    n_samples = nsam;
    m_data_size = data_size;
    dynamic_data_ = dynamic_types;

    if (dynamic_data_) // Dummy type registration
    {
        // Create basic builders
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
    participant_qos.name("MemoryTest_Participant_Subscriber");
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

    // Create Data Subscriber
    std::string profile_name = "subscriber_profile";
    DataReaderQos reader_qos;
    if (reliable)
    {
        reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    }
    reader_qos.properties(property_policy);

    if (m_sXMLConfigFile.length() > 0)
    {
        data_reader_ = subscriber_->create_datareader_with_profile(data_topic_, profile_name, &this->m_datasublistener);
    }
    else
    {
        data_reader_ = subscriber_->create_datareader(data_topic_, reader_qos, &this->m_datasublistener);
    }

    if (nullptr == data_reader_)
    {
        return false;
    }

    //COMMAND PUBLISHER
    std::ostringstream pct;
    pct << "MemoryTest_Command_";
    if (hostname)
    {
        pct << asio::ip::host_name() << "_";
    }
    pct << pid << "_SUB2PUB";
    command_pub_topic_ = participant_->create_topic(pct.str(), "TestCommandType", TOPIC_QOS_DEFAULT);

    if (nullptr == command_pub_topic_)
    {
        return false;
    }

    DataWriterQos writer_qos;
    writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    command_writer_ = publisher_->create_datawriter(command_pub_topic_, writer_qos, &this->m_commandpublistener);

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
    sct << pid << "_PUB2SUB";
    command_sub_topic_ = participant_->create_topic(sct.str(), "TestCommandType", TOPIC_QOS_DEFAULT);

    if (nullptr == command_sub_topic_)
    {
        return false;
    }

    DataReaderQos command_reader_qos;
    command_reader_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    command_reader_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    command_reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    command_reader_ = subscriber_->create_datareader(command_sub_topic_, command_reader_qos,
                    &this->m_commandsublistener);

    if (nullptr == command_reader_)
    {
        return false;
    }

    return true;
}

void MemoryTestSubscriber::DataSubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(up_->mutex_);

    if (0 < info.current_count_change)
    {
        std::cout << C_MAGENTA << "Data Sub Matched " << C_DEF << std::endl;
    }
    else
    {
        std::cout << C_MAGENTA << "Data Sub unmatched " << C_DEF << std::endl;
    }
    up_->disc_count_ += info.current_count_change;

    lock.unlock();
    up_->disc_cond_.notify_one();
}

void MemoryTestSubscriber::CommandPubListener::on_publication_matched(
        DataWriter*,
        const PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(up_->mutex_);

    if (0 < info.current_count_change)
    {
        std::cout << C_MAGENTA << "Command Pub Matched " << C_DEF << std::endl;
    }
    else
    {
        std::cout << C_MAGENTA << "Command Pub unmatched " << C_DEF << std::endl;
    }
    up_->disc_count_ += info.current_count_change;

    lock.unlock();
    up_->disc_cond_.notify_one();
}

void MemoryTestSubscriber::CommandSubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(up_->mutex_);

    if (0 < info.current_count_change)
    {
        std::cout << C_MAGENTA << "Command Sub Matched " << C_DEF << std::endl;
    }
    else
    {
        std::cout << C_MAGENTA << "Command Sub unmatched " << C_DEF << std::endl;
    }
    up_->disc_count_ += info.current_count_change;

    lock.unlock();
    up_->disc_cond_.notify_one();
}

void MemoryTestSubscriber::CommandSubListener::on_data_available(
        DataReader* reader)
{
    TestCommandType command;
    if (RETCODE_OK == reader->take_next_sample(&command, &up_->m_sampleinfo))
    {
        std::cout << "RCOMMAND: " << command.m_command << std::endl;
        if (command.m_command == READY)
        {
            std::cout << "Publisher has new test ready..." << std::endl;
            up_->mutex_.lock();
            ++up_->comm_count_;
            up_->mutex_.unlock();
            up_->comm_cond_.notify_one();
        }
        else if (command.m_command == STOP)
        {
            std::cout << "Publisher has stopped the test" << std::endl;
            up_->mutex_.lock();
            ++up_->data_count_;
            up_->mutex_.unlock();
            up_->data_cond_.notify_one();
        }
        else if (command.m_command == STOP_ERROR)
        {
            std::cout << "Publisher has canceled the test" << std::endl;
            up_->m_status = -1;
            up_->mutex_.lock();
            ++up_->data_count_;
            up_->mutex_.unlock();
            up_->data_cond_.notify_one();
        }
        else if (command.m_command == DEFAULT)
        {
            std::cout << "Something is wrong" << std::endl;
        }
    }
}

void MemoryTestSubscriber::DataSubListener::on_data_available(
        DataReader* reader)
{
    if (up_->dynamic_data_)
    {
        reader->take_next_sample(&up_->m_DynData, &up_->m_sampleinfo);
        ++up_->n_received;
        if (up_->m_echo)
        {
            uint32_t seq_num {0};
            up_->m_DynData->get_uint32_value(seq_num, 0);
            std::cout << "Received data: " << seq_num << "(" << up_->n_received << ")" << std::endl;
        }
    }
    else
    {
        reader->take_next_sample((void*)up_->memory_, &up_->m_sampleinfo);
        ++up_->n_received;
        if (up_->m_echo)
        {
            std::cout << "Received data: " << up_->memory_->seqnum << "(" << up_->n_received << ")" << std::endl;
        }
    }
}

void MemoryTestSubscriber::run()
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 3 Matchings (Comd Pub+Sub and publisher or subscriber)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&]()
            {
                return disc_count_ >= 3;
            });
    disc_lock.unlock();

    test(m_data_size);
}

bool MemoryTestSubscriber::test(
        uint32_t datasize)
{
    std::cout << "Preparing test with data size: " << datasize + 4 << std::endl;

    if (dynamic_data_)
    {
        m_DynData = DynamicDataFactory::get_instance()->create_data(m_pDynType);
    }
    else
    {
        memory_ = new MemoryType(datasize);
    }

    // Finally data matching
    std::unique_lock<std::mutex> disc_lock(mutex_);
    disc_cond_.wait(disc_lock, [&]()
            {
                return disc_count_ >= 3;
            });
    disc_lock.unlock();

    std::cout << C_B_MAGENTA << "DISCOVERY COMPLETE " << C_DEF << std::endl;

    std::unique_lock<std::mutex> lock(mutex_);
    if (comm_count_ == 0)
    {
        comm_cond_.wait(lock);
    }
    --comm_count_;
    lock.unlock();

    m_status = 0;
    n_received = 0;
    TestCommandType command;
    command.m_command = BEGIN;
    std::cout << "Testing with data size: " << datasize + 4 << std::endl;
    command_writer_->write(&command);

    lock.lock();
    data_cond_.wait(lock, [&]()
            {
                return data_count_ > 0;
            });
    --data_count_;
    lock.unlock();

    std::cout << "TEST OF SIZE: " << datasize + 4 << " ENDS" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if (dynamic_data_)
    {
        DynamicDataFactory::get_instance()->delete_data(m_DynData);
    }
    else
    {
        delete(memory_);
    }
    if (m_status == -1)
    {
        return false;
    }
    return true;
}
