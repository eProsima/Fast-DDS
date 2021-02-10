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

#include <cassert>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastdds/rtps/common/Time_t.h>

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;
using namespace eprosima::fastdds::dds;

LatencyTestSubscriber::LatencyTestSubscriber()
    : latency_command_type_(new TestCommandDataType())
    , data_writer_listener_(this)
    , data_reader_listener_(this)
    , command_writer_listener_(this)
    , command_reader_listener_(this)
{
}

LatencyTestSubscriber::~LatencyTestSubscriber()
{
    // Static type endpoints should have been remove for each payload iteration
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
        logError(LATENCYSUBSCRIBER, "ERROR unregistering the DATA type and/or removing the endpoints");
    }

    subscriber_->delete_datareader(command_reader_);
    participant_->delete_subscriber(subscriber_);

    publisher_->delete_datawriter(command_writer_);
    participant_->delete_publisher(publisher_);

    participant_->delete_topic(latency_command_sub_topic_);
    participant_->delete_topic(latency_command_pub_topic_);

    std::string TestCommandType("TestCommandType");
    participant_->unregister_type(TestCommandType);

    DomainParticipantFactory::get_instance()->delete_participant(participant_);

    logInfo(LatencyTest, "Sub: Participant removed");
}

bool LatencyTestSubscriber::init(
        bool echo,
        int samples,
        bool reliable,
        uint32_t pid,
        bool hostname,
        const PropertyPolicy& part_property_policy,
        const PropertyPolicy& property_policy,
        const std::string& xml_config_file,
        bool dynamic_data,
        int forced_domain,
        LatencyDataSizes& latency_data_sizes)
{
    // Initialize state
    xml_config_file_ = xml_config_file;
    echo_ = echo;
    samples_ = samples;
    dynamic_types_ = dynamic_data;
    forced_domain_ = forced_domain;
    pid_ = pid;
    hostname_ = hostname;

    data_size_sub_ = latency_data_sizes.sample_sizes();

    /* Create DomainParticipant*/
    std::string participant_profile_name = "sub_participant_profile";
    DomainParticipantQos pqos;

    // Default domain
    DomainId_t domainId = pid % 230;

    // Default participant name
    pqos.name("latency_test_subscriber");

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
    participant_ = DomainParticipantFactory::get_instance()->create_participant(domainId, pqos);
    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the command type
    if (ReturnCode_t::RETCODE_OK != latency_command_type_.register_type(participant_))
    {
        logError(LATENCYSUBSCRIBER, "ERROR registering the COMMAND type");
        return false;
    }

    /* Create Publisher */
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        logError(LATENCYSUBSCRIBER, "ERROR creating PUBLISHER");
        return false;
    }

    /* Create Subscriber */
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        logError(LATENCYSUBSCRIBER, "ERROR creating SUBSCRIBER");
        return false;
    }

    /* Update DataWriterQoS with xml profile data */
    if (xml_config_file_.length() > 0 )
    {
        std::string sub_profile_name = "sub_subscriber_profile";
        std::string pub_profile_name = "sub_publisher_profile";

        if ( ReturnCode_t::RETCODE_OK != publisher_->get_datawriter_qos_from_profile(pub_profile_name, dw_qos_))
        {
            logError(LATENCYSUBSCRIBER, "ERROR unable to retrieve the " << pub_profile_name << "from XML file");
            return false;
        }

        if ( ReturnCode_t::RETCODE_OK != subscriber_->get_datareader_qos_from_profile(sub_profile_name, dr_qos_))
        {
            logError(LATENCYSUBSCRIBER, "ERROR unable to retrieve the " << sub_profile_name);
            return false;
        }
    }
    // Create QoS Profiles
    else
    {
        ReliabilityQosPolicy rp;
        if (reliable)
        {
            rp.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;

            RTPSReliableWriterQos rw_qos;
            rw_qos.times.heartbeatPeriod.seconds = 0;
            rw_qos.times.heartbeatPeriod.nanosec = 100000000;
            dw_qos_.reliable_writer_qos(rw_qos);
        }
        else
        {
            rp.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
        }

        dr_qos_.reliability(rp);
        dw_qos_.reliability(rp);

        dw_qos_.properties(property_policy);
        dw_qos_.endpoint().history_memory_policy = MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

        dr_qos_.properties(property_policy);
        dr_qos_.endpoint().history_memory_policy = MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
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

        latency_command_sub_topic_ = participant_->create_topic(
            topic_name.str(),
            "TestCommandType",
            TOPIC_QOS_DEFAULT);

        if (latency_command_sub_topic_ == nullptr)
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

        latency_command_pub_topic_ = participant_->create_topic(
            topic_name.str(),
            "TestCommandType",
            TOPIC_QOS_DEFAULT);

        if (latency_command_pub_topic_ == nullptr)
        {
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

        command_writer_ = publisher_->create_datawriter(
            latency_command_pub_topic_,
            cw_qos,
            &command_writer_listener_);

        if (command_writer_ == nullptr)
        {
            return false;
        }
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

void LatencyTestSubscriber::LatencyDataWriterListener::on_publication_matched(
        DataWriter* writer,
        const PublicationMatchedStatus& info)
{
    (void)writer;

    std::unique_lock<std::mutex> lock(latency_subscriber_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        logInfo(LatencyTest, C_MAGENTA << "Data Pub Matched" << C_DEF);
    }

    lock.unlock();
    latency_subscriber_->discovery_cv_.notify_one();
}

void LatencyTestSubscriber::LatencyDataReaderListener::on_subscription_matched(
        DataReader* reader,
        const SubscriptionMatchedStatus& info)
{
    (void)reader;

    std::unique_lock<std::mutex> lock(latency_subscriber_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        logInfo(LatencyTest, C_MAGENTA << "Data Sub Matched" << C_DEF);
    }

    lock.unlock();
    latency_subscriber_->discovery_cv_.notify_one();
}

void LatencyTestSubscriber::ComandWriterListener::on_publication_matched(
        DataWriter* writer,
        const PublicationMatchedStatus& info)
{
    (void)writer;

    std::unique_lock<std::mutex> lock(latency_subscriber_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        logInfo(LatencyTest, C_MAGENTA << "Command Pub Matched" << C_DEF);
    }

    lock.unlock();
    latency_subscriber_->discovery_cv_.notify_one();
}

void LatencyTestSubscriber::CommandReaderListener::on_subscription_matched(
        DataReader* reader,
        const SubscriptionMatchedStatus& info)
{
    (void)reader;

    std::unique_lock<std::mutex> lock(latency_subscriber_->mutex_);

    matched_ = info.total_count;

    if (info.current_count_change > 0)
    {
        logInfo(LatencyTest, C_MAGENTA << "Command Sub Matched" << C_DEF);
    }

    lock.unlock();
    latency_subscriber_->discovery_cv_.notify_one();
}

void LatencyTestSubscriber::CommandReaderListener::on_data_available(
        DataReader* reader)
{
    TestCommandType command;
    SampleInfo info;
    std::ostringstream log;
    bool notify = false;

    if (reader->take_next_sample(
                &command, &info) == ReturnCode_t::RETCODE_OK
            && info.valid_data)
    {
        std::unique_lock<std::mutex> lock(latency_subscriber_->mutex_);

        log << "RCOMMAND: " << command.m_command;
        switch ( command.m_command )
        {
            case READY:
                log << "Publisher has new test ready...";
                break;
            case STOP:
                log << "Publisher has stopped the test";
                break;
            case STOP_ERROR:
                log << "Publisher has canceled the test";
                latency_subscriber_->test_status_ = -1;
                break;
            default:
                log << "Something is wrong";
                break;
        }

        if (command.m_command != DEFAULT)
        {
            ++latency_subscriber_->command_msg_count_;
            notify = true;
        }

        lock.unlock();
        if (notify)
        {
            latency_subscriber_->command_msg_cv_.notify_one();
        }
    }
    else
    {
        log << "Problem reading command message";
    }

    logInfo(LatencyTest, log.str());
}

void LatencyTestSubscriber::LatencyDataReaderListener::on_data_available(
        DataReader* reader)
{
    // Bounce back the message from the Publisher as fast as possible
    // dynamic_data_ and latency_data_type do not require locks
    // because the command message exchange assures this calls atomicity

    SampleInfo info;
    void* data = latency_subscriber_->dynamic_types_ ?
            (void*)latency_subscriber_->dynamic_data_ :
            (void*)latency_subscriber_->latency_data_;

    if (reader->take_next_sample(
                data, &info) == ReturnCode_t::RETCODE_OK
            && info.valid_data)
    {
        if (latency_subscriber_->echo_)
        {
            if (!latency_subscriber_->data_writer_->write(data))
            {
                logInfo(LatencyTest, "Problem echoing Publisher test data");
            }
        }
    }
    else
    {
        logInfo(LatencyTest, "Problem reading Publisher test data");
    }
}

void LatencyTestSubscriber::run()
{
    // WAIT FOR THE DISCOVERY PROCESS FO FINISH ONLY FOR DYNAMIC CASE:
    // EACH SUBSCRIBER NEEDS:
    // DYNAMIC TYPES: 4 Matchings (2 publishers and 2 subscribers)
    // STATIC TYPES: 2 Matchings (1 command publisher and 1 command subscriber)
    wait_for_discovery(
        [this]() -> bool
        {
            return total_matches() == (dynamic_types_ ? 4 : 2);
        });

    logInfo(LatencyTest, C_B_MAGENTA << "Sub: DISCOVERY COMPLETE " << C_DEF);

    for (std::vector<uint32_t>::iterator payload = data_size_sub_.begin(); payload != data_size_sub_.end(); ++payload)
    {
        if (!test(*payload))
        {
            break;
        }
    }
}

bool LatencyTestSubscriber::test(
        uint32_t datasize)
{
    logInfo(LatencyTest, "Preparing test with data size: " << datasize + 4);

    // Wait for the Publisher READY command
    // Assures that LatencyTestSubscriber|Publisher data endpoints creation and
    // destruction is sequential
    wait_for_command(
        [this]()
        {
            return command_msg_count_ != 0;
        });

    if (dynamic_types_)
    {
        // Create the data sample
        MemberId id;
        dynamic_data_ = static_cast<DynamicData*>(dynamic_pub_sub_type_->createData());

        if (nullptr == dynamic_data_)
        {
            logError(LatencyTest, "Iteration failed: Failed to create Dynamic Data");
            return false;
        }

        // Modify the data Sample
        DynamicData* member_data = dynamic_data_->loan_value(
            dynamic_data_->get_member_id_at_index(1));

        for (uint32_t i = 0; i < datasize; ++i)
        {
            member_data->insert_sequence_data(id);
            member_data->set_byte_value(0, id);
        }
        dynamic_data_->return_loaned_value(member_data);
    }
    // Create the static type for the given buffer size and the endpoints
    else if (init_static_types(datasize) && create_data_endpoints())
    {
        // Create the data sample
        latency_data_ = static_cast<LatencyType*>(latency_data_type_.create_data());

        // Wait for new endponts discovery from the LatencyTestPublisher
        wait_for_discovery(
            [this]() -> bool
            {
                return total_matches() == 4;
            });
    }
    else
    {
        logError(LatencyTest, "Error preparing static types and endpoints for testing");
        return false;
    }

    // Send to Publisher the BEGIN command
    test_status_ = 0;
    received_ = 0;
    TestCommandType command;
    command.m_command = BEGIN;
    if (!command_writer_->write(&command))
    {
        logError(LatencyTest, "Subscriber fail to publish the BEGIN command")
        return false;
    }

    logInfo(LatencyTest, "Testing with data size: " << datasize + 4);

    // Wait for the STOP or STOP_ERROR commands
    wait_for_command(
        [this]()
        {
            return command_msg_count_ != 0;
        });

    logInfo(LatencyTest, "TEST OF SIZE: " << datasize + 4 << " ENDS");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (dynamic_types_)
    {
        dynamic_pub_sub_type_->deleteData(dynamic_data_);
        // DynamicDataFactory::get_instance()->delete_data(dynamic_data_);
        //
        // Reset history for the new test
        size_t removed;
        data_writer_->clear_history(&removed);
    }
    else
    {
        latency_data_type_->deleteData(latency_data_);

        // Remove endpoints associated to the given payload size
        if (!destroy_data_endpoints())
        {
            logError(LatencyTest, "Static endpoints for payload size " << datasize << " could not been removed");
        }
    }

    if (test_status_ == -1)
    {
        return false;
    }

    command.m_command = END;
    if (!command_writer_->write(&command))
    {
        logError(LatencyTest, "Subscriber fail to publish the END command")
        return false;
    }

    // prevent the LatencyTestSubscriber from been destroyed while LatencyTestPublisher is waitin for the END command.
    if ( ReturnCode_t::RETCODE_OK != command_writer_->wait_for_acknowledgments(eprosima::fastrtps::c_TimeInfinite))
    {
        logError(LatencyTest, "Subscriber fail to acknowledge the END command")
        return false;
    }

    return true;
}

int32_t LatencyTestSubscriber::total_matches() const
{
    // no need to lock because is used always within a
    // condition variable wait predicate

    int32_t count = data_writer_listener_.get_matches()
            + data_reader_listener_.get_matches()
            + command_writer_listener_.get_matches()
            + command_reader_listener_.get_matches();

    // Each endpoint has a mirror counterpart in the LatencyTestPublisher
    // thus, the maximun number of matches is 4
    assert(count >= 0 && count < 5);
    return count;
}

bool LatencyTestSubscriber::init_dynamic_types()
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (dynamic_pub_sub_type_)
    {
        logError(LATENCYSUBSCRIBER, "ERROR DYNAMIC DATA type already initialized");
        return false;
    }
    else if (participant_->find_type(LatencyDataType::type_name_))
    {
        logError(LATENCYSUBSCRIBER, "ERROR DYNAMIC DATA type already registered");
        return false;
    }

    // Dummy type registration
    // Create basic builders
    DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

    // Add members to the struct.
    struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
    struct_type_builder->add_member(1, "data", DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                DynamicTypeBuilderFactory::get_instance()->create_byte_type(), BOUND_UNLIMITED));
    struct_type_builder->set_name(LatencyDataType::type_name_);
    dynamic_pub_sub_type_.reset(new DynamicPubSubType(struct_type_builder->build()));

    // Register the data type
    if (ReturnCode_t::RETCODE_OK != dynamic_pub_sub_type_.register_type(participant_))
    {
        logError(LATENCYSUBSCRIBER, "ERROR registering the DYNAMIC DATA type");
        return false;
    }

    return true;
}

