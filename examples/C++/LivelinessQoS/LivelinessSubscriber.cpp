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
 * @file LivelinessSubscriber.cpp
 *
 */

#include "LivelinessSubscriber.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

LivelinessSubscriber::LivelinessSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
{
}

bool LivelinessSubscriber::init(
        LivelinessQosPolicyKind kind,
        int liveliness_ms)
{
    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SIMPLE;
    PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.use_WriterLivelinessProtocol = true;
    PParam.rtps.setName("Participant_sub");
    participant_ = Domain::createParticipant(PParam, &part_listener_);
    if (participant_ == nullptr)
    {
        return false;
    }
    Domain::registerType(participant_, &type_);

    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "Topic";
    Rparam.topic.topicName = "Name";
    Rparam.topic.historyQos.depth = 30;
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_liveliness.lease_duration = Duration_t(liveliness_ms * 1e-3);
    Rparam.qos.m_liveliness.announcement_period = Duration_t(liveliness_ms * 1e-3 * 0.5);
    Rparam.qos.m_liveliness.kind = kind;
    subscriber_ = Domain::createSubscriber(participant_, Rparam, &listener_);
    if (subscriber_ == nullptr)
    {
        return false;
    }
    return true;
}

LivelinessSubscriber::~LivelinessSubscriber()
{
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(participant_);
}

void LivelinessSubscriber::SubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Subscriber matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Subscriber unmatched" << std::endl;
    }
}

void LivelinessSubscriber::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    if (sub->takeNextData((void*)&topic, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            this->n_samples++;

            std::cout << "Message with index " << topic.index() << " RECEIVED" << std::endl;
        }
    }
}

void LivelinessSubscriber::SubListener::on_liveliness_changed(
        Subscriber* sub,
        const LivelinessChangedStatus& status)
{
    (void)sub;

    if (status.alive_count_change == 1)
    {
        std::cout << "Publisher " << status.last_publication_handle << " recovered liveliness" << std::endl;
    }
    else if (status.not_alive_count_change == 1)
    {
        std::cout << "Publisher " << status.last_publication_handle << " lost liveliness" << std::endl;
    }
}

void LivelinessSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}

void LivelinessSubscriber::run(
        uint32_t number)
{
    std::cout << "Subscriber running until " << number << "samples have been received" << std::endl;

    while (number > this->listener_.n_samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void LivelinessSubscriber::PartListener::onParticipantDiscovery(
        Participant* participant,
        rtps::ParticipantDiscoveryInfo&& info)
{
    (void)participant;

    if (info.status == rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Participant discovered" << std::endl;
    }
    else if (info.status == rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
    {
        std::cout << "Participant dropped" << std::endl;
    }
    else if (info.status == rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
    {
        std::cout << "Participant removed" << std::endl;
    }
}
