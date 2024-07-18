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

#include <chrono>
#include <thread>
#include <vector>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
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
// ************************************ DATA SUB LISTENER ************************************
// *******************************************************************************************

void ThroughputSubscriber::DataReaderListener::reset()
{
    last_seq_num_ = 0;
    lost_samples_ = 0;
    received_samples_ = 0;
    matched_ = 0;
    enable_ = true;
}

void ThroughputSubscriber::DataReaderListener::disable()
{
    enable_ = false;
}

/*
 * Our current inplementation of MatchedStatus info:
 * - total_count(_change) holds the actual number of matches
 * - current_count(_change) is a flag to signal match or unmatch.
 *   (TODO: review if fits standard definition)
 * */

void ThroughputSubscriber::DataReaderListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{

    if (1 == info.current_count)
    {
        std::cout << C_RED << "Sub: DATA Sub Matched" << C_DEF << std::endl;
    }
    else
    {
        std::cout << C_RED << "DATA SUBSCRIBER MATCHING REMOVAL" << C_DEF << std::endl;
    }

    matched_ = info.total_count;
    throughput_subscriber_.data_discovery_cv_.notify_one();
}

void ThroughputSubscriber::DataReaderListener::on_data_available(
        DataReader* reader)
{
    if (!enable_)
    {
        return;
    }

    // In case the TSubscriber is removing entities because a TEST_ENDS msg, it waits
    auto& sub = throughput_subscriber_;

    if (sub.data_loans_)
    {
        SampleInfoSeq infos;
        LoanableSequence<ThroughputType> data_seq;

        if (RETCODE_OK != reader->take(data_seq, infos))
        {
            EPROSIMA_LOG_INFO(ThroughputTest, "Problem reading Subscriber echoed loaned test data");
            return;
        }

        // Check for lost samples
        auto size = data_seq.length();
        uint32_t last_seq_num = last_seq_num_;

        for (int32_t i = 0; i < size; ++i)
        {
            uint32_t seq_num = std::max(data_seq[i].seqnum, last_seq_num);
            if (seq_num > last_seq_num + 1)
            {
                if (!reader->is_sample_valid(&data_seq[i], &infos[i]))
                {
                    // This was overridden. Counts as a loss
                    ++lost_samples_;
                    ++last_seq_num;
                    continue;
                }
            }
            last_seq_num = seq_num;
            received_samples_ += 1;
        }

        if ((last_seq_num_ + size) < last_seq_num)
        {
            lost_samples_ += last_seq_num - last_seq_num_ - size;
        }
        last_seq_num_ = last_seq_num;

        // release the reader loan
        if (RETCODE_OK != reader->return_loan(data_seq, infos))
        {
            EPROSIMA_LOG_INFO(ThroughputTest, "Problem returning loaned test data");
            return;
        }
    }
    else
    {
        void* data = sub.dynamic_types_ ? (void*)sub.dynamic_data_ : (void*)sub.throughput_data_;
        assert(nullptr != data);

        while (RETCODE_OK == reader->take_next_sample(data, &info_))
        {
            if (info_.valid_data)
            {
                uint32_t seq_num {0};

                if (sub.dynamic_types_)
                {
                    (*sub.dynamic_data_)->get_uint32_value(seq_num, 0);
                }
                else
                {
                    seq_num = sub.throughput_data_->seqnum;
                }

                if ((last_seq_num_ + 1) < seq_num)
                {
                    lost_samples_ += seq_num - last_seq_num_ - 1;
                }
                last_seq_num_ = seq_num;
                received_samples_ += 1;
            }
            else
            {
                std::cout << "invalid data received" << std::endl;
            }
        }
    }
}

void ThroughputSubscriber::DataReaderListener::save_numbers()
{
    saved_last_seq_num_ = last_seq_num_;
    saved_lost_samples_ = lost_samples_;
    saved_received_samples_ = received_samples_;
}

// *******************************************************************************************
// *********************************** COMMAND SUB LISTENER **********************************
// *******************************************************************************************

