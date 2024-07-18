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

#include <chrono>
#include <fstream>
#include <map>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

// *******************************************************************************************
// ********************************* DATA WRITER LISTENER ************************************
// *******************************************************************************************
void ThroughputPublisher::DataWriterListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (1 == info.current_count)
    {
        EPROSIMA_LOG_INFO(THROUGHPUTPUBLISHER, C_RED << "Pub: DATA Pub Matched "
                                                     << info.total_count << "/" << throughput_publisher_.subscribers_ <<
                C_DEF);
    }

    matched_ = info.total_count;
    throughput_publisher_.data_discovery_cv_.notify_one();
}

// *******************************************************************************************
// ********************************* COMMAND READER LISTENER *********************************
// *******************************************************************************************

/*
 * Our current inplementation of MatchedStatus info:
 * - total_count(_change) holds the actual number of matches
 * - current_count(_change) is a flag to signal match or unmatch.
 *   (TODO: review if fits standard definition)
 * */

void ThroughputPublisher::CommandReaderListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (1 == info.current_count)
    {
        EPROSIMA_LOG_INFO(THROUGHPUTPUBLISHER, C_RED << "Pub: COMMAND Sub Matched "
                                                     << info.total_count << "/" << throughput_publisher_.subscribers_ * 2 <<
                C_DEF);
    }

    matched_ = info.total_count;
    throughput_publisher_.command_discovery_cv_.notify_one();
}

// *******************************************************************************************
// ****************************** COMMAND WRITER LISTENER ************************************
// *******************************************************************************************
void ThroughputPublisher::CommandWriterListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (1 == info.current_count)
    {
        EPROSIMA_LOG_INFO(THROUGHPUTPUBLISHER, C_RED << "Pub: COMMAND Pub Matched "
                                                     << info.total_count << "/" << throughput_publisher_.subscribers_ * 2 <<
                C_DEF);
    }

    matched_ = info.total_count;
    throughput_publisher_.command_discovery_cv_.notify_one();
}

// *******************************************************************************************
// ********************************** THROUGHPUT PUBLISHER ***********************************
// *******************************************************************************************
ThroughputPublisher::ThroughputPublisher()
    : data_writer_listener_(*this)
    , command_writer_listener_(*this)
    , command_reader_listener_(*this)
{
}

