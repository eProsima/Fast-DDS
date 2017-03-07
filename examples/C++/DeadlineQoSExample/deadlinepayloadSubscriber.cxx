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

#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastrtps/Domain.h>


#include "deadlinepayloadSubscriber.h"

deadlinepayloadSubscriber::deadlinepayloadSubscriber(asio::steady_timer &timer,asio::io_service &ioserv) : mp_participant(nullptr), mp_subscriber(nullptr), m_listener(timer,ioserv) {}

deadlinepayloadSubscriber::~deadlinepayloadSubscriber() {	Domain::removeParticipant(mp_participant);}

bool deadlinepayloadSubscriber::init()
{
    // Create RTPSParticipant

    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = 0; //MUST BE THE SAME AS IN THE PUBLISHER
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_subscriber"); //You can put the name you want
    mp_participant = Domain::createParticipant(PParam);
    if(mp_participant == nullptr)
        return false;

    //Register the type

    Domain::registerType(mp_participant,(TopicDataType*) &myType);		

    // Create Subscriber

    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = WITH_KEY;
    Rparam.topic.topicDataType = myType.getName(); //Must be registered before the creation of the subscriber
    Rparam.topic.topicName = "deadlinepayloadPubSubTopic";
    Rparam.topic.resourceLimitsQos.max_instances=32;
    Rparam.topic.resourceLimitsQos.max_samples_per_instance=5;
    Rparam.topic.resourceLimitsQos.max_samples = 32*5;
    Rparam.qos.m_reliability.kind= RELIABLE_RELIABILITY_QOS;
    Rparam.topic.historyQos.depth = 5;
    mp_subscriber = Domain::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_listener);
    if(mp_subscriber == nullptr)
        return false;

    return true;
}

void deadlinepayloadSubscriber::SubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
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

void deadlinepayloadSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
    // Take data
    HelloMsg st;
    //InstanceHandle_t myHandle;
    mapable_key handle;
    if(sub->takeNextData(&st, &m_info))
    {
        if(m_info.sampleKind == ALIVE)
        {
            for(int i=0;i<16;i++) handle.value[i] = m_info.iHandle.value[i];
            myDeadline.setFlag(handle);
        }
    }
}

void deadlinepayloadSubscriber::run()
{

    m_listener.myDeadline.run();
    std::cout << "Waiting for Data, press Enter to stop the Subscriber. "<<std::endl;
    std::cout << "------------------------------------------------------"<<std::endl;
    std::cin.ignore();
    std::cout << "Shutting down the Subscriber." << std::endl;
    m_listener.myDeadline.stop();
}

