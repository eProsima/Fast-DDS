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

#include <map>
#include <fstream>
#include <chrono>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// *******************************************************************************************
// ********************************* DATA READER LISTENER ************************************
// *******************************************************************************************
void ThroughputPublisher::DataWriterListener::on_publication_matched(
                eprosima::fastdds::dds::DataWriter*,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(throughput_publisher_.data_mutex_);

    if (1 == info.total_count_change)
    {
        logInfo(THROUGHPUTPUBLISHER, C_RED << "Pub: DATA Pub Matched "
                  << info.total_count << "/" << throughput_publisher_.subscribers_ << C_DEF);
    }

    if ((throughput_publisher_.data_discovery_count_ = info.total_count)
            == static_cast<int>(throughput_publisher_.subscribers_))
    {
        // In case it does not enter the if, the lock will be unlock in destruction
        lock.unlock();
        throughput_publisher_.data_discovery_cv_.notify_one();
    }
}

// *******************************************************************************************
// ********************************* DATA WRITER LISTENER ************************************
// *******************************************************************************************

void ThroughputPublisher::CommandReaderListener::on_subscription_matched(
        eprosima::fastdds::dds::DataReader*,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(throughput_publisher_.command_mutex_);

    if (1 == info.total_count_change)
    {
        logInfo(THROUGHPUTPUBLISHER, C_RED << "Pub: COMMAND Sub Matched "
                  << info.total_count << "/" << throughput_publisher_.subscribers_ * 2 << C_DEF);
    }

    if ((throughput_publisher_.command_discovery_count_ = info.total_count)
            == static_cast<int>(throughput_publisher_.subscribers_ * 2))
    {
        // In case it does not enter the if, the lock will be unlock in destruction
        lock.unlock();

        throughput_publisher_.command_discovery_cv_.notify_one();
    }
}

// *******************************************************************************************
// ****************************** COMMAND WRITER LISTENER ************************************
// *******************************************************************************************
void ThroughputPublisher::CommandWriterListener::on_publication_matched(
                eprosima::fastdds::dds::DataWriter*,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(throughput_publisher_.command_mutex_);

    if (1 == info.total_count_change)
    {
        logInfo(THROUGHPUTPUBLISHER, C_RED << "Pub: COMMAND Pub Matched "
                  << info.total_count << "/" << throughput_publisher_.subscribers_ * 2 << C_DEF);
    }

    if ((throughput_publisher_.command_discovery_count_ = info.total_count)
            == static_cast<int>(throughput_publisher_.subscribers_ * 2))
    {
        // In case it does not enter the if, the lock will be unlock in destruction
        lock.unlock();

        throughput_publisher_.command_discovery_cv_.notify_one();
    }
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
        const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
        const std::string& xml_config_file,
        const std::string& demands_file,
        const std::string& recoveries_file,
        bool dynamic_types,
        int forced_domain)
{
    pid_ = pid;
    hostname_ = hostname;
    dynamic_types_ = dynamic_types;
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
        logError(THROUGHPUTPUBLISHER, "ERROR creating participant");
        ready_ = false;
        return false;
    }

    // Create the command data type
    throughput_command_type_.reset(new ThroughputCommandDataType());

    // Register the command data type
    if (ReturnCode_t::RETCODE_OK
            != throughput_command_type_.register_type(participant_))
    {
        logError(THROUGHPUTPUBLISHER, "ERROR registering command type");
        return false;
    }

    /* Create Publisher */
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        logError(THROUGHPUTPUBLISHER, "ERROR creating the Publisher");
        return false;
    }

    /* Create Subscriber */
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        logError(THROUGHPUTPUBLISHER, "ERROR creating the Subscriber");
        return false;
    }

    /* Update DataWriterQoS with xml profile data */
    std::string profile_name = "publisher_profile";

    if (xml_config_file_.length() > 0
        && ReturnCode_t::RETCODE_OK != publisher_->get_datawriter_qos_from_profile(profile_name, dw_qos_))
    {
        logError(THROUGHPUTPUBLISHER, "ERROR unable to retrieve the " << profile_name);
        return false;
    }
    // Load the property policy specified
    dw_qos_.properties(property_policy);

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
            logError(THROUGHPUTPUBLISHER, "ERROR creating the COMMAND Sub topic");
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
            logError(THROUGHPUTPUBLISHER, "ERROR creating the COMMAND Pub topic");
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

        command_reader_ = subscriber_->create_datareader(
            command_sub_topic_,
            cr_qos,
            &command_reader_listener_);

        if (command_reader_ == nullptr)
        {
            logError(THROUGHPUTPUBLISHER, "ERROR creating the COMMAND DataWriter");
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

        command_writer_ = publisher_->create_datawriter(
                command_pub_topic_,
                cw_qos,
                &command_writer_listener_);

        if (command_writer_ == nullptr)
        {
            logError(THROUGHPUTPUBLISHER, "ERROR creating the COMMAND DataReader");
            return false;
        }
    }

    // Calculate overhead
    t_start_ = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        t_end_ = std::chrono::steady_clock::now();
    }
    t_overhead_ = std::chrono::duration<double, std::micro>(t_end_ - t_start_) / 1001;
    std::cout << "Publisher's clock access overhead: " << t_overhead_.count() << " us"  << std::endl;

    if (command_reader_ == nullptr || command_writer_ == nullptr)
    {
        ready_ = false;
    }

    // Endpoints using dynamic data endpoints span the whole test duration
    // Static types and endpoints are created for each payload iteration
    return dynamic_types_ ? init_dynamic_types() && create_data_endpoints() : true;
}

