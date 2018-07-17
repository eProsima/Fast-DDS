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
 * @file HelloWorldSubscriber.cpp
 *
 */

#include "HelloWorldSubscriber.h"
#include "HelloWorldTypeObject.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
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

HelloWorldSubscriber::HelloWorldSubscriber()
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
    , m_DynType(nullptr)
{
}

bool HelloWorldSubscriber::init(bool dynamic)
{
    ParticipantAttributes PParam;
    PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.domainId = 5;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("DynHelloWorld_sub");
    mp_participant = Domain::createParticipant(PParam, (ParticipantListener*)&m_part_list);
    if(mp_participant==nullptr)
        return false;

    //REGISTER THE TYPE
    m_dynamic = dynamic;
    m_listener.m_dynamic = dynamic;
    if (dynamic)
    {
        // Given
        DynamicTypeBuilder* created_type_ulong(nullptr);
        DynamicTypeBuilder* created_type_string(nullptr);
        DynamicTypeBuilder* struct_type_builder(nullptr);
        // Create basic types
        created_type_ulong = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Type();
        //created_type_ulong = DynamicTypeBuilderFactory::GetInstance()->CreateInt32Type();
        created_type_string = DynamicTypeBuilderFactory::GetInstance()->CreateStringType();
        struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructType();

        // Add members to the struct.
        struct_type_builder->AddMember(0, "index", created_type_ulong);
        struct_type_builder->AddMember(1, "message", created_type_string);
        struct_type_builder->SetName("HelloWorld");

        m_DynType = struct_type_builder->Build();
        m_listener.m_DynHello = DynamicDataFactory::GetInstance()->CreateData(m_DynType);

        DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type_ulong);
        DynamicTypeBuilderFactory::GetInstance()->DeleteType(created_type_string);
        DynamicTypeBuilderFactory::GetInstance()->DeleteType(struct_type_builder);

        Domain::registerDynamicType(mp_participant, m_DynType);
    }
    else
    {
        m_listener.m_Hello = new HelloWorld;
        Domain::registerType(mp_participant,&m_type);
    }

    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "HelloWorld";
    Rparam.topic.topicName = "DynamicHelloWorldTopic";
    Rparam.topic.topicDiscoveryKind = MINIMAL;
    //Rparam.topic.topicDiscoveryKind = NO_CHECK;
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.topic.historyQos.depth = 30;
    Rparam.topic.resourceLimitsQos.max_samples = 50;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    mp_subscriber = Domain::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_listener);

    if(mp_subscriber == nullptr)
        return false;


    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber() {
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(mp_participant);

    if (m_dynamic)
    {
        if (m_DynType != nullptr)
        {
            DynamicTypeBuilderFactory::GetInstance()->DeleteType(m_DynType);
        }
        DynamicDataFactory::GetInstance()->DeleteData(m_listener.m_DynHello);
    }

    Domain::stopAll();
}

void HelloWorldSubscriber::SubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
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

void HelloWorldSubscriber::PartListener::onParticipantDiscovery(Participant*, ParticipantDiscoveryInfo info)
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

void HelloWorldSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
    if (m_dynamic)
    {
        if(sub->takeNextData((void*)m_DynHello, &m_info))
        {
            if(m_info.sampleKind == ALIVE)
            {
                this->n_samples++;
                // Print your structure data here.
                std::string message;
                m_DynHello->GetStringValue(message, 1);
                uint32_t index;
                m_DynHello->GetUint32Value(index, 0);

                std::cout << "Message: "<<message<< " with index: "<<index<< " RECEIVED"<<std::endl;
            }
        }
    }
    else
    {
        if(sub->takeNextData((void*)m_Hello, &m_info))
        {
            if(m_info.sampleKind == ALIVE)
            {
                this->n_samples++;
                // Print your structure data here.
                std::cout << "Message "<<m_Hello->message()<< " "<< m_Hello->index()<< " RECEIVED"<<std::endl;
            }
        }
    }
}


void HelloWorldSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}

void HelloWorldSubscriber::run(uint32_t number)
{
    std::cout << "Subscriber running until "<< number << "samples have been received"<<std::endl;
    while(number > this->m_listener.n_samples)
        eClock::my_sleep(500);
}
