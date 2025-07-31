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

#include <chrono>
#include <cinttypes>
#include <cmath>
#include <fstream>
#include <numeric>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

#define TIME_LIMIT_US 10000

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

LatencyTestPublisher::LatencyTestPublisher()
    : latency_command_type_(new TestCommandDataType())
    , data_writer_listener_(this)
    , data_reader_listener_(this)
    , command_writer_listener_(this)
    , command_reader_listener_(this)
{
}

LatencyTestPublisher::~LatencyTestPublisher()
{
    std::string TestCommandType("TestCommandType");
    participant_->unregister_type(TestCommandType);

    DomainParticipantFactory::get_instance()->delete_participant(participant_);

    EPROSIMA_LOG_INFO(LatencyTest, "Pub: Participant removed");
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
        const std::string& xml_config_file,
        bool dynamic_data,
        Arg::EnablerValue data_sharing,
        bool data_loans,
        Arg::EnablerValue shared_memory,
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
    dynamic_types_ = dynamic_data;
    data_sharing_ = data_sharing;
    data_loans_ = data_loans;
    shared_memory_ = shared_memory;
    forced_domain_ = forced_domain;
    raw_data_file_ = raw_data_file;
    pid_ = pid;
    hostname_ = hostname;

    data_size_pub_ = latency_data_sizes.sample_sizes();

    // Init output files
    output_files_.push_back(std::make_shared<std::stringstream>());
    output_files_.push_back(std::make_shared<std::stringstream>());

    uint32_t data_index = DATA_BASE_INDEX;

    for (std::vector<uint32_t>::iterator it = data_size_pub_.begin(); it != data_size_pub_.end(); ++it)
    {
        // Reliability
        std::string str_reliable = reliable_ ? "reliable" : "besteffort";

        // Summary files
        *output_files_[MINIMUM_INDEX] << "\"" << samples_ << " samples of " << *it << " bytes (us)\"";
        *output_files_[AVERAGE_INDEX] << "\"" << samples_ << " samples of " << *it << " bytes (us)\"";

        if (it != data_size_pub_.end() - 1)
        {
            *output_files_[MINIMUM_INDEX] << ",";
            *output_files_[AVERAGE_INDEX] << ",";
        }

        output_files_.push_back(std::make_shared<std::stringstream>());
        *output_files_[data_index] << "\"Minimum of " << samples_ << " samples (" << str_reliable << ")\",";
        *output_files_[data_index] << "\"Average of " << samples_ << " samples (" << str_reliable << ")\"" << std::endl;

        data_index++;
    }

    *output_files_[MINIMUM_INDEX] << std::endl;
    *output_files_[AVERAGE_INDEX] << std::endl;

    /* Create DomainParticipant*/
    std::string participant_profile_name = "pub_participant_profile";
    DomainParticipantQos pqos;

    // Default domain
    DomainId_t domainId = pid % 230;

    // Default participant name
    pqos.name("latency_test_publisher");

    // Load XML configuration
    if (xml_config_file_.length() > 0)
    {
        if ( RETCODE_OK !=
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

    // Set shared memory transport if it was enable/disable explicitly.
    if (Arg::EnablerValue::ON == shared_memory_)
    {
        std::shared_ptr<eprosima::fastdds::rtps::SharedMemTransportDescriptor> shm_transport =
                std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
        std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> udp_transport =
                std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
        pqos.transport().user_transports.push_back(shm_transport);
        pqos.transport().user_transports.push_back(udp_transport);
        pqos.transport().use_builtin_transports = false;
    }
    else if (Arg::EnablerValue::OFF == shared_memory_)
    {
        std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> udp_transport =
                std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
        pqos.transport().user_transports.push_back(udp_transport);
        pqos.transport().use_builtin_transports = false;
    }

    // Create the participant
    participant_ =
            DomainParticipantFactory::get_instance()->create_participant(domainId, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the command type
    if (RETCODE_OK != latency_command_type_.register_type(participant_))
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR registering the COMMAND type");
        return false;
    }

    /* Create Publisher */
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR creating PUBLISHER");
        return false;
    }

    /* Create Subscriber */
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR creating SUBSCRIBER");
        return false;
    }

    /* Create Data DataWriter and DataReader QoS Profile*/
    {
        // Read profile from XML
        if (xml_config_file_.length() > 0)
        {
            std::string pub_profile_name = "pub_publisher_profile";
            std::string sub_profile_name = "pub_subscriber_profile";

            if (RETCODE_OK != publisher_->get_datawriter_qos_from_profile(pub_profile_name, dw_qos_))
            {
                EPROSIMA_LOG_ERROR(LATENCYPUBLISHER,
                        "ERROR unable to retrive the " << pub_profile_name << "from XML file");
                return false;
            }

            if (RETCODE_OK != subscriber_->get_datareader_qos_from_profile(sub_profile_name, dr_qos_))
            {
                EPROSIMA_LOG_ERROR(LATENCYPUBLISHER,
                        "ERROR unable to retrive the " << sub_profile_name << "from XML file");
            }
        }
        // Create QoS Profiles
        else
        {
            ReliabilityQosPolicy rp;
            if (reliable)
            {
                rp.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;

                RTPSReliableWriterQos rw_qos;
                rw_qos.times.heartbeat_period.seconds = 0;
                rw_qos.times.heartbeat_period.nanosec = 100000000;
                dw_qos_.reliable_writer_qos(rw_qos);
            }
            else
            {
                rp.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
            }

            dw_qos_.reliability(rp);
            dr_qos_.reliability(rp);

            dw_qos_.properties(property_policy);
            dw_qos_.endpoint().history_memory_policy = MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

            dr_qos_.properties(property_policy);
            dr_qos_.endpoint().history_memory_policy = MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        }

        // Set data sharing according with cli.
        if (Arg::EnablerValue::ON == data_sharing_)
        {
            DataSharingQosPolicy dsp;
            dsp.on("");
            dw_qos_.data_sharing(dsp);
            dr_qos_.data_sharing(dsp);
        }
        else if (Arg::EnablerValue::OFF == data_sharing_)
        {
            DataSharingQosPolicy dsp;
            dsp.off();
            dw_qos_.data_sharing(dsp);
            dr_qos_.data_sharing(dsp);
        }

        // Increase payload pool size to prevent loan failures due to outages
        if (data_loans_)
        {
            dw_qos_.resource_limits().extra_samples = 30;
            dr_qos_.resource_limits().extra_samples = 30;
        }
    }

    /* Create Topics */
    {
        std::ostringstream topic_name;
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
    start_time_ = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        end_time_ = std::chrono::steady_clock::now();
    }
    overhead_time_ = std::chrono::duration<double, std::micro>(end_time_ - start_time_) / 1001;
    std::cout << "Overhead " << overhead_time_.count() << " us" << std::endl;

    /* Create the raw_data_file and add the header */
    if (raw_data_file_ != "")
    {
        raw_sample_count_ = 0;
        std::ofstream data_file;
        data_file.open(raw_data_file_);
        data_file << "Sample,Payload [Bytes],Latency [us]" << std::endl;
    }

    // Endpoints using dynamic data endpoints span the whole test duration
    // Static types and endpoints are created for each payload iteration
    return dynamic_types_ ? init_dynamic_types() && create_data_endpoints() : true;
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
        EPROSIMA_LOG_INFO(LatencyTest, C_MAGENTA << "Data Pub Matched" << C_DEF);
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::LatencyDataReaderListener::on_subscription_matched(
        DataReader* reader,
        const SubscriptionMatchedStatus& info)
{
    (void)reader;

    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        EPROSIMA_LOG_INFO(LatencyTest, C_MAGENTA << "Data Sub Matched" << C_DEF);
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::ComandWriterListener::on_publication_matched(
        DataWriter* writer,
        const PublicationMatchedStatus& info)
{
    (void)writer;

    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        EPROSIMA_LOG_INFO(LatencyTest, C_MAGENTA << "Command Pub Matched" << C_DEF);
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::CommandReaderListener::on_subscription_matched(
        DataReader* reader,
        const SubscriptionMatchedStatus& info)
{
    (void)reader;

    std::unique_lock<std::mutex> lock(latency_publisher_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        EPROSIMA_LOG_INFO(LatencyTest, C_MAGENTA << "Command Sub Matched" << C_DEF);
    }

    lock.unlock();
    latency_publisher_->discovery_cv_.notify_one();
}

void LatencyTestPublisher::CommandReaderListener::on_data_available(
        DataReader* reader)
{
    TestCommandType command;
    SampleInfo info;

    if (reader->take_next_sample(
                &command, &info) == RETCODE_OK
            && info.valid_data)
    {
        if (command.m_command == BEGIN
                || command.m_command == END )
        {
            latency_publisher_->mutex_.lock();
            ++latency_publisher_->command_msg_count_;
            latency_publisher_->mutex_.unlock();
            latency_publisher_->command_msg_cv_.notify_one();
        }
    }
    else
    {
        EPROSIMA_LOG_INFO(LatencyTest, "Problem reading command message");
    }
}

void LatencyTestPublisher::LatencyDataReaderListener::on_data_available(
        DataReader* reader)
{
    auto pub = latency_publisher_;

    SampleInfoSeq infos;
    LoanableSequence<LatencyType> data_seq;
    std::chrono::duration<uint32_t, std::nano> bounce_time(0);

    if (pub->data_loans_)
    {
        if (RETCODE_OK != reader->take(data_seq, infos, 1))
        {
            EPROSIMA_LOG_ERROR(LatencyTest, "Problem reading Subscriber echoed loaned test data");
            return;
        }
    }
    else
    {
        SampleInfo info;
        void* data = pub->dynamic_types_ ?
                (void*)pub->dynamic_data_in_:
                (void*)pub->latency_data_in_;

        // Retrieved echoed data
        if (reader->take_next_sample(
                    data, &info) != RETCODE_OK
                || !info.valid_data)
        {
            EPROSIMA_LOG_ERROR(LatencyTest, "Problem reading Subscriber echoed test data");
            return;
        }
    }

    // Atomic managemente of the sample
    bool notify = false;
    {
        std::lock_guard<std::mutex> lock(pub->mutex_);

        if (pub->data_loans_)
        {
            // we have requested a single sample
            assert(infos.length() == 1 && data_seq.length() == 1);
            // we have already released the former loan
            assert(pub->latency_data_in_ == nullptr);
            // reference the loaned data
            pub->latency_data_in_ = &data_seq[0];
            // retrieve the bounce time
            bounce_time = std::chrono::duration<uint32_t, std::nano>(pub->latency_data_in_->bounce);
        }

        // Check if is the expected echo message
        uint32_t dyn_value_in {0};
        uint32_t dyn_value_out {0};
        if (pub->dynamic_types_)
        {
            (*pub->dynamic_data_in_)->get_uint32_value(dyn_value_in, 0);
            (*pub->dynamic_data_out_)->get_uint32_value(dyn_value_out, 0);
        }

        if ((pub->dynamic_types_ && dyn_value_in != dyn_value_out)
                || (!pub->dynamic_types_ && pub->latency_data_in_->seqnum != pub->latency_data_out_->seqnum))
        {
            EPROSIMA_LOG_INFO(LatencyTest, "Echo message received is not the expected one");
        }
        else
        {
            // Factor of 2 below is to calculate the roundtrip divided by two. Note that nor the overhead does not
            // need to be halved, as we access the clock twice per round trip
            pub->end_time_ = std::chrono::steady_clock::now();
            pub->end_time_ -= bounce_time;
            auto roundtrip = std::chrono::duration<double, std::micro>(pub->end_time_ - pub->start_time_) / 2.0;
            roundtrip -= pub->overhead_time_;

            // Discard samples were loan failed due to payload outages
            // in that case the roundtrip will match the os scheduler quantum slice
            if (roundtrip.count() > 0
                    && !(pub->data_loans_ && roundtrip.count() > 10000))
            {
                pub->times_.push_back(roundtrip);
                ++pub->received_count_;
            }

            // Reset seqnum from out data
            if (pub->dynamic_types_)
            {
                (*pub->dynamic_data_out_)->set_uint32_value(0, 0);
            }
            else
            {
                pub->latency_data_out_->seqnum = 0;
            }
        }

        if (pub->data_loans_)
        {
            pub->latency_data_in_ = nullptr;
        }

        ++pub->data_msg_count_;
        notify = pub->data_msg_count_ >= pub->subscribers_;
    }

    if (notify)
    {
        pub->data_msg_cv_.notify_one();
    }

    // release the loan if any
    if (pub->data_loans_
            && RETCODE_OK != reader->return_loan(data_seq, infos))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "Problem returning loaned test data");
    }
}

void LatencyTestPublisher::run()
{
    for (std::vector<uint32_t>::iterator payload = data_size_pub_.begin(); payload != data_size_pub_.end(); ++payload)
    {
        if (!test(*payload))
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (payload != data_size_pub_.end() - 1)
        {
            *output_files_[MINIMUM_INDEX] << ",";
            *output_files_[AVERAGE_INDEX] << ",";
        }
    }

    std::string str_reliable = reliable_ ? "reliable" : "besteffort";

    // Print a summary table with the measurements
    printf("Printing round-trip times in us, statistics for %d samples\n", samples_);
    printf("   Bytes, Samples,   stdev,    mean,     min,     50%%,     90%%,     99%%,  99.99%%,     max\n");
    printf("--------,--------,--------,--------,--------,--------,--------,--------,--------,--------,\n");
    for (uint16_t i = 0; i < stats_.size(); i++)
    {
        print_stats(DATA_BASE_INDEX + i, stats_[i]);

        if (export_csv_)
        {
            export_csv("_" + std::to_string(stats_[i].bytes_) + "_", str_reliable, *output_files_[i + 2]);
        }
    }

    if (export_csv_)
    {
        export_csv("_minimum_", str_reliable, *output_files_[MINIMUM_INDEX]);
        export_csv("_average_", str_reliable, *output_files_[AVERAGE_INDEX]);
    }
}

void LatencyTestPublisher::destroy_user_entities()
{
    // Static type endpoints should have been removed for each payload iteration
    if (dynamic_types_)
    {
        destroy_data_endpoints();
    }
    else if (nullptr != data_writer_
            || nullptr != data_reader_
            || nullptr != latency_data_pub_topic_
            || nullptr != latency_data_sub_topic_
            || !latency_data_type_)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR unregistering the DATA type and/or removing the endpoints");
    }

    subscriber_->delete_datareader(command_reader_);
    participant_->delete_subscriber(subscriber_);

    publisher_->delete_datawriter(command_writer_);
    participant_->delete_publisher(publisher_);

    participant_->delete_topic(latency_command_sub_topic_);
    participant_->delete_topic(latency_command_pub_topic_);
}

