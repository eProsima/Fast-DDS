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

#include "ThroughputSubscriber.hpp"

#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/Domain.h>

#include <dds/core/LengthUnlimited.hpp>
#include <vector>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;


// *******************************************************************************************
// ************************************ DATA SUB LISTENER ************************************
// *******************************************************************************************
ThroughputSubscriber::DataSubListener::DataSubListener(
        ThroughputSubscriber& throughput_subscriber)
    : saved_last_seq_num_(0)
    , saved_lost_samples_(0)
    , throughput_subscriber_(throughput_subscriber)
    , last_seq_num_(0)
    , lost_samples_(0)
    , first_(true)
{
}

ThroughputSubscriber::DataSubListener::~DataSubListener()
{
}

void ThroughputSubscriber::DataSubListener::reset()
{
    last_seq_num_ = 0;
    first_ = true;
    lost_samples_ = 0;
}

void ThroughputSubscriber::DataSubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& match_info)
{
    std::unique_lock<std::mutex> lock(throughput_subscriber_.data_mutex_);
    if (match_info.status == MATCHED_MATCHING)
    {
        std::cout << C_RED << "Sub: DATA Sub Matched" << C_DEF << std::endl;
        ++throughput_subscriber_.data_discovery_count_;
    }
    else
    {
        std::cout << C_RED << "DATA SUBSCRIBER MATCHING REMOVAL" << C_DEF << std::endl;
        --throughput_subscriber_.data_discovery_count_;
    }
    lock.unlock();
    throughput_subscriber_.data_discovery_cv_.notify_one();
}

