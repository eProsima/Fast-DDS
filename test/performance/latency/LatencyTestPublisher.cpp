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
 * @file LatencyPublisher.cpp
 *
 */

#include "LatencyTestPublisher.hpp"
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <numeric>
#include <cmath>
#include <fstream>
#include <inttypes.h>

#define TIME_LIMIT_US 10000

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;
using namespace std;

uint32_t dataspub[] = {12, 28, 60, 124, 252, 508, 1020, 2044, 4092, 8188, 16380};
uint32_t dataspub_large[] = {63996, 131068};

std::vector<uint32_t> data_size_pub;


LatencyTestPublisher::LatencyTestPublisher()
    : participant_(nullptr)
    , data_publisher_(nullptr)
    , command_publisher_(nullptr)
    , data_subscriber_(nullptr)
    , command_subscriber_(nullptr)
    , overhead_time_(0.0)
    , discovery_count_(0)
    , command_msg_count_(0)
    , data_msg_count_(0)
    , received_count_(0)
    , test_status_(0)
    , subscribers_(0)
    , samples_(0)
    , latency_type_in_(nullptr)
    , latency_type_out_(nullptr)
    , dynamic_data_type_in_(nullptr)
    , dynamic_data_type_out_(nullptr)
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
    export_prefix_ = "";
    raw_data_file_ = "";
}

LatencyTestPublisher::~LatencyTestPublisher()
{
    Domain::removeParticipant(participant_);
}

