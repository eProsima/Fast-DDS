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



#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <sstream>



#include "fastrtps/rtps_all.h"


#include "LatencyTestTypes.h"

using namespace eprosima;
using namespace dds;
using namespace rtps;
using namespace std;


#if defined(__LITTLE_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif

#if defined(_WIN32)
#define COPYSTR strcpy_s
#else
#define COPYSTR strcpy
#endif

class PubListener : public PublisherListener
{
public:
	PubListener(){};
	~PubListener(){};
	void onPublicationMatched(MatchingInfo info)
	{
		if(info.status == MATCHED_MATCHING)
			cout <<RTPS_BLUE<< "ADDED Publisher: "<< info.remoteEndpointGuid << RTPS_DEF<<endl;
		else
			cout <<RTPS_RED<< "REMOVED Publisher: "<< info.remoteEndpointGuid << RTPS_DEF<<endl;
	}
};

class SubListener: public SubscriberListener
{
public:
	SubListener(){};
	~SubListener(){};
	void onSubscriptionMatched(MatchingInfo info)
	{
		if(info.status == MATCHED_MATCHING)
			cout <<RTPS_BLUE<< "ADDED Subscriber: "<< info.remoteEndpointGuid << RTPS_DEF<<endl;
		else
			cout <<RTPS_RED<< "REMOVED Subscriber: "<< info.remoteEndpointGuid << RTPS_DEF<<endl;
	}
};


int main(int argc, char** argv){
	RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);

	LatencyDataType latency_t;
	DomainRTPSParticipant::registerType((DDSTopicDataType*)&latency_t);

	TestCommandDataType command_t;
	DomainRTPSParticipant::registerType((DDSTopicDataType*)&command_t);

	//Create RTPSParticipant
	RTPSParticipantAttributes PParam;
	PParam.builtin.domainId = 80;
	PParam.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	TIME_INFINITE(PParam.builtin.leaseDuration);
	PParam.name = "RTPSParticipant_pub";
	RTPSParticipant* part = DomainRTPSParticipant::createRTPSParticipant(PParam);
	//Create Publisher
	PublisherAttributes PubDataparam;
	PubDataparam.topic.topicDataType = "LatencyType";
	PubDataparam.topic.topicKind = NO_KEY;
	PubDataparam.topic.topicName = "Topic1";
	PubListener publistener;
	Publisher* pub = DomainRTPSParticipant::createPublisher(part,PubDataparam,(PublisherListener*)&publistener);

	SubscriberAttributes SubDataparam;
	Locator_t loc;
	loc.port = 7555;
	PubDataparam.unicastLocatorList.push_back(loc);
	SubDataparam.topic.topicDataType = "LatencyType";
	SubDataparam.topic.topicKind = NO_KEY;
	SubDataparam.topic.topicName = "Topic1";
	SubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	SubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	SubListener sublistener;
	Subscriber* sub = DomainRTPSParticipant::createSubscriber(part,SubDataparam,&sublistener);

	//eClock::my_sleep(500);
	//cout << "Getting Publisher Attributes "<<endl;

	PublisherAttributes patt = pub->getAttributes();
	//cout << "topic: "<< patt.topic.topicName << " "<< patt.topic.topicDataType << endl;

	//cout << "Getting Subscriber Attributes "<<endl;
	SubscriberAttributes satt = sub->getAttributes();
	//cout << "topic: "<< satt.topic.topicName << " "<< satt.topic.topicDataType << endl;

	//cout << "Changing Pub Attributes: "<<endl;
	patt.qos.m_partition.names.push_back("A");
	pub->updateAttributes(patt);
	//eClock::my_sleep(1000);

	satt.qos.m_partition.names.push_back("A");
	sub->updateAttributes(satt);
	satt.unicastLocatorList.push_back(Locator_t(6000));
	satt.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	satt.qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS;
	sub->updateAttributes(satt);

	eClock::my_sleep(2000);
	DomainRTPSParticipant::stopAll();
	cout << "EVERYTHING STOPPED FINE"<<endl;

	return 0;
}