bool ThroughputPublisher::init(
        bool reliable,
        uint32_t pid,
        bool hostname,
        const std::string& export_csv,
        const eprosima::fastdds::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastdds::rtps::PropertyPolicy& property_policy,
        const std::string& xml_config_file,
        const std::string& demands_file,
        const std::string& recoveries_file,
        bool dynamic_types,
        Arg::EnablerValue data_sharing,
        bool data_loans,
        Arg::EnablerValue shared_memory,
        int forced_domain)
{
    pid_ = pid;
    hostname_ = hostname;
    dynamic_types_ = dynamic_types;
    data_sharing_ = data_sharing;
    data_loans_ = data_loans;
    shared_memory_ = shared_memory;
    reliable_ = reliable;
    forced_domain_ = forced_domain;
    demands_file_ = demands_file;
    export_csv_ = export_csv;
    xml_config_file_ = xml_config_file;
    recoveries_file_ = recoveries_file;

    /* Create DomainParticipant*/
    std::string participant_profile_name = "pub_participant_profile";
    DomainParticipantQos pqos;

    // Default domain
    DomainId_t domainId = pid % 230;

    // Default participant name
    pqos.name("throughput_test_publisher");

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
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR creating participant");
        ready_ = false;
        return false;
    }

    // Create the command data type
    throughput_command_type_.reset(new ThroughputCommandDataType());

    // Register the command data type
    if (RETCODE_OK
            != throughput_command_type_.register_type(participant_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR registering command type");
        return false;
    }

    /* Create Publisher */
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR creating the Publisher");
        return false;
    }

    /* Create Subscriber */
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR creating the Subscriber");
        return false;
    }

    /* Update DataWriterQoS from xml profile data */
    std::string profile_name = "publisher_profile";

    if (xml_config_file_.length() > 0
            && RETCODE_OK != publisher_->get_datawriter_qos_from_profile(profile_name, dw_qos_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR unable to retrieve the " << profile_name);
        return false;
    }

    // Load the property policy specified
    dw_qos_.properties(property_policy);

    // Reliability
    ReliabilityQosPolicy rp;
    if (reliable_)
    {
        rp.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
        dw_qos_.reliability(rp);

        RTPSReliableWriterQos rw_qos;
        rw_qos.times.heartbeat_period.seconds = 0;
        rw_qos.times.heartbeat_period.nanosec = 100000000;
        rw_qos.times.nack_supression_duration = {0, 0};
        rw_qos.times.nack_response_delay = {0, 0};

        dw_qos_.reliable_writer_qos(rw_qos);
    }
    else
    {
        rp.kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
        dw_qos_.reliability(rp);
    }

    // Set data sharing according with cli. Is disabled by default in all xml profiles
    if (Arg::EnablerValue::ON == data_sharing_)
    {
        DataSharingQosPolicy dsp;
        dsp.on("");
        dw_qos_.data_sharing(dsp);
    }
    else if (Arg::EnablerValue::OFF == data_sharing_)
    {
        DataSharingQosPolicy dsp;
        dsp.off();
        dw_qos_.data_sharing(dsp);
    }

    // Create Command topic
    {
        std::ostringstream topic_name;
        topic_name << "ThroughputTest_Command_";
        if (hostname)
        {
            topic_name << asio::ip::host_name() << "_";
        }
        topic_name << pid << "_SUB2PUB";

        command_sub_topic_ = participant_->create_topic(
            topic_name.str(),
            "ThroughputCommand",
            TOPIC_QOS_DEFAULT);

        if (nullptr == command_sub_topic_)
        {
            EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR creating the COMMAND Sub topic");
            return false;
        }

        topic_name.str("");
        topic_name.clear();
        topic_name << "ThroughputTest_Command_";
        if (hostname)
        {
            topic_name << asio::ip::host_name() << "_";
        }
        topic_name << pid << "_PUB2SUB";

        command_pub_topic_ = participant_->create_topic(
            topic_name.str(),
            "ThroughputCommand",
            TOPIC_QOS_DEFAULT);

        if (nullptr == command_pub_topic_)
        {
            EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR creating the COMMAND Pub topic");
            return false;
        }
    }

    /* Create Command Reader */
    {
        DataReaderQos cr_qos;
        cr_qos.history().kind = KEEP_ALL_HISTORY_QOS;
        cr_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        cr_qos.durability().durabilityKind(TRANSIENT_LOCAL);
        cr_qos.properties(property_policy);

        {
            DataSharingQosPolicy dsp;
            dsp.off();
            cr_qos.data_sharing(dsp);
        }

        command_reader_ = subscriber_->create_datareader(
            command_sub_topic_,
            cr_qos,
            &command_reader_listener_);

        if (command_reader_ == nullptr)
        {
            EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR creating the COMMAND DataWriter");
            return false;
        }
    }

    /* Create Command Writer */
    {
        DataWriterQos cw_qos;
        cw_qos.history().kind = KEEP_ALL_HISTORY_QOS;
        cw_qos.durability().durabilityKind(TRANSIENT_LOCAL);
        cw_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        cw_qos.publish_mode().kind = SYNCHRONOUS_PUBLISH_MODE;
        cw_qos.properties(property_policy);

        {
            DataSharingQosPolicy dsp;
            dsp.off();
            cw_qos.data_sharing(dsp);
        }

        command_writer_ = publisher_->create_datawriter(
            command_pub_topic_,
            cw_qos,
            &command_writer_listener_);

        if (command_writer_ == nullptr)
        {
            EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR creating the COMMAND DataReader");
            return false;
        }
    }

    // Calculate overhead due to clock calls
    t_start_ = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        t_end_ = std::chrono::steady_clock::now();
    }
    t_overhead_ = std::chrono::duration<double, std::micro>(t_end_ - t_start_) / 1001;
    std::cout << "Publisher's clock access overhead: " << t_overhead_.count() << " us"  << std::endl;

    // Endpoints using dynamic data endpoints span the whole test duration
    // Static types and endpoints are created for each payload iteration
    return dynamic_types_ ? init_dynamic_types() && create_data_endpoints(dw_qos_) : true;
}

