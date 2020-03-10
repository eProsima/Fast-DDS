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

#include "LatencyTestSubscriber.hpp"
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;


uint32_t datassub[] = {12, 28, 60, 124, 252, 508, 1020, 2044, 4092, 8188, 16380};
uint32_t datassub_large[] = {63996, 131068};

std::vector<uint32_t> data_size_sub;

LatencyTestSubscriber::LatencyTestSubscriber()
    : participant_(nullptr)
    , data_publisher_(nullptr)
    , command_publisher_(nullptr)
    , data_subscriber_(nullptr)
    , command_subscriber_(nullptr)
    , received_(0)
    , discovery_count_(0)
    , command_msg_count_(0)
    , test_status_(0)
    , echo_(true)
    , samples_(0)
    , latency_type_(nullptr)
    , dynamic_data_type_(nullptr)
    , data_pub_listener_(nullptr)
    , data_sub_listener_(nullptr)
    , command_pub_listener_(nullptr)
    , command_sub_listener_(nullptr)
{
    forced_domain_ = -1;
    data_pub_listener_.latency_publisher_ = this;
    data_sub_listener_.latency_publisher_ = this;
    command_pub_listener_.latency_publisher_ = this;
    command_sub_listener_.latency_publisher_ = this;
}

LatencyTestSubscriber::~LatencyTestSubscriber()
{
    Domain::removeParticipant(participant_);
}