ThroughputPublisher::~ThroughputPublisher()
{
    // Static type endpoints should have been remove for each payload iteration
    if (dynamic_types_)
    {
        destroy_data_endpoints();
    }
    else if (nullptr != data_writer_
            || nullptr != data_pub_topic_
            || !throughput_data_type_)
    {
        logError(THROUGHPUTPUBLISHER, "ERROR unregistering the DATA type");
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
    logInfo(THROUGHPUTPUBLISHER, "Pub: Participant removed");
}

bool ThroughputPublisher::ready()
{
    return ready_;
}

void ThroughputPublisher::run(
        uint32_t test_time,
        uint32_t recovery_time_ms,
        int demand,
        int msg_size,
        uint32_t subscribers)
{
    subscribers_ = subscribers;

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

    std::cout << "Pub: Waiting for command discovery" << std::endl;
    {
        std::unique_lock<std::mutex> disc_lock(command_mutex_);
        std::cout << "Pub: lock command_mutex_ and wait to " << command_discovery_count_ <<
            " == " << static_cast<int>(subscribers_ * 2) << std::endl;     // TODO remove if error disappear"
        command_discovery_cv_.wait(disc_lock, [&]()
                {
                    std::cout << "Pub: wait to " << command_discovery_count_ << " == " <<
                        static_cast<int>(subscribers_ * 2) << std::endl;     // TODO remove if error disappear"
                    return command_discovery_count_ == static_cast<int>(subscribers_ * 2);
                });
    }
    std::cout << "Pub: Discovery command complete" << std::endl;

    ThroughputCommandType command;
    SampleInfo info;
    for (auto sit = demand_payload_.begin(); sit != demand_payload_.end(); ++sit)
    {
        for (auto dit = sit->second.begin(); dit != sit->second.end(); ++dit)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            command.m_size = sit->first;
            command.m_demand = *dit;

            // Check history resources depending on the history kind and demand
            if (dw_qos_.history().kind == KEEP_LAST_HISTORY_QOS)
            {
                // Ensure that the history depth is at least the demand
                if (dw_qos_.history().depth < 0 ||
                        static_cast<uint32_t>(dw_qos_.history().depth) < command.m_demand)
                {
                    logWarning(THROUGHPUTPUBLISHER, "Setting history depth to " << command.m_demand);
                    dw_qos_.resource_limits().max_samples = command.m_demand;
                    dw_qos_.history().depth = command.m_demand;
                }
            }
            // KEEP_ALL case
            else
            {
                // Ensure that the max samples is at least the demand
                if (dw_qos_.resource_limits().max_samples < 0 ||
                        static_cast<uint32_t>(dw_qos_.resource_limits().max_samples) < command.m_demand)
                {
                    logWarning(THROUGHPUTPUBLISHER, "Setting resource limit max samples to " << command.m_demand);
                    dw_qos_.resource_limits().max_samples = command.m_demand;
                }
            }
            // Set the allocated samples to the max_samples. This is because allocated_sample must be <= max_samples
            dw_qos_.resource_limits().allocated_samples = dw_qos_.resource_limits().max_samples;

            for (uint16_t i = 0; i < recovery_times_.size(); i++)
            {
                command.m_command = READY_TO_START;
                command.m_size = sit->first;
                command.m_demand = *dit;
                command_writer_->write(&command);
                uint32_t subscribers_ready_ = 0;
                while (subscribers_ready_ < subscribers_)
                {
                    command_reader_->wait_for_unread_message({20, 0});
                    command_reader_->take_next_sample(&command, &info);
                    if (info.valid_data && command.m_command == BEGIN)
                    {
                        subscribers_ready_++;

                        if (subscribers_ready_ == subscribers_ &&
                                !test(test_time, recovery_times_[i], *dit, sit->first))
                        {
                            command.m_command = ALL_STOPS;
                            command_writer_->write(&command);
                            return;
                        }

                        // Reset data discovery counter
                        std::unique_lock<std::mutex> data_disc_lock(data_mutex_);
                        data_discovery_count_ = 0;
                        data_disc_lock.unlock();
                    }
                }
            }
        }
    }

    command.m_command = ALL_STOPS;
    command_writer_->write(&command);
    bool all_acked = command_writer_->wait_for_acknowledgments({20, 0}) == ReturnCode_t::RETCODE_OK;
    print_results(results_);

    if (!all_acked)
    {
        std::cout << "ALL_STOPS Not acked! in 20(s)" << std::endl;
    }
    else
    {
        // Wait for the disposal of all ThroughputSubscriber publihsers and subscribers. Wait for 5s, checking status
        // every 100 ms. If after 5 s the entities have not been disposed, shutdown ThroughputPublisher without
        // receiving them.
        std::unique_lock<std::mutex> disc_lock(command_mutex_);
        for (uint16_t i = 0; i <= 50; i++)
        {
            if (command_discovery_cv_.wait_for(disc_lock, std::chrono::milliseconds(100), [&]()
                    {
                        return command_discovery_count_ == 0;
                    }))
            {
                std::cout << "Pub: All ThroughputSubscriber publishers and subscribers unmatched" << std::endl;
                break;
            }
        }
    }
}

