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
#include "fastrtps/participant/Participant.h"
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/attributes/SubscriberAttributes.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/Domain.h"
#include "fastrtps/utils/eClock.h"


HelloWorldSubscriber::HelloWorldSubscriber():mp_participant(nullptr),
mp_subscriber(nullptr)
{
}

bool HelloWorldSubscriber::init()
{
	ParticipantAttributes PParam;
	PParam.rtps.defaultSendPort = 10043;
	PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.domainId = 80;
	PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
	PParam.rtps.sendSocketBufferSize = 8712;
	PParam.rtps.listenSocketBufferSize = 17424;
	PParam.rtps.setName("Participant_sub");
	mp_participant = Domain::createParticipant(PParam);
	if(mp_participant==nullptr)
		return false;

	//REGISTER THE TYPE

	Domain::registerType(mp_participant,&m_type);
	//CREATE THE SUBSCRIBER
	SubscriberAttributes Rparam;
	Rparam.topic.topicKind = NO_KEY;
	Rparam.topic.topicDataType = "HelloWorldType";
	Rparam.topic.topicName = "HelloWorldTopic";
	Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Rparam.topic.historyQos.depth = 30;
	Rparam.topic.resourceLimitsQos.max_samples = 50;
	Rparam.topic.resourceLimitsQos.allocated_samples = 20;
	Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	mp_subscriber = Domain::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_listener);

	if(mp_subscriber == nullptr)
		return false;


	return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber() {
	// TODO Auto-generated destructor stub
	Domain::removeParticipant(mp_participant);
}

void HelloWorldSubscriber::SubListener::onSubscriptionMatched(Subscriber* sub,MatchingInfo& info)
{
	if(info.status == MATCHED_MATCHING)
	{
		n_matched++;
		cout << "Subscriber matched"<<endl;
	}
	else
	{
		n_matched--;
		cout << "Subscriber unmatched"<<endl;
	}
}

void HelloWorldSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
	if(sub->takeNextData((void*)&m_Hello, &m_info))
	{
		if(m_info.sampleKind == ALIVE)
		{
			this->n_samples++;
			// Print your structure data here.
			cout << "Message "<<m_Hello.message()<< " "<< m_Hello.index()<< " RECEIVED"<<endl;
		}
	}

}


void HelloWorldSubscriber::run()
{
	cout << "Subscriber running. Please press enter to stop the Subscriber" << endl;
	std::cin.ignore();
}

void HelloWorldSubscriber::run(uint32_t number)
{
	cout << "Subscriber running until "<< number << "samples have been received"<<endl;
	while(number < this->m_listener.n_samples)
		eClock::my_sleep(500);
}
