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
#include <chrono>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

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
    std::string TestCommandType("TestCommandType");
    participant_->unregister_type(TestCommandType);

    DomainParticipantFactory::get_instance()->delete_participant(participant_);

    EPROSIMA_LOG_INFO(LatencyTest, "Sub: Participant removed");
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
        Arg::EnablerValue data_sharing,
        bool data_loans,
        Arg::EnablerValue shared_memory,
        int forced_domain,
        LatencyDataSizes& latency_data_sizes)
{
    // Initialize state
    xml_config_file_ = xml_config_file;
    echo_ = echo;
    samples_ = samples;
    dynamic_types_ = dynamic_data;
    data_sharing_ = data_sharing;
    data_loans_ = data_loans;
    shared_memory_ = shared_memory;
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
    participant_ = DomainParticipantFactory::get_instance()->create_participant(domainId, pqos);
    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the command type
    if (RETCODE_OK != latency_command_type_.register_type(participant_))
    {
        EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR registering the COMMAND type");
        return false;
    }

    /* Create Publisher */
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR creating PUBLISHER");
        return false;
    }

    /* Create Subscriber */
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR creating SUBSCRIBER");
        return false;
    }

    {
        /* Update DataWriterQoS with xml profile data */
        if (xml_config_file_.length() > 0 )
        {
            std::string sub_profile_name = "sub_subscriber_profile";
            std::string pub_profile_name = "sub_publisher_profile";

            if ( RETCODE_OK != publisher_->get_datawriter_qos_from_profile(pub_profile_name, dw_qos_))
            {
                EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER,
                        "ERROR unable to retrieve the " << pub_profile_name << "from XML file");
                return false;
            }

            if ( RETCODE_OK != subscriber_->get_datareader_qos_from_profile(sub_profile_name, dr_qos_))
            {
                EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR unable to retrieve the " << sub_profile_name);
                return false;
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

            dr_qos_.reliability(rp);
            dw_qos_.reliability(rp);

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
        EPROSIMA_LOG_INFO(LatencyTest, C_MAGENTA << "Data Pub Matched" << C_DEF);
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
        EPROSIMA_LOG_INFO(LatencyTest, C_MAGENTA << "Data Sub Matched" << C_DEF);
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
        EPROSIMA_LOG_INFO(LatencyTest, C_MAGENTA << "Command Pub Matched" << C_DEF);
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
        EPROSIMA_LOG_INFO(LatencyTest, C_MAGENTA << "Command Sub Matched" << C_DEF);
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
                &command, &info) == RETCODE_OK
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

    EPROSIMA_LOG_INFO(LatencyTest, log.str());
}

