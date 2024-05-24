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
 * @file AllocTestSubscriber.cpp
 *
 */

#include "AllocTestSubscriber.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include "AllocTestCommon.h"
#include "AllocTestTypePubSubTypes.h"

using namespace eprosima::fastdds::dds;

#define CHECK_RETURN_CODE(ret) \
    if (RETCODE_OK != ret) \
    { \
        return false; \
    }

#define CHECK_ENTITY_CREATION(entity) \
    if (nullptr == entity) \
    { \
        return false; \
    }

AllocTestSubscriber::AllocTestSubscriber()
    : type_(new AllocTestTypePubSubType())
    , participant_(nullptr)
    , topic_(nullptr)
    , subscriber_(nullptr)
    , reader_(nullptr)
    , profile_("")
    , output_file_("")
    , matched_(0)
    , samples_(0)
{
}

AllocTestSubscriber::~AllocTestSubscriber()
{
    if (participant_ != nullptr)
    {
        participant_->delete_contained_entities();
        DomainParticipantFactory::get_shared_instance()->delete_participant(participant_);
        participant_ = nullptr;
    }
}

bool AllocTestSubscriber::init(
        const char* profile,
        uint32_t domain_id,
        const std::string& output_file)
{
    profile_ = profile;
    output_file_ = output_file;

    ReturnCode_t ret = RETCODE_OK;

    std::shared_ptr<DomainParticipantFactory> factory = DomainParticipantFactory::get_shared_instance();
    ret = factory->load_XML_profiles_file("test_xml_profile.xml");
    CHECK_RETURN_CODE(ret);

    DomainParticipantQos pqos;
    ret = factory->get_participant_qos_from_profile("test_participant_profile", pqos);
    CHECK_RETURN_CODE(ret);

    participant_ = factory->create_participant(domain_id, pqos);
    CHECK_ENTITY_CREATION(participant_);

    ret = type_.register_type(participant_);
    CHECK_RETURN_CODE(ret);

    topic_ = participant_->create_topic("AllocTestTopic", type_.get_type_name(), TOPIC_QOS_DEFAULT);
    CHECK_ENTITY_CREATION(topic_);

    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    CHECK_ENTITY_CREATION(subscriber_);

    std::string prof = "test_subscriber_profile_" + profile_;
    reader_ = subscriber_->create_datareader_with_profile(topic_, prof, this);
    CHECK_ENTITY_CREATION(reader_);

    bool show_allocation_traces = std::getenv("FASTDDS_PROFILING_PRINT_TRACES") != nullptr;
    eprosima_profiling::entities_created(show_allocation_traces);
    return ret == RETCODE_OK;
}

void AllocTestSubscriber::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& status)
{
    if (status.current_count_change == 1)
    {
        matched_++;
        std::cout << "Subscriber matched" << std::endl;
    }
    else if (status.current_count_change == -1)
    {
        matched_--;
        std::cout << "Subscriber unmatched" << std::endl;
    }
    else
    {
        std::cout << status.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
    cv_.notify_all();
}

void AllocTestSubscriber::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    if (RETCODE_OK == reader->take_next_sample(&data_, &info))
    {
        if ((info.instance_state == ALIVE_INSTANCE_STATE) && (info.valid_data) &&
                (reader->is_sample_valid(&data_, &info)))
        {
            samples_++;
            std::cout << "Message " << data_.index() << " RECEIVED" << std::endl;
            cv_.notify_all();
        }
    }
}

void AllocTestSubscriber::wait_match()
{
    std::unique_lock<std::mutex> lck(mtx_);
    cv_.wait(lck, [this]()
            {
                return matched_ > 0;
            });
}

void AllocTestSubscriber::wait_unmatch()
{
    std::unique_lock<std::mutex> lck(mtx_);
    cv_.wait(lck, [this]()
            {
                return matched_ <= 0;
            });
}

void AllocTestSubscriber::wait_until_total_received_at_least(
        uint32_t n)
{
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this, n]()
            {
                return samples_ >= n;
            });
}

void AllocTestSubscriber::run(
        bool wait_unmatch)
{
    run(60, wait_unmatch);
}

void AllocTestSubscriber::run(
        uint32_t number,
        bool wait_unmatching)
{
    // Restart callgrind graph
    eprosima_profiling::callgrind_zero_count();

    std::cout << "Subscriber waiting for publisher..." << std::endl;
    wait_match();

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::discovery_finished();

    std::cout << "Subscriber matched. Waiting for first sample..." << std::endl;
    wait_until_total_received_at_least(1ul);

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::first_sample_exchanged();

    std::cout << "First sample received. Waiting for rest of samples..." << std::endl;
    wait_until_total_received_at_least(number);

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::all_samples_exchanged();

    if (wait_unmatching)
    {
        std::cout << "All messages received. Waiting for publisher to stop." << std::endl;
        wait_unmatch();
    }
    else
    {
        std::cout << "All messages received. Waiting a bit to let publisher receive acks." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::undiscovery_finished();
    eprosima_profiling::print_results(output_file_, "subscriber", profile_);
}