void ThroughputSubscriber::CommandReaderListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (1 == info.current_count)
    {
        std::cout << C_RED << "Sub: COMMAND Sub Matched" << C_DEF << std::endl;
    }
    else
    {
        std::cout << C_RED << "Sub: COMMAND SUBSCRIBER MATCHING REMOVAL" << C_DEF << std::endl;
    }

    matched_ = info.total_count;
    throughput_subscriber_.command_discovery_cv_.notify_one();
}

void ThroughputSubscriber::CommandReaderListener::on_data_available(
        DataReader* )
{
}

// *******************************************************************************************
// *********************************** COMMAND PUB LISTENER **********************************
// *******************************************************************************************

void ThroughputSubscriber::CommandWriterListener::on_publication_matched(
        DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if ( 1 == info.current_count)
    {
        std::cout << C_RED << "Sub: COMMAND Pub Matched" << C_DEF << std::endl;
    }
    else
    {
        std::cout << C_RED << "Sub: COMMAND PUBLISHER MATCHING REMOVAL" << C_DEF << std::endl;
    }

    matched_ = info.total_count;
    throughput_subscriber_.command_discovery_cv_.notify_one();
}

// *******************************************************************************************
// ********************************** THROUGHPUT SUBSCRIBER **********************************
// *******************************************************************************************

ThroughputSubscriber::ThroughputSubscriber()
    : data_reader_listener_(*this)
    , command_reader_listener_(*this)
    , command_writer_listener_(*this)
{
}

