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
    xmlparser::XMLProfileManager::loadXMLFile("example_type.xml");

    types::DynamicType_ptr dyn_type = xmlparser::XMLProfileManager::getDynamicTypeByName("TypeLookup")->build();
    TypeSupport m_type(new types::DynamicPubSubType(dyn_type));
    m_Hello = types::DynamicDataFactory::get_instance()->create_data(dyn_type);

    m_Hello->set_string_value("Hello DDS Dynamic World", 0);
    m_Hello->set_uint32_value(0, 1);
    types::DynamicData* inner = m_Hello->loan_value(2);
    inner->set_byte_value(10, 0);
    m_Hello->return_loaned_value(inner);

    DomainParticipantQos pqos;
    WireProtocolConfigQos wp = pqos.wire_protocol();
    wp.builtin.discovery_config.discoveryProtocol = SIMPLE;
    wp.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    wp.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    wp.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    wp.builtin.typelookup_config.use_server = true;
    wp.builtin.use_WriterLivelinessProtocol = false;
    wp.builtin.domainId = 0;
    wp.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    pqos.wire_protocol(wp);
    pqos.name("Participant_pub");
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(wp.builtin.domainId, pqos);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    mp_participant->register_type(m_type);

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "TypeLookup";
    Wparam.topic.topicName = "TypeLookupTopic";
    Wparam.topic.historyQos.kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.topic.auto_fill_type_object = false;
    Wparam.topic.auto_fill_type_information = true; // Share the type with readers.
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
    Wparam.qos.m_reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    mp_publisher = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT, Wparam, nullptr);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    writer_ = mp_publisher->create_datawriter(Wparam.topic, Wparam.qos, &m_listener);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;

}

TypeLookupPublisher::~TypeLookupPublisher()
{
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
