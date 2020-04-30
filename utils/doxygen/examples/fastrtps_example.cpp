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

/*
 * dds_example.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com
 */

using std::cout;
using std::endl;

using namespace eprosima::pubsub;
using namespace eprosima::rtps;

//! [ex_readAllUnreadCache]
using namespace eprosima::pubsub;
using namespace eprosima::rtps;
std::vector<(void*)(TypeStructure*)> vec; //TypeStructure is your own define structure for the topic
readAllUnreadCache(&vec);
for (elements in vec)
	TypeStructure tp = *(TypeStructure*)vec[i];
	//Do something with tp.
//! [ex_readAllUnreadCache]

//! [ex_readMinSeqUnread]
TypeStructure tp; //TypeStructure is your own define structure for the topic
readMinSeqUnread((void*)&tp);
//! [ex_readMinSeqUnread]


//! [ex_PublisherWrite]
TypeStructure tp; //TypeStructure is your own define structure for the topic
//Fill tp with the data you want.
write((void*)&tp);
//! [ex_PublisherWrite]


//! [ex_TopicDataType]
using namespace eprosima::pubsub;
using namespace eprosima::rtps;
typedef struct TestType{
	char name[6]; //KEY
	int32_t value;
	double price;
	TestType()
	{
		value = -1;
		price = 0;
		strcpy(name,"UNDEF");
	}
	void print()
	{
		cout << "Name: ";
		printf("%s",name);
		cout << " |Value: "<< value;
		cout << " |Price: "<< price;
		cout << endl;
	}
}TestType;

class TestTypeDataType:public TopicDataType
{
public:
	TestTypeDataType()
{
		setName("TestType");
		m_typeSize = 6+4+sizeof(double); //This is the maximum size of this type.
		m_isGetKeyDefined = true;
};
	~TestTypeDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
	bool getKey(void*data,InstanceHandle_t* ihandle);
};

//Example Serialization method. The user should ensure that the serialization
//and deserialization methods work always
bool TestTypeDataType::serialize(void*data,SerializedPayload_t* payload)
{
	payload->length = sizeof(TestType);
	payload->encapsulation = CDR_LE;
	memcpy(payload->data,data,payload->length);
	return true;
}

bool TestTypeDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	memcpy(data,payload->data,payload->length);
	return true;
}

//Before using it an object of this type must be defined and registered in the domain RTPSParticipant.
//Different objects with different names should be used if the Publisher/Subscriber are defined in different threads.
//Thread safety would be considered for future releases.
TestTypeDataType TestTypeData;
Participant* part; //CREATED SOMEWHERE ELSE
Domain::registerType(part,(TopicDataType*)&TestTypeData);
//! [ex_TopicDataType]

//! [ex_Publisher]
using namespace eprosima::pubsub;
using namespace eprosima::rtps;
PublisherAttributes PParam;
PParam.historyMaxSize = 20;
PParam.topic.topicKind = WITH_KEY;
PParam.topic.topicDataType = "TestType";
PParam.topic.topicName = "Test_topic";
Publisher* pub1 = DomainRTPSParticipant::createPublisher(p,PParam);

Locator_t loc;
loc.kind = 1;
loc.port = 10469;
loc.set_IP4_address(192,168,1,16);
pub1->addReaderLocator(loc,true);
TestType tp1;
pub1->write((void*)&tp1);
//! [ex_Publisher]

//! [ex_Subscriber]
using namespace eprosima::pubsub;
using namespace eprosima::rtps;
SubscriberAttributes Rparam;
Rparam.historyMaxSize = 15;
Rparam.topic.topicDataType = std::string("TestType");
Rparam.topic.topicName = std::string("Test_Topic");
Rparam.reliability.reliabilityKind = RELIABLE;
Locator_t loc;
loc.port = 10046;
Rparam.unicastLocatorList.push_back(loc); //Listen in the same port
Subscriber* sub = DomainRTPSParticipant::createSubscriber(p,Rparam);
//! [ex_Subscriber]

//! [ex_SubscriberListener]
using namespace eprosima::pubsub;
using namespace eprosima::rtps;
//Create a class that inherits from SubscriberListener and implement the methods you need.
class TestTypeListener: public SubscriberListener{
public:
	TestTypeListener(){};
	~TestTypeListener(){};
	void onNewDataMessage()
	{
		cout <<"New Message"<<endl;
	}
	void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info)
	{
		if(info.status == MATCHED_MATCHING)
			cout << "Discovery"<<endl;
		else if(info.status == REMOVED_MATCHING)
			cout << "Publisher removed"<<endl;
	}
};

//Somewhere in the code, create an object an register assign it to the subscriber.
TestTypeListener listener;
Subscriber* sub = DomainRTPSParticipant::createSubscriber(p,Rparam,(SubscriberListener*)&listener);
//...

//You can also create it and assign it later, although this is not recommended since the onSubscriptionMatched may not be called
// (if the discovery is performed before you assign the Listener.).
TestTypeListener listener2;
sub->assignListener((SubscriberListener*)&listener2);
//! [ex_SubscriberListener]

//! [ex_PublisherListener]
using namespace eprosima::pubsub;
using namespace eprosima::rtps;
//Create a class that inherits from PublisherListener and implement the methods you need.
class TestTypeListener: public PublisherListener
{
public:
	RTPSParticipant* p;
	RTPSParticipantAttributes Pparam;
	eprosima::dds::Publisher* pub;
	PublisherAttributes Pubparam;
	TestTypeListener()
	{
		//The RTPSParticipant should have been created and accessible to this method.
		p = DomainRTPSParticipant::createRTPSParticipant(Pparam);
		//PublisherAttributes must be set to the user preferences.
		pub = DomainRTPSParticipant::createPublisher(p,Pubparam,(PublisherListener*)this);
	};
	~TestTypeListener(){};
	void onHistoryFull()
	{
		pub->removeMinSeqCache();
	}
	void onPublicationMatched(Publisher* pub,MatchingInfo& info)
	{
		if(info.status == MATCHED_MATCHING)
		{
			cout << "Discovery!"<<endl;
		}
		else if(info.status == REMOVED_MATCHING)
		{
			cout << "Subscription removed"<<endl;
		}
	}

};
//! [ex_PublisherListener]

// //! [ex_ RTPSParticipantCreation]
//using namespace eprosima::rtps;
//using namespace eprosima::pubsub;
//RTPSParticipantAttributes PParam;
//PParam.setName("RTPSParticipant");
//PParam.defaultSendPort = 10042;
//PParam.domainId = 50;
//PParam.discovery.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
//PParam.discovery.resendSPDPDataPeriod_sec = 30;
//PParam.discovery.use_STATIC_EndpointDiscoveryProtocol = true;
//PParam.discovery.m_staticEndpointXMLFilename = "StaticEndpointDefinition.xml";
//Locator_t loc;
//loc.kind = 1; loc.port = 10046; loc.set_IP4_address(192,168,1,16);
//PParam.defaultUnicastLocatorList.push_back(loc);
//RTPSParticipant* p = RTPSDomain::createRTPSParticipant(PParam);
//if(p!=nullptr)
//{
//	//RTPSParticipant correctly created
//}
// //! [ex_ RTPSParticipantCreation]


//! [ex_RTPSParticipantCreation]
class MyListener : public RTPSParticipantListener { ... };
MyListener listen;
ParticipantAttributes patt;
RTPSParticipant* p = RTPSDomain::createRTPSParticipant(patt,(RTPSParticipantListener*)&listen);
//! [ex_RTPSParticipantCreation]
