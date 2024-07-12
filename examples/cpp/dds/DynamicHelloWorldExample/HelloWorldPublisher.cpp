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
 * @file HelloWorldPublisher.cpp
 *
 */

#include "HelloWorldPublisher.h"

#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>

using namespace eprosima::fastdds::dds;

HelloWorldPublisher::HelloWorldPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
{
}

bool HelloWorldPublisher::init()
{
    if (RETCODE_OK !=
            DomainParticipantFactory::get_instance()->load_XML_profiles_file("helloworld_example_type_profile.xml"))
    {
        std::cout <<
            "Cannot open XML file \"helloworld_example_type_profile.xml\". Please, run the publisher from the folder "
                  << "that contatins this XML file." << std::endl;
        return false;
    }

    DynamicTypeBuilder::_ref_type dyn_type_builder;
    if (RETCODE_OK !=
            DomainParticipantFactory::get_instance()->get_dynamic_type_builder_from_xml_by_name("HelloWorld",
            dyn_type_builder))
    {
        std::cout <<
            "Error getting dynamic type \"HelloWorld\"." << std::endl;
        return false;
    }

    TypeSupport m_type(new DynamicPubSubType(dyn_type_builder->build()));
    m_Hello = DynamicDataFactory::get_instance()->create_data(dyn_type_builder->build());
    m_Hello->set_string_value(m_Hello->get_member_id_by_name("message"), "Hello DDS Dynamic World");
    m_Hello->set_uint32_value(m_Hello->get_member_id_by_name("index"), 0);
    m_Hello->set_uint32_values(m_Hello->get_member_id_by_name("array"), {10, 20, 30, 40, 50, 60, 70, 80, 90, 100});

    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    //TODO(Xtypes) this property will be removed
    m_type.get()->auto_fill_type_information(false);

    m_type.register_type(mp_participant);

    //CREATE THE PUBLISHER
    mp_publisher = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    topic_ = mp_participant->create_topic("DDSDynHelloWorldTopic", m_type->get_name(), TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    writer_ = mp_publisher->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &m_listener);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;

}

HelloWorldPublisher::~HelloWorldPublisher()
{
    if (writer_ != nullptr)
    {
        mp_publisher->delete_datawriter(writer_);
    }
    if (mp_publisher != nullptr)
    {
        mp_participant->delete_publisher(mp_publisher);
    }
    if (topic_ != nullptr)
    {
        mp_participant->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        firstConnected = true;
        std::cout << "Publisher matched" << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "Publisher unmatched" << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldPublisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    if (samples == 0)
    {
        while (!stop)
        {
            if (publish(false))
            {
                std::string message;
                m_Hello->get_string_value(message, m_Hello->get_member_id_by_name("message"));
                uint32_t index {0};
                m_Hello->get_uint32_value(index, m_Hello->get_member_id_by_name("index"));
                UInt32Seq array;
                m_Hello->get_uint32_values(array, m_Hello->get_member_id_by_name("array"));
                std::string aux_array = "[";

                for (uint32_t i = 0; i < 5; ++i)
                {
                    aux_array += "[";
                    for (uint32_t j = 0; j < 2; ++j)
                    {
                        aux_array += std::to_string(array.at((i * 5) + j)) + (j == 1 ? "]" : ", ");
                    }
                    aux_array += (i == 4 ? "]" : "], ");
                }

                std::cout << "Message: " << message << " with index: " << index
                          << " array: " << aux_array << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
    else
    {
        for (uint32_t s = 0; s < samples; ++s)
        {
            if (!publish())
            {
                --s;
            }
            else
            {
                std::string message;
                m_Hello->get_string_value(message, m_Hello->get_member_id_by_name("message"));
                uint32_t index {0};
                m_Hello->get_uint32_value(index, m_Hello->get_member_id_by_name("index"));
                UInt32Seq array;
                m_Hello->get_uint32_values(array, m_Hello->get_member_id_by_name("array"));
                std::string aux_array = "[";

                for (uint32_t i = 0; i < 5; ++i)
                {
                    aux_array += "[";
                    for (uint32_t j = 0; j < 2; ++j)
                    {
                        aux_array += std::to_string(array.at((i * 5) + j)) + (j == 1 ? "]" : ", ");
                    }
                    aux_array += (i == 4 ? "]" : "], ");
                }

                std::cout << "Message: " << message << " with index: " << index
                          << " array: " << aux_array << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
}

void HelloWorldPublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop = false;
    std::thread thread(&HelloWorldPublisher::runThread, this, samples, sleep);
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
        std::cin.ignore();
        stop = true;
    }
    else
    {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    thread.join();
}

bool HelloWorldPublisher::publish(
        bool waitForListener)
{
    if (m_listener.firstConnected || !waitForListener || m_listener.n_matched > 0)
    {
        uint32_t index;
        m_Hello->get_uint32_value(index, m_Hello->get_member_id_by_name("index"));
        m_Hello->set_uint32_value(m_Hello->get_member_id_by_name("index"), index + 1);
        m_Hello->set_uint32_values(m_Hello->get_member_id_by_name(
                    "array"),
                {10 + index, 20 + index, 30 + index, 40 + index, 50 + index, 60 + index, 70 + index, 80 + index, 90 + index,
                 100 + index});
        writer_->write(&m_Hello);
        return true;
    }
    return false;
}
