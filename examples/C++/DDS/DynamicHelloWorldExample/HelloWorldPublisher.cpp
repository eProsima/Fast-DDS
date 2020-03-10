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
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/DataWriter.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/topic/qos/DataWriterQos.hpp>

#include <fastrtps/types/DynamicDataFactory.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <thread>

using namespace eprosima::fastdds::dds;

HelloWorldPublisher::HelloWorldPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
{
}

bool HelloWorldPublisher::init()
{
    if (eprosima::fastrtps::xmlparser::XMLP_ret::XML_OK !=
            eprosima::fastrtps::xmlparser::XMLProfileManager::loadXMLFile("example_type.xml"))
    {
        std::cout << "Cannot open XML file \"example_type.xml\". Please, run the publisher from the folder "
                  << "that contatins this XML file." << std::endl;
        return false;
    }

    eprosima::fastrtps::types::DynamicType_ptr dyn_type =
            eprosima::fastrtps::xmlparser::XMLProfileManager::getDynamicTypeByName("HelloWorld")->build();
    TypeSupport m_type(new eprosima::fastrtps::types::DynamicPubSubType(dyn_type));
    m_Hello = eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dyn_type);

    m_Hello->set_string_value("Hello DDS Dynamic World", 0);
    m_Hello->set_uint32_value(0, 1);
    eprosima::fastrtps::types::DynamicData* array = m_Hello->loan_value(2);
    array->set_uint32_value(10, array->get_array_index({0, 0}));
    array->set_uint32_value(20, array->get_array_index({1, 0}));
    array->set_uint32_value(30, array->get_array_index({2, 0}));
    array->set_uint32_value(40, array->get_array_index({3, 0}));
    array->set_uint32_value(50, array->get_array_index({4, 0}));
    array->set_uint32_value(60, array->get_array_index({0, 1}));
    array->set_uint32_value(70, array->get_array_index({1, 1}));
    array->set_uint32_value(80, array->get_array_index({2, 1}));
    array->set_uint32_value(90, array->get_array_index({3, 1}));
    array->set_uint32_value(100, array->get_array_index({4, 1}));
    m_Hello->return_loaned_value(array);

    eprosima::fastrtps::ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_pub");
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(PParam);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    mp_participant->register_type(m_type);

    //CREATE THE PUBLISHER
    //PublisherQos qos;
    eprosima::fastrtps::PublisherAttributes Wparam;
    Wparam.topic.topicKind = eprosima::fastrtps::rtps::NO_KEY;
    Wparam.topic.topicDataType = "HelloWorld";
    Wparam.topic.topicName = "DDSDynHelloWorldTopic";
    Wparam.topic.auto_fill_type_object = true; // Share the type with readers.
    Wparam.topic.auto_fill_type_information = false;
    mp_publisher = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT, Wparam, nullptr);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    DataWriterQos qos;
    qos.reliability.kind = RELIABLE_RELIABILITY_QOS;
    writer_ = mp_publisher->create_datawriter(Wparam.topic, qos, &m_listener);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;

}

HelloWorldPublisher::~HelloWorldPublisher()
{
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

void HelloWorldPublisher::PubListener::on_offered_incompatible_qos(
        DataWriter* writer,
        const OfferedIncompatibleQosStatus& status)
{
    DataWriterQos qos = writer->get_qos();
    std::cout << "The Offered Qos is incompatible with the Requested one." << std::endl;
    std::cout << "The Qos causing this incompatibility is " << qos.search_qos_by_id(
        status.last_policy_id) << "." << std::endl;
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
                m_Hello->get_string_value(message, 0);
                uint32_t index;
                m_Hello->get_uint32_value(index, 1);
                std::string aux_array = "[";
                eprosima::fastrtps::types::DynamicData* array = m_Hello->loan_value(2);
                for (uint32_t i = 0; i < 5; ++i)
                {
                    aux_array += "[";
                    for (uint32_t j = 0; j < 2; ++j)
                    {
                        uint32_t elem;
                        array->get_uint32_value(elem, array->get_array_index({i, j}));
                        aux_array += std::to_string(elem) + (j == 1 ? "]" : ", ");
                    }
                    aux_array += (i == 4 ? "]" : "], ");
                }
                m_Hello->return_loaned_value(array);
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
                m_Hello->get_string_value(message, 0);
                uint32_t index;
                m_Hello->get_uint32_value(index, 1);
                std::string aux_array = "[";
                eprosima::fastrtps::types::DynamicData* array = m_Hello->loan_value(2);
                for (uint32_t i = 0; i < 5; ++i)
                {
                    aux_array += "[";
                    for (uint32_t j = 0; j < 2; ++j)
                    {
                        uint32_t elem;
                        array->get_uint32_value(elem, array->get_array_index({i, j}));
                        aux_array += std::to_string(elem) + (j == 1 ? "]" : ", ");
                    }
                    aux_array += (i == 4 ? "]" : "], ");
                }
                m_Hello->return_loaned_value(array);
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
        m_Hello->get_uint32_value(index, 1);
        m_Hello->set_uint32_value(index + 1, 1);

        eprosima::fastrtps::types::DynamicData* array = m_Hello->loan_value(2);
        array->set_uint32_value(10 + index, array->get_array_index({0, 0}));
        array->set_uint32_value(20 + index, array->get_array_index({1, 0}));
        array->set_uint32_value(30 + index, array->get_array_index({2, 0}));
        array->set_uint32_value(40 + index, array->get_array_index({3, 0}));
        array->set_uint32_value(50 + index, array->get_array_index({4, 0}));
        array->set_uint32_value(60 + index, array->get_array_index({0, 1}));
        array->set_uint32_value(70 + index, array->get_array_index({1, 1}));
        array->set_uint32_value(80 + index, array->get_array_index({2, 1}));
        array->set_uint32_value(90 + index, array->get_array_index({3, 1}));
        array->set_uint32_value(100 + index, array->get_array_index({4, 1}));
        m_Hello->return_loaned_value(array);

        writer_->write(m_Hello.get());
        return true;
    }
    return false;
}
