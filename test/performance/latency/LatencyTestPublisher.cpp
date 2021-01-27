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

#include <inttypes.h>

#include <numeric>
#include <cmath>
#include <fstream>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>


#define TIME_LIMIT_US 10000

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;
using namespace std;

LatencyTestPublisher::LatencyTestPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , data_writer_(nullptr)
    , command_writer_(nullptr)
    , subscriber_(nullptr)
    , data_reader_(nullptr)
    , command_reader_(nullptr)
    , overhead_time_(0.0)
    , command_msg_count_(0)
    , data_msg_count_(0)
    , received_count_(0)
    , test_status_(0)
    , subscribers_(0)
    , samples_(0)
    , latency_data_sub_topic_(nullptr)
    , latency_data_pub_topic_(nullptr)
    , latency_command_sub_topic_(nullptr)
    , latency_command_pub_topic_(nullptr)
    , latency_type_in_(nullptr)
    , latency_type_out_(nullptr)
    , latency_command_type_(new TestCommandDataType())
    , dynamic_data_type_in_(nullptr)
    , dynamic_data_type_out_(nullptr)
    , data_writer_listener_(this)
    , data_reader_listener_(this)
    , command_writer_listener_(this)
    , command_reader_listener_(this)
{
    forced_domain_ = -1;
    export_prefix_ = "";
    raw_data_file_ = "";
}