void ThroughputSubscriber::DataSubListener::onNewDataMessage(
        Subscriber* subscriber)
{
    if (throughput_subscriber_.dynamic_data_)
    {
        while (subscriber->takeNextData((void*)throughput_subscriber_.dynamic_data_type_, &info_))
        {
            if (info_.sampleKind == ALIVE)
            {
                if ((last_seq_num_ + 1) < throughput_subscriber_.dynamic_data_type_->get_uint32_value(0))
                {
                    lost_samples_ += throughput_subscriber_.dynamic_data_type_->get_uint32_value(0) - last_seq_num_ - 1;
                }
                last_seq_num_ = throughput_subscriber_.dynamic_data_type_->get_uint32_value(0);
            }
            else
            {
                std::cout << "NOT ALIVE DATA RECEIVED" << std::endl;
            }
        }
    }
    else
    {
        if (throughput_subscriber_.throughput_type_ != nullptr)
        {
            while (subscriber->takeNextData((void*)throughput_subscriber_.throughput_type_, &info_))
            {
                if (info_.sampleKind == ALIVE)
                {
                    if ((last_seq_num_ + 1) < throughput_subscriber_.throughput_type_->seqnum)
                    {
                        lost_samples_ += throughput_subscriber_.throughput_type_->seqnum - last_seq_num_ - 1;
                    }
                    last_seq_num_ = throughput_subscriber_.throughput_type_->seqnum;
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

void ThroughputSubscriber::DataSubListener::save_numbers()
{
    saved_last_seq_num_ = last_seq_num_;
    saved_lost_samples_ = lost_samples_;
}

// *******************************************************************************************
// *********************************** COMMAND SUB LISTENER **********************************
// *******************************************************************************************
ThroughputSubscriber::CommandSubListener::CommandSubListener(
        ThroughputSubscriber& throughput_subscriber)
    : throughput_subscriber_(throughput_subscriber)
{
}

ThroughputSubscriber::CommandSubListener::~CommandSubListener()
{
}

void ThroughputSubscriber::CommandSubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& match_info)
{
    std::unique_lock<std::mutex> lock(throughput_subscriber_.command_mutex_);
    if (match_info.status == MATCHED_MATCHING)
    {
        std::cout << C_RED << "Sub: COMMAND Sub Matched" << C_DEF << std::endl;
        ++throughput_subscriber_.command_discovery_count_;
    }
    else
    {
        std::cout << C_RED << "Sub: COMMAND SUBSCRIBER MATCHING REMOVAL" << C_DEF << std::endl;
        --throughput_subscriber_.command_discovery_count_;
    }

    lock.unlock();
    throughput_subscriber_.command_discovery_cv_.notify_one();
}

void ThroughputSubscriber::CommandSubListener::onNewDataMessage(
        Subscriber*)
{
}

// *******************************************************************************************
// *********************************** COMMAND PUB LISTENER **********************************
// *******************************************************************************************
ThroughputSubscriber::CommandPubListener::CommandPubListener(
        ThroughputSubscriber& throughput_subscriber)
    : throughput_subscriber_(throughput_subscriber)
{
}

ThroughputSubscriber::CommandPubListener::~CommandPubListener()
{
}

void ThroughputSubscriber::CommandPubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(throughput_subscriber_.command_mutex_);
    if (info.status == MATCHED_MATCHING)
    {
        std::cout << C_RED << "Sub: COMMAND Pub Matched" << C_DEF << std::endl;
        ++throughput_subscriber_.command_discovery_count_;
    }
    else
    {
        std::cout << C_RED << "Sub: COMMAND PUBLISHER MATCHING REMOVAL" << C_DEF << std::endl;
        --throughput_subscriber_.command_discovery_count_;
    }
    lock.unlock();
    throughput_subscriber_.command_discovery_cv_.notify_one();
}

// *******************************************************************************************
// ********************************** THROUGHPUT SUBSCRIBER **********************************
// *******************************************************************************************

ThroughputSubscriber::~ThroughputSubscriber()
{
    Domain::removeParticipant(participant_);
    std::cout << "Sub: Participant removed" << std::endl;
}

ThroughputSubscriber::ThroughputSubscriber(
        bool reliable,
        uint32_t pid,
        bool hostname,
        const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
        const std::string& xml_config_file,
        bool dynamic_types,
        int forced_domain)
    : command_discovery_count_(0)
    , data_discovery_count_(0)
    , throughput_type_(nullptr)
    , dynamic_data_(dynamic_types)
    , ready_(true)
    , stop_count_(0)
    , data_size_(0)
    , demand_(0)
    , forced_domain_(forced_domain)
    , xml_config_file_(xml_config_file)
#pragma warning(disable:4355)
    , data_sub_listener_(*this)
    , command_sub_listener_(*this)
    , command_pub_listener_(*this)
{
    // Dummy type registration
    if (dynamic_data_)
    {
        // Create basic builders
        DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

        // Add members to the struct.
        struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
        struct_type_builder->add_member(1, "data", DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                    DynamicTypeBuilderFactory::get_instance()->create_byte_type(), ::dds::core::LENGTH_UNLIMITED));
        struct_type_builder->set_name("ThroughputType");
        dynamic_type_ = struct_type_builder->build();
        dynamic_pub_sub_type_.SetDynamicType(dynamic_type_);
    }

    /* Create RTPSParticipant */
    std::string participant_profile_name = "sub_participant_profile";
    ParticipantAttributes participant_attributes;

    // Default domain
    participant_attributes.domainId = pid % 230;

    // Default participant name
    participant_attributes.rtps.setName("throughput_test_subscriber");

    // Load XML file
    if (xml_config_file_.length() > 0)
    {
        if (eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK !=
                eprosima::fastrtps::xmlparser::XMLProfileManager::fillParticipantAttributes(participant_profile_name,
                participant_attributes))
        {
            ready_ = false;
            return;
        }
    }

    // Apply user's force domain
    if (forced_domain_ >= 0)
    {
        participant_attributes.domainId = forced_domain_;
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
        std::cout << "ERROR creating participant" << std::endl;
        ready_ = false;
        return;
    }

    // Register the data type
    throughput_data_type_ = nullptr;
    Domain::registerType(participant_, (TopicDataType*)&throuput_command_type_);

    /* Create Data Subscriber */
    std::string profile_name = "subscriber_profile";
    sub_attrs_.topic.topicDataType = "ThroughputType";
    sub_attrs_.topic.topicKind = NO_KEY;

    // Default topic
    std::ostringstream data_topic;
    data_topic << "ThroughputTest_";
    if (hostname)
    {
        data_topic << asio::ip::host_name() << "_";
    }
    data_topic << pid << "_UP";
    sub_attrs_.topic.topicName = data_topic.str();

    // Reliability
    if (reliable)
    {
        sub_attrs_.times.heartbeatResponseDelay = TimeConv::MilliSeconds2Time_t(0).to_duration_t();
        sub_attrs_.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        sub_attrs_.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    }

    // Load XML file
    if (xml_config_file_.length() > 0)
    {
        if (xmlparser::XMLP_ret::XML_OK
                != xmlparser::XMLProfileManager::fillSubscriberAttributes(profile_name, sub_attrs_))
        {
            std::cout << "Cannot read subscriber profile " << profile_name << std::endl;
        }
    }

    // If the user has specified a publisher property policy with command line arguments, it overrides whatever the
    // XML configures.
    if (PropertyPolicyHelper::length(property_policy) > 0)
    {
        sub_attrs_.properties = property_policy;
    }
    data_subscriber_ = nullptr;

    // COMMAND
    PublisherAttributes command_publisher_attrs;
    command_publisher_attrs.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    command_publisher_attrs.topic.topicDataType = "ThroughputCommand";
    command_publisher_attrs.topic.topicKind = NO_KEY;
    std::ostringstream pub_command_topic;
    pub_command_topic << "ThroughputTest_Command_";
    if (hostname)
    {
        pub_command_topic << asio::ip::host_name() << "_";
    }
    pub_command_topic << pid << "_SUB2PUB";
    command_publisher_attrs.topic.topicName = pub_command_topic.str();
    command_publisher_attrs.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    command_publisher_attrs.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    command_publisher_attrs.qos.m_publishMode.kind = SYNCHRONOUS_PUBLISH_MODE;
    command_publisher_attrs.properties = property_policy;

    command_publisher_ = Domain::createPublisher(participant_, command_publisher_attrs,
                    (PublisherListener*)&this->command_pub_listener_);

    SubscriberAttributes command_subscriber_attrs;
    command_subscriber_attrs.topic.topicDataType = "ThroughputCommand";
    command_subscriber_attrs.topic.topicKind = NO_KEY;
    command_subscriber_attrs.topic.topicName = "ThroughputCommandP2S";
    std::ostringstream sub_command_topic;
    sub_command_topic << "ThroughputTest_Command_";
    if (hostname)
    {
        sub_command_topic << asio::ip::host_name() << "_";
    }
    sub_command_topic << pid << "_PUB2SUB";
    command_subscriber_attrs.topic.topicName = sub_command_topic.str();
    command_subscriber_attrs.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    command_subscriber_attrs.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    command_subscriber_attrs.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    command_subscriber_attrs.properties = property_policy;

    command_subscriber_ = Domain::createSubscriber(participant_, command_subscriber_attrs,
                    (SubscriberListener*)&this->command_sub_listener_);

    // Calculate overhead
    t_start_ = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        t_end_ = std::chrono::steady_clock::now();
    }
    t_overhead_ = std::chrono::duration<double, std::micro>(t_end_ - t_start_) / 1001;
    std::cout << "Subscriber's clock access overhead: " << t_overhead_.count() << " us" << std::endl;

    if (command_subscriber_ == nullptr || command_publisher_ == nullptr)
    {
        ready_ = false;
    }
}

void ThroughputSubscriber::process_message()
{
    if (command_subscriber_->wait_for_unread_samples({100, 0}))
    {
        if (command_subscriber_->takeNextData((void*)&command_sub_listener_.command_type_,
                &command_sub_listener_.info_))
        {
            switch (command_sub_listener_.command_type_.m_command)
            {
                case (DEFAULT):
                {
                    break;
                }
                case (BEGIN):
                {
                    break;
                }
                case (READY_TO_START):
                {
                    std::cout << "-----------------------------------------------------------------------" << std::endl;
                    std::cout << "Command: READY_TO_START" << std::endl;
                    data_size_ = command_sub_listener_.command_type_.m_size;
                    demand_ = command_sub_listener_.command_type_.m_demand;

                    if (dynamic_data_)
                    {
                        // Create basic builders
                        DynamicTypeBuilder_ptr struct_type_builder(
                            DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

                        // Add members to the struct.
                        struct_type_builder->add_member(0, "seqnum",
                                DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
                        struct_type_builder->add_member(1, "data",
                                DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                                    DynamicTypeBuilderFactory::get_instance()->create_byte_type(), data_size_));

                        struct_type_builder->set_name("ThroughputType");
                        dynamic_type_ = struct_type_builder->build();
                        dynamic_pub_sub_type_.CleanDynamicType();
                        dynamic_pub_sub_type_.SetDynamicType(dynamic_type_);

                        Domain::registerType(participant_, &dynamic_pub_sub_type_);

                        dynamic_data_type_ = DynamicDataFactory::get_instance()->create_data(dynamic_type_);
                    }
                    else
                    {
                        delete(throughput_data_type_);
                        delete(throughput_type_);

                        throughput_data_type_ = new ThroughputDataType(data_size_);
                        Domain::registerType(participant_, throughput_data_type_);
                        throughput_type_ = new ThroughputType(data_size_);
                    }

                    data_subscriber_ = Domain::createSubscriber(participant_, sub_attrs_, &data_sub_listener_);

                    ThroughputCommandType command_sample(BEGIN);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    data_sub_listener_.reset();
                    command_publisher_->write(&command_sample);

                    std::cout << "Waiting for data discovery" << std::endl;
                    std::unique_lock<std::mutex> data_disc_lock(data_mutex_);
                    data_discovery_cv_.wait(data_disc_lock, [&]()
                            {
                                return data_discovery_count_ > 0;
                            });
                    data_disc_lock.unlock();
                    std::cout << "Discovery data complete" << std::endl;
                    break;
                }
                case (TEST_STARTS):
                {
                    std::cout << "Command: TEST_STARTS" << std::endl;
                    t_start_ = std::chrono::steady_clock::now();
                    break;
                }
                case (TEST_ENDS):
                {
                    t_end_ = std::chrono::steady_clock::now();
                    std::cout << "Command: TEST_ENDS" << std::endl;
                    data_sub_listener_.save_numbers();
                    std::unique_lock<std::mutex> lock(command_mutex_);
                    stop_count_ = 1;
                    lock.unlock();
                    if (dynamic_data_)
                    {
                        DynamicTypeBuilderFactory::delete_instance();
                        DynamicDataFactory::get_instance()->delete_data(dynamic_data_type_);
                    }
                    else
                    {
                        delete(throughput_type_);
                        throughput_type_ = nullptr;
                    }
                    sub_attrs_ = data_subscriber_->getAttributes();
                    break;
                }
                case (ALL_STOPS):
                {
                    std::cout << "-----------------------------------------------------------------------" << std::endl;
                    std::unique_lock<std::mutex> lock(command_mutex_);
                    stop_count_ = 2;
                    lock.unlock();
                    std::cout << "Command: ALL_STOPS" << std::endl;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
}

bool ThroughputSubscriber::ready()
{
    return ready_;
}

void ThroughputSubscriber::run()
{
    if (!ready_)
    {
        return;
    }
    std::cout << "Sub Waiting for command discovery" << std::endl;
    {
        std::unique_lock<std::mutex> lock(command_mutex_);
        command_discovery_cv_.wait(lock, [&]()
                {
                    return command_discovery_count_ >= 2;
                });
    }
    std::cout << "Sub Discovery command complete" << std::endl;

    do
    {
        process_message();

        if (stop_count_ == 1)
        {
            std::cout << "Waiting for data matching removal" << std::endl;
            std::unique_lock<std::mutex> data_disc_lock(data_mutex_);
            data_discovery_cv_.wait(data_disc_lock, [&]()
                    {
                        return data_discovery_count_ == 0;
                    });
            data_disc_lock.unlock();

            std::cout << "Waiting clean state" << std::endl;
            while (!data_subscriber_->isInCleanState())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            std::cout << "Sending results" << std::endl;
            ThroughputCommandType command_sample;
            command_sample.m_command = TEST_RESULTS;
            command_sample.m_demand = demand_;
            command_sample.m_size = data_size_ + 4 + 4;
            command_sample.m_lastrecsample = data_sub_listener_.saved_last_seq_num_;
            command_sample.m_lostsamples = data_sub_listener_.saved_lost_samples_;

            double total_time_count =
                    (std::chrono::duration<double, std::micro>(t_end_ - t_start_) - t_overhead_).count();

            if (total_time_count < std::numeric_limits<uint64_t>::min())
            {
                command_sample.m_totaltime = std::numeric_limits<uint64_t>::min();
            }
            else if (total_time_count > std::numeric_limits<uint64_t>::max())
            {
                command_sample.m_totaltime = std::numeric_limits<uint64_t>::max();
            }
            else
            {
                command_sample.m_totaltime = static_cast<uint64_t>(total_time_count);
            }

            std::cout << "Last Received Sample: " << command_sample.m_lastrecsample << std::endl;
            std::cout << "Lost Samples: " << command_sample.m_lostsamples << std::endl;
            std::cout << "Samples per second: "
                      << (double)(command_sample.m_lastrecsample - command_sample.m_lostsamples) * 1000000 /
                command_sample.m_totaltime
                      << std::endl;
            std::cout << "Test of size " << command_sample.m_size << " and demand " << command_sample.m_demand <<
                " ends." << std::endl;
            command_publisher_->write(&command_sample);

            stop_count_ = 0;

            Domain::removeSubscriber(data_subscriber_);
            std::cout << "Sub: Data subscriber removed" << std::endl;
            data_subscriber_ = nullptr;
            Domain::unregisterType(participant_, "ThroughputType");
            std::cout << "Sub: ThroughputType unregistered" << std::endl;

            if (!dynamic_data_)
            {
                delete throughput_data_type_;
                throughput_data_type_ = nullptr;
            }
            std::cout << "-----------------------------------------------------------------------" << std::endl;
        }
    } while (stop_count_ != 2);

    // ThroughputPublisher is waiting for all ThroughputSubscriber publishers and subscribers to unmatch. Leaving the
    // destruction of the entities to ~ThroughputSubscriber() is not enough for the intraprocess case, because
    // main_ThroughputTests first joins the publisher run thread and only then it joins this thread. This means that
    // ~ThroughputSubscriber() is only called when all the ThroughputSubscriber publishers and subscribers are disposed.
    Domain::removePublisher(command_publisher_);
    std::cout << "Sub: Command publisher removed" << std::endl;
    Domain::removeSubscriber(command_subscriber_);
    std::cout << "Sub: Command subscriber removed" << std::endl;

    return;
}
