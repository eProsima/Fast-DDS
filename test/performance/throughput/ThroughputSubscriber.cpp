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

#include <vector>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// *******************************************************************************************
// ************************************ DATA SUB LISTENER ************************************
// *******************************************************************************************

void ThroughputSubscriber::DataReaderListener::reset()
{
    last_seq_num_ = 0;
    first_ = true;
    lost_samples_ = 0;
    matched_ = 0;
}

/*
 * Our current inplementation of MatchedStatus info:
 * - total_count(_change) holds the actual number of matches
 * - current_count(_change) is a flag to signal match or unmatch.
 *   (TODO: review if fits standard definition)
 * */

void ThroughputSubscriber::DataReaderListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& match_info)
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

void ThroughputSubscriber::DataReaderListener::on_data_available(DataReader* reader)
{
    // In case the TSubscriber is removing entities because a TEST_ENDS msg, it waits
    auto& sub = throughput_subscriber_;
    void * data = sub.dynamic_types_ ? sub.dynamic_data_ : sub.throughput_type_;

    if (nullptr == data)
    {
        std::cout << "DATA MESSAGE RECEIVED BEFORE COMMAND READY_TO_START" << std::endl;
        return;
    }

    while (reader_->take_next_sample(
                (void*)throughput_subscriber_.dynamic_data_,
                &info_))
    {
        if (info_.sampleKind == ALIVE)
        {
            uint32_t seq_num = sub.dynamic_types_
                ? sub.dynamic_data_type_->get_uint32_value(0)
                : sub.throughput_type_->seqnum;

            if ((last_seq_num_ + 1) < seq_num)
            {
                lost_samples_ += seq_num - last_seq_num_ - 1;
            }
            last_seq_num_ = seq_num;
        }
        else
        {
            std::cout << "NOT ALIVE DATA RECEIVED" << std::endl;
        }
    }
}

void ThroughputSubscriber::DataReaderListener::save_numbers()
{
    saved_last_seq_num_ = last_seq_num_;
    saved_lost_samples_ = lost_samples_;
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

void ThroughputSubscriber::CommandReaderListener::on_data_available(DataReader* reader) {}

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

ThroughputSubscriber::ThroughputSubscriber(
    : data_sub_listener_(*this)
    , command_sub_listener_(*this)
    , command_pub_listener_(*this)
{
}

ThroughputSubscriber::~ThroughputSubscriber()
{
    if (dynamic_types_)
    {
        destroy_data_endpoints();
    }
    else if (nullptr != data_reader_
            || nullptr != data_pub_topic_
            || !throughput_data_type_)
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR unregistering the DATA type");
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
    logInfo(THROUGHPUTSUBSCRIBER, "Sub: Participant removed");
}

bool ThroughputSubscriber::init(
        bool reliable,
        uint32_t pid,
        bool hostname,
        const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
        const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
        const std::string& xml_config_file,
        bool dynamic_types,
        int forced_domain)
{
    pid_ = pid;
    hostname_ = hostname;
    dynamic_types_ = dynamic_types;
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
        std::cout << "ERROR creating participant" << std::endl;
        ready_ = false;
        return;
    }

    // Create the command data type
    throughput_command_type_.reset(new ThroughputCommandDataType());

    // Register the command data type
    if (ReturnCode_t::RETCODE_OK
            != throughput_command_type_.register_type(participant_))
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR registering command type");
        return false;
    }

    /* Create Publisher */
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR creating the Publisher");
        return false;
    }

    /* Create Subscriber */
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR creating the Subscriber");
        return false;
    }

    /* Update DataReaderQoS from xml profile data */
    std::string profile_name = "subscriber_profile";

    if (xml_config_file_.length() > 0
        && ReturnCode_t::RETCODE_OK != subscriber_->get_datareader_qos_from_profile(profile_name, dr_qos_))
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR unable to retrieve the " << profile_name);
        return false;
    }
    // Load the property policy specified
    dr_qos_.properties(property_policy);

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
            logError(THROUGHPUTSUBSCRIBER, "ERROR creating the COMMAND Sub topic");
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
            logError(THROUGHPUTSUBSCRIBER, "ERROR creating the COMMAND Pub topic");
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
            logError(THROUGHPUTSUBSCRIBER, "ERROR creating the COMMAND DataWriter");
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
            logError(THROUGHPUTSUBSCRIBER, "ERROR creating the COMMAND DataReader");
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
    return dynamic_types_ ? init_dynamic_types() && create_data_endpoints() : true;
}