void LatencyTestSubscriber::LatencyDataReaderListener::on_data_available(
        DataReader* reader)
{
    auto sub = latency_subscriber_;

    // Bounce back the message from the Publisher as fast as possible
    // dynamic_data_ and latency_data_type do not require locks
    // because the command message exchange assures this calls atomicity
    if (sub->data_loans_)
    {
        SampleInfoSeq infos;
        LoanableSequence<LatencyType> data_seq;
        // reader loan buffer
        LatencyType* echoed_data = nullptr;
        // writer loan buffer
        void* echoed_loan = nullptr;

        if (RETCODE_OK != reader->take(data_seq, infos, 1))
        {
            EPROSIMA_LOG_INFO(LatencyTest, "Problem reading Subscriber echoed loaned test data");
            return;
        }

        // we have requested a single sample
        assert(infos.length() == 1 && data_seq.length() == 1);
        // the buffer must be there
        assert(sub->latency_data_ != nullptr);
        // reference the loan
        echoed_data = &data_seq[0];

        // echo the sample
        if (sub->echo_)
        {
            // begin measuring overhead = loan->buffer copy + write loan + buffer->loan copy
            auto start_time = std::chrono::steady_clock::now();

            // Copy the data from reader loan to aux buffer
            auto data_type = std::static_pointer_cast<LatencyDataType>(sub->latency_data_type_);
            data_type->copy_data(*echoed_data, *sub->latency_data_);

            // release the reader loan
            if (RETCODE_OK != reader->return_loan(data_seq, infos))
            {
                EPROSIMA_LOG_INFO(LatencyTest, "Problem returning loaned test data");
                return;
            }

            // writer loan
            int trials = 10;
            bool loaned = false;
            while (trials-- != 0 && !loaned)
            {
                loaned = (RETCODE_OK
                        == sub->data_writer_->loan_sample(
                            echoed_loan,
                            DataWriter::LoanInitializationKind::NO_LOAN_INITIALIZATION));

                std::this_thread::yield();

                if (!loaned)
                {
                    EPROSIMA_LOG_ERROR(LatencyTest, "Subscriber trying to loan: " << trials);
                }
            }

            if (!loaned)
            {
                EPROSIMA_LOG_INFO(LatencyTest, "Problem echoing Publisher test data with loan");
                // release the reader loan
                reader->return_loan(data_seq, infos);
                return;
            }

            // copy the data from aux buffer to writer loan
            data_type->copy_data(*sub->latency_data_, *(LatencyType*)echoed_loan);

            //end measuring overhead
            auto end_time = std::chrono::steady_clock::now();
            std::chrono::duration<uint32_t, std::nano> bounce_time(end_time - start_time);
            reinterpret_cast<LatencyType*>(echoed_loan)->bounce = bounce_time.count();

            if (RETCODE_OK != sub->data_writer_->write(echoed_loan))
            {
                EPROSIMA_LOG_ERROR(LatencyTest, "Problem echoing Publisher test data with loan");
                sub->data_writer_->discard_loan(echoed_loan);
            }
        }
        else
        {
            // release the loan
            if (RETCODE_OK != reader->return_loan(data_seq, infos))
            {
                EPROSIMA_LOG_ERROR(LatencyTest, "Problem returning loaned test data");
            }
        }
    }
    else
    {
        SampleInfo info;
        void* data = sub->dynamic_types_ ?
                (void*)sub->dynamic_data_ :
                (void*)sub->latency_data_;

        if (reader->take_next_sample(
                    data, &info) == RETCODE_OK
                && info.valid_data)
        {
            if (sub->echo_)
            {
                // no bounce overload recorded
                if (!sub->dynamic_types_)
                {
                    reinterpret_cast<LatencyType*>(data)->bounce = 0;
                }

                if (RETCODE_OK != sub->data_writer_->write(data))
                {
                    EPROSIMA_LOG_INFO(LatencyTest, "Problem echoing Publisher test data");
                }
            }
        }
        else
        {
            EPROSIMA_LOG_INFO(LatencyTest, "Problem reading Publisher test data");
        }
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

    EPROSIMA_LOG_INFO(LatencyTest, C_B_MAGENTA << "Sub: DISCOVERY COMPLETE " << C_DEF);

    for (std::vector<uint32_t>::iterator payload = data_size_sub_.begin(); payload != data_size_sub_.end(); ++payload)
    {
        if (!test(*payload))
        {
            break;
        }
    }
}

void LatencyTestSubscriber::destroy_user_entities()
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
        EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR unregistering the DATA type and/or removing the endpoints");
    }

    subscriber_->delete_datareader(command_reader_);
    participant_->delete_subscriber(subscriber_);

    publisher_->delete_datawriter(command_writer_);
    participant_->delete_publisher(publisher_);

    participant_->delete_topic(latency_command_sub_topic_);
    participant_->delete_topic(latency_command_pub_topic_);
}