void LatencyTestPublisher::export_csv(
        const std::string& data_name,
        const std::string& str_reliable,
        const std::stringstream& data_stream)
{
    std::ofstream out_file;

    std::string prefix = export_prefix_;
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

    if (dynamic_types_)
    {
        dynamic_data_in_ = static_cast<DynamicData::_ref_type*>(dynamic_pub_sub_type_->create_data());
        dynamic_data_out_ = static_cast<DynamicData::_ref_type*>(dynamic_pub_sub_type_->create_data());

        if (nullptr == dynamic_data_in_)
        {
            EPROSIMA_LOG_ERROR(LATENCYPUBLISHER,
                    "Iteration failed: Failed to create Dynamic Data In");
            return false;
        }

        if (nullptr == dynamic_data_out_)
        {
            EPROSIMA_LOG_ERROR(LATENCYPUBLISHER,
                    "Iteration failed: Failed to create Dynamic Data Out");
            return false;
        }

        DynamicData::_ref_type data_in = (*dynamic_data_in_)->loan_value(
            (*dynamic_data_in_)->get_member_id_at_index(1));
        DynamicData::_ref_type data_out = (*dynamic_data_out_)->loan_value(
            (*dynamic_data_out_)->get_member_id_at_index(1));

        // fill until complete the desired payload size
        uint32_t padding = datasize - 4; // sequence number is a DWORD

        for (uint32_t i = 0; i < padding; ++i)
        {
            data_in->set_byte_value(i, 0);
            data_out->set_byte_value(i, 0);
        }

        (*dynamic_data_in_)->return_loaned_value(data_in);
        (*dynamic_data_out_)->return_loaned_value(data_out);
    }
    else if (init_static_types(datasize) && create_data_endpoints())
    {
        if (!data_loans_)
        {
            // Create the reception data sample
            latency_data_in_ = static_cast<LatencyType*>(latency_data_type_->create_data());
        }
        // On loans scenario this object will be kept only to check the echoed sample is correct
        // On the ordinary case it keeps the object to send
        latency_data_out_ = static_cast<LatencyType*>(latency_data_type_->create_data());
    }
    else
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "Error preparing types and endpoints for testing");
        return false;
    }

    // Signal the subscribers the publisher is READY
    times_.clear();
    TestCommandType command;
    command.m_command = READY;
    if (RETCODE_OK != command_writer_->write(&command))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "Publisher cannot publish READY command");
        return false;
    }

    // WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    // EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    wait_for_discovery(
        [this]() -> bool
        {
            return total_matches() == 4 * subscribers_;
        });

    EPROSIMA_LOG_INFO(LatencyTest, C_B_MAGENTA << "Pub: DISCOVERY COMPLETE " << C_DEF);

    // Wait for Subscriber's BEGIN command
    wait_for_command(
        [this]()
        {
            return command_msg_count_ >= subscribers_;
        });

    // The first measurement it's usually not representative, so we take one more and then drop the first one.
    for (unsigned int count = 1; count <= samples_ + 1; ++count)
    {
        void* data {nullptr};

        if (dynamic_types_)
        {
            (*dynamic_data_in_)->set_uint32_value(0, 0);
            (*dynamic_data_out_)->set_uint32_value(0, count);
            data = dynamic_data_out_;
        }
        else
        {
            // Initialize the sample to send
            latency_data_out_->seqnum = count;

            // loan each sample
            if (data_loans_)
            {
                latency_data_in_ = nullptr;
                int trials = 10;
                bool loaned = false;

                while (trials-- != 0 && !loaned)
                {
                    loaned = (RETCODE_OK
                            ==  data_writer_->loan_sample(
                                data,
                                DataWriter::LoanInitializationKind::NO_LOAN_INITIALIZATION));

                    std::this_thread::yield();

                    if (!loaned)
                    {
                        EPROSIMA_LOG_INFO(LatencyTest, "Publisher trying to loan: " << trials);
                    }
                }

                if (!loaned)
                {
                    EPROSIMA_LOG_ERROR(LatencyTest, "Problem on Publisher test data with loan");
                    continue; // next iteration
                }

                // copy the data to the loan
                auto data_type = std::static_pointer_cast<LatencyDataType>(latency_data_type_);
                data_type->copy_data(*latency_data_out_, *(LatencyType*)data);
            }
            else
            {
                data = latency_data_out_;
            }

            // reset the reception sample data
            if (latency_data_in_)
            {
                latency_data_in_->seqnum = 0;
            }
        }

        start_time_ = std::chrono::steady_clock::now();

        // Data publishing
        if (RETCODE_OK != data_writer_->write(data))
        {
            // return the loan
            if (data_loans_)
            {
                data_writer_->discard_loan(data);
            }

            EPROSIMA_LOG_ERROR(LatencyTest, "Publisher write operation failed");
            return false;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        // the wait timeouts due possible message leaks
        data_msg_cv_.wait_for(lock,
                std::chrono::milliseconds(100),
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
        EPROSIMA_LOG_ERROR(LatencyTest, "Error in test");
        return false;
    }

    // Wait for Subscriber's END command
    // Assures that LatencyTestSubscriber|Publisher data endpoints creation and
    // destruction is sequential
    wait_for_command(
        [this]()
        {
            return command_msg_count_ >= subscribers_;
        });

    // TEST FINISHED:

    // Delete Data Sample
    if (dynamic_types_)
    {
        dynamic_pub_sub_type_->delete_data(dynamic_data_in_);
        dynamic_pub_sub_type_->delete_data(dynamic_data_out_);
    }
    else
    {
        if (!data_loans_)
        {
            latency_data_type_.delete_data(latency_data_in_);
        }
        latency_data_type_.delete_data(latency_data_out_);
    }

    // Free resources

    if (dynamic_types_)
    {
        size_t removed = 0;
        data_writer_->clear_history(&removed);
    }
    // Remove endpoints associated with the given payload size
    else if (!destroy_data_endpoints())
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "Endpoints for payload size " << datasize << " could not been removed");
        return false;
    }

    // Drop the first measurement, as it's usually not representative
    times_.erase(times_.begin());

    // Log all data to CSV file if specified
    if (raw_data_file_ != "")
    {
        export_raw_data(datasize);
    }

    analyze_times(datasize);

    return true;
}