void ThroughputSubscriber::process_message()
{
    if (command_reader_->wait_for_unread_message({100, 0}))
    {
        if (command_reader_->take_next_sample(
                (void*)&command_reader_listener_.command_type_,
                &command_reader_listener_.info_))
        {
            switch (command_reader_listener_.command_type_.m_command)
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
                    if (dynamic_types_)
                    {
                        assert(nullptr == dynamic_data_);

                        // Create the data sample
                        MemberId id;
                        dynamic_data_ = static_cast<DynamicData*>(dynamic_pub_sub_type_->createData());

                        if (nullptr == dynamic_data_)
                        {
                            logError(THROUGHPUTSUBSCRIBER,"Iteration failed: Failed to create Dynamic Data");
                            return 2;
                        }

                        // Modify the data Sample
                        DynamicData* member_data = dynamic_data_->loan_value(
                                dynamic_data_->get_member_id_at_index(1));

                        for (uint32_t i = 0; i < command_sub_listener_.command_type_.m_size ; ++i)
                        {
                            member_data->insert_sequence_data(id);
                            member_data->set_byte_value(0, id);
                        }
                        dynamic_data_->return_loaned_value(member_data);
                    }
                    else
                    {
                        // Validate QoS settings
                        uint32_t max_demmand = command_reader_listener_.command_type_.m_demand;
                        if (dr_qos_.history().kind == KEEP_LAST_HISTORY_QOS)
                        {
                            // Ensure that the history depth is at least the demand
                            if (dr_qos_.history().depth < 0 ||
                                    static_cast<uint32_t>(dr_qos_.history().depth) < max_demand)
                            {
                                logWarning(THROUGHPUTSUBSCRIBER, "Setting history depth to " << max_demand);
                                dr_qos_.resource_limits().max_samples = max_demand;
                                dr_qos_.history().depth = max_demand;
                            }
                        }
                        // KEEP_ALL case
                        else
                        {
                            // Ensure that the max samples is at least the demand
                            if (dr_qos_.resource_limits().max_samples < 0 ||
                                    static_cast<uint32_t>(dr_qos_.resource_limits().max_samples) < max_demand)
                            {
                                logWarning(THROUGHPUTSUBSCRIBER, "Setting resource limit max samples to " << max_demand);
                                dr_qos_.resource_limits().max_samples = max_demand;
                            }
                        }
                        // Set the allocated samples to the max_samples. This is because allocated_sample must be <= max_samples
                        dr_qos_.resource_limits().allocated_samples = dr_qos_.resource_limits().max_samples;

                        if (init_static_types(data_size) && create_data_endpoints())
                        {
                            assert(nullptr == throughput_data_);
                            // Create the data sample
                            throughput_data_ = static_cast<ThroughputType*>(throughput_data_type_.create_data());

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
                            logError(THROUGHPUTSUBSCRIBER, "Error preparing static types and endpoints for testing");
                            return 2;
                        }
                    }

                    break;
                }
                case (READY_TO_START):
                {
                    std::cout << "-----------------------------------------------------------------------" << std::endl;
                    std::cout << "Command: READY_TO_START" << std::endl;
                    data_size_ = command_sub_listener_.command_type_.m_size;
                    demand_ = command_sub_listener_.command_type_.m_demand;

                    ThroughputCommandType command_sample(BEGIN);
                    data_reader_listener_.reset();
                    command_publisher_->write(&command_sample);
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
                    {
                        return 1; // results processing is done outside
                    }
                    break;
                }
                case TYPE_DISPOSE:
                {
                    // Remove the dynamic_data_ object, protect form ongoing callbacks
                    if (dynamic_types_)
                    {
                        DynamicDataFactory::get_instance()->delete_data(dynamic_data_);
                        dynamic_data_ = nullptr;
                    }
                    else
                    {
                        // remove the data endpoints on static case
                        if (destroy_data_endpoints())
                        {
                            throughput_data_type_.delete_data(throughput_data_);
                            throughput_type_ = nullptr;
                        }
                        else
                        {
                            logError(THROUGHPUTSUBSCRIBER,"Iteration failed: Failed to remove static data endpoints");
                            return 2;
                        }
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
}

void ThroughputSubscriber::run()
{
    std::cout << "Sub Waiting for command discovery" << std::endl;
    {
        std::unique_lock<std::mutex> disc_lock(mutex_);
        command_discovery_cv_.wait(disc_lock, [&]()
                {
                    if (dynamic_types)
                    {
                        // full command and data endpoints discovery
                        return total_matches() == 3;
                    }
                    else
                    {
                        // The only endpoints present should be command ones
                        return total_matches() == 2);
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
            while (dynamic_types_ && data_reader_->wait_for_unread_message({0,1000}))
            {
                std::cout << "Waiting clean state" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            std::cout << "Sending results" << std::endl;
            ThroughputCommandType command_sample;
            command_sample.m_command = TEST_RESULTS;
            command_sample.m_demand = demand_;
            command_sample.m_size = data_size_ + ThroughputType::overhead;
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
            command_writer_->write(&command_sample);

            std::cout << "-----------------------------------------------------------------------" << std::endl;
        }

    } while (stop_count != 2);

    if (!dynamic_types)
    {
        std::cout << "Sub Waiting for command undiscovery" << std::endl;

        std::unique_lock<std::mutex> disc_lock(mutex_);
        command_discovery_cv_.wait(disc_lock, [&]()
                {
                // The only endpoints present should be command ones
                return total_matches() == 2);
                });
        std::cout << "Sub un-Discovery command complete" << std::endl;
    }

    return;
}

bool ThroughputSubscriber::init_dynamic_types()
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (dynamic_pub_sub_type_)
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR DYNAMIC DATA type already initialized");
        return false;
    }
    else if(participant_->find_type(ThroughputDataType::type_name_))
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR DYNAMIC DATA type already registered");
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
        logError(THROUGHPUTSUBSCRIBER, "ERROR registering the DYNAMIC DATA topic");
        return false;
    }

    return true;
}

bool ThroughputSubscriber::init_static_types(uint32_t payload)
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (throughput_data_type_)
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR STATIC DATA type already initialized");
        return false;
    }
    else if(participant_->find_type(ThroughputDataType::type_name_))
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR STATIC DATA type already registered");
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

