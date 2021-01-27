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
#include <cassert>

#include "LatencyTestSubscriber.hpp"
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;
using namespace eprosima::fastdds::dds;
using namespace std;

LatencyTestSubscriber::LatencyTestSubscriber()
    : participant_(nullptr)
    , publisher_(nullptr)
    , data_writer_(nullptr)
    , command_writer_(nullptr)
    , subscriber_(nullptr)
    , data_reader_(nullptr)
    , command_reader_(nullptr)
    , received_(0)
    , command_msg_count_(0)
    , test_status_(0)
    , echo_(true)
    , samples_(0)
    , latency_data_sub_topic_(nullptr)
    , latency_data_pub_topic_(nullptr)
    , latency_command_sub_topic_(nullptr)
    , latency_command_pub_topic_(nullptr)
    , latency_type_(nullptr)
    , latency_command_type_(new TestCommandDataType())
    , dynamic_data_type_(nullptr)
    , data_writer_listener_(this)
    , data_reader_listener_(this)
    , command_writer_listener_(this)
    , command_reader_listener_(this)
{
    forced_domain_ = -1;
}

LatencyTestSubscriber::~LatencyTestSubscriber()
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

bool LatencyTestSubscriber::init(
        bool echo,
        int samples,
        bool reliable,
        uint32_t pid,
        bool hostname,
        const PropertyPolicy& part_property_policy,
        const PropertyPolicy& property_policy,
        const string& xml_config_file,
        bool dynamic_data,
        int forced_domain,
        LatencyDataSizes& latency_data_sizes)
{
    data_size_sub_ = latency_data_sizes.sample_sizes();

    xml_config_file_ = xml_config_file;
    echo_ = echo;
    samples_ = samples;
    dynamic_data_ = dynamic_data;
    forced_domain_ = forced_domain;

    // Init dynamic data
    if (dynamic_data_)
    {
        // Create basic builders
        DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

        // Add members to the struct.
        struct_type_builder->add_member(0, "seqnum", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
        struct_type_builder->add_member(1, "data",
                DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(
                    DynamicTypeBuilderFactory::get_instance()->create_byte_type(), data_size_sub_.back()
                    ));
        struct_type_builder->set_name("LatencyType");
        dynamic_pub_sub_type_.reset(new DynamicPubSubType(struct_type_builder->build()));
    }
    else
    {
        // Create basic builders
        latency_data_type_.reset(new LatencyDataType());
    }

    /* Create DomainParticipant*/
    string participant_profile_name = "sub_participant_profile";
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

    // Register the data type
    if (dynamic_data_)
    {
        dynamic_pub_sub_type_.register_type(participant_);
    }
    else
    {
        latency_data_type_.register_type(participant_);
    }

    // Register the command type
    latency_command_type_.register_type(participant_);

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
        topic_name << "LatencyTest_";

        if (hostname)
        {
            topic_name << asio::ip::host_name() << "_";
        }
        topic_name << pid << "_SUB2PUB";

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

    /* Create Echo DataWriter */
    {
        string profile_name = "sub_publisher_profile";

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

    /* Create Data Reader */
    {
        string profile_name = "sub_subscriber_profile";

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

    return true;
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
    ostringstream log;
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
    // dynamic_data_type_ and latency_data_type do not require locks
    // because the command message exchange assures this calls atomicity

    SampleInfo info;
    void* data = latency_subscriber_->dynamic_data_ ?
            (void*)latency_subscriber_->dynamic_data_type_ :
            (void*)latency_subscriber_->latency_type_;

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
    // WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    // EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    unique_lock<mutex> disc_lock(mutex_);
    discovery_cv_.wait(
        disc_lock,
        [this]() -> bool
        {
            return total_matches() == 4;
        });
    disc_lock.unlock();

    logInfo(LatencyTest, C_B_MAGENTA << "Sub: DISCOVERY COMPLETE " << C_DEF);

    for (vector<uint32_t>::iterator payload = data_size_sub_.begin(); payload != data_size_sub_.end(); ++payload)
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
    if (dynamic_data_)
    {
        dynamic_data_type_ = static_cast<DynamicData*>(dynamic_pub_sub_type_->createData());

        MemberId id;
        DynamicData* dyn_data = dynamic_data_type_->loan_value(dynamic_data_type_->get_member_id_at_index(1));
        for (uint32_t i = 0; i < datasize; ++i)
        {
            dyn_data->insert_sequence_data(id);
            dyn_data->set_byte_value(0, id);
        }
        dynamic_data_type_->return_loaned_value(dyn_data);
    }
    else
    {
        latency_type_ = static_cast<LatencyType*>(latency_data_type_->createData());
        latency_type_->data.resize(datasize, 0);
    }

    // Wait for the Publisher READY command
    unique_lock<mutex> lock(mutex_);
    command_msg_cv_.wait(
        lock,
        [this]()
        {
            return command_msg_count_ != 0;
        });
    --command_msg_count_;
    lock.unlock();

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
    lock.lock();
    command_msg_cv_.wait(
        lock,
        [this]()
        {
            return command_msg_count_ != 0;
        });
    --command_msg_count_;
    lock.unlock();

    logInfo(LatencyTest, "TEST OF SIZE: " << datasize + 4 << " ENDS");
    this_thread::sleep_for(chrono::milliseconds(50));

    size_t removed;
    data_writer_->clear_history(&removed);

    if (dynamic_data_)
    {
        dynamic_pub_sub_type_->deleteData(dynamic_data_type_);
    }
    else
    {
        latency_data_type_->deleteData(latency_type_);
    }

    if (test_status_ == -1)
    {
        return false;
    }
    return true;

}

int32_t LatencyTestSubscriber::total_matches() const
{
    // no need to lock because is used always within a
    // condition variable wait predicate

    int32_t count = data_writer_listener_.matched_
            + data_reader_listener_.matched_
            + command_writer_listener_.matched_
            + command_reader_listener_.matched_;

    // Each endpoint has a mirror counterpart in the LatencyTestPublisher
    // thus, the maximun number of matches is 4
    assert(count >= 0 && count < 5);
    return count;
}
