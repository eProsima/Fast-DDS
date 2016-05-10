#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/publisher/Publisher.h>

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include "types/HelloWorldType.h"
#include <string>
#include <chrono>
#include <thread>
#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/common/CDRMessage_t.h>

#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <gtest/gtest.h>

bool haveIbeenCalled = false;

class pub_dummy_listener:public PublisherListener
{
public:

	void onPublicationMatched(Publisher* /*pub*/, MatchingInfo& /*info*/){};

};
class sub_dummy_listener:public SubscriberListener
{
public:
		
	void onNewDataMessage(Subscriber* /*sub */){}
	void onSubscriptionMatched(Subscriber* /*sub*/, MatchingInfo& /*info*/){};

};

class gettopicnamesandtypesReaderListener:public ReaderListener
{
	public:
	void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in){
		CacheChange_t* change = (CacheChange_t*) change_in;
		haveIbeenCalled = true;
		if(change->kind == ALIVE){
			WriterProxyData proxyData;
			CDRMessage_t tempMsg;
			tempMsg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
			tempMsg.length = change->serializedPayload.length;
			memcpy(tempMsg.buffer,change->serializedPayload.data,tempMsg.length);
			//if(proxyData.readFromCDRMessage(&tempMsg)){
			//	topicNtypes[proxyData.m_topicName].insert(proxyData.m_typeName);		
			//}
		}
	}
	//std::map<std::string,std::set<std::string>> topicNtypes;
	//bool haveIbeenCalled;

};

TEST(ros2features, EDPSlaveReaderAttachment)
{
	Participant *my_participant;
	Publisher *my_publisher;
	ParticipantAttributes p_attr;
	PublisherAttributes pub_attr;
	HelloWorldType my_type;
	pub_dummy_listener my_dummy_listener;
	ReaderListener *slave_listener = new(ReaderListener);
	bool result;	
	p_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
	my_participant = Domain::createParticipant(p_attr);

	ASSERT_NE(my_participant, nullptr);
	ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);

	pub_attr.topic.topicKind = NO_KEY;
	pub_attr.topic.topicDataType = "HelloWorldType";
	my_publisher = Domain::createPublisher(my_participant, pub_attr, &my_dummy_listener);
	ASSERT_NE(my_publisher, nullptr);

	std::pair<StatefulReader*,StatefulReader*> EDP_Readers = my_participant->getEDPReaders();
	InfectableReaderListener* target = dynamic_cast<InfectableReaderListener*>(EDP_Readers.first->getListener());
	result = target->hasReaderAttached();
	ASSERT_EQ(result,false);

	target->attachListener(slave_listener);
	result = target->hasReaderAttached();
	ASSERT_EQ(result, true);


}

TEST(ros2features, PubSubPoll)
{
	const uint8_t max_elements = 4;
	Participant *my_participant;
	Publisher* pub_array[max_elements];
	Subscriber* sub_array[max_elements];
	ParticipantAttributes p_attr;
	HelloWorldType my_type;
	pub_dummy_listener p_listener_array[max_elements];
	sub_dummy_listener s_listener_array[max_elements];

	std::string str("HelloWorldType");
	char *cstr = new  char[str.length()+1];
	std::strcpy(cstr,str.c_str());

	p_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230 + 2;
	my_participant = Domain::createParticipant(p_attr);
	//Register type
	
	ASSERT_NE(my_participant, nullptr);
	ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);

	for(int i=0; i<max_elements;i++){
		//Create Publisher
		PublisherAttributes p_attr;
		p_attr.topic.topicKind = NO_KEY;
		p_attr.topic.topicDataType = "HelloWorldType";
		p_attr.topic.topicName = "HelloWorldType";
		//configPublisher(p_attr);
		pub_array[i] = Domain::createPublisher(my_participant,p_attr, &p_listener_array[i]);
		ASSERT_NE(pub_array[i],nullptr);
		//Poll no.Pubs
		ASSERT_EQ(my_participant->get_no_publishers(cstr),i+1);
		//Create Subscriber
		SubscriberAttributes s_attr;
		s_attr.topic.topicKind = NO_KEY;
		s_attr.topic.topicDataType = "HelloWorldType";		
		s_attr.topic.topicName = "HelloWorldType";
		//configSubscriber(s_attr);
	 	sub_array[i] =Domain::createSubscriber(my_participant,s_attr, &s_listener_array[i]);	
		ASSERT_NE(sub_array[i],nullptr);		
		//Poll no.Subs
		ASSERT_EQ(my_participant->get_no_subscribers(cstr),i+1);
	}

}
TEST(ros2features, SlaveListenerCallback){
	Participant *my_participant;
	Publisher *my_publisher;
	ParticipantAttributes p_attr;
	PublisherAttributes pub_attr;
	HelloWorldType my_type;
	pub_dummy_listener my_dummy_listener;
	gettopicnamesandtypesReaderListener* slave_listener = new(gettopicnamesandtypesReaderListener);
	haveIbeenCalled = false;
	gettopicnamesandtypesReaderListener* slave_target;
	bool result;	
	p_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230 + 35;
	my_participant = Domain::createParticipant(p_attr);

	ASSERT_NE(my_participant, nullptr);
	ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);

	pub_attr.topic.topicKind = NO_KEY;
	pub_attr.topic.topicDataType = "HelloWorldType";
	my_publisher = Domain::createPublisher(my_participant, pub_attr, &my_dummy_listener);
	ASSERT_NE(my_publisher, nullptr);

	std::pair<StatefulReader*,StatefulReader*> EDP_Readers = my_participant->getEDPReaders();
	InfectableReaderListener* target = dynamic_cast<InfectableReaderListener*>(EDP_Readers.second->getListener());
	target->attachListener(slave_listener);
	result = target->hasReaderAttached();
	ASSERT_EQ(result,true);
	slave_target =  dynamic_cast<gettopicnamesandtypesReaderListener*>(target->getAttachedListener());
	//ASSERT_EQ(slave_target->topicNtypes.size(),0);
	ASSERT_EQ(haveIbeenCalled,false);

	Participant *my_participant2;
	Publisher *my_publisher2;
	ParticipantAttributes p_attr2;
	PublisherAttributes pub_attr2;
	pub_dummy_listener my_dummy_listener2;
	p_attr2.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230 + 35;
	my_participant2 = Domain::createParticipant(p_attr2);

	ASSERT_NE(my_participant2, nullptr);
	ASSERT_EQ(Domain::registerType(my_participant2, &my_type), true);


	pub_attr2.topic.topicKind = NO_KEY;
	pub_attr2.topic.topicDataType = "HelloWorldType";
	my_publisher2 = Domain::createPublisher(my_participant2, pub_attr2, &my_dummy_listener2);
	ASSERT_NE(my_publisher2, nullptr);
	std::this_thread::sleep_for(std::chrono::seconds(5));
	//ASSERT_EQ(slave_target->topicNtypes.size(),1);
	ASSERT_EQ(haveIbeenCalled,true);
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