bool ThroughputSubscriber::create_data_endpoints()
{
    if (nullptr != data_pub_topic_)
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR topic already initialized");
        return false;
    }

    if (nullptr != data_reader_)
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR data_writer_ already initialized");
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
        logError(THROUGHPUTSUBSCRIBER, "ERROR creating the DATA topic");
        return false;
    }

    // Create the DataReader
    // Reliability
    ReliabilityQosPolicy rp;
    rp.kind = reliable_ ? eprosima::fastrtps::RELIABLE_RELIABILITY_QOS: eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
    dr_qos_.reliability(rp);

    // Create the endpoint
    if (nullptr !=
            (data_writer_ = publisher_->create_datareader(
                data_sub_topic_,
                dw_qos_,
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
            || ReturnCode_t::RETCODE_OK != subscriber_->delete_datareader(data_reader_))
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR destroying the DataWriter");
        return false;
    }
    data_reader_ = nullptr;
    data_reader_listener_.reset();

    // Delete the Topic
    if (nullptr == data_sub_topic_
            || ReturnCode_t::RETCODE_OK != participant_->delete_topic(data_sub_topic_))
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR destroying the DATA topic");
        return false;
    }
    data_sub_topic_ = nullptr;

    // Delete the Type
    if (ReturnCode_t::RETCODE_OK
            !=participant_->unregister_type(ThroughputDataType::type_name_))
    {
        logError(THROUGHPUTSUBSCRIBER, "ERROR unregistering the DATA type");
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