void LatencyTestPublisher::analyze_times(
        uint32_t datasize)
{
    // Collect statistics
    TimeStats stats;
    stats.bytes_ = datasize;
    stats.received_ = received_count_ - 1;  // Because we are not counting the first one.
    stats.minimum_ = *min_element(times_.begin(), times_.end());
    stats.maximum_ = *max_element(times_.begin(), times_.end());
    stats.mean_ = accumulate(times_.begin(), times_.end(),
                    std::chrono::duration<double, std::micro>(0)).count() / times_.size();

    double aux_stdev = 0;
    for (std::vector<std::chrono::duration<double, std::micro>>::iterator tit = times_.begin(); tit != times_.end();
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
    *output_files_[data_index] << "\"" << stats.minimum_.count() << "\",\"" << stats.mean_ << "\"" << std::endl;


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
    std::ofstream data_file;
    data_file.open(raw_data_file_, std::fstream::app);
    for (std::vector<std::chrono::duration<double, std::micro>>::iterator tit = times_.begin(); tit != times_.end();
            ++tit)
    {
        data_file << ++raw_sample_count_ << "," << datasize << "," << (*tit).count() << std::endl;
    }
    data_file.close();
}

int32_t LatencyTestPublisher::total_matches() const
{
    // no need to lock because is used always within a
    // condition variable wait predicate

    int32_t count = data_writer_listener_.get_matches()
            + data_reader_listener_.get_matches()
            + command_writer_listener_.get_matches()
            + command_reader_listener_.get_matches();

    // Each endpoint has a mirror counterpart in the LatencyTestPublisher
    // thus, the maximun number of matches is 4 * total number of subscribers
    assert(count >= 0 && count <= 4 * subscribers_);
    return count;
}

bool LatencyTestPublisher::init_dynamic_types()
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (dynamic_pub_sub_type_)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR DYNAMIC DATA type already initialized");
        return false;
    }
    else if (participant_->find_type(LatencyDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR DYNAMIC DATA type already registered");
        return false;
    }

    // Dummy type registration
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    // Create basic builders
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(LatencyDataType::type_name_);

    DynamicTypeBuilder::_ref_type struct_type_builder {factory->create_type(type_descriptor)};

    // Add members to the struct.
    MemberDescriptor::_ref_type member_descriptor {traits<MemberDescriptor>::make_shared()};
    member_descriptor->name("seqnum");
    member_descriptor->type(factory->get_primitive_type(TK_UINT32));
    struct_type_builder->add_member(member_descriptor);
    member_descriptor->name("data");
    member_descriptor->type(factory->create_sequence_type(
                factory->get_primitive_type(TK_BYTE), static_cast<uint32_t>(LENGTH_UNLIMITED))->build());
    struct_type_builder->add_member(member_descriptor);
    dynamic_pub_sub_type_.reset(new DynamicPubSubType(struct_type_builder->build()));

    // Register the data type
    if (RETCODE_OK != dynamic_pub_sub_type_.register_type(participant_))
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR registering the DYNAMIC DATA type");
        return false;
    }

    return true;
}

