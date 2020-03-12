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
#include <fastdds/dds/topic/DataWriter.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/topic/qos/DataWriterQos.hpp>

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

    DomainParticipantQos part_qos = PARTICIPANT_QOS_DEFAULT;
    part_qos.part_attr.rtps.builtin.discovery_config.discoveryProtocol = SIMPLE;
    part_qos.part_attr.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    part_qos.part_attr.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    part_qos.part_attr.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    part_qos.part_attr.rtps.builtin.typelookup_config.use_server = true;
    part_qos.part_attr.rtps.builtin.use_WriterLivelinessProtocol = false;
    part_qos.part_attr.rtps.builtin.domainId = 0;
    part_qos.part_attr.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    part_qos.part_attr.rtps.setName("Participant_pub");
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(
        part_qos.part_attr.rtps.builtin.domainId, part_qos);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    mp_participant->register_type(m_type);

    //CREATE THE PUBLISHER
    PublisherQos p_qos = PUBLISHER_QOS_DEFAULT;
    p_qos.pub_attr.topic.topicKind = NO_KEY;
    p_qos.pub_attr.topic.topicDataType = "TypeLookup";
    p_qos.pub_attr.topic.topicName = "TypeLookupTopic";
    p_qos.pub_attr.topic.historyQos.kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
    p_qos.pub_attr.topic.historyQos.depth = 30;
    p_qos.pub_attr.topic.resourceLimitsQos.max_samples = 50;
    p_qos.pub_attr.topic.resourceLimitsQos.allocated_samples = 20;
    p_qos.pub_attr.topic.auto_fill_type_object = false;
    p_qos.pub_attr.topic.auto_fill_type_information = true; // Share the type with readers.
    p_qos.pub_attr.times.heartbeatPeriod.seconds = 2;
    p_qos.pub_attr.times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;

    mp_publisher = mp_participant->create_publisher(p_qos, nullptr);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    //CREATE TOPIC
    TopicQos topicQos = TOPIC_QOS_DEFAULT;
    topicQos.topic_attr = p_qos.pub_attr.topic;

    topic_ = mp_participant->create_topic("TypeLookupTopic", "TypeLookup", topicQos);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    DataWriterQos wqos;
    wqos.reliability.kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    writer_ = mp_publisher->create_datawriter(topic_, wqos, &m_listener);

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
    std::cout << "On Publication Matched" << std::endl;
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

void TypeLookupPublisher::PubListener::on_offered_incompatible_qos(
        DataWriter* writer,
        const OfferedIncompatibleQosStatus& status)
{
    DataWriterQos qos = writer->get_qos();
    std::cout << "The Offered Qos is incompatible with the Requested one." << std::endl;
    std::cout << "The Qos causing this incompatibility is " << qos.search_qos_by_id(
        status.last_policy_id) << "." << std::endl;
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
