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
 * @file BenchMarkSubscriber.cpp
 *
 */

#include "BenchmarkSubscriber.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>

#include <fastrtps/Domain.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

using Locator_t = eprosima::fastrtps::rtps::Locator_t;
using IPLocator = eprosima::fastrtps::rtps::IPLocator;

BenchMarkSubscriber::BenchMarkSubscriber()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , mp_subscriber(nullptr)
    , mp_writer(nullptr)
    , mp_reader(nullptr)
    , mp_topic_pub(nullptr)
    , mp_topic_sub(nullptr)
    , m_pubListener(this)
    , m_subListener(this)
    , m_type(new BenchMarkPubSubType())
    , m_typeSmall(new BenchMarkSmallPubSubType())
    , m_typeMedium(new BenchMarkMediumPubSubType())
    , m_typeBig(new BenchMarkBigPubSubType())
{
}

bool BenchMarkSubscriber::init(
        int transport,
        ReliabilityQosPolicyKind reliabilityKind,
        const std::string& topicName,
        int domain,
        int size)
{
    m_iSize = size;

    //CREATE THE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(1, 0);
    pqos.name("Participant_sub");

    if (transport == 1)
    {
        pqos.transport().use_builtin_transports = true;
    }
    else if (transport == 2)
    {
        int32_t kind = LOCATOR_KIND_TCPv4;

        Locator_t initial_peer_locator;
        initial_peer_locator.kind = kind;
        IPLocator::setIPv4(initial_peer_locator, "127.0.0.1");
        initial_peer_locator.port = 5100;
        pqos.wire_protocol().builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's channel

        pqos.transport().use_builtin_transports = false;
        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        pqos.transport().user_transports.push_back(descriptor);
    }
    else if (transport == 3)
    {
        //uint32_t kind = LOCATOR_KIND_UDPv6;
    }
    else if (transport == 4)
    {
        uint32_t kind = LOCATOR_KIND_TCPv6;
        pqos.transport().use_builtin_transports = false;

        Locator_t initial_peer_locator;
        initial_peer_locator.kind = kind;
        IPLocator::setIPv6(initial_peer_locator, "::1");
        initial_peer_locator.port = 5100;
        pqos.wire_protocol().builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's channel

        pqos.transport().use_builtin_transports = false;
        std::shared_ptr<TCPv6TransportDescriptor> descriptor = std::make_shared<TCPv6TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        pqos.transport().user_transports.push_back(descriptor);
    }

    mp_participant = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    std::string data_type;
    switch (m_iSize)
    {
        default:
        case 0:
            data_type = "BenchMark";
            m_type.register_type(mp_participant);
            break;
        case 1:
            data_type = "BenchMarkSmall";
            m_typeSmall.register_type(mp_participant);
            break;
        case 2:
            data_type = "BenchMarkMedium";
            m_typeMedium.register_type(mp_participant);
            break;
        case 3:
            data_type = "BenchMarkBig";
            m_typeBig.register_type(mp_participant);
            break;
    }

    //CREATE THE SUBSCRIBER
    mp_subscriber = mp_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

    //CREATE THE PUBLISHER
    mp_publisher = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    mp_topic_sub = mp_participant->create_topic(topicName + "_1", data_type, TOPIC_QOS_DEFAULT);

    if (mp_topic_sub == nullptr)
    {
        return false;
    }

    //CREATE THE DATAREADER
    DataReaderQos rqos;
    rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos.history().depth = 30;
    rqos.resource_limits().max_samples = 50;
    rqos.resource_limits().allocated_samples = 20;
    rqos.reliability().kind = reliabilityKind;
    rqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    mp_reader = mp_subscriber->create_datareader(mp_topic_sub, rqos, &m_subListener);

    if (mp_reader == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    mp_topic_pub = mp_participant->create_topic(topicName + "_2", data_type, TOPIC_QOS_DEFAULT);

    if (mp_topic_pub == nullptr)
    {
        return false;
    }

    //CREATE THE DATAWRITER
    DataWriterQos wqos;
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 1;
    wqos.resource_limits().max_samples = 1;
    wqos.resource_limits().allocated_samples = 1;
    wqos.reliable_writer_qos().times.heartbeatPeriod.seconds = 2;
    wqos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
    wqos.reliability().kind = reliabilityKind;
    wqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;

    mp_writer = mp_publisher->create_datawriter(mp_topic_pub, wqos, &m_pubListener);

    if (mp_writer == nullptr)
    {
        return false;
    }

    return true;
}

BenchMarkSubscriber::~BenchMarkSubscriber()
{
    if (mp_writer != nullptr)
    {
        mp_publisher->delete_datawriter(mp_writer);
    }
    if (mp_publisher != nullptr)
    {
        mp_participant->delete_publisher(mp_publisher);
    }
    if (mp_topic_pub != nullptr)
    {
        mp_participant->delete_topic(mp_topic_pub);
    }
    if (mp_reader != nullptr)
    {
        mp_subscriber->delete_datareader(mp_reader);
    }
    if (mp_subscriber != nullptr)
    {
        mp_participant->delete_subscriber(mp_subscriber);
    }
    if (mp_topic_sub != nullptr)
    {
        mp_participant->delete_topic(mp_topic_sub);
    }
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
}

BenchMarkSubscriber::PubListener::PubListener(
        BenchMarkSubscriber* parent)
    : mParent(parent)
    , matched_(0)
    , first_connected_(false)
{
}

void BenchMarkSubscriber::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        first_connected_ = true;
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

BenchMarkSubscriber::SubListener::SubListener(
        BenchMarkSubscriber* parent)
    : mParent(parent)
    , matched_(0)
    , samples_(0)
{
}

void BenchMarkSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void BenchMarkSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo m_info;
    switch (mParent->m_iSize)
    {
        default:
        case 0:
        {
            if (reader->take_next_sample((void*)&mParent->m_Hello, &m_info) == ReturnCode_t::RETCODE_OK)
            {
                if (m_info.instance_state == eprosima::fastdds::dds::ALIVE)
                {
                    mParent->m_Hello.index(mParent->m_Hello.index() + 1);
                    mParent->mp_writer->write((void*)&mParent->m_Hello);
                }
            }
        }
        break;
        case 1:
        {
            if (reader->take_next_sample((void*)&mParent->m_HelloSmall, &m_info) == ReturnCode_t::RETCODE_OK)
            {
                if (m_info.instance_state == eprosima::fastdds::dds::ALIVE)
                {
                    mParent->m_HelloSmall.index(mParent->m_HelloSmall.index() + 1);
                    mParent->mp_writer->write((void*)&mParent->m_HelloSmall);
                }
            }
        }
        break;
        case 2:
        {
            if (reader->take_next_sample((void*)&mParent->m_HelloMedium, &m_info) == ReturnCode_t::RETCODE_OK)
            {
                if (m_info.instance_state == eprosima::fastdds::dds::ALIVE)
                {
                    mParent->m_HelloMedium.index(mParent->m_HelloMedium.index() + 1);
                    mParent->mp_writer->write((void*)&mParent->m_HelloMedium);
                }
            }
        }
        break;
        case 3:
        {
            if (reader->take_next_sample((void*)&mParent->m_HelloBig, &m_info) == ReturnCode_t::RETCODE_OK)
            {
                if (m_info.instance_state == eprosima::fastdds::dds::ALIVE)
                {
                    mParent->m_HelloBig.index(mParent->m_HelloBig.index() + 1);
                    mParent->mp_writer->write((void*)&mParent->m_HelloBig);
                }
            }
        }
        break;
    }
}

void BenchMarkSubscriber::run()
{
    std::cout << "Subscriber running..." << std::endl;
    std::cin.ignore();
}
