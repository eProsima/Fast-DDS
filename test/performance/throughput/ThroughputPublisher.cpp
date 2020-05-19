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

#include "ThroughputPublisher.hpp"

#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/Domain.h>

#include <dds/core/LengthUnlimited.hpp>

#include <map>
#include <fstream>
#include <chrono>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// *******************************************************************************************
// ************************************ DATA PUB LISTENER ************************************
// *******************************************************************************************
ThroughputPublisher::DataPubListener::DataPubListener(
        ThroughputPublisher& throughput_publisher)
    : throughput_publisher_(throughput_publisher)
{
}

ThroughputPublisher::DataPubListener::~DataPubListener()
{
}

void ThroughputPublisher::DataPubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(throughput_publisher_.data_mutex_);
    if (info.status == MATCHED_MATCHING)
    {
        ++throughput_publisher_.data_discovery_count_;
    }
    else
    {
        --throughput_publisher_.data_discovery_count_;
    }

    lock.unlock();
    throughput_publisher_.data_discovery_cv_.notify_one();
}

// *******************************************************************************************
// ********************************** COMMAND SUB LISTENER ***********************************
// *******************************************************************************************
ThroughputPublisher::CommandSubListener::CommandSubListener(
        ThroughputPublisher& throughput_publisher)
    : throughput_publisher_(throughput_publisher)
{
}

ThroughputPublisher::CommandSubListener::~CommandSubListener()
{
}

void ThroughputPublisher::CommandSubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(throughput_publisher_.command_mutex_);
    if (info.status == MATCHED_MATCHING)
    {
        std::cout << C_RED << "COMMAND Sub Matched" << C_DEF << std::endl;
        ++throughput_publisher_.command_discovery_count_;
    }
    else
    {
        std::cout << C_RED << "COMMAND SUBSCRIBER MATCHING REMOVAL" << C_DEF << std::endl;
        --throughput_publisher_.command_discovery_count_;
    }

    lock.unlock();
    throughput_publisher_.command_discovery_cv_.notify_one();
}

// *******************************************************************************************
// ********************************** COMMAND PUB LISTENER ***********************************
// *******************************************************************************************
ThroughputPublisher::CommandPubListener::CommandPubListener(
        ThroughputPublisher& throughput_publisher)
    : throughput_publisher_(throughput_publisher)
{
}

ThroughputPublisher::CommandPubListener::~CommandPubListener()
{
}

void ThroughputPublisher::CommandPubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(throughput_publisher_.command_mutex_);
    if (info.status == MATCHED_MATCHING)
    {
        std::cout << C_RED << "COMMAND Pub Matched" << C_DEF << std::endl;
        ++throughput_publisher_.command_discovery_count_;
    }
    else
    {
        std::cout << C_RED << "COMMAND PUBLISHER MATCHING REMOVAL" << C_DEF << std::endl;
        --throughput_publisher_.command_discovery_count_;
    }

    lock.unlock();
    throughput_publisher_.command_discovery_cv_.notify_one();
}

// *******************************************************************************************
// ********************************** THROUGHPUT PUBLISHER ***********************************
// *******************************************************************************************
ThroughputPublisher::ThroughputPublisher(
        bool reliable,
        uint32_t pid,
        bool hostname,
        const std::string& export_csv,
        const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
        const std::string& xml_config_file,
        const std::string& demands_file,
        const std::string& recoveries_file,
        bool dynamic_types,
        int forced_domain)
    : command_discovery_count_(0)
    , data_discovery_count_(0)
    , dynamic_data_(dynamic_types)
    , ready_(true)
    , reliable_(reliable)
    , forced_domain_(forced_domain)
    , demands_file_(demands_file)
    , export_csv_(export_csv)
    , xml_config_file_(xml_config_file)
    , recoveries_file_(recoveries_file)
