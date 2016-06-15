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

#include "fastrtps/rtps_all.h"

using namespace std;

#define WR 1 //Writer 1, Reader 2

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

struct SampleType{
	long sampleId;
	SampleType()
	{
		sampleId = 0;
	}
};

class SampleTypeDataType:public DDSTopicDataType
{
public:
	SampleTypeDataType()
{
		m_topicDataTypeName = "SampleType";
		m_typeSize = sizeof(long);
		m_isGetKeyDefined = false;
};
	~SampleTypeDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
	bool getKey(void*data,InstanceHandle_t* ihandle);
};

//Funciones de serializacion y deserializacion para el ejemplo
bool SampleTypeDataType::serialize(void*data,SerializedPayload_t* payload)
{
	SampleType* st = (SampleType*)data;
	payload->length = sizeof(long);
	payload->encapsulation = CDR_LE;
	memcpy(payload->data,&st->sampleId,payload->length);
	return true;
}

bool SampleTypeDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	SampleType* st = (SampleType*)data;
	memcpy(&st->sampleId,payload->data,payload->length);
	return true;
}

bool SampleTypeDataType::getKey(void*data,InstanceHandle_t* handle)
{
	return false;
}

boost::interprocess::interprocess_semaphore sema(0);


class MyPubListener:public PublisherListener
{
	void onPublicationMatched()
	{
		cout << RTPS_B_RED<<"PUBLICATION MATCHED"<<RTPS_DEF<<endl;
		sema.post();
	}
};

class MySubListener:public SubscriberListener
{
	void onSubscriptionMatched()
	{
		cout <<RTPS_B_RED<< "SUBSCRIPTION MATCHED "<<RTPS_DEF<<endl;
		sema.post();
	}
	void onNewDataMessage()
		{
			cout << RTPS_B_RED<<"New Message"<<RTPS_DEF<<endl;
		}
};


int main(int argc, char** argv)
{
	RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
	cout << "Starting "<< endl;
	pInfo("Starting"<<endl)
	int type = 1;
	if(argc > 1)
	{
		if(strcmp(argv[1],"publisher")==0)
			type = 1;
		else if(strcmp(argv[1],"subscriber")==0)
			type = 2;
	}


	SampleTypeDataType SampleTypeData;
	DomainRTPSParticipant::registerType((DDSTopicDataType*)&SampleTypeData);


	RTPSParticipantAttributes PParam;
	PParam.defaultSendPort = 10042;
	PParam.discovery.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.discovery.use_SIMPLE_EndpointDiscoveryProtocol = true;

	PParam.discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;


	switch(type)
	{
	case 1:
	{
		PParam.name = "RTPSParticipant1";
		//In this side we only have a Publisher so we don't need all discovery endpoints

		RTPSParticipant* p = DomainRTPSParticipant::createRTPSParticipant(PParam);
		PublisherAttributes Wparam;
		Wparam.topic.topicKind = NO_KEY;
		Wparam.topic.topicDataType = "SampleType";
		Wparam.topic.topicName = "Example SampleType";
		Wparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
		Wparam.topic.resourceLimitsQos.max_samples = 50;
		Wparam.topic.resourceLimitsQos.allocated_samples = 50;
		Wparam.times.heartbeatPeriod.seconds = 2;
		Wparam.times.heartbeatPeriod.fraction = 200*1000*1000;
		Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

		MyPubListener mylisten;
		Publisher* pub = DomainRTPSParticipant::createPublisher(p,Wparam,(PublisherListener*)&mylisten);
		if(pub == NULL)
			return 0;
		cout << "Waiting for discovery"<<endl;
		sema.wait();
		p->stopRTPSParticipantAnnouncement();

		//PREPARE DATA FOR TESTS
		SampleType ST;
		int n;
		cout << "Enter number to start: ";
		cin >> n;
		for(uint8_t i = 1;i<=10;i++)
		{
			ST.sampleId++;
//			if(i == 3 || i==5 ||i ==6)
//			{
//				//THIS METHOD SOULD BE USED WITH GREAT CARE. IT DOES NOT CHECK WHO IS SENDING THE NEXT PACKET
//				//DEPENDING ON THE TIMER PERIODS IT CAN PREVENT HB or ACKNACK packets from being sent
//				p->loose_next_change();
//			}
			pub->write((void*)&ST);
			cout << "Going to sleep "<< (int)i <<endl;
			eClock::my_sleep(1000);
			cout << "Wakes "<<endl;
		}
		//pub->dispose((void*)&ST);
//		eClock::my_sleep(1000);
//		cout << "Wakes "<<endl;
//		pub->unregister((void*)&ST);
//		pub->unregister((void*)&tp2);
//		eClock::my_sleep(1000);
//		cout << "Wakes "<<endl;
		break;
	}
	case 2:
	{
		PParam.name = "RTPSParticipant2";
		RTPSParticipant* p = DomainRTPSParticipant::createRTPSParticipant(PParam);
		SubscriberAttributes Rparam;
		Rparam.topic.topicDataType = "SampleType";
		Rparam.topic.topicName = "Example SampleType";
		Rparam.topic.topicKind = NO_KEY;
		Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
		Rparam.topic.historyQos.depth = 2;
		Rparam.topic.resourceLimitsQos.max_samples = 30;
		Rparam.topic.resourceLimitsQos.max_instances = 3;
		Rparam.topic.resourceLimitsQos.max_samples_per_instance = 3; //NOT USED IN KEEP_LAST
		Rparam.topic.resourceLimitsQos.allocated_samples = 30;
		Rparam.times.heartbeatResponseDelay.fraction = 200*1000*1000;
		Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

		MySubListener mylisten;
		//cout << "length: "<<Rparam.qos.m_presentation.length << endl;
		Subscriber* sub = DomainRTPSParticipant::createSubscriber(p,Rparam,(SubscriberListener*)&mylisten);
		cout << "Waiting for discovery"<<endl;
		sema.wait();
		//p->stopRTPSParticipantAnnouncement(); //Only for tests to see more clearly the communication
		int i = 0;
		while(i<30)
		{
			cout << "Waiting for new message "<<endl;
			sub->waitForUnreadMessage();
			SampleType ST;
			SampleInfo_t info;
			if(sub->readNextData((void*)&ST,&info))
				cout << "New sample: "<< ST.sampleId << endl;
			if(sub->getHistoryElementsNumber() >= 0.5*Rparam.topic.resourceLimitsQos.max_samples)
			{
				cout << "Taking all" <<endl;
				while(sub->takeNextData((void*)&ST,&info))
					cout << "Sample taken: "<< ST.sampleId << endl;
			}
			i++;
		}
		break;
	}

	}

	cout << "Enter numer to stop "<< endl;
	int n;
	cin >> n;
	DomainRTPSParticipant::stopAll();


	return 0;

}