bool LatencyTestSubscriber::init_static_types(
        uint32_t payload)
{
    assert(participant_ != nullptr);

    // Check if it has been initialized before
    if (latency_data_type_)
    {
        logError(LATENCYSUBSCRIBER, "ERROR STATIC DATA type already initialized");
        return false;
    }
    else if (participant_->find_type(LatencyDataType::type_name_))
    {
        logError(LATENCYSUBSCRIBER, "ERROR STATIC DATA type already registered");
        return false;
    }

    // Create the static type
    latency_data_type_.reset(new LatencyDataType(payload));
    // Register the static type
    if (ReturnCode_t::RETCODE_OK != latency_data_type_.register_type(participant_))
    {
        logError(LATENCYSUBSCRIBER, "ERROR registering the STATIC DATA type");
        return false;
    }

    return true;
}

bool LatencyTestSubscriber::create_data_endpoints()
{
    if (nullptr != latency_data_sub_topic_
            || nullptr != latency_data_pub_topic_)
    {
        logError(LatencyTest, "ERROR topics already initialized");
        return false;
    }

    if (nullptr != data_writer_)
    {
        logError(LatencyTest, "ERROR data_writer_ already initialized");
        return false;
    }

    if (nullptr != data_reader_)
    {
        logError(LatencyTest, "ERROR data_reader_ already initialized");
        return false;
    }

    // Create the topic
    std::ostringstream topic_name;
    topic_name << "LatencyTest_";
    if (hostname_)
    {
        topic_name << asio::ip::host_name() << "_";
    }
    topic_name << pid_ << "_PUB2SUB";

    latency_data_sub_topic_ = participant_->create_topic(
        topic_name.str(),
        LatencyDataType::type_name_,
        TOPIC_QOS_DEFAULT);

    if (nullptr == latency_data_sub_topic_)
    {
        logError(LatencyTest, "ERROR creating the DATA TYPE for the subscriber data reader topic");
        return false;
    }

    /* Create Topics */
    topic_name.str("");
    topic_name.clear();
    topic_name << "LatencyTest_";

    if (hostname_)
    {
        topic_name << asio::ip::host_name() << "_";
    }
    topic_name << pid_ << "_SUB2PUB";

    latency_data_pub_topic_ = participant_->create_topic(
        topic_name.str(),
        LatencyDataType::type_name_,
        TOPIC_QOS_DEFAULT);

    if (latency_data_pub_topic_ == nullptr)
    {
        logError(LatencyTest, "ERROR creating the DATA TYPE for the subscriber data writer topic");
        return false;
    }

    // Create the endpoints
    if (nullptr ==
            (data_writer_ = publisher_->create_datawriter(
                latency_data_pub_topic_,
                dw_qos_,
                &data_writer_listener_)))
    {
        logError(LatencyTest, "ERROR creating the subscriber data writer");
        return false;
    }

    if (nullptr ==
            (data_reader_ = subscriber_->create_datareader(
                latency_data_sub_topic_,
                dr_qos_,
                &data_reader_listener_)))
    {
        logError(LatencyTest, "ERROR creating the subscriber data reader");
        return false;
    }

    return true;
}