LatencyTestPublisher::~LatencyTestPublisher()
{
    subscriber_->delete_datareader(data_reader_);
    subscriber_->delete_datareader(command_reader_);
    participant_->delete_subscriber(subscriber_);

    publisher_->delete_datawriter(data_writer_);
    publisher_->delete_datawriter(command_writer_);
    participant_->delete_publisher(publisher_);

    participant_->delete_topic(latency_data_sub_topic_);
    participant_->delete_topic(latency_data_pub_topic_);
    participant_->delete_topic(latency_command_sub_topic_);
    participant_->delete_topic(latency_command_pub_topic_);

    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool LatencyTestPublisher::init(
        int subscribers,
        int samples,
        bool reliable,
        uint32_t pid,
        bool hostname,
        bool export_csv,
        const string& export_prefix,
        string raw_data_file,
        const PropertyPolicy& part_property_policy,
        const PropertyPolicy& property_policy,
        const string& xml_config_file,
        bool dynamic_data,
        int forced_domain,
        LatencyDataSizes& latency_data_sizes)
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

    data_size_pub_ = latency_data_sizes.sample_sizes();

    // Init dynamic data
    if (dynamic_data_)
    {
        // Create basic builders
        DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

        // Add members to the struct.
        struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
        struct_type_builder->add_member(1, "data",
                DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                    DynamicTypeBuilderFactory::get_instance()->create_byte_type(), data_size_pub_.back()
                    ));
        struct_type_builder->set_name("LatencyType");
        dynamic_pub_sub_type_.reset(new DynamicPubSubType(struct_type_builder->build()));
    }
    else
    {
        // Create the static builder
        latency_data_type_.reset(new LatencyDataType());
    }

    // Init output files
    output_files_.push_back(make_shared<stringstream>());
    output_files_.push_back(make_shared<stringstream>());

    uint32_t data_index = DATA_BASE_INDEX;

    for (vector<uint32_t>::iterator it = data_size_pub_.begin(); it != data_size_pub_.end(); ++it)
    {
        // Reliability
        string str_reliable = reliable_ ? "reliable" : "besteffort";

        // Summary files
        *output_files_[MINIMUM_INDEX] << "\"" << samples_ << " samples of " << *it + 4 << " bytes (us)\"";
        *output_files_[AVERAGE_INDEX] << "\"" << samples_ << " samples of " << *it + 4 << " bytes (us)\"";

        if (it != data_size_pub_.end() - 1)
        {
            *output_files_[MINIMUM_INDEX] << ",";
            *output_files_[AVERAGE_INDEX] << ",";
        }

        output_files_.push_back(make_shared<stringstream>());
        *output_files_[data_index] << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
        *output_files_[data_index] << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << endl;

        data_index++;
    }

    *output_files_[MINIMUM_INDEX] << endl;
    *output_files_[AVERAGE_INDEX] << endl;

    /* Create DomainParticipant*/
    string participant_profile_name = "pub_participant_profile";
    DomainParticipantQos pqos;

    // Default domain
    DomainId_t domainId = pid % 230;

    // Default participant name
    pqos.name("latency_test_publisher");

    // Load XML configuration
    if (xml_config_file_.length() > 0)
    {
        if ( ReturnCode_t::RETCODE_OK !=
                DomainParticipantFactory::get_instance()->
                        get_participant_qos_from_profile(
                    participant_profile_name,
                    pqos))
        {
            return false;
        }
    }

    // Apply user's force domain
    if (forced_domain_ >= 0)
    {
        domainId = forced_domain_;
    }

    // If the user has specified a participant property policy with command line arguments, it overrides whatever the
    // XML configures.
    if (PropertyPolicyHelper::length(part_property_policy) > 0)
    {
        pqos.properties(part_property_policy);
    }

    // Create the participant
    participant_ =
            DomainParticipantFactory::get_instance()->create_participant(domainId, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the data type
    ReturnCode_t res(ReturnCode_t::RETCODE_OK);

    if (dynamic_data_)
    {
        res = dynamic_pub_sub_type_.register_type(participant_);
    }
    else
    {
        res = latency_data_type_.register_type(participant_);
    }

    if (ReturnCode_t::RETCODE_OK != res)
    {
        return false;
    }

    // Register the command type
    res = latency_command_type_.register_type(participant_);

    if (ReturnCode_t::RETCODE_OK != res)
    {
        return false;
    }

    /* Create Publisher */
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        return false;
    }

    /* Create Subscriber */
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        return false;
    }

    /* Create Topics */
    {
        ostringstream topic_name;
        topic_name << "LatencyTest_";
        if (hostname)
        {
            topic_name << asio::ip::host_name() << "_";
        }
        topic_name << pid << "_PUB2SUB";

        latency_data_pub_topic_ = participant_->create_topic(
            topic_name.str(),
            "LatencyType",
            TOPIC_QOS_DEFAULT);

        if (latency_data_pub_topic_ == nullptr)
        {
            return false;
        }

        topic_name.str("");
        topic_name.clear();
        topic_name << "LatencyTest_";

        if (hostname)
        {
            topic_name << asio::ip::host_name() << "_";
        }
        topic_name << pid << "_SUB2PUB";

        latency_data_sub_topic_ = participant_->create_topic(
            topic_name.str(),
            "LatencyType",
            TOPIC_QOS_DEFAULT);

        if (latency_data_sub_topic_ == nullptr)
        {
            return false;
        }

        topic_name.str("");
        topic_name.clear();
        topic_name << "LatencyTest_Command_";

        if (hostname)
        {
            topic_name << asio::ip::host_name() << "_";
        }
        topic_name << pid << "_PUB2SUB";

        latency_command_pub_topic_ = participant_->create_topic(
            topic_name.str(),
            "TestCommandType",
            TOPIC_QOS_DEFAULT);

        if (latency_command_pub_topic_ == nullptr)
        {
            return false;
        }

        topic_name.str("");
        topic_name.clear();
        topic_name << "LatencyTest_Command_";

        if (hostname)
        {
            topic_name << asio::ip::host_name() << "_";
        }
        topic_name << pid << "_SUB2PUB";

        latency_command_sub_topic_ = participant_->create_topic(
            topic_name.str(),
            "TestCommandType",
            TOPIC_QOS_DEFAULT);

        if (latency_command_sub_topic_ == nullptr)
        {
            return false;
        }
    }

    /* Create Data DataWriter */
    {
        string profile_name = "pub_publisher_profile";

        if (xml_config_file_.length() > 0)
        {
            data_writer_ = publisher_->create_datawriter_with_profile(
                latency_data_pub_topic_,
                profile_name,
                &data_writer_listener_);
        }
        else
        {
            DataWriterQos dw_qos;

            if (reliable)
            {
                RTPSReliableWriterQos rw_qos;
                rw_qos.times.heartbeatPeriod.seconds = 0;
                rw_qos.times.heartbeatPeriod.nanosec = 100000000;
                dw_qos.reliable_writer_qos(rw_qos);
            }
            else
            {
                ReliabilityQosPolicy rp;
                rp.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
                dw_qos.reliability(rp);
            }

            dw_qos.properties(property_policy);
            dw_qos.endpoint().history_memory_policy = MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

            data_writer_ = publisher_->create_datawriter(
                latency_data_pub_topic_,
                dw_qos,
                &data_writer_listener_);
        }

        if (data_writer_ == nullptr)
        {
            return false;
        }
    }

    /* Create Data Echo Reader */
    {
        string profile_name = "pub_subscriber_profile";

        if (xml_config_file_.length() > 0)
        {
            data_reader_ = subscriber_->create_datareader_with_profile(
                latency_data_sub_topic_,
                profile_name,
                &data_reader_listener_);
        }
        else
        {
            DataReaderQos dr_qos;

            if (reliable)
            {
                ReliabilityQosPolicy rp;
                rp.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
                dr_qos.reliability(rp);
            }

            dr_qos.properties(property_policy);
            dr_qos.endpoint().history_memory_policy = MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

            data_reader_ = subscriber_->create_datareader(
                latency_data_sub_topic_,
                dr_qos,
                &data_reader_listener_);
        }

        if (data_reader_ == nullptr)
        {
            return false;
        }
    }

    /* Create Command Writer */
    DataWriterQos cw_qos;
    cw_qos.history().kind = KEEP_ALL_HISTORY_QOS;
    cw_qos.durability().durabilityKind(TRANSIENT_LOCAL);
    cw_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    cw_qos.publish_mode().kind = SYNCHRONOUS_PUBLISH_MODE;

    command_writer_ = publisher_->create_datawriter(
        latency_command_pub_topic_,
        cw_qos,
        &command_writer_listener_);

    if (command_writer_ == nullptr)
    {
        return false;
    }

    /* Create Command Reader */
    {
        DataReaderQos cr_qos;
        cr_qos.history().kind = KEEP_ALL_HISTORY_QOS;
        cr_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        cr_qos.durability().durabilityKind(TRANSIENT_LOCAL);

        command_reader_ = subscriber_->create_datareader(
            latency_command_sub_topic_,
            cr_qos,
            &command_reader_listener_);

        if (command_reader_ == nullptr)
        {
            return false;
        }
    }

    /* Calculate Overhead */
    start_time_ = chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        end_time_ = chrono::steady_clock::now();
    }
    overhead_time_ = chrono::duration<double, micro>(end_time_ - start_time_) / 1001;
    cout << "Overhead " << overhead_time_.count() << " ns" << endl;

    /* Create the raw_data_file and add the header */
    if (raw_data_file_ != "")
    {
        raw_sample_count_ = 0;
        ofstream data_file;
        data_file.open(raw_data_file_);
        data_file << "Sample,Payload [Bytes],Latency [us]" << endl;
    }

    return true;
}