#pragma warning(disable:4355)
    , data_pub_listener_(*this)
    , command_pub_listener_(*this)
    , command_sub_listener_(*this)
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
    std::string participant_profile_name = "pub_participant_profile";
    ParticipantAttributes participant_attributes;

    // Default domain
    participant_attributes.domainId = pid % 230;

    // Default participant name
    participant_attributes.rtps.setName("throughput_test_publisher");

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

    // Register the date type
    throughput_type_ = nullptr;
    Domain::registerType(participant_, (TopicDataType*)&throuput_command_type_);

    /* Create Data Publisher */
    std::string profile_name = "publisher_profile";
    pub_attrs_.topic.topicDataType = "ThroughputType";
    pub_attrs_.topic.topicKind = NO_KEY;

    // Default topic
    std::ostringstream data_topic;
    data_topic << "ThroughputTest_";
    if (hostname)
    {
        data_topic << asio::ip::host_name() << "_";
    }
    data_topic << pid << "_UP";
    pub_attrs_.topic.topicName = data_topic.str();

    // Reliability
    if (reliable)
    {
        pub_attrs_.times.heartbeatPeriod = TimeConv::MilliSeconds2Time_t(100).to_duration_t();
        pub_attrs_.times.nackSupressionDuration = TimeConv::MilliSeconds2Time_t(0).to_duration_t();
        pub_attrs_.times.nackResponseDelay = TimeConv::MilliSeconds2Time_t(0).to_duration_t();
        pub_attrs_.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    }
    else
    {
        pub_attrs_.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    }

    // Load XML file
    if (xml_config_file_.length() > 0)
    {
        if (xmlparser::XMLP_ret::XML_OK
                != xmlparser::XMLProfileManager::fillPublisherAttributes(profile_name, pub_attrs_))
        {
            std::cout << "Cannot read publisher profile " << profile_name << std::endl;
        }
    }

    // If the user has specified a publisher property policy with command line arguments, it overrides whatever the
    // XML configures.
    if (PropertyPolicyHelper::length(property_policy) > 0)
    {
        pub_attrs_.properties = property_policy;
    }
    data_publisher_ = nullptr;

    // COMMAND SUBSCRIBER
    SubscriberAttributes command_subscriber_attrs;
    command_subscriber_attrs.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    command_subscriber_attrs.topic.topicDataType = "ThroughputCommand";
    command_subscriber_attrs.topic.topicKind = NO_KEY;
    std::ostringstream sub_command_topic;
    sub_command_topic << "ThroughputTest_Command_";
    if (hostname)
    {
        sub_command_topic << asio::ip::host_name() << "_";
    }
    sub_command_topic << pid << "_SUB2PUB";
    command_subscriber_attrs.topic.topicName = sub_command_topic.str();
    command_subscriber_attrs.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    command_subscriber_attrs.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    command_subscriber_attrs.properties = property_policy;

    command_subscriber_ = Domain::createSubscriber(participant_, command_subscriber_attrs,
                    (SubscriberListener*)&this->command_sub_listener_);

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
    pub_command_topic << pid << "_PUB2SUB";
    command_publisher_attrs.topic.topicName = pub_command_topic.str();
    command_publisher_attrs.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    command_publisher_attrs.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    command_publisher_attrs.qos.m_publishMode.kind = SYNCHRONOUS_PUBLISH_MODE;
    command_publisher_attrs.properties = property_policy;

    command_publisher_ = Domain::createPublisher(participant_, command_publisher_attrs,
                    (PublisherListener*)&this->command_pub_listener_);

    // Calculate overhead
    t_start_ = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        t_end_ = std::chrono::steady_clock::now();
    }
    t_overhead_ = std::chrono::duration<double, std::micro>(t_end_ - t_start_) / 1001;
    std::cout << "Publisher's clock access overhead: " << t_overhead_.count() << " us"  << std::endl;

    if (command_subscriber_ == nullptr || command_publisher_ == nullptr)
    {
        ready_ = false;
    }
}

ThroughputPublisher::~ThroughputPublisher()
{
    Domain::removeParticipant(participant_);
}

