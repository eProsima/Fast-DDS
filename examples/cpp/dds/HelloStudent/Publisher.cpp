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
 * @file Publisher.cpp
 *
 */

#include "Publisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <thread>

using namespace eprosima::fastdds::dds;

namespace student
{

    Publisher::Publisher()
        : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr), type_(new StudentInfoPubSubType())
    {
    }

    bool Publisher::init(
        bool use_env)
    {
        hello_.id(0);
        hello_.name("StudentInfo");
        DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
        pqos.name("Participant_pub");
        auto factory = DomainParticipantFactory::get_instance();

        if (use_env)
        {
            factory->load_profiles();
            factory->get_default_participant_qos(pqos);
        }

        participant_ = factory->create_participant(0, pqos);

        if (participant_ == nullptr)
        {
            return false;
        }

        // REGISTER THE TYPE
        type_.register_type(participant_);

        // CREATE THE PUBLISHER
        PublisherQos pubqos = PUBLISHER_QOS_DEFAULT;

        if (use_env)
        {
            participant_->get_default_publisher_qos(pubqos);
        }

        publisher_ = participant_->create_publisher(
            pubqos,
            nullptr);

        if (publisher_ == nullptr)
        {
            return false;
        }

        // CREATE THE TOPIC
        TopicQos tqos = TOPIC_QOS_DEFAULT;

        if (use_env)
        {
            participant_->get_default_topic_qos(tqos);
        }

        topic_ = participant_->create_topic(
            "StudentInfoTopic",
            "StudentInfo",
            tqos);

        if (topic_ == nullptr)
        {
            return false;
        }

        // CREATE THE WRITER
        DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;

        if (use_env)
        {
            publisher_->get_default_datawriter_qos(wqos);
        }

        writer_ = publisher_->create_datawriter(
            topic_,
            wqos,
            &listener_);

        // writer_ = publisher_->create_datawriter_with_profile(
        //     topic_,
        //     "datawriter_profile",
        //     &listener_);

        if (writer_ == nullptr)
        {
            return false;
        }

        return true;
    }

    Publisher::~Publisher()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }
        if (publisher_ != nullptr)
        {
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    void Publisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter *,
        const eprosima::fastdds::dds::PublicationMatchedStatus &info)
    {
        if (info.current_count_change == 1)
        {
            matched_ = info.total_count;
            firstConnected_ = true;
            std::cout << "Publisher matched." << std::endl;
        }
        else if (info.current_count_change == -1)
        {
            matched_ = info.total_count;
            std::cout << "Publisher unmatched." << std::endl;
        }
        else
        {
            std::cout << info.current_count_change
                      << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
        }
    }

    void Publisher::runThread(uint32_t sleep)
    {
        while (1)
        {
            if (publish(false))
            {
                std::cout << "Message: " << hello_.name() << " with index: " << hello_.id()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }

    void Publisher::run(uint32_t sleep)
    {
        std::thread thread(&Publisher::runThread, this, sleep);
        thread.join();
    }

    bool Publisher::publish(
        bool waitForListener)
    {
        if (listener_.firstConnected_ || !waitForListener || listener_.matched_ > 0)
        {
            hello_.id(hello_.id() + 1);
            writer_->write(&hello_);
            return true;
        }
        return false;
    }

}
