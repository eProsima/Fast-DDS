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
 * @file xtypesExamplePublisher.cpp
 *
 */

#include <thread>

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <xtypes/idl/idl.hpp>
#include <xtypes/idl/parser.hpp>

#include "GenericTopicDataType.h"
#include "xtypesExamplePublisher.h"

using namespace eprosima::fastdds::dds;

xtypesExamplePublisher::xtypesExamplePublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
        // Create a Type Support with type xTypesExampleType
        // Type describred in xTypesExample.idl
    , type_(new GenericTopicDataType(IDL_FILE_NAME, TYPE_NAME_PUB))
    , samples_sent_(0)
    , stop_(false)
{
}

bool xtypesExamplePublisher::init()
{
    // CREATE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.name("xTypesExamplePublisher");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER TYPE
    if (ReturnCode_t::RETCODE_OK != type_.register_type(participant_))
    {
        return false;
    }

    //CREATE PUBLISHER
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (publisher_ == nullptr)
    {
        return false;
    }

    //CREATE TOPIC
    topic_ = participant_->create_topic(TOPIC_NAME, TYPE_NAME_PUB, TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE WRITER
    writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;
}

xtypesExamplePublisher::~xtypesExamplePublisher()
{
    if (participant_)
    {
        if (publisher_ != nullptr)
        {
            if (writer_ != nullptr)
            {
                publisher_->delete_datawriter(writer_);
            }
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void xtypesExamplePublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void xtypesExamplePublisher::runThread()
{
    eprosima::xtypes::DynamicData* data = reinterpret_cast<eprosima::xtypes::DynamicData*>(type_.create_data());

    (*data)["message"] = "xTypes Hello World message with type: " TYPE_NAME_PUB;

    while (!stop_)
    {
        // Set new index to data
        uint32_t current_index = (*data)["index"].value<uint32_t>();
        (*data)["index"] = current_index + 1;

        // Publish data
        writer_->write(data);
        std::cout << "Message: '" << (*data)["message"].value<std::string>()
            << "' with index: " << (*data)["index"].value<uint32_t> ()<< " SENT" << std::endl;

        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }

    type_.delete_data(data);
}

void xtypesExamplePublisher::run()
{
    stop_ = false;
    std::thread thread(&xtypesExamplePublisher::runThread, this);

    std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
    std::cin.ignore();
    std::cout << "Publisher finishing." << std::endl;

    stop_ = true;

    thread.join();
}