bool ThroughputPublisher::ready()
{
    return ready_;
}

void ThroughputPublisher::run(
        uint32_t test_time,
        uint32_t recovery_time_ms,
        int demand,
        int msg_size)
{
    if (!ready_)
    {
        return;
    }

    if (demand == 0 || msg_size == 0)
    {
        if (!this->load_demands_payload())
        {
            return;
        }
    }
    else
    {
        payload_ = msg_size;
        demand_payload_[msg_size - 8].push_back(demand);
    }

    /* Populate the recovery times vector */
    if (recoveries_file_ != "")
    {
        if (!load_recoveries())
        {
            return;
        }
    }
    else
    {
        recovery_times_.push_back(recovery_time_ms);
    }

    std::cout << "Recovery times: ";
    for (uint16_t i = 0; i < recovery_times_.size(); i++)
    {
        std::cout << recovery_times_[i] << ", ";
    }
    std::cout << std::endl;

    /* Create the export_csv_ file and add the header */
    if (export_csv_ != "")
    {
        std::ofstream data_file;
        data_file.open(export_csv_);
        data_file << "Payload [Bytes],Demand [sample/burst],Recovery time [ms],Sent [samples],Publication time [us],"
                  << "Publication sample rate [Sample/s],Publication throughput [Mb/s],Received [samples],"
                  << "Lost [samples],Subscription time [us],Subscription sample rate [Sample/s],"
                  << "Subscription throughput [Mb/s]" << std::endl;
        data_file.flush();
        data_file.close();
    }

    std::cout << "Pub Waiting for command discovery" << std::endl;
    {
        std::unique_lock<std::mutex> disc_lock(command_mutex_);
        command_discovery_cv_.wait(disc_lock, [&]()
        {
            return command_discovery_count_ == 2;
        });
    }
    std::cout << "Pub Discovery command complete" << std::endl;

    ThroughputCommandType command;
    SampleInfo_t info;
    for (auto sit = demand_payload_.begin(); sit != demand_payload_.end(); ++sit)
    {
        for (auto dit = sit->second.begin(); dit != sit->second.end(); ++dit)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            command.m_size = sit->first;
            command.m_demand = *dit;

            // Check history resources depending on the history kind and demand
            if (pub_attrs_.topic.historyQos.kind == KEEP_LAST_HISTORY_QOS)
            {
                // Ensure that the history depth is at least the demand
                if (pub_attrs_.topic.historyQos.depth < 0 ||
                        static_cast<uint32_t>(pub_attrs_.topic.historyQos.depth) < command.m_demand)
                {
                    logWarning(THROUGHPUTPUBLISHER, "Setting history depth to " << command.m_demand);
                    pub_attrs_.topic.resourceLimitsQos.max_samples = command.m_demand;
                    pub_attrs_.topic.historyQos.depth = command.m_demand;
                }
            }
            // KEEP_ALL case
            else
            {
                // Ensure that the max samples is at least the demand
                if (pub_attrs_.topic.resourceLimitsQos.max_samples < 0 ||
                        static_cast<uint32_t>(pub_attrs_.topic.resourceLimitsQos.max_samples) < command.m_demand)
                {
                    logWarning(THROUGHPUTPUBLISHER, "Setting resource limit max samples to " << command.m_demand);
                    pub_attrs_.topic.resourceLimitsQos.max_samples = command.m_demand;
                }
            }
            // Set the allocated samples to the max_samples. This is because allocated_sample must be <= max_samples
            pub_attrs_.topic.resourceLimitsQos.allocated_samples = pub_attrs_.topic.resourceLimitsQos.max_samples;

            for (uint16_t i = 0; i < recovery_times_.size(); i++)
            {
                command.m_command = READY_TO_START;
                command.m_size = sit->first;
                command.m_demand = *dit;
                command_publisher_->write((void*)&command);
                command_subscriber_->wait_for_unread_samples({20, 0});
                command_subscriber_->takeNextData((void*)&command, &info);
                if (command.m_command == BEGIN)
                {
                    if (!test(test_time, recovery_times_[i], *dit, sit->first))
                    {
                        command.m_command = ALL_STOPS;
                        command_publisher_->write((void*)&command);
                        return;
                    }
                }
            }
        }
    }

    command.m_command = ALL_STOPS;
    command_publisher_->write((void*)&command);
    bool all_acked = command_publisher_->wait_for_all_acked(eprosima::fastrtps::Time_t(20, 0));
    print_results(results_);

    if (!all_acked)
    {
        std::cout << "ALL_STOPS Not acked! in 20(s)" << std::endl;
    }
    else
    {
        // Wait for the subscriber unmatch.
        std::unique_lock<std::mutex> disc_lock(command_mutex_);
        command_discovery_cv_.wait(disc_lock, [&]()
            {
                return command_discovery_count_ == 0;
            });
    }
}

