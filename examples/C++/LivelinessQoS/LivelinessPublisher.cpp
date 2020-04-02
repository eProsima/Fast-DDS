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
 * @file LivelinessPublisher.cpp
 *
 */

#include "LivelinessPublisher.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>

#include <thread>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

LivelinessPublisher::LivelinessPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
{
}

bool LivelinessPublisher::init(
        LivelinessQosPolicyKind kind,
        int liveliness_ms)
{
    topic_.index(0);
    topic_.message("HelloWorld");

    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SIMPLE;
    PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.use_WriterLivelinessProtocol = true;
    PParam.rtps.setName("Participant_pub");
    participant_ = Domain::createParticipant(PParam);
    if (participant_ == nullptr)
    {
        return false;
    }
    Domain::registerType(participant_, &type_);

    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "Topic";
    Wparam.topic.topicName = "Name";
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.qos.m_liveliness.lease_duration = Duration_t(liveliness_ms * 1e-3);
    Wparam.qos.m_liveliness.announcement_period = Duration_t(liveliness_ms * 1e-3 * 0.5);
    Wparam.qos.m_liveliness.kind = kind;
    publisher_ = Domain::createPublisher(participant_, Wparam, &listener_);
    if (publisher_ == nullptr)
    {
        return false;
    }
    return true;
}

LivelinessPublisher::~LivelinessPublisher()
{
    Domain::removeParticipant(participant_);
}

void LivelinessPublisher::PubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
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

void LivelinessPublisher::PubListener::on_liveliness_lost(
        Publisher* pub,
        const LivelinessLostStatus& status)
{
    std::cout << "Publisher " << pub->getGuid() << " lost liveliness: " << status.total_count << std::endl;
}

void LivelinessPublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    std::thread thread1(&LivelinessPublisher::runThread, this, publisher_, samples, sleep);
    thread1.join();
}

void LivelinessPublisher::runThread(
        Publisher* pub,
        uint32_t samples,
        uint32_t sleep)
{

    for (uint32_t i = 0; i < samples; ++i)
    {
        if (!publish(pub))
        {
            --i;
        }
        else
        {
            std::cout << "Message with index: " << topic_.index() << " SENT by publisher " << pub->getGuid() <<
                    std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }

    std::cin.ignore();
}

bool LivelinessPublisher::publish(
        Publisher* pub,
        bool waitForListener)
{
    if (listener_.first_connected || !waitForListener || listener_.n_matched > 0)
    {
        topic_.index(topic_.index() + 1);
        pub->write((void*)&topic_);
        return true;
    }
    return false;
}
