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
#include "fastrtps/participant/Participant.h"
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/Domain.h"
#include "fastrtps/utils/eClock.h"
#include "fastrtps/utils/TimeConversion.h"


HelloWorldPublisher::HelloWorldPublisher():mp_participant(nullptr),
mp_publisher(nullptr)
{


}

bool HelloWorldPublisher::init()
{
	m_Hello.index(0);
	m_Hello.message("HelloWorld");
	ParticipantAttributes PParam;
	PParam.rtps.defaultSendPort = 11511;
	PParam.rtps.use_IP6_to_send = true;
	PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.domainId = 80;
	PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
	PParam.rtps.sendSocketBufferSize = 8712;
	PParam.rtps.listenSocketBufferSize = 17424;
	PParam.rtps.setName("Participant_pub");
	mp_participant = Domain::createParticipant(PParam);

	if(mp_participant==nullptr)
		return false;
	//REGISTER THE TYPE

	Domain::registerType(mp_participant,&m_type);

	//CREATE THE PUBLISHER
	PublisherAttributes Wparam;
	Wparam.topic.topicKind = NO_KEY;
	Wparam.topic.topicDataType = "HelloWorldType";
	Wparam.topic.topicName = "HelloWorldTopic";
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 30;
	Wparam.topic.resourceLimitsQos.max_samples = 50;
	Wparam.topic.resourceLimitsQos.allocated_samples = 20;
	Wparam.times.heartbeatPeriod.seconds = 2;
	Wparam.times.heartbeatPeriod.fraction = 200*1000*1000;
	Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	Wparam.qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
	Wparam.qos.m_liveliness.lease_duration = TimeConv::MilliSeconds2Time_t(5001);
	Wparam.qos.m_liveliness.announcement_period = TimeConv::MilliSeconds2Time_t(5000);

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

void HelloWorldPublisher::PubListener::onPublicationMatched(Publisher* pub,MatchingInfo& info)
{
	if(info.status == MATCHED_MATCHING)
	{
		n_matched++;
		cout << "Publisher matched"<<endl;
	}
	else
	{
		n_matched--;
		cout << "Publisher unmatched"<<endl;
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
			cout << "Message: "<<m_Hello.message()<< " with index: "<< m_Hello.index()<< " SENT"<<endl;
			std::cin.ignore();
		}
		eClock::my_sleep(25);
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