bool ThroughputPublisher::test(
        uint32_t test_time,
        uint32_t recovery_time_ms,
        uint32_t demand,
        uint32_t msg_size)
{
    if (dynamic_data_)
    {
        // Create basic builders
        DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

        // Add members to the struct.
        struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
        struct_type_builder->add_member(1, "data", DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                    DynamicTypeBuilderFactory::get_instance()->create_byte_type(), msg_size));
        struct_type_builder->set_name("ThroughputType");
        dynamic_type_ = struct_type_builder->build();
        dynamic_pub_sub_type_.CleanDynamicType();
        dynamic_pub_sub_type_.SetDynamicType(dynamic_type_);

        Domain::registerType(participant_, &dynamic_pub_sub_type_);
        dynamic_data_type_ = DynamicDataFactory::get_instance()->create_data(dynamic_type_);

        MemberId id;
        DynamicData* dynamic_data = dynamic_data_type_->loan_value(dynamic_data_type_->get_member_id_at_index(1));
        for (uint32_t i = 0; i < msg_size; ++i)
        {
            dynamic_data->insert_sequence_data(id);
            dynamic_data->set_byte_value(0, id);
        }
        dynamic_data_type_->return_loaned_value(dynamic_data);
    }
    else
    {
        throughput_data_type_ = new ThroughputDataType(msg_size);
        Domain::registerType(participant_, throughput_data_type_);
        throughput_type_ = new ThroughputType((uint16_t)msg_size);
    }

    data_publisher_ = Domain::createPublisher(participant_, pub_attrs_, &data_pub_listener_);

    std::unique_lock<std::mutex> data_disc_lock(data_mutex_);
    data_discovery_cv_.wait(data_disc_lock, [&]()
    {
        return data_discovery_count_ > 0;
    });
    data_disc_lock.unlock();

    // Declare test time variables
    std::chrono::duration<double, std::micro> clock_overhead(0);
    std::chrono::duration<double, std::nano> test_time_ns = std::chrono::seconds(test_time);
    std::chrono::duration<double, std::nano> recovery_duration_ns = std::chrono::milliseconds(recovery_time_ms);
    std::chrono::steady_clock::time_point batch_start;

    // Send a TEST_STARTS and sleep for a while to give the subscriber time to set up
    uint32_t samples = 0;
    size_t aux;
    ThroughputCommandType command_sample;
    SampleInfo_t info;
    command_sample.m_command = TEST_STARTS;
    command_publisher_->write((void*)&command_sample);

    // If the subscriber does not acknowledge the TEST_STARTS in time, we consider something went wrong.
    std::chrono::steady_clock::time_point test_start_sent_tp = std::chrono::steady_clock::now();
    if (!command_publisher_->wait_for_all_acked(eprosima::fastrtps::Time_t(20, 0)))
    {
        std::cout << "Something went wrong: The subscriber has not acknowledged the TEST_STARTS command." << std::endl;
        return false;
    }
    // Calculate how low has it takes for the subscriber to acknowledge TEST_START
    std::chrono::duration<double, std::micro> test_start_ack_duration =
            std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - test_start_sent_tp);

    // Send batches until test_time_ns is reached
    t_start_ = std::chrono::steady_clock::now();
    while ((t_end_ - t_start_) < test_time_ns)
    {
        // Get start time
        batch_start = std::chrono::steady_clock::now();
        // Send a batch of size demand
        for (uint32_t sample = 0; sample < demand; sample++)
        {
            if (dynamic_data_)
            {
                dynamic_data_type_->set_uint32_value(dynamic_data_type_->get_uint32_value(0) + 1, 0);
                data_publisher_->write((void*)dynamic_data_type_);
            }
            else
            {
                throughput_type_->seqnum++;
                data_publisher_->write((void*)throughput_type_);
            }
        }
        // Get end time
        t_end_ = std::chrono::steady_clock::now();
        // Add the number of sent samples
        samples += demand;

        /*
            If the batch took less than the recovery time, sleep for the difference recovery_duration - batch_duration.
            Else, go ahead with the next batch without time to recover.
            The previous is achieved with a call to sleep_for(). If the duration specified for sleep_for is negative,
            all implementations we know about return without setting the thread to sleep.
         */
        std::this_thread::sleep_for(recovery_duration_ns - (t_end_ - batch_start));

        clock_overhead += t_overhead_ * 2; // We access the clock twice per batch.
    }
    command_sample.m_command = TEST_ENDS;

    command_publisher_->write((void*)&command_sample);
    data_publisher_->removeAllChange();

    // If the subscriber does not acknowledge the TEST_ENDS in time, we consider something went wrong.
    if (!command_publisher_->wait_for_all_acked(eprosima::fastrtps::Time_t(20, 0)))
    {
        std::cout << "Something went wrong: The subscriber has not acknowledged the TEST_ENDS command." << std::endl;
        return false;
    }

    if (dynamic_data_)
    {
        DynamicTypeBuilderFactory::delete_instance();
        DynamicDataFactory::get_instance()->delete_data(dynamic_data_type_);
    }
    else
    {
        delete(throughput_type_);
    }
    pub_attrs_ = data_publisher_->getAttributes();
    Domain::removePublisher(data_publisher_);
    data_publisher_ = nullptr;
    Domain::unregisterType(participant_, "ThroughputType");
    if (!dynamic_data_)
    {
        delete throughput_data_type_;
    }

    command_subscriber_->wait_for_unread_samples({20, 0});
    if (command_subscriber_->takeNextData((void*)&command_sample, &info))
    {
        if (command_sample.m_command == TEST_RESULTS)
        {
            TroughputResults result;
            result.payload_size = msg_size + 4 + 4;
            result.demand = demand;
            result.recovery_time_ms = recovery_time_ms;

            result.publisher.send_samples = samples;
            result.publisher.totaltime_us =
                    std::chrono::duration<double, std::micro>(t_end_ - t_start_) - clock_overhead;

            result.subscriber.recv_samples = command_sample.m_lastrecsample - command_sample.m_lostsamples;
            result.subscriber.lost_samples = command_sample.m_lostsamples;
            result.subscriber.totaltime_us =
                    std::chrono::microseconds(command_sample.m_totaltime) - test_start_ack_duration - clock_overhead;

            result.compute();
            results_.push_back(result);

            /* Log data to CSV file */
            if (export_csv_ != "")
            {
                std::ofstream data_file;
                data_file.open(export_csv_, std::fstream::app);
                data_file << std::fixed << std::setprecision(3)
                          << result.payload_size << ","
                          << result.demand << ","
                          << result.recovery_time_ms << ","
                          << result.publisher.send_samples << ","
                          << result.publisher.totaltime_us.count() << ","
                          << result.publisher.Packssec << ","
                          << result.publisher.MBitssec << ","
                          << result.subscriber.recv_samples << ","
                          << result.subscriber.lost_samples << ","
                          << result.subscriber.totaltime_us.count() << ","
                          << result.subscriber.Packssec << ","
                          << result.subscriber.MBitssec << std::endl;
                data_file.flush();
                data_file.close();
            }
            command_publisher_->removeAllChange(&aux);
            return true;
        }
        else
        {
            std::cout << "The test expected results, stopping" << std::endl;
        }
    }
    else
    {
        std::cout << "PROBLEM READING RESULTS;" << std::endl;
    }

    return false;
}

