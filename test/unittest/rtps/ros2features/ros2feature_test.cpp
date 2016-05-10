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


#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <gtest/gtest.h>


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

/*TEST(ros2features, EDPSlaveReaderAttachment)
{






}
*/
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

	p_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
	my_participant = Domain::createParticipant(p_attr);
	//Register type
	
	ASSERT_NE(my_participant, nullptr);
	ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);

	for(int i=0; i<max_elements;i++){
		//Create Publisher
		PublisherAttributes p_attr;
		p_attr.topic.topicKind = NO_KEY;
		p_attr.topic.topicDataType = "HelloWorldType";
		//configPublisher(p_attr);
		pub_array[i] = Domain::createPublisher(my_participant,p_attr, &p_listener_array[i]);
		ASSERT_NE(pub_array[i],nullptr);
		//Poll no.Pubs
		ASSERT_EQ(my_participant->get_no_publishers(cstr),i+1);
		//Create Subscriber
		SubscriberAttributes s_attr;
		s_attr.topic.topicKind = NO_KEY;
		s_attr.topic.topicDataType = "HelloWorldType";		
		//configSubscriber(s_attr);
	 	sub_array[i] =Domain::createSubscriber(my_participant,s_attr, &s_listener_array[i]);	
		ASSERT_NE(sub_array[i],nullptr);		
		//Poll no.Subs
		ASSERT_EQ(my_participant->get_no_subscribers(cstr),i+1);
	}

}
/*
TEST(ros2features, TopicsAndTypesDiscovery){




}
*/
int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