ThroughputPublisher::~ThroughputPublisher()
{
    // Static type endpoints should have been remove for each demand map iteration
    if (dynamic_types_)
    {
        destroy_data_endpoints();
    }
    else if (nullptr != data_writer_
            || nullptr != data_pub_topic_
            || throughput_data_type_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR unregistering the DATA type");
        return;
    }

    // Remove command endpoints
    subscriber_->delete_datareader(command_reader_);
    participant_->delete_subscriber(subscriber_);

    publisher_->delete_datawriter(command_writer_);
    participant_->delete_publisher(publisher_);

    participant_->delete_topic(command_sub_topic_);
    participant_->delete_topic(command_pub_topic_);

    // Remove the participant
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
    EPROSIMA_LOG_INFO(THROUGHPUTPUBLISHER, "Pub: Participant removed");
}

void ThroughputPublisher::run(
        uint32_t test_time,
        uint32_t recovery_time_ms,
        int demand,
        uint32_t msg_size,
        uint32_t subscribers)
{
    subscribers_ = subscribers;

    if (demand == 0 || msg_size == 0)
    {
        if (!load_demands_payload())
        {
            return;
        }
    }
    else
    {
        payload_ = msg_size;
        demand_payload_[msg_size - (uint32_t)(dynamic_types_ ? 8 : ThroughputType::overhead)].push_back(demand);
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

    std::cout << "Pub: Waiting for command discovery" << std::endl;
    {
        std::unique_lock<std::mutex> disc_lock(mutex_);
        command_discovery_cv_.wait(disc_lock, [&]()
                {
                    if (dynamic_types_)
                    {
                        // full command and data endpoints discovery
                        return total_matches() == static_cast<int>(subscribers_ * 3);
                    }
                    else
                    {
                        // The only endpoints present should be command ones
                        return total_matches() == static_cast<int>(subscribers_ * 2);
                    }
                });
    }
    std::cout << "Pub: Discovery command complete" << std::endl;

    ThroughputCommandType command;
    SampleInfo info;
    bool test_failure = false;

    // Iterate over message sizes
    for (auto sit = demand_payload_.begin(); sit != demand_payload_.end(); ++sit)
    {
        auto dw_qos = dw_qos_;

        // Check history resources depending on the history kind and demand
        uint32_t max_demand = 0;
        for ( auto current_demand : sit->second )
        {
            max_demand = std::max(current_demand, max_demand);
        }

        if (dw_qos.history().kind == KEEP_LAST_HISTORY_QOS)
        {
            // Ensure that the history depth is at least the demand
            if (dw_qos.history().depth < 0 ||
                    static_cast<uint32_t>(dw_qos.history().depth) < max_demand)
            {
                EPROSIMA_LOG_WARNING(THROUGHPUTPUBLISHER, "Setting history depth to " << max_demand);
                dw_qos.resource_limits().max_samples = max_demand;
                dw_qos.history().depth = max_demand;
            }
        }
        // KEEP_ALL case
        else
        {
            // Ensure that the max samples is at least the demand
            if (dw_qos.resource_limits().max_samples <= 0 ||
                    static_cast<uint32_t>(dw_qos.resource_limits().max_samples) < max_demand)
            {
                EPROSIMA_LOG_WARNING(THROUGHPUTPUBLISHER, "Setting resource limit max samples to " << max_demand);
                dw_qos.resource_limits().max_samples = max_demand;
            }
        }
        // Set the allocated samples to the max_samples. This is because allocated_sample must be <= max_samples
        dw_qos.resource_limits().allocated_samples = dw_qos.resource_limits().max_samples;

        // Notify the new static type to the subscribers
        command.m_command = TYPE_NEW;
        command.m_size = sit->first;
        command.m_demand = max_demand;
        command_writer_->write(&command);

        if (dynamic_types_)
        {
            assert(nullptr == dynamic_data_);
            // Create the data sample
            dynamic_data_ = static_cast<DynamicData::_ref_type*>(dynamic_pub_sub_type_->create_data());

            if (nullptr == dynamic_data_)
            {
                EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER,
                        "Iteration failed: Failed to create Dynamic Data");
                return;
            }

            // Modify the data Sample
            (*dynamic_data_)->set_uint32_value(0, 0);
            DynamicData::_ref_type member_data = (*dynamic_data_)->loan_value(
                (*dynamic_data_)->get_member_id_at_index(1));

            for (uint32_t i = 0; i < msg_size; ++i)
            {
                member_data->set_byte_value(i, 0);
            }
            (*dynamic_data_)->return_loaned_value(member_data);
        }
        else
        {
            assert(nullptr == throughput_data_);

            // Create the data endpoints if using static types (right after modifying the QoS)
            if (init_static_types(command.m_size) && create_data_endpoints(dw_qos))
            {
                // Wait for the data endpoints discovery
                std::unique_lock<std::mutex> data_disc_lock(mutex_);
                data_discovery_cv_.wait(data_disc_lock, [&]()
                        {
                            // all command and data endpoints must be discovered
                            return total_matches() == static_cast<int>(subscribers_ * 3);
                        });
            }
            else
            {
                EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "Error preparing static types and endpoints for testing");
                test_failure = true;
                break;
            }

            if (!data_loans_)
            {
                // Create the data sample
                throughput_data_ = static_cast<ThroughputType*>(throughput_data_type_.create_data());
            }
        }

        // Iterate over burst of messages to send
        for (auto dit = sit->second.begin(); dit != sit->second.end(); ++dit)
        {
            command.m_size = sit->first;
            command.m_demand = *dit;

            for (uint16_t i = 0; i < recovery_times_.size(); i++)
            {
                // awake the subscribers
                command.m_command = READY_TO_START;
                command.m_size = sit->first;
                command.m_demand = *dit;
                command_writer_->write(&command);

                // wait till subscribers are rigged
                uint32_t subscribers_ready = 0;
                while (subscribers_ready < subscribers_)
                {
                    command_reader_->wait_for_unread_message({20, 0});
                    command_reader_->take_next_sample(&command, &info);
                    if (info.valid_data && command.m_command == BEGIN)
                    {
                        ++subscribers_ready;
                    }
                }

                // run this test iteration test
                if (!test(test_time, recovery_times_[i], *dit, sit->first))
                {
                    test_failure = true;
                    break;
                }
            }
        }

        // Notify the current type will no longer be used
        command.m_command = TYPE_DISPOSE;
        command.m_size = sit->first;
        command.m_demand = max_demand;
        command_writer_->write(&command);

        // Delete the Data Sample
        if (dynamic_types_)
        {
            dynamic_pub_sub_type_.delete_data(dynamic_data_);
            dynamic_data_ = nullptr;
        }
        else
        {
            if (!data_loans_)
            {
                throughput_data_type_.delete_data(throughput_data_);
            }
            throughput_data_ = nullptr;

            // Destroy the data endpoints if using static types
            if (!destroy_data_endpoints())
            {
                EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "Error removing static types and endpoints for testing");
                test_failure = true;
                break;
            }

            // wait till subscribers are rigged
            uint32_t subscribers_ready = 0;
            while (subscribers_ready < subscribers_)
            {
                command_reader_->wait_for_unread_message({20, 0});
                command_reader_->take_next_sample(&command, &info);
                if (info.valid_data && command.m_command == TYPE_REMOVED)
                {
                    ++subscribers_ready;
                }
            }
        }
    }

    // We are done kill the subscribers
    command.m_command = ALL_STOPS;
    command_writer_->write(&command);

    if (test_failure)
    {
        return;
    }

    bool all_acked = command_writer_->wait_for_acknowledgments({20, 0}) == RETCODE_OK;
    print_results(results_);

    if (!all_acked)
    {
        std::cout << "ALL_STOPS Not acked! in 20(s)" << std::endl;
    }
}

