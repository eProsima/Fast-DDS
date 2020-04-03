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
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>
#include <fastrtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/IPLocator.h>

#include <thread>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

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
    , mp_subscriber(nullptr)
    , m_pubListener(this)
    , m_subListener(this)
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

    ParticipantAttributes PParam;
    PParam.domainId = domain;
    PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PParam.rtps.builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(1, 0);
    PParam.rtps.setName("Participant_pub");

    if (transport == 1)
    {
        PParam.rtps.useBuiltinTransports = true;
    }
    else if (transport == 2)
    {
        PParam.rtps.useBuiltinTransports = false;

        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        descriptor->add_listener_port(5100);
        PParam.rtps.userTransports.push_back(descriptor);
    }
    else if (transport == 3)
    {
        //uint32_t kind = LOCATOR_KIND_UDPv6;
    }
    else if (transport == 4)
    {
        PParam.rtps.useBuiltinTransports = false;

        std::shared_ptr<TCPv6TransportDescriptor> descriptor = std::make_shared<TCPv6TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        descriptor->add_listener_port(5100);
        PParam.rtps.userTransports.push_back(descriptor);
    }

    mp_participant = Domain::createParticipant(PParam);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;

    //REGISTER THE TYPE
    switch (m_iSize)
    {
        default:
        case 0:
            Wparam.topic.topicDataType = "BenchMark";
            Domain::registerType(mp_participant, &m_type);
            break;
        case 1:
            Wparam.topic.topicDataType = "BenchMarkSmall";
            Domain::registerType(mp_participant, &m_typeSmall);
            break;
        case 2:
            Wparam.topic.topicDataType = "BenchMarkMedium";
            Domain::registerType(mp_participant, &m_typeMedium);
            break;
        case 3:
            Wparam.topic.topicDataType = "BenchMarkBig";
            Domain::registerType(mp_participant, &m_typeBig);
            break;
    }

    Wparam.topic.topicName = topicName + "_1";
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    //Wparam.topic.historyQos.depth = 30;
    //Wparam.topic.resourceLimitsQos.max_samples = 50;
    //Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
    Wparam.qos.m_reliability.kind = reliabilityKind;
    Wparam.qos.m_publishMode.kind = ASYNCHRONOUS_PUBLISH_MODE;
    //Wparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    //Wparam.setUserDefinedID(1);
    //Wparam.setEntityID(2);

    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*)&m_pubListener);
    if (mp_publisher == nullptr)
    {
        return false;
    }

    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    switch (m_iSize)
    {
        default:
        case 0:
            Rparam.topic.topicDataType = "BenchMark";
            break;
        case 1:
            Rparam.topic.topicDataType = "BenchMarkSmall";
            break;
        case 2:
            Rparam.topic.topicDataType = "BenchMarkMedium";
            break;
        case 3:
            Rparam.topic.topicDataType = "BenchMarkBig";
            break;
    }
    Rparam.topic.topicName = topicName + "_2";
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    //Rparam.topic.historyQos.depth = 30;
    //Rparam.topic.resourceLimitsQos.max_samples = 50;
    //Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.topic.historyQos.depth = 30;
    Rparam.topic.resourceLimitsQos.max_samples = 50;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.qos.m_reliability.kind = reliabilityKind;
    //Rparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*)&m_subListener);

    if (mp_subscriber == nullptr)
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
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(mp_participant);
}

BenchMarkPublisher::SubListener::SubListener(
        BenchMarkPublisher* parent)
    : mParent(parent)
{
}

void BenchMarkPublisher::SubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        std::cout << "Subscriber matched" << std::endl;
    }
    else
    {
        std::cout << "Subscriber unmatched" << std::endl;
    }
}

void BenchMarkPublisher::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    if (!mParent->m_bBenchmarkFinished)
    {
        switch (mParent->m_iSize)
        {
            default:
            case 0:
            {
                if (sub->takeNextData((void*)&m_Hello, &m_info))
                {
                    if (m_info.sampleKind == ALIVE)
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
                        mParent->mp_publisher->write((void*)&m_Hello);
                    }
                }
            }
            break;
            case 1:
            {
                if (sub->takeNextData((void*)&m_HelloSmall, &m_info))
                {
                    if (m_info.sampleKind == ALIVE)
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
                        mParent->mp_publisher->write((void*)&m_HelloSmall);
                    }
                }
            }
            break;
            case 2:
            {
                if (sub->takeNextData((void*)&m_HelloMedium, &m_info))
                {
                    if (m_info.sampleKind == ALIVE)
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
                        mParent->mp_publisher->write((void*)&m_HelloMedium);
                    }
                }
            }
            break;
            case 3:
            {
                if (sub->takeNextData((void*)&m_HelloBig, &m_info))
                {
                    if (m_info.sampleKind == ALIVE)
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
                        mParent->mp_publisher->write((void*)&m_HelloBig);
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
    , n_matched(0)
{
}

void BenchMarkPublisher::PubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        std::cout << "Publisher matched. Test starts..." << std::endl;
        if (n_matched == 0)
        {
            mParent->m_testStartTime = std::chrono::system_clock::now();
        }
        n_matched++;
    }
    else
    {
        mParent->m_bBenchmarkFinished = true;
        std::cout << "Publisher unmatched. Test Aborted" << std::endl;
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
    if (m_pubListener.n_matched > 0)
    {
        switch (m_iSize)
        {
            default:
            case 0:
            {
                m_Hello.index(0);
                mp_publisher->write((void*)&m_Hello);
                return true;
            }
            case 1:
            {
                m_HelloSmall.index(0);
                mp_publisher->write((void*)&m_HelloSmall);
                return true;
            }
            case 2:
            {
                m_HelloMedium.index(0);
                mp_publisher->write((void*)&m_HelloMedium);
                return true;
            }
            case 3:
            {
                m_HelloBig.index(0);
                mp_publisher->write((void*)&m_HelloBig);
                return true;
            }
        }
    }
    return false;
}