bool ThroughputPublisher::load_demands_payload()
{
    std::ifstream fi(demands_file_);

    std::cout << "Reading demands file: " << demands_file_ << std::endl;
    std::string DELIM = ";";
    if (!fi.is_open())
    {
        std::cout << "Could not open demands file: " << demands_file_ << " , closing." << std::endl;
        return false;
    }

    std::string line;
    size_t start;
    size_t end;
    bool first = true;
    bool more = true;
    while (std::getline(fi, line))
    {
        start = 0;
        end = line.find(DELIM);
        first = true;
        uint32_t demand;
        more = true;
        while (more)
        {
            std::istringstream iss(line.substr(start, end - start));
            if (first)
            {
                iss >> payload_;
                if (payload_ < 8)
                {
                    std::cout << "Minimum payload is 16 bytes" << std::endl;
                    return false;
                }
                payload_ -= 8;
                first = false;
            }
            else
            {
                iss >> demand;
                demand_payload_[payload_].push_back(demand);
            }

            start = end + DELIM.length();
            end = line.find(DELIM, start);
            if (end == std::string::npos)
            {
                more = false;
                std::istringstream n_iss(line.substr(start, end - start));
                if (n_iss >> demand)
                {
                    demand_payload_[payload_].push_back(demand);
                }
            }
        }
    }
    fi.close();

    payload_ += 8;

    std::cout << "Performing test with this payloads/demands:" << std::endl;
    for (auto sit = demand_payload_.begin(); sit != demand_payload_.end(); ++sit)
    {
        printf("Payload: %6d; Demands: ", sit->first + 8);
        for (auto dit = sit->second.begin(); dit != sit->second.end(); ++dit)
        {
            printf("%6d, ", *dit);
        }
        printf("\n");
    }
    return true;
}

