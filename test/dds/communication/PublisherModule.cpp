// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PublisherModule.cpp
 */
#include <asio.hpp>

#include "PublisherModule.hpp"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>

#include <fstream>
#include <string>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

PublisherModule::~PublisherModule()
{
    if (nullptr != writer_)
    {
        publisher_->delete_datawriter(writer_);
    }

    if (nullptr != publisher_)
    {
        participant_->delete_publisher(publisher_);
    }

    if (nullptr != topic_)
    {
        participant_->delete_topic(topic_);
    }

    if (nullptr != participant_)
    {
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

bool PublisherModule::init(
        uint32_t seed,
        const std::string& magic)
{
    std::cout << "Initializing Publisher" << std::endl;

    participant_ =
            DomainParticipantFactory::get_instance()->create_participant(seed % 230, PARTICIPANT_QOS_DEFAULT, this);

    if (participant_ == nullptr)
    {
        std::cout << "Error creating publisher participant" << std::endl;
        return false;
    }

    // Construct a FixedSizedType if fixed type is required, defult HelloWro
    if (fixed_type_)
    {
        type_.reset(new FixedSizedPubSubType());
    }
    else
    {
        type_.reset(new HelloWorldPubSubType());
    }

    type_.register_type(participant_);

    // Generate topic name
    std::ostringstream topic_name;
    topic_name << "DDSCommunicationTestsTopic_" << ((magic.empty()) ? asio::ip::host_name() : magic) << "_" << seed;

    //CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, this);
    if (publisher_ == nullptr)
    {
        std::cout << "Error creating publisher" << std::endl;
        return false;
    }

    topic_ = participant_->create_topic(topic_name.str(), type_.get_type_name(), TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        std::cout << "Error creating publisher topic" << std::endl;
        return false;
    }

    DataWriterQos wqos = publisher_->get_default_datawriter_qos();
    wqos.liveliness().lease_duration = 3;
    wqos.liveliness().announcement_period = 1;
    wqos.liveliness().kind = AUTOMATIC_LIVELINESS_QOS;

    writer_ = publisher_->create_datawriter(topic_, wqos, this);
    if (writer_ == nullptr)
    {
        std::cout << "Error creating publisher datawriter" << std::endl;
        return false;
    }
    std::cout << "Writer created correctly in topic " << topic_->get_name()
              << " with type " << type_.get_type_name() << std::endl;

    std::cout << "Publisher initialized correctly" << std::endl;

    return true;
}

void PublisherModule::wait_discovery(
        uint32_t how_many)
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [&]
            {
                return matched_ >= how_many;
            });
}

void PublisherModule::run(
        uint32_t samples,
        const uint32_t loops)
{
    uint32_t current_loop = 0;
    uint16_t index = 1;
    void* sample = nullptr;

    while (run_ && (loops == 0 || loops > current_loop))
    {
        if (zero_copy_)
        {
            if (ReturnCode_t::RETCODE_OK == writer_->loan_sample(sample))
            {
                FixedSized* data = static_cast<FixedSized*>(sample);
                data->index(index);
            }
        }
        else
        {
            sample = type_.create_data();
            if (fixed_type_)
            {
                FixedSized* data = static_cast<FixedSized*>(sample);
                data->index(index);
                // FixedSized has no message
            }
            else
            {
                HelloWorld* data = static_cast<HelloWorld*>(sample);
                data->index(index);
                data->message("HelloWorld");
            }
        }
        std::cout << "Publisher writting index " << index << std::endl;
        writer_->write(sample);

        if (index == samples)
        {
            index = 1;
            ++current_loop;
        }
        else
        {
            ++index;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void PublisherModule::on_publication_matched(
        DataWriter* /*publisher*/,
        const PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (info.current_count_change == 1)
    {
        std::cout << "Publisher matched with subscriber " << info.last_subscription_handle
                  << ": " << ++matched_ << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Publisher unmatched with subscriber " << info.last_subscription_handle
                  << ": " << --matched_ << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
    cv_.notify_all();
}

void PublisherModule::on_participant_discovery(
        DomainParticipant* /*participant*/,
        ParticipantDiscoveryInfo&& info)
{
    if (info.status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Publisher participant " << //participant->getGuid() <<
            " discovered participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
    {
        std::cout << "Publisher participant " << //participant->getGuid() <<
            " detected changes on participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
    {
        std::cout << "Publisher participant " << // participant->getGuid() <<
            " removed participant " << info.info.m_guid << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
    {
        std::cout << "Publisher participant " << //participant->getGuid() <<
            " dropped participant " << info.info.m_guid << std::endl;
        if (exit_on_lost_liveliness_)
        {
            run_ = false;
        }
    }
}

#if HAVE_SECURITY
void PublisherModule::onParticipantAuthentication(
        DomainParticipant* participant,
        ParticipantAuthenticationInfo&& info)
{
    if (ParticipantAuthenticationInfo::AUTHORIZED_PARTICIPANT == info.status)
    {
        std::cout << "Publisher participant " << participant->guid() <<
            " authorized participant " << info.guid << std::endl;
    }
    else
    {
        std::cout << "Publisher participant " << participant->guid() <<
            " unauthorized participant " << info.guid << std::endl;
    }
}

#endif // if HAVE_SECURITY
