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
 * @file LifespanPublisher.cpp
 *
 */

#include "LifespanPublisher.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

#include <thread>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

LifespanPublisher::LifespanPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
{
}

bool LifespanPublisher::init(uint32_t lifespan_ms)
{
    m_Hello.index(0);
    m_Hello.message("Lifespan");

    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = 0;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_pub");
    mp_participant = Domain::createParticipant(PParam);
    if( mp_participant == nullptr )
    {
        return false;
    }

    Domain::registerType(mp_participant,&m_type);

    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "Lifespan";
    Wparam.topic.topicName = "LifespanTopic";
    Wparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Wparam.qos.m_lifespan.duration = lifespan_ms * 1e-3;
    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*) &m_listener);
    if( mp_publisher == nullptr )
    {
        return false;
    }

    return true;
}

LifespanPublisher::~LifespanPublisher()
{
    Domain::removeParticipant(mp_participant);
}

void LifespanPublisher::PubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        n_matched++;
        firstConnected = true;
        std::cout << "Publisher matched"<<std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Publisher unmatched"<<std::endl;
    }
}

void LifespanPublisher::run(uint32_t samples, uint32_t write_sleep_ms, uint32_t sleep_ms)
{
    samples = ( samples == 0 ) ? 10 : samples;
    for( uint32_t i = 0; i < samples; ++i )
    {
        if( !publish() )
        {
            --i;
        }
        else
        {
            std::cout << "Message with index: " << m_Hello.index() << " SENT" << std::endl;
        }
        eClock::my_sleep(write_sleep_ms);
    }

    // Wait and try to clear history
    std::cout << std::endl << "Publisher waiting for " << sleep_ms << " milliseconds" << std::endl;
    eClock::my_sleep(sleep_ms);

    size_t removed = 0;
    mp_publisher->removeAllChange(&removed);

    std::cout << "Publisher removed " << removed << " samples from its history" << std::endl;

    std::cout << "Please press enter to stop the Publisher" << std::endl;
    std::cin.ignore();
}

bool LifespanPublisher::publish(bool waitForListener)
{
    if(m_listener.firstConnected || !waitForListener || m_listener.n_matched>0)
    {
        m_Hello.index(m_Hello.index()+1);
        mp_publisher->write((void*)&m_Hello);
        return true;
    }
    return false;
}