bool ThroughputPublisher::load_recoveries()
{
    std::ifstream fi(recoveries_file_);

    std::cout << "Reading recoveries file: " << recoveries_file_ << std::endl;
    std::string DELIM = ";";
    if (!fi.is_open())
    {
        std::cout << "Could not open recoveries file: " << recoveries_file_ << " , closing." << std::endl;
        return false;
    }

    std::string line;
    size_t start;
    size_t end;
    uint32_t recovery;
    int32_t input_recovery;
    bool more = true;
    while (std::getline(fi, line))
    {
        start = 0;
        end = line.find(DELIM);
        more = true;
        while (more)
        {
            std::istringstream iss(line.substr(start, end - start));
            iss >> input_recovery;
            if (input_recovery < 0)
            {
                std::cout << "Recovery times must be positive. " << input_recovery << " found" << std::endl;
                return false;
            }
            recovery = static_cast<uint32_t>(input_recovery);
            // Only add if it was not there already
            if (std::find(recovery_times_.begin(), recovery_times_.end(), recovery) == recovery_times_.end())
            {
                recovery_times_.push_back(recovery);
            }

            start = end + DELIM.length();
            end = line.find(DELIM, start);
            if (end == std::string::npos)
            {
                more = false;
                std::istringstream n_iss(line.substr(start, end - start));
                if (n_iss >> recovery)
                {
                    // Only add if it was not there already
                    if (std::find(recovery_times_.begin(), recovery_times_.end(), recovery) == recovery_times_.end())
                    {
                        recovery_times_.push_back(recovery);
                    }
                }
            }
        }
    }
    fi.close();
    return true;
}