ThroughputSubscriber::~ThroughputSubscriber()
{
    if (dynamic_types_)
    {
        destroy_data_endpoints();
    }
    else if (nullptr != data_reader_
            || nullptr != data_sub_topic_
            || throughput_data_type_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR unregistering the DATA type");
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
    EPROSIMA_LOG_INFO(THROUGHPUTSUBSCRIBER, "Sub: Participant removed");
}

bool ThroughputSubscriber::init(
        bool reliable,
        uint32_t pid,
        bool hostname,
        const eprosima::fastdds::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastdds::rtps::PropertyPolicy& property_policy,
        const std::string& xml_config_file,
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
    shared_memory_ = shared_memory;
    data_loans_ = data_loans;
    reliable_ = reliable;
    forced_domain_ = forced_domain;
    xml_config_file_ = xml_config_file;

    /* Create DomainParticipant*/
    std::string participant_profile_name = "sub_participant_profile";
    DomainParticipantQos pqos;

    // Default domain
    DomainId_t domainId = pid % 230;

    // Default participant name
    pqos.name("throughput_test_subscriber");

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
        std::cout << "ERROR creating participant" << std::endl;
        return false;
    }

    // Create the command data type
    throughput_command_type_.reset(new ThroughputCommandDataType());

    // Register the command data type
    if (RETCODE_OK
            != throughput_command_type_.register_type(participant_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR registering command type");
        return false;
    }

    /* Create Publisher */
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR creating the Publisher");
        return false;
    }

    /* Create Subscriber */
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR creating the Subscriber");
        return false;
    }

    /* Update DataReaderQoS from xml profile data */
    std::string profile_name = "subscriber_profile";

    if (xml_config_file_.length() > 0
            && RETCODE_OK != subscriber_->get_datareader_qos_from_profile(profile_name, dr_qos_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR unable to retrieve the " << profile_name);
        return false;
    }

    // Load the property policy specified
    dr_qos_.properties(property_policy);

    // Reliability
    ReliabilityQosPolicy rp;
    rp.kind =
            reliable_ ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS: eprosima::fastdds::dds::
                    BEST_EFFORT_RELIABILITY_QOS;
    dr_qos_.reliability(rp);

    // Set data sharing according with cli. Is disabled by default in all xml profiles
    if (Arg::EnablerValue::ON == data_sharing_)
    {
        DataSharingQosPolicy dsp;
        dsp.on("");
        dr_qos_.data_sharing(dsp);
    }
    else if (Arg::EnablerValue::OFF == data_sharing_)
    {
        DataSharingQosPolicy dsp;
        dsp.off();
        dr_qos_.data_sharing(dsp);
    }

    // Create Command topic
    {
        std::ostringstream topic_name;
        topic_name << "ThroughputTest_Command_";
        if (hostname)
        {
            topic_name << asio::ip::host_name() << "_";
        }
        topic_name << pid << "_PUB2SUB";

        command_sub_topic_ = participant_->create_topic(
            topic_name.str(),
            "ThroughputCommand",
            TOPIC_QOS_DEFAULT);

        if (nullptr == command_sub_topic_)
        {
            EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR creating the COMMAND Sub topic");
            return false;
        }

        topic_name.str("");
        topic_name.clear();
        topic_name << "ThroughputTest_Command_";
        if (hostname)
        {
            topic_name << asio::ip::host_name() << "_";
        }
        topic_name << pid << "_SUB2PUB";

        command_pub_topic_ = participant_->create_topic(
            topic_name.str(),
            "ThroughputCommand",
            TOPIC_QOS_DEFAULT);

        if (nullptr == command_pub_topic_)
        {
            EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR creating the COMMAND Pub topic");
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
            EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR creating the COMMAND DataWriter");
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
            EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR creating the COMMAND DataReader");
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
    std::cout << "Subscriber's clock access overhead: " << t_overhead_.count() << " us" << std::endl;

    // Endpoints using dynamic data endpoints span the whole test duration
    // Static types and endpoints are created for each payload iteration
    return dynamic_types_ ? init_dynamic_types() && create_data_endpoints(dr_qos_) : true;
}

int ThroughputSubscriber::process_message()
{
    ThroughputCommandType command;
    SampleInfo info;

    if (command_reader_->wait_for_unread_message({100, 0}))
    {
        if (RETCODE_OK == command_reader_->take_next_sample(
                    (void*)&command,
                    &info))
        {
            switch (command.m_command)
            {
                case (DEFAULT):
                {
                    break;
                }
                case (BEGIN):
                {
                    break;
                }
                case TYPE_NEW:
                {
                    auto dr_qos = dr_qos_;

                    if (dynamic_types_)
                    {
                        assert(nullptr == dynamic_data_);

                        // Create the data sample
                        dynamic_data_ =
                                static_cast<DynamicData::_ref_type*>(dynamic_pub_sub_type_->create_data());

                        if (nullptr == dynamic_data_)
                        {
                            EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER,
                                    "Iteration failed: Failed to create Dynamic Data");
                            return 2;
                        }

                        // Modify the data Sample
                        DynamicData::_ref_type member_data = (*dynamic_data_)->loan_value(
                            (*dynamic_data_)->get_member_id_at_index(1));

                        for (uint32_t i = 0; i < command.m_size; ++i)
                        {
                            member_data->set_byte_value(i, 0);
                        }
                        (*dynamic_data_)->return_loaned_value(member_data);
                    }
                    else
                    {
                        // Validate QoS settings
                        uint32_t max_demand = command.m_demand;
                        if (dr_qos.history().kind == KEEP_LAST_HISTORY_QOS)
                        {
                            // Ensure that the history depth is at least the demand
                            if (dr_qos.history().depth < 0 ||
                                    static_cast<uint32_t>(dr_qos.history().depth) < max_demand)
                            {
                                EPROSIMA_LOG_WARNING(THROUGHPUTSUBSCRIBER, "Setting history depth to " << max_demand);
                                dr_qos.resource_limits().max_samples = max_demand;
                                dr_qos.history().depth = max_demand;
                            }
                        }
                        // KEEP_ALL case
                        else
                        {
                            // Ensure that the max samples is at least the demand
                            if (dr_qos.resource_limits().max_samples <= 0 ||
                                    static_cast<uint32_t>(dr_qos.resource_limits().max_samples) < max_demand)
                            {
                                EPROSIMA_LOG_WARNING(THROUGHPUTSUBSCRIBER,
                                        "Setting resource limit max samples to " << max_demand);
                                dr_qos.resource_limits().max_samples = max_demand;
                            }
                        }
                        // Set the allocated samples to the max_samples. This is because allocated_sample must be <= max_samples
                        dr_qos.resource_limits().allocated_samples = dr_qos.resource_limits().max_samples;

                        if (init_static_types(command.m_size) && create_data_endpoints(dr_qos))
                        {
                            assert(nullptr == throughput_data_);

                            if (!data_loans_)
                            {
                                // Create the data sample
                                throughput_data_ = static_cast<ThroughputType*>(throughput_data_type_.create_data());
                            }

                            // wait for data endpoint discovery
                            {
                                std::cout << "Waiting for data discovery" << std::endl;
                                std::unique_lock<std::mutex> data_disc_lock(mutex_);
                                data_discovery_cv_.wait(data_disc_lock, [&]()
                                        {
                                            return total_matches() == 3;
                                        });
                                std::cout << "Discovery data complete" << std::endl;
                            }
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER,
                                    "Error preparing static types and endpoints for testing");
                            return 2;
                        }
                    }

                    break;
                }
                case (READY_TO_START):
                {
                    std::cout << "-----------------------------------------------------------------------" << std::endl;
                    std::cout << "Command: READY_TO_START" << std::endl;
                    data_size_ = command.m_size;
                    demand_ = command.m_demand;

                    SampleInfoSeq infos;
                    LoanableSequence<ThroughputType> data_seq;

                    // Consume history
                    while (data_reader_->wait_for_unread_message({0, 1000000}))
                    {
                        while (RETCODE_OK == data_reader_->take(data_seq, infos))
                        {
                            if (RETCODE_OK != data_reader_->return_loan(data_seq, infos))
                            {
                                EPROSIMA_LOG_INFO(ThroughputTest, "Problem returning loan");
                            }
                        }
                    }
                    data_reader_listener_.reset();

                    ThroughputCommandType command_sample(BEGIN);
                    command_writer_->write(&command_sample);
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
                    data_reader_listener_.save_numbers();
                    data_reader_listener_.disable();
                    return 1; // results processing is done outside
                }
                case TYPE_DISPOSE:
                {
                    // Remove the dynamic_data_ object, protect form ongoing callbacks
                    if (dynamic_types_)
                    {
                        dynamic_pub_sub_type_.delete_data(dynamic_data_);
                        dynamic_data_ = nullptr;
                    }
                    else
                    {
                        if (!data_loans_)
                        {
                            // data removal
                            throughput_data_type_.delete_data(throughput_data_);
                        }
                        throughput_data_ = nullptr;

                        // remove the data endpoints on static case
                        if (!destroy_data_endpoints())
                        {
                            EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER,
                                    "Iteration failed: Failed to remove static data endpoints");
                            return 2;
                        }

                        // announced the endpoints associated to the data type are removed
                        ThroughputCommandType command_sample(TYPE_REMOVED);
                        command_writer_->write(&command_sample);

                        // we don't need to wait acknowledgement of the above message because ThroughputPublisher
                        // will only send ALL_STOPS when it's received
                    }
                    break;
                }
                case (ALL_STOPS):
                {
                    std::cout << "-----------------------------------------------------------------------" << std::endl;
                    std::cout << "Command: ALL_STOPS" << std::endl;
                    return 2;
                }
                default:
                {
                    break;
                }
            }
        }
    }

    return 0;
}

