// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeIntrospectionSubscriber.cpp
 *
 */

#include <csignal>
#include <functional>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>

#include "types/types.hpp"

#include "TypeIntrospectionSubscriber.h"

using namespace eprosima::fastdds::dds;

std::atomic<bool> TypeIntrospectionSubscriber::type_discovered_(false);
std::atomic<bool> TypeIntrospectionSubscriber::type_registered_(false);
std::mutex TypeIntrospectionSubscriber::type_discovered_cv_mtx_;
std::condition_variable TypeIntrospectionSubscriber::type_discovered_cv_;
std::atomic<bool> TypeIntrospectionSubscriber::stop_(false);
std::mutex TypeIntrospectionSubscriber::terminate_cv_mtx_;
std::condition_variable TypeIntrospectionSubscriber::terminate_cv_;

TypeIntrospectionSubscriber::TypeIntrospectionSubscriber(
        const std::string& topic_name,
        uint32_t domain)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , topic_name_(topic_name)
    , samples_(0)
{
    DomainParticipantQos pqos;
    pqos.name("TypeIntrospectionExample_Participant_Subscriber");

    // Create listener mask so the data do not go to on_data_on_readers from subscriber
    StatusMask mask;
    mask.any();
    mask << StatusMask::data_available();
    mask << StatusMask::subscription_matched();

    // CREATE THE PARTICIPANT
    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos, this, mask);

    if (participant_ == nullptr)
    {
        throw std::runtime_error("Error creating participant");
    }

    // CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Error creating subscriber");
    }

    std::cout <<
        "Participant < " << participant_->guid() <<
        " > created in domain < " << participant_->get_domain_id() <<
        " > waiting to discover type in topic < " << topic_name <<
        std::endl;
}

bool TypeIntrospectionSubscriber::is_stopped()
{
    return stop_;
}

void TypeIntrospectionSubscriber::stop()
{
    stop_ = true;

    type_discovered_cv_.notify_all();
    terminate_cv_.notify_all();
}

TypeIntrospectionSubscriber::~TypeIntrospectionSubscriber()
{
    if (participant_ != nullptr)
    {
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            if (reader_ != nullptr)
            {
                subscriber_->delete_datareader(reader_);
            }
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void TypeIntrospectionSubscriber::on_participant_discovery(
        DomainParticipant*,
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info,
        bool&)
{
    if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Participant found with guid: " << info.info.m_guid << std::endl;
    }
}

void TypeIntrospectionSubscriber::on_data_writer_discovery(
        DomainParticipant*,
        eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info,
        bool&)
{
    std::string type_name = info.info.typeName().to_string();

    // Check if the type is already created
    if (nullptr == participant_->find_type(type_name))
    {
        //CREATE THE DYNAMIC TYPE
        xtypes::TypeObject type_object;
        if (RETCODE_OK != DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    info.info.type_information().type_information.complete().typeid_with_size().type_id(), type_object))

        {
            std::cout << "ERROR: TypeObject cannot be retrieved for type: " << type_name << std::endl;
            return;
        }

        // Create DynamicType
        auto dyn_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(type_object)->build();
        if (!dyn_type)
        {
            std::cout << "ERROR: DynamicType cannot be created for type: " << type_name << std::endl;
            return;
        }

        on_type_discovered_and_registered_(dyn_type);
    }
}

void TypeIntrospectionSubscriber::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched with Writer: " << info.last_publication_handle << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched with Writer: " << info.last_publication_handle << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void TypeIntrospectionSubscriber::on_data_available(
        DataReader* reader)
{
    // Dynamic DataType
    DynamicData::_ref_type new_data =
                        DynamicDataFactory::get_instance()->create_data(dyn_type_);

    SampleInfo info;

    while ((reader->take_next_sample(&new_data, &info) == RETCODE_OK) && !is_stopped())
    {
        if (info.instance_state == ALIVE_INSTANCE_STATE)
        {
            // Add instance to the set of instances
            instances_.insert(info.instance_handle);

            samples_++;

            std::cout << "Message number " << samples_ << " RECEIVED:\n" << std::setw(4) << new_data << std::endl;

            // Stop if max messages has already been read
            if (max_messages_ > 0 && (samples_ >= max_messages_))
            {
                stop();
            }
        }
    }
}

void TypeIntrospectionSubscriber::run(
        uint32_t samples)
{
    stop_ = false;
    max_messages_ = samples;

    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Subscriber execution." << std::endl;
                static_cast<void>(signum);
                TypeIntrospectionSubscriber::stop();
            });

    // WAIT FOR TYPE DISCOVERY
    std::cout << "Subscriber waiting to discover type for topic < " << topic_name_
              << " > . Please press CTRL+C to stop the Subscriber." << std::endl;

    // Wait for type discovered
    {
        std::unique_lock<std::mutex> lck(type_discovered_cv_mtx_);
        type_discovered_cv_.wait(lck, []
                {
                    return is_stopped() || (type_discovered_.load() && type_registered_.load());
                });
    }

    if (is_stopped())
    {
        return;
    }

    std::cout <<
        "Subscriber < " << reader_->guid() <<
        " > listening for data in topic < " << topic_name_ <<
        " > found data type < " << dyn_type_->get_name() <<
        " >" << std::endl;

    // WAIT FOR SAMPLES READ
    if (samples > 0)
    {
        std::cout << "Running until " << samples <<
            " samples have been received. Please press CTRL+C to stop the Subscriber at any time." << std::endl;
    }
    else
    {
        std::cout << "Please press CTRL+C to stop the Subscriber." << std::endl;
    }

    // Wait for signal or thread max samples received
    {
        std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
        terminate_cv_.wait(lck, []
                {
                    return is_stopped();
                });
    }

    // Print number of data receive
    std::cout <<
        "Subscriber received " << samples_ <<
        " samples from " << instances_.size() <<
        " instances." << std::endl;
}

void TypeIntrospectionSubscriber::on_type_discovered_and_registered_(
        const DynamicType::_ref_type& type)
{
    // Copy dynamic type
    dyn_type_ = type;

    // Register type
    TypeSupport m_type(new DynamicPubSubType(type));
    m_type.register_type(participant_);

    // Create topic
    topic_ = participant_->create_topic(
        topic_name_,
        m_type->getName(),
        TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return;
    }

    // Create DataReader
    reader_ = subscriber_->create_datareader(
        topic_,
        DATAREADER_QOS_DEFAULT,
        this);

    std::cout <<
        "Participant < " << participant_->guid() <<
        " > in domain < " << participant_->get_domain_id() <<
        " > created reader < " << reader_->guid() <<
        " > in topic < " << topic_name_ <<
        " > with data type < " << type->get_name() << " > " <<
        ((reader_->type()->m_isGetKeyDefined) ? ". Topic with @key ." : "") <<
        std::endl;

    std::cout << "Data Type for this Subscriber is: " << type << std::endl;

    type_discovered_.store(true);
    type_registered_.store(true);
    type_discovered_cv_.notify_all();
}
