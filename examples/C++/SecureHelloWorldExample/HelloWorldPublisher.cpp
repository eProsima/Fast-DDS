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
 * @file HelloWorldPublisher.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps;

HelloWorldPublisher::HelloWorldPublisher():mp_participant(nullptr),
mp_publisher(nullptr)
{


}

bool HelloWorldPublisher::init()
{
    m_Hello.index(0);
    m_Hello.message("HelloWorld");
    ParticipantAttributes PParam;

    PropertyPolicy participant_property_policy;
    participant_property_policy.properties().emplace_back("dds.sec.auth.plugin",
            "builtin.PKI-DH");
    participant_property_policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://certs/maincacert.pem");
    participant_property_policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://certs/mainpubcert.pem");
    participant_property_policy.properties().emplace_back("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://certs/mainpubkey.pem");
    participant_property_policy.properties().emplace_back(Property("dds.sec.access.plugin",
                    "builtin.Access-Permissions"));
    participant_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions_ca",
                    "file://certs/maincacert.pem"));
    participant_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
                    "file://certs/governance.smime"));
    participant_property_policy.properties().emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
                    "file://certs/permissions.smime"));
    participant_property_policy.properties().emplace_back("dds.sec.crypto.plugin",
            "builtin.AES-GCM-GMAC");
    PParam.rtps.properties = participant_property_policy;

    mp_participant = Domain::createParticipant(PParam);

    if(mp_participant==nullptr)
        return false;
    //REGISTER THE TYPE

    Domain::registerType(mp_participant,&m_type);

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "HelloWorld";
    Wparam.topic.topicName = "HelloWorldTopic";
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.nanosec = 200*1000*1000;
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    mp_publisher = Domain::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_listener);
    if(mp_publisher == nullptr)
        return false;

    return true;

}

HelloWorldPublisher::~HelloWorldPublisher()
{
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(mp_participant);
}

void HelloWorldPublisher::PubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
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

void HelloWorldPublisher::run(uint32_t samples)
{
    for(uint32_t i = 0;i<samples;++i)
    {
        if(!publish())
            --i;
        else
        {
            std::cout << "Message: "<<m_Hello.message()<< " with index: "<< m_Hello.index()<< " SENT"<<std::endl;
        }
    }
}

bool HelloWorldPublisher::publish()
{
    if(m_listener.n_matched>0)
    {
        m_Hello.index(m_Hello.index()+1);
        mp_publisher->write((void*)&m_Hello);
        return true;
    }
    return false;
}