bool LatencyTestSubscriber::test(
        uint32_t datasize)
{
    EPROSIMA_LOG_INFO(LatencyTest, "Preparing test with data size: " << datasize );

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
        dynamic_data_ = static_cast<DynamicData::_ref_type*>(dynamic_pub_sub_type_->create_data());

        if (nullptr == dynamic_data_)
        {
            EPROSIMA_LOG_ERROR(LatencyTest,
                    "Iteration failed: Failed to create Dynamic Data");
            return false;
        }

        // Modify the data Sample
        DynamicData::_ref_type member_data = (*dynamic_data_)->loan_value(
            (*dynamic_data_)->get_member_id_at_index(1));

        // fill until complete the desired payload size
        uint32_t padding = datasize - 4; // sequence number is a DWORD

        for (uint32_t i = 0; i < padding; ++i)
        {
            member_data->set_byte_value(i, 0);
        }
        (*dynamic_data_)->return_loaned_value(member_data);
    }
    // Create the static type for the given buffer size and the endpoints
    else if (init_static_types(datasize) && create_data_endpoints())
    {
        // Create the data sample as buffer
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
        EPROSIMA_LOG_ERROR(LatencyTest, "Error preparing static types and endpoints for testing");
        return false;
    }

    // Send to Publisher the BEGIN command
    test_status_ = 0;
    received_ = 0;
    TestCommandType command;
    command.m_command = BEGIN;
    if (RETCODE_OK != command_writer_->write(&command))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "Subscriber fail to publish the BEGIN command");
        return false;
    }

    EPROSIMA_LOG_INFO(LatencyTest, "Testing with data size: " << datasize);

    // Wait for the STOP or STOP_ERROR commands
    wait_for_command(
        [this]()
        {
            return command_msg_count_ != 0;
        });

    EPROSIMA_LOG_INFO(LatencyTest, "TEST OF SIZE: " << datasize << " ENDS");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (dynamic_types_)
    {
        dynamic_pub_sub_type_->delete_data(dynamic_data_);
        //
        // Reset history for the new test
        size_t removed;
        data_writer_->clear_history(&removed);
    }
    else
    {
        // release the buffer next iteration will require different size
        latency_data_type_->delete_data(latency_data_);

        // Remove endpoints associated to the given payload size
        if (!destroy_data_endpoints())
        {
            EPROSIMA_LOG_ERROR(LatencyTest,
                    "Static endpoints for payload size " << datasize << " could not been removed");
        }
    }

    if (test_status_ == -1)
    {
        return false;
    }

    command.m_command = END;
    if (RETCODE_OK != command_writer_->write(&command))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "Subscriber fail to publish the END command");
        return false;
    }

    // prevent the LatencyTestSubscriber from been destroyed while LatencyTestPublisher is waitin for the END command.
    if ( RETCODE_OK != command_writer_->wait_for_acknowledgments(eprosima::fastdds::dds::c_TimeInfinite))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "Subscriber fail to acknowledge the END command");
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
        EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR DYNAMIC DATA type already initialized");
        return false;
    }
    else if (participant_->find_type(LatencyDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR DYNAMIC DATA type already registered");
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
        EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR registering the DYNAMIC DATA type");
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
        EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR STATIC DATA type already initialized");
        return false;
    }
    else if (participant_->find_type(LatencyDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR STATIC DATA type already registered");
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
        EPROSIMA_LOG_ERROR(LATENCYSUBSCRIBER, "ERROR registering the STATIC DATA type");
        return false;
    }

    return true;
}

bool LatencyTestSubscriber::create_data_endpoints()
{
    if (nullptr != latency_data_sub_topic_
            || nullptr != latency_data_pub_topic_)
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR topics already initialized");
        return false;
    }

    if (nullptr != data_writer_)
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR data_writer_ already initialized");
        return false;
    }

    if (nullptr != data_reader_)
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR data_reader_ already initialized");
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
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR creating the DATA TYPE for the subscriber data reader topic");
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
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR creating the DATA TYPE for the subscriber data writer topic");
        return false;
    }

    // Create the endpoints
    if (nullptr ==
            (data_writer_ = publisher_->create_datawriter(
                latency_data_pub_topic_,
                dw_qos_,
                &data_writer_listener_)))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR creating the subscriber data writer");
        return false;
    }

    if (nullptr ==
            (data_reader_ = subscriber_->create_datareader(
                latency_data_sub_topic_,
                dr_qos_,
                &data_reader_listener_)))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR creating the subscriber data reader");
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
            || RETCODE_OK != publisher_->delete_datawriter(data_writer_))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR destroying the DataWriter");
        return false;
    }
    data_writer_ = nullptr;
    data_writer_listener_.reset();

    if (nullptr == data_reader_
            || RETCODE_OK != subscriber_->delete_datareader(data_reader_))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR destroying the DataReader");
        return false;
    }
    data_reader_ = nullptr;
    data_reader_listener_.reset();

    // Delete the Topics
    if (nullptr == latency_data_pub_topic_
            || RETCODE_OK != participant_->delete_topic(latency_data_pub_topic_))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR destroying the DATA pub topic");
        return false;
    }
    latency_data_pub_topic_ = nullptr;
    if (nullptr == latency_data_sub_topic_
            || RETCODE_OK != participant_->delete_topic(latency_data_sub_topic_))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR destroying the DATA sub topic");
        return false;
    }
    latency_data_sub_topic_ = nullptr;

    // Delete the Type
    if (RETCODE_OK
            != participant_->unregister_type(LatencyDataType::type_name_))
    {
        EPROSIMA_LOG_ERROR(LatencyTest, "ERROR unregistering the DATA type");
        return false;
    }

    latency_data_type_.reset();
    dynamic_pub_sub_type_.reset();
    DynamicTypeBuilderFactory::delete_instance();

    return true;
}