bool ThroughputPublisher::test(
        uint32_t test_time,
        uint32_t recovery_time_ms,
        uint32_t demand,
        uint32_t msg_size)
{
    // This method expects all data and command endpoints matched
    assert(total_matches() == static_cast<int>(subscribers_ * 3));

    // Declare test time variables
    std::chrono::duration<double, std::micro> clock_overhead(0);
    std::chrono::duration<double, std::nano> test_time_ns = std::chrono::seconds(test_time);
    std::chrono::duration<double, std::nano> recovery_duration_ns = std::chrono::milliseconds(recovery_time_ms);
    std::chrono::steady_clock::time_point batch_start;

    // Send a TEST_STARTS and sleep for a while to give the subscriber time to set up
    uint32_t samples = 0;
    ThroughputCommandType command_sample;
    SampleInfo info;
    command_sample.m_command = TEST_STARTS;
    command_writer_->write(&command_sample);

    // If the subscriber does not acknowledge the TEST_STARTS in time, we consider something went wrong.
    std::chrono::steady_clock::time_point test_start_sent_tp = std::chrono::steady_clock::now();
    if (RETCODE_OK != command_writer_->wait_for_acknowledgments({20, 0}))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER,
                "Something went wrong: The subscriber has not acknowledged the TEST_STARTS command.");
        return false;
    }
    // Calculate how low has it takes for the subscriber to acknowledge TEST_START
    std::chrono::duration<double, std::micro> test_start_ack_duration =
            std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - test_start_sent_tp);

    // Send batches until test_time_ns is reached
    t_start_ = std::chrono::steady_clock::now();
    uint32_t seqnum = 0;
    while ((t_end_ - t_start_) < test_time_ns)
    {
        // Get start time
        batch_start = std::chrono::steady_clock::now();
        // Send a batch of size demand
        for (uint32_t sample = 0; sample < demand; sample++)
        {
            if (dynamic_types_)
            {
                (*dynamic_data_)->set_uint32_value(0, ++seqnum);
                data_writer_->write(dynamic_data_);
            }
            else if (data_loans_)
            {
                // Try loan a sample
                void* data = nullptr;
                if (RETCODE_OK
                        ==  data_writer_->loan_sample(
                            data,
                            DataWriter::LoanInitializationKind::NO_LOAN_INITIALIZATION))
                {
                    // initialize and send the sample
                    static_cast<ThroughputType*>(data)->seqnum = ++seqnum;

                    if (RETCODE_OK != data_writer_->write(data))
                    {
                        data_writer_->discard_loan(data);
                    }
                }
                else
                {
                    std::this_thread::yield();
                    // try again this sample
                    --sample;
                    continue;
                }
            }
            else
            {
                throughput_data_->seqnum = ++seqnum;
                data_writer_->write(throughput_data_);
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
            Note that if the duration specified is less than what remains of thread quantum (time slice),
            the sleep lasts until the next OS scheduled quantum.
         */
        std::this_thread::sleep_for(recovery_duration_ns - (t_end_ - batch_start));

        clock_overhead += t_overhead_ * 2; // We access the clock twice per batch.
    }

    size_t removed = 0;
    data_writer_->clear_history(&removed);

    command_sample.m_command = TEST_ENDS;
    command_writer_->write(&command_sample);

    // If the subscriber does not acknowledge the TEST_ENDS in time, we consider something went wrong.
    if (RETCODE_OK
            != command_writer_->wait_for_acknowledgments({20, 0}))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER,
                "Something went wrong: The subscriber has not acknowledged the TEST_ENDS command.");
        return false;
    }

    // Results processing
    uint32_t num_results_received = 0;
    bool results_error = false;
    while ( !results_error && num_results_received < subscribers_ )
    {
        if (command_reader_->wait_for_unread_message({20, 0})
                && RETCODE_OK == command_reader_->take_next_sample(&command_sample, &info)
                && info.valid_data)
        {
            if (command_sample.m_command == TEST_RESULTS)
            {
                num_results_received++;

                TroughputResults result;
                result.payload_size = msg_size + (uint32_t)(dynamic_types_ ? 8 : ThroughputType::overhead);
                result.demand = demand;
                result.recovery_time_ms = recovery_time_ms;

                result.publisher.send_samples = samples;
                result.publisher.totaltime_us =
                        std::chrono::duration<double, std::micro>(t_end_ - t_start_) - clock_overhead;

                result.subscriber.recv_samples = command_sample.m_receivedsamples;
                assert(samples >= command_sample.m_receivedsamples);
                result.subscriber.lost_samples = samples - (uint32_t)command_sample.m_receivedsamples;
                result.subscriber.totaltime_us =
                        std::chrono::microseconds(command_sample.m_totaltime)
                        - test_start_ack_duration - clock_overhead;

                result.compute();

                if (num_results_received == 1)
                {
                    results_.push_back(result);
                }
                else
                {
                    auto& results_entry = results_.back();
                    results_entry.publisher.send_samples += result.publisher.send_samples;
                    results_entry.subscriber.recv_samples += result.subscriber.recv_samples;
                    results_entry.subscriber.lost_samples += result.subscriber.lost_samples;
                    results_entry.subscriber.MBitssec += result.subscriber.MBitssec;
                    results_entry.subscriber.Packssec += result.subscriber.Packssec;
                }

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
                command_writer_->clear_history(&removed);
            }
            else
            {
                std::cout << "The test expected results, stopping" << std::endl;
                results_error = true;
            }
        }
        else
        {
            std::cout << "PROBLEM READING RESULTS;" << std::endl;
            results_error = true;
        }
    }

    return !results_error;
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
    auto overhead = uint32_t(dynamic_types_ ? 8 : ThroughputType::overhead);
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
                if (payload_ < overhead)
                {
                    std::cout << "Minimum payload is 16 bytes" << std::endl;
                    return false;
                }
                payload_ -= overhead;
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

    payload_ += overhead;

    std::cout << "Performing test with this payloads/demands:" << std::endl;
    for (auto sit = demand_payload_.begin(); sit != demand_payload_.end(); ++sit)
    {
        printf("Payload: %6d; Demands: ", sit->first + overhead);
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

bool ThroughputPublisher::init_dynamic_types()
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (dynamic_pub_sub_type_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR DYNAMIC DATA type already initialized");
        return false;
    }
    else if (participant_->find_type(ThroughputDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR DYNAMIC DATA type already registered");
        return false;
    }

    // Dummy type registration
    DynamicTypeBuilderFactory::_ref_type factory {DynamicTypeBuilderFactory::get_instance()};
    // Create basic builders
    TypeDescriptor::_ref_type type_descriptor {traits<TypeDescriptor>::make_shared()};
    type_descriptor->kind(TK_STRUCTURE);
    type_descriptor->name(ThroughputDataType::type_name_);

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
    if (RETCODE_OK
            != dynamic_pub_sub_type_.register_type(participant_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR registering the DYNAMIC DATA topic");
        return false;
    }

    return true;
}

bool ThroughputPublisher::init_static_types(
        uint32_t payload)
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (throughput_data_type_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR STATIC DATA type already initialized");
        return false;
    }
    else if (participant_->find_type(ThroughputDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR STATIC DATA type already registered");
        return false;
    }

    // Create the static type
    throughput_data_type_.reset(new ThroughputDataType(payload));
    // Register the static type
    if (RETCODE_OK
            != throughput_data_type_.register_type(participant_))
    {
        return false;
    }

    return true;
}

bool ThroughputPublisher::create_data_endpoints(
        const DataWriterQos& dw_qos)
{
    if (nullptr != data_pub_topic_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR topic already initialized");
        return false;
    }

    if (nullptr != data_writer_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR data_writer_ already initialized");
        return false;
    }

    // Create the topic
    std::ostringstream topic_name;
    topic_name << "ThroughputTest_";
    if (hostname_)
    {
        topic_name << asio::ip::host_name() << "_";
    }
    topic_name << pid_ << "_UP";

    data_pub_topic_ = participant_->create_topic(
        topic_name.str(),
        ThroughputDataType::type_name_,
        TOPIC_QOS_DEFAULT);

    if (nullptr == data_pub_topic_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR creating the DATA topic");
        return false;
    }

    // Create the endpoint
    if (nullptr ==
            (data_writer_ = publisher_->create_datawriter(
                data_pub_topic_,
                dw_qos,
                &data_writer_listener_)))
    {
        return false;
    }

    return true;
}

bool ThroughputPublisher::destroy_data_endpoints()
{
    assert(nullptr != participant_);
    assert(nullptr != publisher_);

    // Delete the endpoint
    if (nullptr == data_writer_
            || RETCODE_OK != publisher_->delete_datawriter(data_writer_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR destroying the DataWriter");
        return false;
    }
    data_writer_ = nullptr;
    data_writer_listener_.reset();

    // Delete the Topic
    if (nullptr == data_pub_topic_
            || RETCODE_OK != participant_->delete_topic(data_pub_topic_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR destroying the DATA topic");
        return false;
    }
    data_pub_topic_ = nullptr;

    // Delete the Type
    if (RETCODE_OK
            != participant_->unregister_type(ThroughputDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTPUBLISHER, "ERROR unregistering the DATA type");
        return false;
    }

    throughput_data_type_.reset();

    return true;
}

int ThroughputPublisher::total_matches() const
{
    // no need to lock because is used always within a
    // condition variable wait predicate

    int count = data_writer_listener_.get_matches()
            + command_writer_listener_.get_matches()
            + command_reader_listener_.get_matches();

    // Each endpoint has a mirror counterpart in the ThroughputSubscriber
    // thus, the maximun number of matches is 3 * total number of subscribers
    assert(count >= 0 && count <= 3 * (int)subscribers_);
    return count;
}
