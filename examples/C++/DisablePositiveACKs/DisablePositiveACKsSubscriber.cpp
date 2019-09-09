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
 * @file DisablePositiveACKsSubscriber.cpp
 *
 */

#include "DisablePositiveACKsSubscriber.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

DisablePositiveACKsSubscriber::DisablePositiveACKsSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
{
}

bool DisablePositiveACKsSubscriber::init(bool disable_positive_acks)
{
    ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_sub");
    participant_ = Domain::createParticipant(PParam);
    if( participant_ == nullptr )
    {
        return false;
    }

    Domain::registerType(participant_,&type_);

    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "Topic";
    Rparam.topic.topicName = "DisablePositiveACKsTopic";
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Rparam.qos.m_disablePositiveACKs.enabled = disable_positive_acks;
    subscriber_ = Domain::createSubscriber(participant_, Rparam, (SubscriberListener*) &listener);
    if( subscriber_ == nullptr )
    {
        return false;
    }

    return true;
}

DisablePositiveACKsSubscriber::~DisablePositiveACKsSubscriber()
{
    Domain::removeParticipant(participant_);
}

void DisablePositiveACKsSubscriber::SubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& matching_info)
{
    if(matching_info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Subscriber matched"<<std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Subscriber unmatched"<<std::endl;
    }
}

void DisablePositiveACKsSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
    if( sub->readNextData((void*) &hello, &info) )
    {
        if( info.sampleKind == ALIVE )
        {
            this->n_samples++;

            std::cout << "Message with index " << hello.index() << " RECEIVED" << std::endl;
        }
    }
}

void DisablePositiveACKsSubscriber::run(uint32_t number)
{
    std::cout << "Subscriber running until "<< number << " samples have been received"<<std::endl;
    while( number > this->listener.n_samples )
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