bool LatencyTestPublisher::init_static_types(
        uint32_t payload)
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (latency_data_type_)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR STATIC DATA type already initialized");
        return false;
    }
    else if (participant_->find_type(LatencyDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR STATIC DATA type already registered");
        return false;
    }

    // calculate the padding for the desired demand
    ::size_t padding = payload - LatencyType::overhead;
    assert(padding > 0);
    // Create the static type
    latency_data_type_.reset(new LatencyDataType(padding));
    // Register the static type
    if (RETCODE_OK != latency_data_type_.register_type(participant_))
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR registering the STATIC DATA type");
        return false;
    }

    return true;
}

bool LatencyTestPublisher::create_data_endpoints()
{
    if (nullptr != latency_data_pub_topic_ || nullptr != latency_data_sub_topic_)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR latency_data_pub_topic_ already initialized");
        return false;
    }

    if (nullptr != latency_data_sub_topic_)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR latency_data_sub_topic_ already initialized");
        return false;
    }

    if (nullptr != data_writer_)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR data_writer_ already initialized");
        return false;
    }

    if (nullptr != data_reader_)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR data_reader_ already initialized");
        return false;
    }

    /* Create Data Topics */
    std::ostringstream topic_name;
    topic_name << "LatencyTest_";
    if (hostname_)
    {
        topic_name << asio::ip::host_name() << "_";
    }
    topic_name << pid_ << "_PUB2SUB";

    latency_data_pub_topic_ = participant_->create_topic(
        topic_name.str(),
        LatencyDataType::type_name_,
        TOPIC_QOS_DEFAULT);

    if (nullptr == latency_data_pub_topic_)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR creating latency_data_pub_topic_");
        return false;
    }

    topic_name.str("");
    topic_name.clear();
    topic_name << "LatencyTest_";

    if (hostname_)
    {
        topic_name << asio::ip::host_name() << "_";
    }
    topic_name << pid_ << "_SUB2PUB";

    latency_data_sub_topic_ = participant_->create_topic(
        topic_name.str(),
        LatencyDataType::type_name_,
        TOPIC_QOS_DEFAULT);

    if (latency_data_sub_topic_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR creating latency_data_sub_topic_");
        return false;
    }

    // Create DataWriter
    data_writer_ = publisher_->create_datawriter(
        latency_data_pub_topic_,
        dw_qos_,
        &data_writer_listener_);

    if (data_writer_ == nullptr)
    {
        return false;
    }

    // Create Echo DataReader
    data_reader_ = subscriber_->create_datareader(
        latency_data_sub_topic_,
        dr_qos_,
        &data_reader_listener_);

    if (data_reader_ == nullptr)
    {
        return false;
    }

    return true;
}

