// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LivelinessPublishers.cpp
 *
 */

#include "LivelinessPublishers.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

#include <thread>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

LivelinessPublishers::LivelinessPublishers()
    : participant_(nullptr)
    , publisher_1_(nullptr)
{
}

bool LivelinessPublishers::init(
        LivelinessQosPolicyKind first_kind,
        LivelinessQosPolicyKind second_kind,
        int first_liveliness_ms,
        int second_liveliness_ms)
{
    topic_.index(0);
    topic_.message("HelloWorld");

    ParticipantAttributes PParam;
    PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.domainId = 0;
    PParam.rtps.builtin.use_WriterLivelinessProtocol = true;
    PParam.rtps.setName("Participant_pub");
    participant_ = Domain::createParticipant(PParam);
    if(participant_==nullptr)
    {
        return false;
    }
    Domain::registerType(participant_,&type_);

    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "Topic";
    Wparam.topic.topicName = "Name";
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;

    // First publisher
    Wparam.qos.m_liveliness.lease_duration = Duration_t(first_liveliness_ms * 1e-3);
    Wparam.qos.m_liveliness.announcement_period = Duration_t(first_liveliness_ms * 1e-3 * 0.5);
    Wparam.qos.m_liveliness.kind = first_kind;
    publisher_1_ = Domain::createPublisher(participant_, Wparam, &listener_);
    if(publisher_1_ == nullptr)
    {
        return false;
    }

    // Second publisher
    Wparam.qos.m_liveliness.lease_duration = Duration_t(second_liveliness_ms * 1e-3);
    Wparam.qos.m_liveliness.announcement_period = Duration_t(second_liveliness_ms * 1e-3 * 0.5);
    Wparam.qos.m_liveliness.kind = second_kind;
    publisher_2_ = Domain::createPublisher(participant_, Wparam, &listener_);
    if(publisher_2_ == nullptr)
    {
        return false;
    }

    std::cout << "Publisher 1 using:" << std::endl;
    std::cout << "Lease duration: " << first_liveliness_ms << std::endl;
    if (first_kind == eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
    {
        std::cout << "Kind: AUTOMATIC" << std::endl;
    }
    else if (first_kind == eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        std::cout << "Kind: MANUAL_BY_PARTICIPANT_LIVELINESS_QOS" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Publisher 2 using:" << std::endl;
    std::cout << "Lease duration: " << second_liveliness_ms << std::endl;
    if (second_kind == eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
    {
        std::cout << "Kind: AUTOMATIC" << std::endl;
    }
    else if (second_kind == eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        std::cout << "Kind: MANUAL_BY_PARTICIPANT_LIVELINESS_QOS" << std::endl;
    }
    std::cout << std::endl;

    return true;
}

LivelinessPublishers::~LivelinessPublishers()
{
    Domain::removeParticipant(participant_);
}

void LivelinessPublishers::PubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    std::unique_lock<std::recursive_mutex> lock(listener_mutex_);
    if(info.status == MATCHED_MATCHING)
    {
        n_matched++;
        first_connected = true;
        std::cout << "Publisher matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Publisher unmatched" << std::endl;
    }
}

void LivelinessPublishers::run(uint32_t samples, uint32_t sleep)
{
    std::thread thread1(&LivelinessPublishers::runThread, this, publisher_1_, samples, sleep);
    std::thread thread2(&LivelinessPublishers::runThread, this, publisher_2_, samples, sleep);
    thread1.join();
    thread2.join();
}

void LivelinessPublishers::runThread(
        Publisher* pub,
        uint32_t samples,
        uint32_t sleep)
{

    for(uint32_t i = 0;i<samples;++i)
    {
        if(!publish(pub))
        {
            --i;
        }
        else
        {
            std::unique_lock<std::recursive_mutex> lock(pub_mutex);
            std::cout << "Message with index " << topic_.index()<< " SENT by publisher " << pub->getGuid() << std::endl;
        }
        eClock::my_sleep(sleep);
    }

    std::cin.ignore();
}

bool LivelinessPublishers::publish(
        Publisher* pub,
        bool waitForListener)
{
    if(listener_.first_connected || !waitForListener || listener_.n_matched > 0)
    {
        std::unique_lock<std::recursive_mutex> lock(pub_mutex);
        topic_.index(topic_.index()+1);
        pub->write((void*)&topic_);

        return true;
    }
    return false;
}
