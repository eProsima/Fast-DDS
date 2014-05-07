/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of eProsimaRTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * dds_example.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com
 */

//! [ex_readAllUnreadCache]
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


//! [ex_DDSTopicDataType]
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

class TestTypeDataType:public DDSTopicDataType
{
public:
	TestTypeDataType()
{
		m_topicDataTypeName = "TestType";
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
	if(payload->data !=NULL)
		free(payload->data);
	payload->data = (octet*)malloc(payload->length);
	memcpy(payload->data,data,payload->length);
	return true;
}

bool TestTypeDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	memcpy(data,payload->data,payload->length);
	return true;
}

//Before using it an object of this type must be defined and registered in the domain participant.
//Different objects with different names should be used if the Publisher/Subscriber are defined in different threads.
//Thread safety would be considered for future releases.
TestTypeDataType TestTypeData;
DomainParticipant::registerType((DDSTopicDataType*)&TestTypeData);

//! [ex_DDSTopicDataType]

//! [ex_Publisher]
PublisherAttributes PParam;
PParam.historyMaxSize = 20;
PParam.topic.topicKind = WITH_KEY;
PParam.topic.topicDataType = "TestType";
PParam.topic.topicName = "Test_topic";
Publisher* pub1 = DomainParticipant::createPublisher(p,PParam);

Locator_t loc;
loc.kind = 1;
loc.port = 10469;
loc.set_IP4_address(192,168,1,16);
pub1->addReaderLocator(loc,true);
TestType tp1;
pub1->write((void*)&tp1);
//! [ex_Publisher]

//! [ex_Subscriber]
SubscriberAttributes Rparam;
Rparam.historyMaxSize = 15;
Rparam.topic.topicDataType = std::string("TestType");
Rparam.topic.topicName = std::string("Test_Topic");
Rparam.reliability.reliabilityKind = RELIABLE;
Locator_t loc;
loc.port = 10046;
Rparam.unicastLocatorList.push_back(loc); //Listen in the same port
Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);
//! [ex_Subscriber]

//! [ex_SubscriberListener]
//Create a class that inherits from SubscriberListener and implement the methods you need.
class TestTypeListener: public SubscriberListener{
public:
	TestTypeListener(){};
	~TestTypeListener(){};
	void onNewDataMessage()
	{
		cout <<"New Message"<<endl;
	}
};

//Somewhere in the code, create an object an register assign it to the subscriber.
Subscriber* sub = DomainParticipant::createSubscriber(p,Rparam);
TestTypeListener listener;
sub->assignListener((SubscriberListener*)&listener);
//! [ex_SubscriberListener]

//! [ex_PublisherListener]
//Create a class that inherits from PublisherListener and implement the methods you need.
class TestTypeListener: public PublisherListener{
public:
	TestTypeListener()
	{
		//The participant should have been created and accessible to this method.
		pub = DomainParticipant::createPublisher(p,param);
		pub->assingListener((PublisherListener*)this);

	};
	~TestTypeListener(){};
	void onHistoryFull()
	{
		pub->removeMinSeqCache();
	}
	Publisher* pub;
};
//! [ex_PublisherListener]