bool LatencyTestPublisher::destroy_data_endpoints()
{
    assert(nullptr != participant_);
    assert(nullptr != publisher_);
    assert(nullptr != subscriber_);

    // Delete the endpoints
    if (nullptr == data_writer_
            || RETCODE_OK != publisher_->delete_datawriter(data_writer_))
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR destroying the DataWriter");
        return false;
    }
    data_writer_ = nullptr;
    data_writer_listener_.reset();

    if (nullptr == data_reader_
            || RETCODE_OK != subscriber_->delete_datareader(data_reader_))
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR destroying the DataReader");
        return false;
    }
    data_reader_ = nullptr;
    data_reader_listener_.reset();

    // Delete the Topics
    if (nullptr == latency_data_pub_topic_
            || RETCODE_OK != participant_->delete_topic(latency_data_pub_topic_))
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR destroying the DATA PUB topic");
        return false;
    }
    latency_data_pub_topic_ = nullptr;

    if (nullptr == latency_data_sub_topic_
            || RETCODE_OK != participant_->delete_topic(latency_data_sub_topic_))
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR destroying the DATA SUB topic");
        return false;
    }
    latency_data_sub_topic_ = nullptr;

    // Delete the Type
    if (RETCODE_OK != participant_->unregister_type(LatencyDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(LATENCYPUBLISHER, "ERROR unregistering the DATA type");
        return false;
    }

    latency_data_type_.reset();
    dynamic_pub_sub_type_.reset();
    DynamicTypeBuilderFactory::delete_instance();

    return true;
}
