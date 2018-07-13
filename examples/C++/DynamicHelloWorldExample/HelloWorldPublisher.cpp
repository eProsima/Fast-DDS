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
#include "HelloWorldTypeObject.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicType.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

HelloWorldPublisher::HelloWorldPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , m_DynType(nullptr)
    , m_dynamic(false)
{
}

bool HelloWorldPublisher::init(bool dynamic)
{
    m_dynamic = dynamic;
    if (dynamic)
    {
        // Given
        DynamicTypeBuilder* created_type_ulong(nullptr);
        DynamicTypeBuilder* created_type_string(nullptr);
        DynamicTypeBuilder* struct_type_builder(nullptr);
        // Create basic types
        created_type_ulong = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Type();
        created_type_string = DynamicTypeBuilderFactory::GetInstance()->CreateStringType();
        struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructType();

        // Add members to the struct.
        struct_type_builder->AddMember(0, "index", created_type_ulong);
        struct_type_builder->AddMember(1, "message", created_type_string);
        struct_type_builder->SetName("HelloWorld");

        m_DynType = struct_type_builder->Build();
        m_DynHello = DynamicDataFactory::GetInstance()->CreateData(m_DynType);
        m_DynHello->SetUint32Value(0, 0);
        m_DynHello->SetStringValue("HelloWorld", 1);

        DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type_ulong);
        DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type_string);
        DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type_builder);
    }
    else
    {
        m_Hello.index(0);
        m_Hello.message("HelloWorld");
    }

    ParticipantAttributes PParam;
    PParam.rtps.use_IP6_to_send = true;
    PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.domainId = 5;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("DynHelloWorld_pub");
    mp_participant = Domain::createParticipant(PParam, (ParticipantListener*)&m_part_list);

    if(mp_participant==nullptr)
        return false;
    //REGISTER THE TYPE

    if (m_dynamic)
    {
        Domain::registerType(mp_participant, m_DynType);
    }
    else
    {
        Domain::registerType(mp_participant, &m_type);
    }

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "HelloWorld";
    Wparam.topic.topicName = "DynamicHelloWorldTopic";
    Wparam.topic.topicDiscoveryKind = MINIMAL;
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.fraction = 200*1000*1000;
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    mp_publisher = Domain::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_listener);
    if(mp_publisher == nullptr)
        return false;

    return true;

}

HelloWorldPublisher::~HelloWorldPublisher()
{
    Domain::removeParticipant(mp_participant);

    // TODO Auto-generated destructor stub
    if (m_dynamic)
    {
        DynamicTypeBuilderFactory::GetInstance()->DeleteType(m_DynType);
        DynamicDataFactory::GetInstance()->DeleteData(m_DynHello);
    }
}

void HelloWorldPublisher::PubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
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

void HelloWorldPublisher::PartListener::onParticipantDiscovery(Participant*, ParticipantDiscoveryInfo info)
{
    if (info.rtps.m_status == DISCOVERED_RTPSPARTICIPANT)
    {
        std::cout << "Participant " << info.rtps.m_RTPSParticipantName << " discovered" << std::endl;
    }
    else if (info.rtps.m_status == REMOVED_RTPSPARTICIPANT)
    {
        std::cout << "Participant " << info.rtps.m_RTPSParticipantName << " removed" << std::endl;
    }
    else if (info.rtps.m_status == DROPPED_RTPSPARTICIPANT)
    {
        std::cout << "Participant " << info.rtps.m_RTPSParticipantName << " dropped" << std::endl;
    }
}

void HelloWorldPublisher::runThread(uint32_t samples, uint32_t sleep)
{
    if (samples == 0)
    {
        while(!stop)
        {
            if(publish(false))
            {
                if (m_dynamic)
                {
                    std::string message;
                    m_DynHello->GetStringValue(message, 1);
                    uint32_t index;
                    m_DynHello->GetUint32Value(index, 0);

                    std::cout << "Message: "<<message<< " with index: "<<index<< " SENT"<<std::endl;
                }
                else
                {
                    std::cout << "Message: "<<m_Hello.message()<< " with index: "<< m_Hello.index()<< " SENT"<<std::endl;
                }
            }
            eClock::my_sleep(sleep);
        }
    }
    else
    {
        for(uint32_t i = 0;i<samples;++i)
        {
            if(!publish())
                --i;
            else
            {
                if (m_dynamic)
                {
                    std::string message;
                    m_DynHello->GetStringValue(message, 1);
                    uint32_t index;
                    m_DynHello->GetUint32Value(index, 0);

                    std::cout << "Message: "<<message<< " with index: "<<index<< " SENT"<<std::endl;
                }
                else
                {
                    std::cout << "Message: "<<m_Hello.message()<< " with index: "<< m_Hello.index()<< " SENT"<<std::endl;
                }
            }
            eClock::my_sleep(sleep);
        }
    }
}

void HelloWorldPublisher::run(uint32_t samples, uint32_t sleep)
{
    stop = false;
    std::thread thread(&HelloWorldPublisher::runThread, this, samples, sleep);
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
        std::cin.ignore();
        stop = true;
    }
    else
    {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    thread.join();
}

bool HelloWorldPublisher::publish(bool waitForListener)
{
    if(m_listener.firstConnected || !waitForListener || m_listener.n_matched>0)
    {
        if (m_dynamic)
        {
            uint32_t index;
            m_DynHello->GetUint32Value(index, 0);
            m_DynHello->SetUint32Value(index+1, 0);
            mp_publisher->write((void*)m_DynHello);
        }
        else
        {
            m_Hello.index(m_Hello.index()+1);
            mp_publisher->write((void*)&m_Hello);
        }
        return true;
    }
    return false;
}
