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
 * @file TypeLookupPublisher.cpp
 *
 */

#include "TypeLookupPublisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastrtps/types/DynamicDataFactory.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <thread>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TypeLookupPublisher::TypeLookupPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
{
}

bool TypeLookupPublisher::init()
{
    if (xmlparser::XMLP_ret::XML_OK !=
        xmlparser::XMLProfileManager::loadXMLFile("example_type.xml"))
    {
        std::cout << "Cannot open XML file \"example_type.xml\". Please, run the publisher from the folder "
                  << "that contatins this XML file." << std::endl;
        return false;
    }

    types::DynamicType_ptr dyn_type = xmlparser::XMLProfileManager::getDynamicTypeByName("TypeLookup")->build();
    TypeSupport m_type(new types::DynamicPubSubType(dyn_type));
    m_Hello = types::DynamicDataFactory::get_instance()->create_data(dyn_type);

    m_Hello->set_string_value("Hello DDS Dynamic World", 0);
    m_Hello->set_uint32_value(0, 1);
    types::DynamicData* inner = m_Hello->loan_value(2);
    inner->set_byte_value(10, 0);
    m_Hello->return_loaned_value(inner);

    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol = SIMPLE;
    pqos.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    pqos.wire_protocol().builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    pqos.wire_protocol().builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    pqos.wire_protocol().builtin.typelookup_config.use_server = true;
    pqos.wire_protocol().builtin.use_WriterLivelinessProtocol = false;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = c_TimeInfinite;
    pqos.name("Participant_pub");
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    m_type.get()->auto_fill_type_information(true);
    m_type.get()->auto_fill_type_object(false);
    m_type.register_type(mp_participant);

    //CREATE THE PUBLISHER
    mp_publisher = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    topic_ = mp_participant->create_topic("TypeLookupTopic", "TypeLookup", TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    DataWriterQos wqos;
    wqos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 30;
    wqos.resource_limits().max_samples = 50;
    wqos.resource_limits().allocated_samples = 20;
    wqos.reliable_writer_qos().times.heartbeatPeriod.seconds = 2;
    wqos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
    writer_ = mp_publisher->create_datawriter(topic_, wqos, &m_listener);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;

}

TypeLookupPublisher::~TypeLookupPublisher()
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

void TypeLookupPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched++;
        firstConnected = true;
        std::cout << "Publisher matched" << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched--;
        std::cout << "Publisher unmatched" << std::endl;
    }
    else
    {
        std::cout << "Publisher received an invalid value for PublicationMatchedStatus." << std::endl;
    }
}

void TypeLookupPublisher::runThread(
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
                std::cout << "Message: " << message << " with index: " << index << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
    else
    {
        for (uint32_t i = 0; i < samples; ++i)
        {
            if (!publish())
            {
                --i;
            }
            else
            {
                std::string message;
                m_Hello->get_string_value(message, 0);
                uint32_t index;
                m_Hello->get_uint32_value(index, 1);
                std::cout << "Message: " << message << " with index: " << index << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
}

void TypeLookupPublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop = false;
    std::thread thread(&TypeLookupPublisher::runThread, this, samples, sleep);
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

bool TypeLookupPublisher::publish(
        bool waitForListener)
{
    if (m_listener.firstConnected || !waitForListener || m_listener.n_matched > 0)
    {
        uint32_t index;
        m_Hello->get_uint32_value(index, 1);
        m_Hello->set_uint32_value(index + 1, 1);
        types::DynamicData* inner = m_Hello->loan_value(2);
        octet inner_count;
        inner->get_byte_value(inner_count, 0);
        inner->set_byte_value(inner_count + 1, 0);
        m_Hello->return_loaned_value(inner);
        writer_->write(m_Hello.get());
        return true;
    }
    return false;
}
