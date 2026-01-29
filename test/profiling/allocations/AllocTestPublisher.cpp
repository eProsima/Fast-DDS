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
 * @file AllocTestPublisher.cpp
 *
 */

#include "AllocTestPublisher.hpp"

#include <thread>
#include <chrono>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include "AllocTestCommon.h"
#include "AllocTestTypePubSubTypes.hpp"

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

AllocTestPublisher::AllocTestPublisher()
    : type_(new AllocTestTypePubSubType())
    , participant_(nullptr)
    , topic_(nullptr)
    , publisher_(nullptr)
    , writer_(nullptr)
    , profile_("")
    , output_file_("")
    , matched_(0)
{
}

AllocTestPublisher::~AllocTestPublisher()
{
    if (participant_ != nullptr)
    {
        participant_->delete_contained_entities();
        DomainParticipantFactory::get_shared_instance()->delete_participant(participant_);
        participant_ = nullptr;
    }
}

bool AllocTestPublisher::init(
        const char* profile,
        uint32_t domain_id,
        const std::string& output_file)
{
    data_.index(0);

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

    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
    CHECK_ENTITY_CREATION(publisher_);

    std::string prof = "test_publisher_profile_" + profile_;
    writer_ = publisher_->create_datawriter_with_profile(topic_, prof, this);
    CHECK_ENTITY_CREATION(writer_);

    bool show_allocation_traces = std::getenv("FASTDDS_PROFILING_PRINT_TRACES") != nullptr;
    eprosima_profiling::entities_created(show_allocation_traces);
    return ret == RETCODE_OK;
}

void AllocTestPublisher::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& status)
{
    if (status.current_count_change == 1)
    {
        matched_++;
        std::cout << "Publisher matched" << std::endl;
    }
    else if (status.current_count_change == -1)
    {
        matched_--;
        std::cout << "Publisher unmatched" << std::endl;
    }
    else
    {
        std::cout << status.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
    cv_.notify_all();
}

bool AllocTestPublisher::is_matched()
{
    return matched_ > 0;
}

void AllocTestPublisher::wait_match()
{
    std::unique_lock<std::mutex> lck(mtx_);
    cv_.wait(lck, [this]()
            {
                return matched_ > 0;
            });
}

void AllocTestPublisher::wait_unmatch()
{
    std::unique_lock<std::mutex> lck(mtx_);
    cv_.wait(lck, [this]()
            {
                return matched_ <= 0;
            });
}

void AllocTestPublisher::run(
        uint32_t samples,
        bool wait_unmatching)
{
    // Restart callgrind graph
    eprosima_profiling::callgrind_zero_count();

    std::cout << "Publisher waiting for subscriber..." << std::endl;
    wait_match();

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::discovery_finished();

    std::cout << "Publisher matched. Sending samples" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (uint32_t i = 0; i < samples; ++i)
    {
        if (!publish())
        {
            --i;
        }
        else
        {
            std::cout << "Message with index: " << data_.index() << " SENT" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        if (i == 0)
        {
            // Flush callgrind graph
            eprosima_profiling::callgrind_dump();
            eprosima_profiling::first_sample_exchanged();

            std::cout << "First message has been sent" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::all_samples_exchanged();

    if (wait_unmatching)
    {
        std::cout << "All messages have been sent. Waiting for subscriber to stop." << std::endl;
        wait_unmatch();
    }
    else
    {
        std::cout << "All messages have been sent. Waiting a bit to let subscriber receive samples." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Flush callgrind graph
    eprosima_profiling::callgrind_dump();
    eprosima_profiling::undiscovery_finished();
    eprosima_profiling::print_results(output_file_, "publisher", profile_);
}

bool AllocTestPublisher::publish()
{
    bool ret = false;
    if (is_matched())
    {
        data_.index(data_.index() + 1);
        ret = (RETCODE_OK == writer_->write(&data_));
    }
    return ret;
}
