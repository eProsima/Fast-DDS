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
 * @file LifespanSubscriber.cpp
 *
 */

#include "LifespanSubscriber.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

LifespanSubscriber::LifespanSubscriber()
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
{
}

bool LifespanSubscriber::init(uint32_t lifespan_ms)
{
    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = 0;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_sub");
    mp_participant = Domain::createParticipant(PParam);
    if( mp_participant == nullptr )
    {
        return false;
    }

    Domain::registerType(mp_participant,&m_type);

    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "Lifespan";
    Rparam.topic.topicName = "LifespanTopic";
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.topic.historyQos.depth = 30;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Rparam.qos.m_lifespan.duration = lifespan_ms * 1e-3;
    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*) &m_listener);
    if( mp_subscriber == nullptr )
    {
        return false;
    }

    return true;
}

LifespanSubscriber::~LifespanSubscriber()
{
    Domain::removeParticipant(mp_participant);
}

void LifespanSubscriber::SubListener::onSubscriptionMatched(Subscriber* /*sub*/, MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
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

void LifespanSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
    if( sub->readNextData((void*) &m_Hello, &m_info) )
    {
        if( m_info.sampleKind == ALIVE )
        {
            this->n_samples++;

            std::cout << "Message " << m_Hello.message() << " " << m_Hello.index() << " RECEIVED" << std::endl;
        }
    }
}

void LifespanSubscriber::run(uint32_t number, uint32_t sleep_ms)
{
    std::cout << "Subscriber running until "<< number << " samples have been received"<<std::endl;
    while( number > this->m_listener.n_samples )
    {
        eClock::my_sleep(500);
    }

    // Now wait and try to remove from history
    std::cout << std::endl << "Subscriber waiting for " << sleep_ms << " milliseconds" << std::endl << std::endl;
    eClock::my_sleep(sleep_ms);

    LifespanType::type data;
    SampleInfo_t info;

    for( uint32_t i = 0; i < m_listener.n_samples; i++ )
    {
        if( mp_subscriber->takeNextData((void*) &data, &info) )
        {
            std::cout << "Message " << data.message() << " " << data.index() << " read from history" << std::endl;
        }
        else
        {
            std::cout << "Could not read message " << i << " from history" << std::endl;
        }
    }
}