bool LatencyTestSubscriber::destroy_data_endpoints()
{
    assert(nullptr != participant_);
    assert(nullptr != publisher_);
    assert(nullptr != subscriber_);

    // Delete the endpoints
    if (nullptr == data_writer_
            || ReturnCode_t::RETCODE_OK != publisher_->delete_datawriter(data_writer_))
    {
        logError(LatencyTest, "ERROR destroying the DataWriter");
        return false;
    }
    data_writer_ = nullptr;
    data_writer_listener_.reset();

    if (nullptr == data_reader_
            || ReturnCode_t::RETCODE_OK != subscriber_->delete_datareader(data_reader_))
    {
        logError(LatencyTest, "ERROR destroying the DataReader");
        return false;
    }
    data_reader_ = nullptr;
    data_reader_listener_.reset();

    // Delete the Topics
    if (nullptr == latency_data_pub_topic_
            || ReturnCode_t::RETCODE_OK != participant_->delete_topic(latency_data_pub_topic_))
    {
        logError(LatencyTest, "ERROR destroying the DATA pub topic");
        return false;
    }
    latency_data_pub_topic_ = nullptr;
    if (nullptr == latency_data_sub_topic_
            || ReturnCode_t::RETCODE_OK != participant_->delete_topic(latency_data_sub_topic_))
    {
        logError(LatencyTest, "ERROR destroying the DATA sub topic");
        return false;
    }
    latency_data_sub_topic_ = nullptr;

    // Delete the Type
    if (ReturnCode_t::RETCODE_OK
            != participant_->unregister_type(LatencyDataType::type_name_))
    {
        logError(LatencyTest, "ERROR unregistering the DATA type");
        return false;
    }

    latency_data_type_.reset();
    dynamic_pub_sub_type_.reset();
    DynamicTypeBuilderFactory::delete_instance();

    return true;
}