void ThroughputSubscriber::run()
{
    std::cout << "Sub Waiting for command discovery" << std::endl;
    {
        std::unique_lock<std::mutex> disc_lock(mutex_);
        command_discovery_cv_.wait(disc_lock, [&]()
                {
                    if (dynamic_types_)
                    {
                        // full command and data endpoints discovery
                        return total_matches() == 3;
                    }
                    else
                    {
                        // The only endpoints present should be command ones
                        return total_matches() == 2;
                    }
                });
    }
    std::cout << "Sub Discovery command complete" << std::endl;

    int stop_count;
    do
    {
        stop_count = process_message();

        if (stop_count == 1)
        {
            // Here the static data endpoints and type still exists
            while (dynamic_types_ && data_reader_->wait_for_unread_message({0, 1000}))
            {
                std::cout << "Waiting clean state" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            std::cout << "Sending results" << std::endl;
            ThroughputCommandType command_sample;
            command_sample.m_command = TEST_RESULTS;
            command_sample.m_demand = demand_;
            command_sample.m_size = data_size_ + (uint32_t)ThroughputType::overhead;
            command_sample.m_lastrecsample = data_reader_listener_.saved_last_seq_num_;
            command_sample.m_lostsamples = data_reader_listener_.saved_lost_samples_;
            command_sample.m_receivedsamples = data_reader_listener_.saved_received_samples_;

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
            std::cout << "Received Samples: " << command_sample.m_receivedsamples << std::endl;
            std::cout << "Samples per second: "
                      << (double)(command_sample.m_receivedsamples) * 1000000 /
                command_sample.m_totaltime
                      << std::endl;
            std::cout << "Test of size " << command_sample.m_size << " and demand " << command_sample.m_demand <<
                " ends." << std::endl;
            command_writer_->write(&command_sample);

            std::cout << "-----------------------------------------------------------------------" << std::endl;
        }

    } while (stop_count != 2);

    return;
}

bool ThroughputSubscriber::init_dynamic_types()
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (dynamic_pub_sub_type_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR DYNAMIC DATA type already initialized");
        return false;
    }
    else if (participant_->find_type(ThroughputDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR DYNAMIC DATA type already registered");
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
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR registering the DYNAMIC DATA topic");
        return false;
    }

    return true;
}

bool ThroughputSubscriber::init_static_types(
        uint32_t payload)
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (throughput_data_type_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR STATIC DATA type already initialized");
        return false;
    }
    else if (participant_->find_type(ThroughputDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR STATIC DATA type already registered");
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

bool ThroughputSubscriber::create_data_endpoints(
        const DataReaderQos& dr_qos)
{
    if (nullptr != data_sub_topic_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR topic already initialized");
        return false;
    }

    if (nullptr != data_reader_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR data_writer_ already initialized");
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

    data_sub_topic_ = participant_->create_topic(
        topic_name.str(),
        ThroughputDataType::type_name_,
        TOPIC_QOS_DEFAULT);

    if (nullptr == data_sub_topic_)
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR creating the DATA topic");
        return false;
    }

    // Create the endpoint
    if (nullptr ==
            (data_reader_ = subscriber_->create_datareader(
                data_sub_topic_,
                dr_qos,
                &data_reader_listener_)))
    {
        return false;
    }

    return true;
}

bool ThroughputSubscriber::destroy_data_endpoints()
{
    assert(nullptr != participant_);
    assert(nullptr != subscriber_);

    // Delete the endpoint
    if (nullptr == data_reader_
            || RETCODE_OK != subscriber_->delete_datareader(data_reader_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR destroying the DataWriter");
        return false;
    }
    data_reader_ = nullptr;
    data_reader_listener_.reset();

    // Delete the Topic
    if (nullptr == data_sub_topic_
            || RETCODE_OK != participant_->delete_topic(data_sub_topic_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR destroying the DATA topic");
        return false;
    }
    data_sub_topic_ = nullptr;

    // Delete the Type
    if (RETCODE_OK
            != participant_->unregister_type(ThroughputDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(THROUGHPUTSUBSCRIBER, "ERROR unregistering the DATA type");
        return false;
    }

    throughput_data_type_.reset();

    return true;
}

int ThroughputSubscriber::total_matches() const
{
    // no need to lock because is used always within a
    // condition variable wait predicate

    int count = data_reader_listener_.get_matches()
            + command_writer_listener_.get_matches()
            + command_reader_listener_.get_matches();

    // Each endpoint has a mirror counterpart in the ThroughputPublisher
    // thus, the maximun number of matches is 3
    assert(count >= 0 && count <= 3 );
    return count;
}