bool LatencyTestSubscriber::init(
        bool echo,
        int samples,
        bool reliable,
        uint32_t pid,
        bool hostname,
        const PropertyPolicy& part_property_policy,
        const PropertyPolicy& property_policy,
        bool large_data,
        const std::string& xml_config_file,
        bool dynamic_data,
        int forced_domain)
{
    // Payloads for which the test runs
    if (!large_data)
    {
        data_size_sub.assign(datassub, datassub + sizeof(datassub) / sizeof(uint32_t) );
    }
    else
    {
        data_size_sub.assign(datassub_large, datassub_large + sizeof(datassub_large) / sizeof(uint32_t) );
    }

    xml_config_file_ = xml_config_file;
    echo_ = echo;
    samples_ = samples;
    dynamic_data_ = dynamic_data;
    forced_domain_ = forced_domain;

    // Init dynamic data
    if (dynamic_data_)
    {
        // Create basic builders
        DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

        // Add members to the struct.
        struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
        struct_type_builder->add_member(1, "data",
                DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                    DynamicTypeBuilderFactory::get_instance()->create_byte_type(), data_size_sub.back()
                    ));
        struct_type_builder->set_name("LatencyType");

        dynamic_type_ = struct_type_builder->build();
        dynamic_data_pub_sub_type_.SetDynamicType(dynamic_type_);
    }

    /* Create RTPSParticipant */
    std::string participant_profile_name = "sub_participant_profile";
    ParticipantAttributes participant_attributes;

    // Default domain
    participant_attributes.rtps.builtin.domainId = pid % 230;

    // Default participant name
    participant_attributes.rtps.setName("latency_test_subscriber");

    participant_attributes.rtps.properties = part_property_policy;

    // Load XML configuration
    if (xml_config_file_.length() > 0)
    {
        if (eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK !=
                eprosima::fastrtps::xmlparser::XMLProfileManager::fillParticipantAttributes(
                    participant_profile_name, participant_attributes))
        {
            return false;
        }
    }

    // Apply user's force domain
    if (forced_domain_ >= 0)
    {
        participant_attributes.rtps.builtin.domainId = forced_domain_;
    }

    // If the user has specified a participant property policy with command line arguments, it overrides whatever the
    // XML configures.
    if (PropertyPolicyHelper::length(part_property_policy) > 0)
    {
        participant_attributes.rtps.properties = part_property_policy;
    }

    // Create the participant
    participant_ = Domain::createParticipant(participant_attributes);
    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the data type
    if (dynamic_data_)
    {
        Domain::registerType(participant_, &dynamic_data_pub_sub_type_);
    }
    else
    {
        Domain::registerType(participant_, (TopicDataType*)&latency_data_type_);
    }

    // Register the command type
    Domain::registerType(participant_, (TopicDataType*)&latency_command_type_);

    /* Create Data Echo Publisher */
    std::string profile_name = "sub_publisher_profile";
    PublisherAttributes publisher_data_attributes;
    publisher_data_attributes.topic.topicDataType = "LatencyType";
    publisher_data_attributes.topic.topicKind = NO_KEY;
    std::ostringstream data_pub_topic_name;
    data_pub_topic_name << "LatencyTest_";
    if (hostname)
    {
        data_pub_topic_name << asio::ip::host_name() << "_";
    }
    data_pub_topic_name << pid << "_SUB2PUB";
    publisher_data_attributes.topic.topicName = data_pub_topic_name.str();
    publisher_data_attributes.times.heartbeatPeriod.seconds = 0;
    publisher_data_attributes.times.heartbeatPeriod.nanosec = 100000000;

    if (!reliable)
    {
        publisher_data_attributes.qos.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
    }
    publisher_data_attributes.properties = property_policy;

    if (large_data)
    {
        publisher_data_attributes.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        publisher_data_attributes.qos.m_publishMode.kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
    }

    if (xml_config_file_.length() > 0)
    {
        data_publisher_ = Domain::createPublisher(participant_, profile_name,
                (PublisherListener*)&this->data_pub_listener_);
    }
    else
    {
        data_publisher_ = Domain::createPublisher(participant_, publisher_data_attributes,
                (PublisherListener*)&this->data_pub_listener_);
    }

    if (data_publisher_ == nullptr)
    {
        return false;
    }

    /* Create Data Subscriber */
    profile_name = "sub_subscriber_profile";
    SubscriberAttributes subscriber_data_attributes;
    subscriber_data_attributes.topic.topicDataType = "LatencyType";
    subscriber_data_attributes.topic.topicKind = NO_KEY;
    std::ostringstream data_sub_topic_name;
    data_sub_topic_name << "LatencyTest_";
    if (hostname)
    {
        data_sub_topic_name << asio::ip::host_name() << "_";
    }
    data_sub_topic_name << pid << "_PUB2SUB";
    subscriber_data_attributes.topic.topicName = data_sub_topic_name.str();

    if (reliable)
    {
        subscriber_data_attributes.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    }
    subscriber_data_attributes.properties = property_policy;

    if (large_data)
    {
        subscriber_data_attributes.historyMemoryPolicy =
                eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    }

    if (xml_config_file_.length() > 0)
    {
        data_subscriber_ = Domain::createSubscriber(participant_, profile_name, &this->data_sub_listener_);
    }
    else
    {
        data_subscriber_ = Domain::createSubscriber(participant_, subscriber_data_attributes,
                &this->data_sub_listener_);
    }

    if (data_subscriber_ == nullptr)
    {
        return false;
    }

    /* Create Command Publisher */
    PublisherAttributes publisher_command_attributes;
    publisher_command_attributes.topic.topicDataType = "TestCommandType";
    publisher_command_attributes.topic.topicKind = NO_KEY;
    std::ostringstream command_pub_topic_name;
    command_pub_topic_name << "LatencyTest_Command_";
    if (hostname)
    {
        command_pub_topic_name << asio::ip::host_name() << "_";
    }
    command_pub_topic_name << pid << "_SUB2PUB";
    publisher_command_attributes.topic.topicName = command_pub_topic_name.str();
    publisher_command_attributes.topic.historyQos.kind =  eprosima::fastrtps::KEEP_ALL_HISTORY_QOS;
    publisher_command_attributes.qos.m_durability.kind =  eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS;
    publisher_command_attributes.qos.m_reliability.kind =  eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
    publisher_command_attributes.qos.m_publishMode.kind = eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE;

    command_publisher_ = Domain::createPublisher(participant_, publisher_command_attributes,
            &this->command_pub_listener_);

    if (command_publisher_ == nullptr)
    {
        return false;
    }

    /* Create Command Subscriber */
    SubscriberAttributes subscriber_command_attributes;
    subscriber_command_attributes.topic.topicDataType = "TestCommandType";
    subscriber_command_attributes.topic.topicKind = NO_KEY;
    std::ostringstream command_sub_topic_name;
    command_sub_topic_name << "LatencyTest_Command_";
    if (hostname)
    {
        command_sub_topic_name << asio::ip::host_name() << "_";
    }
    command_sub_topic_name << pid << "_PUB2SUB";
    subscriber_command_attributes.topic.topicName = command_sub_topic_name.str();
    subscriber_command_attributes.topic.historyQos.kind = eprosima::fastrtps::KEEP_ALL_HISTORY_QOS;
    subscriber_command_attributes.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
    subscriber_command_attributes.qos.m_durability.kind = eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS;

    command_subscriber_ = Domain::createSubscriber(participant_, subscriber_command_attributes,
            &this->command_sub_listener_);

    if (command_subscriber_ == nullptr)
    {
        return false;
    }
    return true;
}