/*
 * Our current inplementation of MatchedStatus info:
 * - total_count(_change) holds the actual number of matches
 * - current_count(_change) is a flag to signal match or unmatch.
 *   (TODO: review if fits standard definition)
 * */

void LatencyTestPublisher::LatencyDataWriterListener::on_publication_matched(
        DataWriter* writer,
        const PublicationMatchedStatus& info)
{
    (void)writer;

    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        logInfo(LatencyTest, C_MAGENTA << "Data Pub Matched" << C_DEF);
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::LatencyDataReaderListener::on_subscription_matched(
        DataReader* reader,
        const SubscriptionMatchedStatus& info)
{
    (void)reader;

    lock_guard<mutex> lock(latency_publisher_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        logInfo(LatencyTest, C_MAGENTA << "Data Sub Matched" << C_DEF);
    }

    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::ComandWriterListener::on_publication_matched(
        DataWriter* writer,
        const PublicationMatchedStatus& info)
{
    (void)writer;

    lock_guard<mutex> lock(latency_publisher_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        logInfo(LatencyTest, C_MAGENTA << "Command Pub Matched" << C_DEF);
    }

    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::CommandReaderListener::on_subscription_matched(
        DataReader* reader,
        const SubscriptionMatchedStatus& info)
{
    (void)reader;

    lock_guard<mutex> lock(latency_publisher_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        logInfo(LatencyTest, C_MAGENTA << "Command Sub Matched" << C_DEF);
    }

    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::CommandReaderListener::on_data_available(
        DataReader* reader)
{
    TestCommandType command;
    SampleInfo info;

    if (reader->take_next_sample(
                &command, &info) == ReturnCode_t::RETCODE_OK
            && info.valid_data)
    {
        if (command.m_command == BEGIN)
        {
            lock_guard<mutex> lock(latency_publisher_->mutex_);
            ++latency_publisher_->command_msg_count_;
            latency_publisher_->command_msg_cv_.notify_one();
        }
    }
    else
    {
        logInfo(LatencyTest, "Problem reading command message");
    }
}

void LatencyTestPublisher::LatencyDataReaderListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    void* data = latency_publisher_->dynamic_data_ ?
            (void*)latency_publisher_->dynamic_data_type_in_:
            (void*)latency_publisher_->latency_type_in_;

    // Retrieved echoed data
    if (reader->take_next_sample(
                data, &info) != ReturnCode_t::RETCODE_OK
            || !info.valid_data)
    {
        logInfo(LatencyTest, "Problem reading Subscriber echoed test data");
        return;
    }

    lock_guard<mutex> lock(latency_publisher_->mutex_);

    // Check if is the expected echo message
    if ((latency_publisher_->dynamic_data_ &&
            (latency_publisher_->dynamic_data_type_in_->get_uint32_value(0) !=
            latency_publisher_->dynamic_data_type_out_->get_uint32_value(0)))
            || (!latency_publisher_->dynamic_data_ &&
            (latency_publisher_->latency_type_in_->seqnum != latency_publisher_->latency_type_out_->seqnum)))
    {
        return;
    }

    // Factor of 2 below is to calculate the roundtrip divided by two. Note that the overhead does not
    // need to be halved, as we access the clock twice per round trip
    latency_publisher_->end_time_ = chrono::steady_clock::now();
    latency_publisher_->times_.push_back(chrono::duration<double, micro>(
                latency_publisher_->end_time_ - latency_publisher_->start_time_) / 2. -
            latency_publisher_->overhead_time_);
    ++latency_publisher_->received_count_;

    // Reset seqnum from out data
    if (latency_publisher_->dynamic_data_)
    {
        latency_publisher_->dynamic_data_type_out_->set_uint32_value(0, 0);
    }
    else
    {
        latency_publisher_->latency_type_out_->seqnum = 0;
    }

    ++latency_publisher_->data_msg_count_;
    if (latency_publisher_->data_msg_count_ >= latency_publisher_->subscribers_)
    {
        latency_publisher_->data_msg_cv_.notify_one();
    }
}

void LatencyTestPublisher::run()
{
    // WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    // EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    unique_lock<mutex> disc_lock(mutex_);
    discovery_cv_.wait(
        disc_lock,
        [this]() -> bool
        {
            return total_matches() == 4 * subscribers_;
        });
    disc_lock.unlock();

    logInfo(LatencyTest, C_B_MAGENTA << "Pub: DISCOVERY COMPLETE " << C_DEF);

    for (vector<uint32_t>::iterator payload = data_size_pub_.begin(); payload != data_size_pub_.end(); ++payload)
    {
        if (!test(*payload))
        {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
        if (payload != data_size_pub_.end() - 1)
        {
            *output_files_[MINIMUM_INDEX] << ",";
            *output_files_[AVERAGE_INDEX] << ",";
        }
    }

    string str_reliable = reliable_ ? "reliable" : "besteffort";

    // Print a summary table with the measurements
    printf("Printing round-trip times in us, statistics for %d samples\n", samples_);
    printf("   Bytes, Samples,   stdev,    mean,     min,     50%%,     90%%,     99%%,  99.99%%,     max\n");
    printf("--------,--------,--------,--------,--------,--------,--------,--------,--------,--------,\n");
    for (uint16_t i = 0; i < stats_.size(); i++)
    {
        print_stats(DATA_BASE_INDEX + i, stats_[i]);

        if (export_csv_)
        {
            export_csv("_" + to_string(stats_[i].bytes_) + "_", str_reliable, *output_files_[i + 2]);
        }
    }

    if (export_csv_)
    {
        export_csv("_minimum_", str_reliable, *output_files_[MINIMUM_INDEX]);
        export_csv("_average_", str_reliable, *output_files_[AVERAGE_INDEX]);
    }
}

void LatencyTestPublisher::export_csv(
        const string& data_name,
        const string& str_reliable,
        const stringstream& data_stream)
{
    ofstream out_file;

    string prefix = export_prefix_;
    if (prefix.length() == 0)
    {
        prefix = "perf_LatencyTest";
    }

    out_file.open(prefix + data_name + str_reliable + ".csv");
    out_file << data_stream.str();
    out_file.close();
}

bool LatencyTestPublisher::test(
        uint32_t datasize)
{
    test_status_ = 0;
    received_count_ = 0;
    if (dynamic_data_)
    {
        dynamic_data_type_in_ = static_cast<DynamicData*>(dynamic_pub_sub_type_->createData());
        dynamic_data_type_out_ = static_cast<DynamicData*>(dynamic_pub_sub_type_->createData());

        MemberId id_in;
        MemberId id_out;
        DynamicData* data_in = dynamic_data_type_in_->loan_value(
            dynamic_data_type_in_->get_member_id_at_index(1));
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
        latency_type_in_ = static_cast<LatencyType*>(latency_data_type_->createData());
        latency_type_out_ = static_cast<LatencyType*>(latency_data_type_->createData());
        latency_type_in_->data.resize(datasize, 0);
        latency_type_out_->data.resize(datasize, 0);
    }

    times_.clear();
    TestCommandType command;
    command.m_command = READY;
    if (!command_writer_->write(&command))
    {
        logError(LatencyTest, "Publisher cannot publish READY command");
        return false;
    }

    // Wait for Subscriber's BEGIN command
    {
        unique_lock<mutex> lock(mutex_);
        command_msg_cv_.wait(lock, [&]()
                {
                    return command_msg_count_ >= subscribers_;
                });
        command_msg_count_ = 0;
    }

    // The first measurement it's usually not representative, so we take one more and then drop the first one.
    for (unsigned int count = 1; count <= samples_ + 1; ++count)
    {
        void* data = nullptr;

        if (dynamic_data_)
        {
            dynamic_data_type_in_->set_uint32_value(0, 0);
            dynamic_data_type_out_->set_uint32_value(count, 0);
            data = dynamic_data_type_out_;
        }
        else
        {
            latency_type_in_->seqnum = 0;
            latency_type_out_->seqnum = count;
            data = latency_type_out_;
        }

        start_time_ = chrono::steady_clock::now();
        data_writer_->write(data);

        unique_lock<mutex> lock(mutex_);
        // the wait timeouts due possible message leaks
        data_msg_cv_.wait_for(lock,
                chrono::milliseconds(4),
                [&]()
                {
                    return data_msg_count_ >= subscribers_;
                });
        data_msg_count_ = 0;
    }

    command.m_command = STOP;
    command_writer_->write(&command);

    if (test_status_ != 0)
    {
        logError(LatencyTest, "Error in test");
        return false;
    }

    // TEST FINISHED:
    size_t removed = 0;
    data_writer_->clear_history(&removed);

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
        dynamic_pub_sub_type_->deleteData(dynamic_data_type_in_);
        dynamic_pub_sub_type_->deleteData(dynamic_data_type_out_);
    }
    else
    {
        latency_data_type_->deleteData(latency_type_in_);
        latency_data_type_->deleteData(latency_type_out_);
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
    stats.minimum_ = *min_element(times_.begin(), times_.end());
    stats.maximum_ = *max_element(times_.begin(), times_.end());
    stats.mean_ = accumulate(times_.begin(), times_.end(),
                    chrono::duration<double, micro>(0)).count() / times_.size();

    double aux_stdev = 0;
    for (vector<chrono::duration<double, micro>>::iterator tit = times_.begin(); tit != times_.end();
            ++tit)
    {
        aux_stdev += pow(((*tit).count() - stats.mean_), 2);
    }
    aux_stdev = sqrt(aux_stdev / times_.size());
    stats.stdev_ = aux_stdev;

    /* Percentiles */
    sort(times_.begin(), times_.end());

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
        uint32_t data_index,
        TimeStats& stats)
{
    *output_files_[MINIMUM_INDEX] << "\"" << stats.minimum_.count() << "\"";
    *output_files_[AVERAGE_INDEX] << "\"" << stats.mean_ << "\"";
    *output_files_[data_index] << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << endl;


#ifdef _WIN32
    printf("%8I64u,%8u,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f \n",
            stats.bytes_, stats.received_, stats.stdev_, stats.mean_, stats.minimum_.count(), stats.percentile_50_,
            stats.percentile_90_, stats.percentile_99_, stats.percentile_9999_, stats.maximum_.count());
#else
    printf("%8" PRIu64 ",%8u,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f \n",
            stats.bytes_, stats.received_, stats.stdev_, stats.mean_, stats.minimum_.count(), stats.percentile_50_,
            stats.percentile_90_, stats.percentile_99_, stats.percentile_9999_, stats.maximum_.count());
#endif // ifdef _WIN32
}

void LatencyTestPublisher::export_raw_data(
        uint32_t datasize)
{
    ofstream data_file;
    data_file.open(raw_data_file_, fstream::app);
    for (vector<chrono::duration<double, micro>>::iterator tit = times_.begin(); tit != times_.end();
            ++tit)
    {
        data_file << ++raw_sample_count_ << "," << datasize << "," << (*tit).count() << endl;
    }
    data_file.close();
}

int32_t LatencyTestPublisher::total_matches() const
{
    // no need to lock because is used always within a
    // condition variable wait predicate

    int32_t count = data_writer_listener_.matched_
            + data_reader_listener_.matched_
            + command_writer_listener_.matched_
            + command_reader_listener_.matched_;

    // Each endpoint has a mirror counterpart in the LatencyTestPublisher
    // thus, the maximun number of matches is 4 * total number of subscribers
    assert(count >= 0 && count <= 4 * subscribers_);
    return count;
}
