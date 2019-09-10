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
 * @file DisablePositiveACKsPublisher.cpp
 *
 */

#include "DisablePositiveACKsPublisher.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>

#include <thread>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

DisablePositiveACKsPublisher::DisablePositiveACKsPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
{
}

bool DisablePositiveACKsPublisher::init(
        bool disable_positive_acks,
        uint32_t keep_duration_ms)
{
    hello_.index(0);
    hello_.message("DisablePositiveACKs");

    ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_pub");
    participant_ = Domain::createParticipant(PParam);
    if( participant_ == nullptr )
    {
        return false;
    }

    Domain::registerType(participant_,&type_);

    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "Topic";
    Wparam.topic.topicName = "DisablePositiveACKsTopic";
    Wparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Wparam.qos.m_disablePositiveACKs.enabled = disable_positive_acks;
    Wparam.qos.m_disablePositiveACKs.duration = Duration_t(keep_duration_ms * 1e-3);
    publisher_ = Domain::createPublisher(participant_, Wparam, (PublisherListener*) &listener);
    if( publisher_ == nullptr )
    {
        return false;
    }

    return true;
}

DisablePositiveACKsPublisher::~DisablePositiveACKsPublisher()
{
    Domain::removeParticipant(participant_);
}

void DisablePositiveACKsPublisher::PubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Publisher matched"<<std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Publisher unmatched"<<std::endl;
    }
}

void DisablePositiveACKsPublisher::run(
        uint32_t samples,
        uint32_t write_sleep_ms)
{
    std::cout << "Publisher running" << std::endl;

    samples = ( samples == 0 ) ? 10 : samples;
    for( uint32_t i = 0; i < samples; ++i )
    {
        if( !publish() )
        {
            --i;
        }
        else
        {
            std::cout << "Message with index: " << hello_.index() << " SENT" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(write_sleep_ms));
    }

    std::cout << "Please press enter to stop the Publisher" << std::endl;
    std::cin.ignore();
}

bool DisablePositiveACKsPublisher::publish(bool waitForListener)
{
    if(!waitForListener || listener.n_matched>0)
    {
        hello_.index(hello_.index()+1);
        publisher_->write((void*)&hello_);
        return true;
    }
    return false;
}