bool LatencyTestPublisher::init(
        int subscribers,
        int samples,
        bool reliable,
        uint32_t pid,
        bool hostname,
        bool export_csv,
        const std::string& export_prefix,
        std::string raw_data_file,
        const PropertyPolicy& part_property_policy,
        const PropertyPolicy& property_policy,
        bool large_data,
        const std::string& xml_config_file,
        bool dynamic_data,
        int forced_domain)
{
    // Initialize state
    xml_config_file_ = xml_config_file;
    samples_ = samples;
    subscribers_ = subscribers;
    export_csv_ = export_csv;
    export_prefix_ = export_prefix;
    reliable_ = reliable;
    dynamic_data_ = dynamic_data;
    forced_domain_ = forced_domain;
    raw_data_file_ = raw_data_file;

    // Payloads for which the test runs
    if (!large_data)
    {
        data_size_pub.assign(dataspub, dataspub + sizeof(dataspub) / sizeof(uint32_t) );
    }
    else
    {
        data_size_pub.assign(dataspub_large, dataspub_large + sizeof(dataspub_large) / sizeof(uint32_t) );
    }

    // Init dynamic data
    if (dynamic_data_)
    {
        // Create basic builders
        DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

        // Add members to the struct.
        struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
        struct_type_builder->add_member(1, "data",
                DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                    DynamicTypeBuilderFactory::get_instance()->create_byte_type(), data_size_pub.back()
                    ));
        struct_type_builder->set_name("LatencyType");

        dynamic_type_ = struct_type_builder->build();
        dynamic_pub_sub_type_.SetDynamicType(dynamic_type_);
    }

    // Init output files
    for (std::vector<uint32_t>::iterator it = data_size_pub.begin(); it != data_size_pub.end(); ++it)
    {
        // Reliability
        std::string str_reliable = "besteffort";
        if (reliable_)
        {
            str_reliable = "reliable";
        }

        // Summary files
        output_file_minimum_ << "\"" << samples_ << " samples of " << *it + 4 << " bytes (us)\"";
        output_file_average_ << "\"" << samples_ << " samples of " << *it + 4 << " bytes (us)\"";
        if (it != data_size_pub.end() - 1)
        {
            output_file_minimum_ << ",";
            output_file_average_ << ",";
        }

        // Files by payload
        switch (*it + 4)
        {
            case 16:
                output_file_16_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_16_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 32:
                output_file_32_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_32_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 64:
                output_file_64_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_64_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 128:
                output_file_128_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_128_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 256:
                output_file_256_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_256_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 512:
                output_file_512_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_512_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 1024:
                output_file_1024_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_1024_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 2048:
                output_file_2048_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_2048_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 4096:
                output_file_4096_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_4096_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 8192:
                output_file_8192_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_8192_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 16384:
                output_file_16384_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_16384_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 64000:
                output_file_64000_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_64000_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 131072:
                output_file_131072_ << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
                output_file_131072_ << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" <<
                    std::endl;
                break;
            default:
                break;
        }
    }
    output_file_minimum_ << std::endl;
    output_file_average_ << std::endl;

    /* Create RTPSParticipant */
    std::string participant_profile_name = "pub_participant_profile";
    ParticipantAttributes participant_attributes;

    // Default domain
    participant_attributes.domainId = pid % 230;

    // Default participant name
    participant_attributes.rtps.setName("latency_test_publisher");

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
        return false;
    }

    // Register the data type
    if (dynamic_data_)
    {
        Domain::registerType(participant_, &dynamic_pub_sub_type_);
    }
    else
    {
        Domain::registerType(participant_, (TopicDataType*)&latency_data_type_);
    }

    // Register the command type
    Domain::registerType(participant_, (TopicDataType*)&latency_command_type_);

    /* Create Data Publisher */
    std::string profile_name = "pub_publisher_profile";
    PublisherAttributes publisher_data_attributes;
    publisher_data_attributes.topic.topicDataType = "LatencyType";
    publisher_data_attributes.topic.topicKind = NO_KEY;
    std::ostringstream data_pub_topic_name;
    data_pub_topic_name << "LatencyTest_";
    if (hostname)
    {
        data_pub_topic_name << asio::ip::host_name() << "_";
    }
    data_pub_topic_name << pid << "_PUB2SUB";
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
        data_publisher_ =
                Domain::createPublisher(participant_, profile_name, (PublisherListener*)&this->data_pub_listener_);
    }
    else
    {
        data_publisher_ =
                Domain::createPublisher(participant_, publisher_data_attributes,
                        (PublisherListener*)&this->data_pub_listener_);
    }

    if (data_publisher_ == nullptr)
    {
        return false;
    }

    /* Create Data Echo Subscriber */
    profile_name = "pub_subscriber_profile";
    SubscriberAttributes subscriber_data_attributes;
    subscriber_data_attributes.topic.topicDataType = "LatencyType";
    subscriber_data_attributes.topic.topicKind = NO_KEY;
    std::ostringstream data_sub_topic_name;
    data_sub_topic_name << "LatencyTest_";
    if (hostname)
    {
        data_sub_topic_name << asio::ip::host_name() << "_";
    }
    data_sub_topic_name << pid << "_SUB2PUB";
    subscriber_data_attributes.topic.topicName = data_sub_topic_name.str();

    if (reliable)
    {
        subscriber_data_attributes.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
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
    command_pub_topic_name << pid << "_PUB2SUB";
    publisher_command_attributes.topic.topicName = command_pub_topic_name.str();
    publisher_command_attributes.topic.historyQos.kind = eprosima::fastrtps::KEEP_ALL_HISTORY_QOS;
    publisher_command_attributes.qos.m_durability.kind = eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS;
    publisher_command_attributes.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
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
    command_sub_topic_name << pid << "_SUB2PUB";
    subscriber_command_attributes.topic.topicName = command_sub_topic_name.str();
    subscriber_command_attributes.topic.historyQos.kind = eprosima::fastrtps::KEEP_ALL_HISTORY_QOS;
    subscriber_command_attributes.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
    subscriber_command_attributes.qos.m_durability.kind = eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS;
    publisher_command_attributes.qos.m_publishMode.kind = eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE;

    command_subscriber_ = Domain::createSubscriber(participant_, subscriber_command_attributes,
                    &this->command_sub_listener_);

    if (command_subscriber_ == nullptr)
    {
        return false;
    }

    /* Calculate Overhead */
    start_time_ = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        end_time_ = std::chrono::steady_clock::now();
    }
    overhead_time_ = std::chrono::duration<double, std::micro>(end_time_ - start_time_) / 1001;
    cout << "Overhead " << overhead_time_.count() << " ns" << endl;

    /* Create the raw_data_file and add the header */
    if (raw_data_file_ != "")
    {
        raw_sample_count_ = 0;
        std::ofstream data_file;
        data_file.open(raw_data_file_);
        data_file << "Sample,Payload [Bytes],Latency [us]" << std::endl;
    }

    return true;
}

