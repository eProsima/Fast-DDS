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
 * @file BenchMarkPublisher.cpp
 *
 */

#include "BenchmarkPublisher.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

#include <thread>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

BenchMarkPublisher::BenchMarkPublisher()
    : m_testStartTime(std::chrono::system_clock::now())
    , m_bBenchmarkFinished(false)
    , m_iTestTimeMs(10000)
    , m_iTickTime(100)
    , m_iWaitTime(1000)
    , m_iSize(0)
    , m_vSamples(nullptr)
    , m_iSamplesCount(0)
    , mp_participant(nullptr)
    , mp_publisher(nullptr)
    , mp_writer(nullptr)
    , mp_subscriber(nullptr)
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
    m_iCount = 0;
}

bool BenchMarkPublisher::init(
        int transport,
        ReliabilityQosPolicyKind reliabilityKind,
        int time,
        int tick_time,
        int wait_time,
        const std::string& topicName,
        int domain,
        int size)
{
    m_iTestTimeMs = time;
    m_iTickTime = tick_time;
    m_iWaitTime = wait_time;
    m_iSize = size;
    m_iSamplesSize = abs(m_iTestTimeMs / m_iTickTime) + 1;
    m_vSamples = new int[m_iSamplesSize];
    memset(m_vSamples, 0, sizeof(int) * m_iSamplesSize);
    m_iSamplesCount = 0;

    switch (m_iSize)
    {
        default:
        case 0:
            m_Hello.index(0);
            break;
        case 1:
            m_HelloSmall.index(0);
            break;
        case 2:
            m_HelloMedium.index(0);
            break;
        case 3:
            m_HelloBig.index(0);
            break;
    }

    //CREATE THE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(1, 0);
    pqos.name("Participant_pub");

    if (transport == 1)
    {
        pqos.transport().use_builtin_transports = true;
    }
    else if (transport == 2)
    {
        pqos.transport().use_builtin_transports = false;

        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        descriptor->add_listener_port(5100);
        pqos.transport().user_transports.push_back(descriptor);
    }
    else if (transport == 3)
    {
        //uint32_t kind = LOCATOR_KIND_UDPv6;
    }
    else if (transport == 4)
    {
        pqos.transport().use_builtin_transports = false;

        std::shared_ptr<TCPv6TransportDescriptor> descriptor = std::make_shared<TCPv6TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        descriptor->add_listener_port(5100);
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

    //CREATE THE PUBLISHER
    mp_publisher = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    mp_topic_pub = mp_participant->create_topic(topicName + "_1", data_type, TOPIC_QOS_DEFAULT);

    if (mp_topic_pub == nullptr)
    {
        return false;
    }

    //CREATE THE DATAWRITER
    DataWriterQos wqos;
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 30;
    wqos.resource_limits().max_samples = 50;
    wqos.resource_limits().allocated_samples = 20;
    wqos.reliable_writer_qos().times.heartbeatPeriod.seconds = 2;
    wqos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
    wqos.reliability().kind = reliabilityKind;
    wqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;

    mp_writer = mp_publisher->create_datawriter(mp_topic_pub, wqos, &m_pubListener);

    if (mp_writer == nullptr)
    {
        return false;
    }

    //CREATE THE SUBSCRIBER
    mp_subscriber = mp_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    mp_topic_sub = mp_participant->create_topic(topicName + "_2", data_type, TOPIC_QOS_DEFAULT);

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
    return true;
}

BenchMarkPublisher::~BenchMarkPublisher()
{
    if (m_vSamples != nullptr)
    {
        delete(m_vSamples);
        m_vSamples = nullptr;
    }
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

BenchMarkPublisher::SubListener::SubListener(
        BenchMarkPublisher* parent)
    : mParent(parent)
{
}

void BenchMarkPublisher::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void BenchMarkPublisher::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo m_info;
    if (!mParent->m_bBenchmarkFinished)
    {
        switch (mParent->m_iSize)
        {
            default:
            case 0:
            {
                if (reader->take_next_sample((void*)&m_Hello, &m_info) == ReturnCode_t::RETCODE_OK)
                {
                    if (m_info.valid_data)
                    {
                        if (m_Hello.index() > mParent->m_iCount)
                        {
                            m_Hello.index(mParent->m_iCount);
                        }
                        else
                        {
                            m_Hello.index(m_Hello.index() + 1);
                        }

                        mParent->m_iCount = m_Hello.index() + 1;
                        mParent->mp_writer->write((void*)&m_Hello);
                    }
                }
            }
            break;
            case 1:
            {
                if (reader->take_next_sample((void*)&m_HelloSmall, &m_info) == ReturnCode_t::RETCODE_OK)
                {
                    if (m_info.valid_data)
                    {
                        if (m_HelloSmall.index() > mParent->m_iCount)
                        {
                            m_HelloSmall.index(mParent->m_iCount);
                        }
                        else
                        {
                            m_HelloSmall.index(m_HelloSmall.index() + 1);
                        }

                        mParent->m_iCount = m_HelloSmall.index() + 1;
                        mParent->mp_writer->write((void*)&m_HelloSmall);
                    }
                }
            }
            break;
            case 2:
            {
                if (reader->take_next_sample((void*)&m_HelloMedium, &m_info) == ReturnCode_t::RETCODE_OK)
                {
                    if (m_info.valid_data)
                    {
                        if (m_HelloMedium.index() > mParent->m_iCount)
                        {
                            m_HelloMedium.index(mParent->m_iCount);
                        }
                        else
                        {
                            m_HelloMedium.index(m_HelloMedium.index() + 1);
                        }

                        mParent->m_iCount = m_HelloMedium.index() + 1;
                        mParent->mp_writer->write((void*)&m_HelloMedium);
                    }
                }
            }
            break;
            case 3:
            {
                if (reader->take_next_sample((void*)&m_HelloBig, &m_info) == ReturnCode_t::RETCODE_OK)
                {
                    if (m_info.valid_data)
                    {
                        if (m_HelloBig.index() > mParent->m_iCount)
                        {
                            m_HelloBig.index(mParent->m_iCount);
                        }
                        else
                        {
                            m_HelloBig.index(m_HelloBig.index() + 1);
                        }

                        mParent->m_iCount = m_HelloBig.index() + 1;
                        mParent->mp_writer->write((void*)&m_HelloBig);
                    }
                }
            }
            break;
        }
    }
}

BenchMarkPublisher::PubListener::PubListener(
        BenchMarkPublisher* parent)
    : mParent(parent)
    , matched_(0)
{
}

void BenchMarkPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Publisher matched. Test starts..." << std::endl;
        if (matched_ == 0)
        {
            mParent->m_testStartTime = std::chrono::system_clock::now();
        }
        matched_++;
    }
    else if (info.current_count_change == -1)
    {
        mParent->m_bBenchmarkFinished = true;
        std::cout << "Publisher unmatched. Test Aborted" << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void BenchMarkPublisher::runThread()
{
    int iPrevCount = 0;
    std::cout << "Publisher running..." << std::endl;
    while (!publish())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(m_iWaitTime));
    m_iCount = 0;

    while (!m_bBenchmarkFinished)
    {
        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_testStartTime);
        if (elapsed.count() > m_iTestTimeMs)
        {
            m_bBenchmarkFinished = true;
        }
        else
        {
            if (m_iSamplesCount < m_iSamplesSize)
            {
                m_vSamples[m_iSamplesCount++] = m_iCount - iPrevCount;
                iPrevCount = m_iCount;
            }

            // WAIT
            std::this_thread::sleep_for(std::chrono::milliseconds(m_iTickTime));
        }
    }
}

void BenchMarkPublisher::run()
{
    std::thread thread(&BenchMarkPublisher::runThread, this);
    //std::cout << "Publisher running..." << std::endl;
    thread.join();

    std::cout << "RESULTS after " << m_iTestTimeMs << " milliseconds:" << std::endl;
    std::cout << "COUNT: " << m_iCount << std::endl;
    std::cout << "SAMPLES: ";

    for (int i = 0; i < m_iSamplesCount; ++i)
    {
        std::cout << m_vSamples[i] << ",";
    }
    std::cout << std::endl;
}

bool BenchMarkPublisher::publish()
{
    if (m_pubListener.matched_ > 0)
    {
        switch (m_iSize)
        {
            default:
            case 0:
            {
                m_Hello.index(0);
                mp_writer->write((void*)&m_Hello);
                return true;
            }
            case 1:
            {
                m_HelloSmall.index(0);
                mp_writer->write((void*)&m_HelloSmall);
                return true;
            }
            case 2:
            {
                m_HelloMedium.index(0);
                mp_writer->write((void*)&m_HelloMedium);
                return true;
            }
            case 3:
            {
                m_HelloBig.index(0);
                mp_writer->write((void*)&m_HelloBig);
                return true;
            }
        }
    }
    return false;
}