bool ThroughputPublisher::test(
        uint32_t test_time,
        uint32_t recovery_time_ms,
        uint32_t demand,
        uint32_t msg_size)
{
    if (dynamic_types_)
    {
        // Create the data sample
        MemberId id;
        dynamic_data_ = static_cast<DynamicData*>(dynamic_pub_sub_type_->createData());

        if (nullptr == dynamic_data_)
        {
            logError(THROUGHPUTPUBLISHER,"Iteration failed: Failed to create Dynamic Data");
            return false;
        }

        // Modify the data Sample
        DynamicData* member_data = dynamic_data_->loan_value(
                dynamic_data_->get_member_id_at_index(1));

        for (uint32_t i = 0; i < msg_size; ++i)
        {
            member_data->insert_sequence_data(id);
            member_data->set_byte_value(0, id);
        }
        dynamic_data_->return_loaned_value(member_data);
    }
    // Create the static type for the given buffer size and the endpoints
    else if (init_static_types(msg_size) && create_data_endpoints())
    {
        // Create the data sample
        throughput_data_ = static_cast<ThroughputType*>(throughput_data_type_.create_data());
    }
    else
    {
        logError(THROUGHPUTPUBLISHER, "Error preparing static types and endpoints for testing");
        return false;
    }

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
    ThroughputCommandType command_sample;
    SampleInfo info;
    command_sample.m_command = TEST_STARTS;
    command_writer_->write(&command_sample);

    // If the subscriber does not acknowledge the TEST_STARTS in time, we consider something went wrong.
    std::chrono::steady_clock::time_point test_start_sent_tp = std::chrono::steady_clock::now();
    if (ReturnCode_t::RETCODE_OK != command_writer_->wait_for_acknowledgments({20,0}))
    {
        logError(THROUGHPUTPUBLISHER, "Something went wrong: The subscriber has not acknowledged the TEST_STARTS command.");
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
            if (dynamic_types_)
            {
                dynamic_data_->set_uint32_value(dynamic_data_->get_uint32_value(0) + 1, 0);
                data_writer_->write(dynamic_data_);
            }
            else
            {
                throughput_data_->seqnum++;
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
         */
        std::this_thread::sleep_for(recovery_duration_ns - (t_end_ - batch_start));

        clock_overhead += t_overhead_ * 2; // We access the clock twice per batch.
    }
    command_sample.m_command = TEST_ENDS;

    command_writer_->write(&command_sample);
    size_t removed = 0;
    data_writer_->clear_history(&removed);

    // If the subscriber does not acknowledge the TEST_ENDS in time, we consider something went wrong.
    if (ReturnCode_t::RETCODE_OK
            != command_writer_->wait_for_acknowledgments({20,0}))
    {
        logError(THROUGHPUTPUBLISHER, "Something went wrong: The subscriber has not acknowledged the TEST_ENDS command.");
        return false;
    }

    // Delete the Data Sample
    if (dynamic_types_)
    {
        DynamicDataFactory::get_instance()->delete_data(dynamic_data_);
    }
    else
    {
        throughput_data_type_.delete_data(throughput_data_);

        // Remove endpoints associated to the given payload size
        if(!destroy_data_endpoints())
        {
            logError(THROUGHPUTPUBLISHER, "Static endpoints for payload size " << msg_size << " could not been removed");
        }
    }

    // Results processing
    uint32_t num_results_received = 0;
    bool results_error = false;
    while ( !results_error && num_results_received < subscribers_ )
    {
        if (ReturnCode_t::RETCODE_OK != command_reader_->wait_for_unread_message({20,0})
            && ReturnCode_t::RETCODE_OK != command_reader_->take_next_sample(&command_sample, &info)
            && info.valid_data)
        {
            if (command_sample.m_command == TEST_RESULTS)
            {
                num_results_received++;

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

bool ThroughputPublisher::init_dynamic_types()
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (dynamic_pub_sub_type_)
    {
        logError(THROUGHPUTPUBLISHER, "ERROR DYNAMIC DATA type already initialized");
        return false;
    }
    else if(participant_->find_type(ThroughputDataType::type_name_))
    {
        logError(THROUGHPUTPUBLISHER, "ERROR DYNAMIC DATA type already registered");
        return false;
    }

    // Dummy type registration
    // Create basic builders
    DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

    // Add members to the struct.
    struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
    struct_type_builder->add_member(1, "data", DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                DynamicTypeBuilderFactory::get_instance()->create_byte_type(), BOUND_UNLIMITED));
    struct_type_builder->set_name(ThroughputDataType::type_name_);
    dynamic_pub_sub_type_.reset(new DynamicPubSubType(struct_type_builder->build()));

    // Register the data type
    if (ReturnCode_t::RETCODE_OK
            != dynamic_pub_sub_type_.register_type(participant_))
    {
        logError(THROUGHPUTPUBLISHER, "ERROR registering the DYNAMIC DATA topic");
        return false;
    }

    return true;
}

bool ThroughputPublisher::init_static_types(uint32_t payload)
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (throughput_data_type_)
    {
        logError(THROUGHPUTPUBLISHER, "ERROR STATIC DATA type already initialized");
        return false;
    }
    else if(participant_->find_type(ThroughputDataType::type_name_))
    {
        logError(THROUGHPUTPUBLISHER, "ERROR STATIC DATA type already registered");
        return false;
    }

    // Create the static type
    throughput_data_type_.reset(new ThroughputDataType(payload));
    // Register the static type
    if (ReturnCode_t::RETCODE_OK
            != throughput_data_type_.register_type(participant_))
    {
        return false;
    }

    return true;
}

bool ThroughputPublisher::create_data_endpoints()
{
    if (nullptr != data_pub_topic_)
    {
        logError(THROUGHPUTPUBLISHER, "ERROR topic already initialized");
        return false;
    }

    if (nullptr != data_writer_)
    {
        logError(THROUGHPUTPUBLISHER, "ERROR data_writer_ already initialized");
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
        logError(THROUGHPUTPUBLISHER, "ERROR creating the DATA topic");
        return false;
    }

    // Create the DataWriter
    // Reliability
    ReliabilityQosPolicy rp;
    if (reliable_)
    {
        rp.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
        dw_qos_.reliability(rp);

        RTPSReliableWriterQos rw_qos;
        rw_qos.times.heartbeatPeriod.seconds = 0;
        rw_qos.times.heartbeatPeriod.nanosec = 100000000;
        rw_qos.times.nackSupressionDuration = {0,0};
        rw_qos.times.nackResponseDelay = {0,0};

        dw_qos_.reliable_writer_qos(rw_qos);
    }
    else
    {
        rp.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
        dw_qos_.reliability(rp);
    }

    // Create the endpoint
    if (nullptr !=
            (data_writer_ = publisher_->create_datawriter(
                data_pub_topic_,
                dw_qos_,
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
            || ReturnCode_t::RETCODE_OK != publisher_->delete_datawriter(data_writer_))
    {
        logError(THROUGHPUTPUBLISHER, "ERROR destroying the DataWriter");
        return false;
    }
    data_writer_ = nullptr;

    // Delete the Topic
    if (nullptr == data_pub_topic_
            || ReturnCode_t::RETCODE_OK != participant_->delete_topic(data_pub_topic_))
    {
        logError(THROUGHPUTPUBLISHER, "ERROR destroying the DATA topic");
        return false;
    }
    data_pub_topic_ = nullptr;

    // Delete the Type
    if (ReturnCode_t::RETCODE_OK
            !=participant_->unregister_type(ThroughputDataType::type_name_))
    {
        logError(THROUGHPUTPUBLISHER, "ERROR unregistering the DATA type");
        return false;
    }

    throughput_data_type_.reset();
    dynamic_pub_sub_type_.reset();
    DynamicTypeBuilderFactory::delete_instance();

    return true;
}