void LatencyTestPublisher::DataPubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    if (info.status == MATCHED_MATCHING)
    {
        std::cout << C_MAGENTA << "Data Pub Matched" << C_DEF << std::endl;

        matched_++;
        if (matched_ > latency_publisher_->subscribers_)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            latency_publisher_->test_status_ = -1;
        }

        ++latency_publisher_->discovery_count_;
    }
    else
    {
        --latency_publisher_->discovery_count_;
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::DataSubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    if (info.status == MATCHED_MATCHING)
    {
        std::cout << C_MAGENTA << "Data Sub Matched" << C_DEF << std::endl;

        matched_++;
        if (matched_ > latency_publisher_->subscribers_)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            latency_publisher_->test_status_ = -1;
        }

        ++latency_publisher_->discovery_count_;
    }
    else
    {
        --latency_publisher_->discovery_count_;
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::CommandPubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    if (info.status == MATCHED_MATCHING)
    {
        std::cout << C_MAGENTA << "Command Pub Matched" << C_DEF << std::endl;

        matched_++;
        if (matched_ > latency_publisher_->subscribers_)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            latency_publisher_->test_status_ = -1;
        }

        ++latency_publisher_->discovery_count_;
    }
    else
    {
        --latency_publisher_->discovery_count_;
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::CommandSubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    if (info.status == MATCHED_MATCHING)
    {
        std::cout << C_MAGENTA << "Command Sub Matched" << C_DEF << std::endl;

        matched_++;
        if (matched_ > latency_publisher_->subscribers_)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            latency_publisher_->test_status_ = -1;
        }

        ++latency_publisher_->discovery_count_;
    }
    else
    {
        --latency_publisher_->discovery_count_;
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::CommandSubListener::onNewDataMessage(
        Subscriber* subscriber)
{
    TestCommandType command;
    SampleInfo_t info;
    if (subscriber->takeNextData((void*)&command, &info))
    {
        if (info.sampleKind == ALIVE)
        {
            if (command.m_command == BEGIN)
            {
                latency_publisher_->mutex_.lock();
                ++latency_publisher_->command_msg_count_;
                latency_publisher_->mutex_.unlock();
                latency_publisher_->command_msg_cv_.notify_one();
            }
        }
    }
    else
    {
        std::cout << "Problem reading" << std::endl;
    }
}

void LatencyTestPublisher::DataSubListener::onNewDataMessage(
        Subscriber* subscriber)
{
    if (latency_publisher_->dynamic_data_)
    {
        subscriber->takeNextData((void*)latency_publisher_->dynamic_data_type_in_, &latency_publisher_->sampleinfo_);
        if (latency_publisher_->dynamic_data_type_in_->get_uint32_value(0) ==
                latency_publisher_->dynamic_data_type_out_->get_uint32_value(0))
        {
            // Factor of 2 below is to calculate the roundtrip divided by two. Note that the overhead does not
            // need to be halved, as we access the clock twice per round trip
            latency_publisher_->end_time_ = std::chrono::steady_clock::now();
            latency_publisher_->times_.push_back(std::chrono::duration<double, std::micro>(
                        latency_publisher_->end_time_ - latency_publisher_->start_time_) / 2. -
                    latency_publisher_->overhead_time_);
            latency_publisher_->received_count_++;

            // Reset seqnum from out data
            latency_publisher_->dynamic_data_type_out_->set_uint32_value(0, 0);

            latency_publisher_->mutex_.lock();
            if (latency_publisher_->data_msg_count_ == 0)
            {
                ++latency_publisher_->data_msg_count_;
                latency_publisher_->data_msg_cv_.notify_one();
            }
            latency_publisher_->mutex_.unlock();
        }
    }
    else
    {
        subscriber->takeNextData((void*)latency_publisher_->latency_type_in_, &latency_publisher_->sampleinfo_);
        if (latency_publisher_->latency_type_in_->seqnum == latency_publisher_->latency_type_out_->seqnum)
        {
            latency_publisher_->end_time_ = std::chrono::steady_clock::now();
            latency_publisher_->times_.push_back(std::chrono::duration<double, std::micro>(
                        latency_publisher_->end_time_ - latency_publisher_->start_time_) / 2. -
                    latency_publisher_->overhead_time_);
            latency_publisher_->received_count_++;

            // Reset seqnum from out data
            latency_publisher_->latency_type_out_->seqnum = 0;

            latency_publisher_->mutex_.lock();
            if (latency_publisher_->data_msg_count_ == 0)
            {
                ++latency_publisher_->data_msg_count_;
                latency_publisher_->data_msg_cv_.notify_one();
            }
            latency_publisher_->mutex_.unlock();
        }
    }
}

void LatencyTestPublisher::run()
{
    // WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    // EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    while (discovery_count_ != (subscribers_ * 4))
    {
        discovery_cv_.wait(disc_lock);
    }
    disc_lock.unlock();
    std::cout << C_B_MAGENTA << "DISCOVERY COMPLETE " << C_DEF << std::endl;

    for (std::vector<uint32_t>::iterator payload = data_size_pub.begin(); payload != data_size_pub.end(); ++payload)
    {
        if (!this->test(*payload))
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (payload != data_size_pub.end() - 1)
        {
            output_file_minimum_ << ",";
            output_file_average_ << ",";
        }
    }
    std::cout << "REMOVING PUBLISHER" << std::endl;
    Domain::removePublisher(this->command_publisher_);
    std::cout << "REMOVING SUBSCRIBER" << std::endl;
    Domain::removeSubscriber(command_subscriber_);

    // Print a summary table with the measurements
    printf("Printing round-trip times in us, statistics for %d samples\n", samples_);
    printf("   Bytes, Samples,   stdev,    mean,     min,     50%%,     90%%,     99%%,  99.99%%,     max\n");
    printf("--------,--------,--------,--------,--------,--------,--------,--------,--------,--------,\n");
    for (uint16_t i = 0; i < stats_.size(); i++)
    {
        print_stats(stats_[i]);
    }

    std::string str_reliable = "besteffort";
    if (reliable_)
    {
        str_reliable = "reliable";
    }

    if (export_csv_)
    {
        std::ofstream out_file;

        std::string prefix = export_prefix_;
        if (prefix.length() == 0)
        {
            prefix = "perf_LatencyTest";
        }

        out_file.open(prefix + "_minimum_" + str_reliable + ".csv");
        out_file << output_file_minimum_.str();
        out_file.close();
        out_file.open(prefix + "_average_" + str_reliable + ".csv");
        out_file << output_file_average_.str();
        out_file.close();
        out_file.open(prefix + "_16_" + str_reliable + ".csv");
        out_file << output_file_16_.str();
        out_file.close();
        out_file.open(prefix + "_32_" + str_reliable + ".csv");
        out_file << output_file_32_.str();
        out_file.close();
        out_file.open(prefix + "_64_" + str_reliable + ".csv");
        out_file << output_file_64_.str();
        out_file.close();
        out_file.open(prefix + "_128_" + str_reliable + ".csv");
        out_file << output_file_128_.str();
        out_file.close();
        out_file.open(prefix + "_256_" + str_reliable + ".csv");
        out_file << output_file_256_.str();
        out_file.close();
        out_file.open(prefix + "_512_" + str_reliable + ".csv");
        out_file << output_file_512_.str();
        out_file.close();
        out_file.open(prefix + "_1024_" + str_reliable + ".csv");
        out_file << output_file_1024_.str();
        out_file.close();
        out_file.open(prefix + "_2048_" + str_reliable + ".csv");
        out_file << output_file_2048_.str();
        out_file.close();
        out_file.open(prefix + "_4096_" + str_reliable + ".csv");
        out_file << output_file_4096_.str();
        out_file.close();
        out_file.open(prefix + "_8192_" + str_reliable + ".csv");
        out_file << output_file_8192_.str();
        out_file.close();
        out_file.open(prefix + "_16384_" + str_reliable + ".csv");
        out_file << output_file_16384_.str();
        out_file.close();
    }
}

bool LatencyTestPublisher::test(
        uint32_t datasize)
{
    test_status_ = 0;
    received_count_ = 0;
    if (dynamic_data_)
    {
        dynamic_data_type_in_ = DynamicDataFactory::get_instance()->create_data(dynamic_type_);
        dynamic_data_type_out_ = DynamicDataFactory::get_instance()->create_data(dynamic_type_);

        MemberId id_in;
        MemberId id_out;
        DynamicData* data_in = dynamic_data_type_in_->loan_value(dynamic_data_type_in_->get_member_id_at_index(1));
        DynamicData* data_out = dynamic_data_type_out_->loan_value(
            dynamic_data_type_out_->get_member_id_at_index(1));
        for (uint32_t i = 0; i < datasize; ++i)
        {
            data_in->insert_sequence_data(id_in);
            data_in->set_byte_value(0, id_in);
            data_out->insert_sequence_data(id_out);
            data_out->set_byte_value(0, id_out);
        }
        dynamic_data_type_in_->return_loaned_value(data_in);
        dynamic_data_type_out_->return_loaned_value(data_out);
    }
    else
    {
        latency_type_in_ = new LatencyType(datasize);
        latency_type_out_ = new LatencyType(datasize);
    }

    times_.clear();
    TestCommandType command;
    command.m_command = READY;
    command_publisher_->write(&command);

    std::unique_lock<std::mutex> lock(mutex_);
    command_msg_cv_.wait(lock, [&]() {
        return command_msg_count_ >= subscribers_;
    });
    command_msg_count_ = 0;
    lock.unlock();

    // The first measurement it's usually not representative, so we take one more and then drop the first one.
    for (unsigned int count = 1; count <= samples_ + 1; ++count)
    {
        if (dynamic_data_)
        {
            dynamic_data_type_in_->set_uint32_value(0, 0);
            dynamic_data_type_out_->set_uint32_value(count, 0);
            start_time_ = std::chrono::steady_clock::now();
            data_publisher_->write((void*)dynamic_data_type_out_);
        }
        else
        {
            latency_type_in_->seqnum = 0;
            latency_type_out_->seqnum = count;
            start_time_ = std::chrono::steady_clock::now();
            data_publisher_->write((void*)latency_type_out_);
        }

        lock.lock();
        data_msg_cv_.wait_for(lock, std::chrono::seconds(1), [&]() {
            return data_msg_count_ > 0;
        });
        data_msg_count_ = 0;
        lock.unlock();
    }

    command.m_command = STOP;
    command_publisher_->write(&command);

    if (test_status_ != 0)
    {
        std::cout << "Error in test " << std::endl;
        return false;
    }

    // TEST FINISHED:
    size_t removed = 0;
    data_publisher_->removeAllChange(&removed);

    // Drop the first measurement, as it's usually not representative
    times_.erase(times_.begin());

    // Log all data to CSV file if specified
    if (raw_data_file_ != "")
    {
        export_raw_data(datasize + 4);
    }

    analyze_times(datasize);

    if (dynamic_data_)
    {
        DynamicDataFactory::get_instance()->delete_data(dynamic_data_type_in_);
        DynamicDataFactory::get_instance()->delete_data(dynamic_data_type_out_);
    }
    else
    {
        delete(latency_type_in_);
        delete(latency_type_out_);
    }

    return true;
}

void LatencyTestPublisher::analyze_times(
        uint32_t datasize)
{
    // Collect statistics
    TimeStats stats;
    stats.bytes_ = datasize + 4;
    stats.received_ = received_count_ - 1;  // Because we are not counting the first one.
    stats.minimum_ = *std::min_element(times_.begin(), times_.end());
    stats.maximum_ = *std::max_element(times_.begin(), times_.end());
    stats.mean_ = std::accumulate(times_.begin(), times_.end(),
                    std::chrono::duration<double, std::micro>(0)).count() / times_.size();

    double aux_stdev = 0;
    for (std::vector<std::chrono::duration<double, std::micro> >::iterator tit = times_.begin(); tit != times_.end();
            ++tit)
    {
        aux_stdev += pow(((*tit).count() - stats.mean_), 2);
    }
    aux_stdev = sqrt(aux_stdev / times_.size());
    stats.stdev_ = aux_stdev;

    /* Percentiles */
    std::sort(times_.begin(), times_.end());

    size_t elem = 0;
    elem = static_cast<size_t>(times_.size() * 0.5);
    if (elem > 0 && elem <= times_.size())
    {
        stats.percentile_50_ = times_.at(--elem).count();
    }
    else
    {
        stats.percentile_50_ = NAN;
    }

    elem = static_cast<size_t>(times_.size() * 0.9);
    if (elem > 0 && elem <= times_.size())
    {
        stats.percentile_90_ = times_.at(--elem).count();
    }
    else
    {
        stats.percentile_90_ = NAN;
    }

    elem = static_cast<size_t>(times_.size() * 0.99);
    if (elem > 0 && elem <= times_.size())
    {
        stats.percentile_99_ = times_.at(--elem).count();
    }
    else
    {
        stats.percentile_99_ = NAN;
    }

    elem = static_cast<size_t>(times_.size() * 0.9999);
    if (elem > 0 && elem <= times_.size())
    {
        stats.percentile_9999_ = times_.at(--elem).count();
    }
    else
    {
        stats.percentile_9999_ = NAN;
    }

    stats_.push_back(stats);
}

void LatencyTestPublisher::print_stats(
        TimeStats& stats)
{
    output_file_minimum_ << "\"" << stats.minimum_.count() << "\"";
    output_file_average_ << "\"" << stats.mean_ << "\"";

    switch (stats.bytes_)
    {
        case 16:
            output_file_16_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 32:
            output_file_32_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 64:
            output_file_64_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 128:
            output_file_128_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 256:
            output_file_256_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 512:
            output_file_512_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 1024:
            output_file_1024_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 2048:
            output_file_2048_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 4096:
            output_file_4096_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 8192:
            output_file_8192_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 16384:
            output_file_16384_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 64000:
            output_file_64000_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        case 131072:
            output_file_131072_ << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;
            break;
        default:
            break;
    }

#ifdef _WIN32
    printf("%8I64u,%8u,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f \n",
            stats.bytes_, stats.received_, stats.stdev_, stats.mean_, stats.minimum_.count(), stats.percentile_50_,
            stats.percentile_90_, stats.percentile_99_, stats.percentile_9999_, stats.maximum_.count());
#else
    printf("%8" PRIu64 ",%8u,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f \n",
            stats.bytes_, stats.received_, stats.stdev_, stats.mean_, stats.minimum_.count(), stats.percentile_50_,
            stats.percentile_90_, stats.percentile_99_, stats.percentile_9999_, stats.maximum_.count());
#endif
}

void LatencyTestPublisher::export_raw_data(
        uint32_t datasize)
{
    std::ofstream data_file;
    data_file.open(raw_data_file_, std::fstream::app);
    for (std::vector<std::chrono::duration<double, std::micro> >::iterator tit = times_.begin(); tit != times_.end();
            ++tit)
    {
        data_file << ++raw_sample_count_ << "," << datasize << "," << (*tit).count() << std::endl;
    }
    data_file.close();
}