void LatencyTestSubscriber::DataPubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    if (info.status == MATCHED_MATCHING)
    {
        logInfo(LatencyTest, "Data Pub Matched");
        ++latency_publisher_->discovery_count_;
    }
    else
    {
        --latency_publisher_->discovery_count_;
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestSubscriber::DataSubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    if (info.status == MATCHED_MATCHING)
    {
        logInfo(LatencyTest, "Data Sub Matched");
        ++latency_publisher_->discovery_count_;
    }
    else
    {
        --latency_publisher_->discovery_count_;
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestSubscriber::CommandPubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    if (info.status == MATCHED_MATCHING)
    {
        logInfo(LatencyTest, "Command Pub Matched");
        ++latency_publisher_->discovery_count_;
    }
    else
    {
        --latency_publisher_->discovery_count_;
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestSubscriber::CommandSubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    if (info.status == MATCHED_MATCHING)
    {
        logInfo(LatencyTest, "Command Sub Matched");
        ++latency_publisher_->discovery_count_;
    }
    else
    {
        --latency_publisher_->discovery_count_;
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestSubscriber::CommandSubListener::onNewDataMessage(
        Subscriber* subscriber)
{
    TestCommandType command;
    if (subscriber->takeNextData(&command, &latency_publisher_->sample_info_))
    {
        std::cout << "RCOMMAND: " << command.m_command << std::endl;
        if (command.m_command == READY)
        {
            std::cout << "Publisher has new test ready..." << std::endl;
            latency_publisher_->mutex_.lock();
            ++latency_publisher_->command_msg_count_;
            latency_publisher_->mutex_.unlock();
            latency_publisher_->command_msg_cv_.notify_one();
        }
        else if (command.m_command == STOP)
        {
            std::cout << "Publisher has stopped the test" << std::endl;
            latency_publisher_->mutex_.lock();
            ++latency_publisher_->command_msg_count_;
            latency_publisher_->mutex_.unlock();
            latency_publisher_->command_msg_cv_.notify_one();
        }
        else if (command.m_command == STOP_ERROR)
        {
            std::cout << "Publisher has canceled the test" << std::endl;
            latency_publisher_->test_status_ = -1;
            latency_publisher_->mutex_.lock();
            ++latency_publisher_->command_msg_count_;
            latency_publisher_->mutex_.unlock();
            latency_publisher_->command_msg_cv_.notify_one();
        }
        else if (command.m_command == DEFAULT)
        {
            std::cout << "Something is wrong" << std::endl;
        }
    }
}

void LatencyTestSubscriber::DataSubListener::onNewDataMessage(
        Subscriber* subscriber)
{
    if (latency_publisher_->dynamic_data_)
    {
        subscriber->takeNextData((void*)latency_publisher_->dynamic_data_type_, &latency_publisher_->sample_info_);
        if (latency_publisher_->echo_)
        {
            latency_publisher_->data_publisher_->write((void*)latency_publisher_->dynamic_data_type_);
        }
    }
    else
    {
        subscriber->takeNextData((void*)latency_publisher_->latency_type_, &latency_publisher_->sample_info_);
        if (latency_publisher_->echo_)
        {
            latency_publisher_->data_publisher_->write((void*)latency_publisher_->latency_type_);
        }
    }
}

void LatencyTestSubscriber::run()
{
    // WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    // EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    while (discovery_count_ != 4)
    {
        discovery_cv_.wait(disc_lock);
    }
    disc_lock.unlock();

    std::cout << C_B_MAGENTA << "DISCOVERY COMPLETE " << C_DEF << std::endl;

    for (std::vector<uint32_t>::iterator payload = data_size_sub.begin(); payload != data_size_sub.end(); ++payload)
    {
        if (!this->test(*payload))
        {
            break;
        }
    }
}

bool LatencyTestSubscriber::test(
        uint32_t datasize)
{
    std::cout << "Preparing test with data size: " << datasize + 4 << std::endl;
    if (dynamic_data_)
    {
        dynamic_data_type_ = DynamicDataFactory::get_instance()->create_data(dynamic_type_);

        MemberId id;
        DynamicData* dyn_data = dynamic_data_type_->loan_value(dynamic_data_type_->get_member_id_at_index(1));
        for (uint32_t i = 0; i < datasize; ++i)
        {
            dyn_data->insert_sequence_data(id);
            dyn_data->set_byte_value(0, id);
        }
        dynamic_data_type_->return_loaned_value(dyn_data);
    }
    else
    {
        latency_type_ = new LatencyType(datasize);
    }

    std::unique_lock<std::mutex> lock(mutex_);
    if (command_msg_count_ == 0)
    {
        command_msg_cv_.wait(lock);
    }
    --command_msg_count_;
    lock.unlock();

    test_status_ = 0;
    received_ = 0;
    TestCommandType command;
    command.m_command = BEGIN;
    std::cout << "Testing with data size: " << datasize + 4 << std::endl;
    command_publisher_->write(&command);

    lock.lock();
    command_msg_cv_.wait(lock, [&]()
    {
        return command_msg_count_ > 0;
    });
    --command_msg_count_;
    lock.unlock();

    std::cout << "TEST OF SIZE: " << datasize + 4 << " ENDS" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    size_t removed;
    this->data_publisher_->removeAllChange(&removed);

    if (dynamic_data_)
    {
        DynamicDataFactory::get_instance()->delete_data(dynamic_data_type_);
    }
    else
    {
        delete(latency_type_);
    }

    if (test_status_ == -1)
    {
        return false;
    }
    return true;
}
