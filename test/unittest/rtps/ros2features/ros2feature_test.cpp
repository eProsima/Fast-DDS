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

#include <mutex>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

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
        std::mutex mapmutex;
        std::map<std::string,std::set<std::string>> topicNtypes;
        void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in){
            (void)reader;
            CacheChange_t* change = (CacheChange_t*) change_in;
            if(change->kind == ALIVE){
                WriterProxyData proxyData;
                CDRMessage_t tempMsg;
                tempMsg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
                tempMsg.length = change->serializedPayload.length;
                memcpy(tempMsg.buffer,change->serializedPayload.data,tempMsg.length);
                if(proxyData.readFromCDRMessage(&tempMsg)){
                    mapmutex.lock();
                    topicNtypes[proxyData.topicName()].insert(proxyData.typeName());		
                    mapmutex.unlock();
                }
            }
        }
};

TEST(ros2features, EDPSlaveReaderAttachment_DynamicMode)
{
    Participant *my_participant;
    Publisher *my_publisher;
    ParticipantAttributes p_attr;
    PublisherAttributes pub_attr;
    HelloWorldType my_type;
    pub_dummy_listener my_dummy_listener;
    ReaderListener slave_listener;
    bool result;
    p_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
    my_participant = Domain::createParticipant(p_attr);

    ASSERT_NE(my_participant, nullptr);
    ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);

    pub_attr.topic.topicKind = NO_KEY;
    pub_attr.topic.topicDataType = "HelloWorldType";
    std::ostringstream t;
    t << std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())
        << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
    pub_attr.topic.topicName = t.str();
    pub_attr.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
    my_publisher = Domain::createPublisher(my_participant, pub_attr, &my_dummy_listener);
    ASSERT_NE(my_publisher, nullptr);

    std::pair<StatefulReader*,StatefulReader*> EDP_Readers = my_participant->getEDPReaders();
    //target->attachListener(slave_listener);
    result = EDP_Readers.first->setListener(&slave_listener);
    ASSERT_EQ(result, true);
    EDP_Readers.first->setListener(nullptr);
    eprosima::fastrtps::Domain::removeParticipant(my_participant);
}

TEST(ros2features, EDPSlaveReaderAttachment_StaticMode)
{
    Participant *my_participant;
    Publisher *my_publisher;
    ParticipantAttributes p_attr;
    PublisherAttributes pub_attr;
    HelloWorldType my_type;
    pub_dummy_listener my_dummy_listener;
    ReaderListener slave_listener;
    bool result;
    p_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
    my_participant = Domain::createParticipant(p_attr);

    ASSERT_NE(my_participant, nullptr);
    ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);

    pub_attr.topic.topicKind = NO_KEY;
    pub_attr.topic.topicDataType = "HelloWorldType";
    std::ostringstream t;
    t << std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())
        << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
    pub_attr.topic.topicName = t.str();
    pub_attr.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
    my_publisher = Domain::createPublisher(my_participant, pub_attr, &my_dummy_listener);
    ASSERT_NE(my_publisher, nullptr);

    std::pair<StatefulReader*,StatefulReader*> EDP_Readers = my_participant->getEDPReaders();
    //target->attachListener(slave_listener);
    result = EDP_Readers.first->setListener(&slave_listener);
    ASSERT_EQ(result, true);
    EDP_Readers.first->setListener(nullptr);
    eprosima::fastrtps::Domain::removeParticipant(my_participant);
}

TEST(ros2features, PubSubPoll_DynamicMode)
{
    const uint8_t max_elements = 4;
    Participant *my_participant;
    Publisher* pub_array[max_elements];
    Subscriber* sub_array[max_elements];
    HelloWorldType my_type;
    ParticipantAttributes part_attr;
    PublisherAttributes p_attr[max_elements];
    SubscriberAttributes s_attr[max_elements];
    pub_dummy_listener p_listener_array[max_elements];
    sub_dummy_listener s_listener_array[max_elements];

    std::ostringstream t;
    t << std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())
        << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
    std::string str = t.str();

    part_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
    my_participant = Domain::createParticipant(part_attr);
    //Register type

    ASSERT_NE(my_participant, nullptr);
    ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);
    for(int i=0; i<max_elements;i++){
        //Create Publisher
        p_attr[i].topic.topicKind = NO_KEY;
        p_attr[i].topic.topicDataType = "HelloWorldType";
        p_attr[i].topic.topicName = str;
        p_attr[i].historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
        //configPublisher(p_attr);
        pub_array[i] = Domain::createPublisher(my_participant,p_attr[i], &p_listener_array[i]);
        ASSERT_NE(pub_array[i],nullptr);
        //Poll no.Pubs
        ASSERT_EQ(my_participant->get_no_publishers( const_cast<char*>( str.c_str() ) ),i+1);
        //Create Subscriber
        s_attr[i].topic.topicKind = NO_KEY;
        s_attr[i].topic.topicDataType = "HelloWorldType";
        s_attr[i].topic.topicName = str;
        s_attr[i].historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
        //configSubscriber(s_attr);
        sub_array[i] =Domain::createSubscriber(my_participant,s_attr[i], &s_listener_array[i]);	
        ASSERT_NE(sub_array[i],nullptr);
        //Poll no.Subs
        ASSERT_EQ(my_participant->get_no_subscribers( const_cast<char*>( str.c_str() ) ),i+1);
    }
    eprosima::fastrtps::Domain::removeParticipant(my_participant);
}

TEST(ros2features, PubSubPoll_StaticMode)
{
    const uint8_t max_elements = 4;
    Participant *my_participant;
    Publisher* pub_array[max_elements];
    Subscriber* sub_array[max_elements];
    HelloWorldType my_type;
    ParticipantAttributes part_attr;
    PublisherAttributes p_attr[max_elements];
    SubscriberAttributes s_attr[max_elements];
    pub_dummy_listener p_listener_array[max_elements];
    sub_dummy_listener s_listener_array[max_elements];

    std::ostringstream t;
    t << std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())
        << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
    std::string str = t.str();

    part_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230 + 2;
    my_participant = Domain::createParticipant(part_attr);
    //Register type

    ASSERT_NE(my_participant, nullptr);
    ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);
    for(int i=0; i<max_elements;i++){
        //Create Publisher
        p_attr[i].topic.topicKind = NO_KEY;
        p_attr[i].topic.topicDataType = "HelloWorldType";
        p_attr[i].topic.topicName = str;
        p_attr[i].historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
        //configPublisher(p_attr);
        pub_array[i] = Domain::createPublisher(my_participant,p_attr[i], &p_listener_array[i]);
        ASSERT_NE(pub_array[i],nullptr);
        //Poll no.Pubs
        ASSERT_EQ(my_participant->get_no_publishers( const_cast<char*>( str.c_str() ) ),i+1);
        //Create Subscriber
        s_attr[i].topic.topicKind = NO_KEY;
        s_attr[i].topic.topicDataType = "HelloWorldType";
        s_attr[i].topic.topicName = str;
        s_attr[i].historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
        //configSubscriber(s_attr);
        sub_array[i] =Domain::createSubscriber(my_participant,s_attr[i], &s_listener_array[i]);	
        ASSERT_NE(sub_array[i],nullptr);
        //Poll no.Subs
        ASSERT_EQ(my_participant->get_no_subscribers( const_cast<char*>( str.c_str() ) ),i+1);
    }
    eprosima::fastrtps::Domain::removeParticipant(my_participant);
}


TEST(ros2features, SlaveListenerCallback_DynamicMode){
    Participant *my_participant;
    Publisher *my_publisher;
    ParticipantAttributes p_attr;
    PublisherAttributes pub_attr;
    HelloWorldType my_type;
    pub_dummy_listener my_dummy_listener;
    gettopicnamesandtypesReaderListener slave_listener;
    bool result;
    p_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230 + 35;
    my_participant = Domain::createParticipant(p_attr);
	ASSERT_NE(my_participant, nullptr);

	std::pair<StatefulReader*,StatefulReader*> EDP_Readers = my_participant->getEDPReaders();
	result = EDP_Readers.second->setListener(&slave_listener);
	ASSERT_EQ(result,true);

    ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);

    pub_attr.topic.topicKind = NO_KEY;
    pub_attr.topic.topicDataType = "HelloWorldType";
    std::ostringstream t;
    t << std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())
        << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
    pub_attr.topic.topicName = t.str();
    pub_attr.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
    my_publisher = Domain::createPublisher(my_participant, pub_attr, &my_dummy_listener);
    ASSERT_NE(my_publisher, nullptr);

    slave_listener.mapmutex.lock();
	ASSERT_EQ(slave_listener.topicNtypes.size(),1);
    slave_listener.mapmutex.unlock();

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
    std::ostringstream t2;
    t2 << std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())
        << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
    pub_attr2.topic.topicName = t2.str();
    pub_attr2.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
    my_publisher2 = Domain::createPublisher(my_participant2, pub_attr2, &my_dummy_listener2);
    ASSERT_NE(my_publisher2, nullptr);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    slave_listener.mapmutex.lock();
    ASSERT_EQ(slave_listener.topicNtypes.size(),1);
    slave_listener.mapmutex.unlock();
    EDP_Readers.second->setListener(nullptr);

    eprosima::fastrtps::Domain::removeParticipant(my_participant);
    eprosima::fastrtps::Domain::removeParticipant(my_participant2);
}

TEST(ros2features, SlaveListenerCallback_StaticMode){
    Participant *my_participant;
    Publisher *my_publisher;
    ParticipantAttributes p_attr;
    PublisherAttributes pub_attr;
    HelloWorldType my_type;
    pub_dummy_listener my_dummy_listener;
    gettopicnamesandtypesReaderListener slave_listener;
    bool result;
    p_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
    my_participant = Domain::createParticipant(p_attr);
	ASSERT_NE(my_participant, nullptr);

	std::pair<StatefulReader*,StatefulReader*> EDP_Readers = my_participant->getEDPReaders();
	result = EDP_Readers.second->setListener(&slave_listener);
	ASSERT_EQ(result,true);

    ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);

    pub_attr.topic.topicKind = NO_KEY;
    pub_attr.topic.topicDataType = "HelloWorldType";
    std::ostringstream t;
    t << std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())
        << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
    pub_attr.topic.topicName = t.str();
    pub_attr.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
    my_publisher = Domain::createPublisher(my_participant, pub_attr, &my_dummy_listener);
    ASSERT_NE(my_publisher, nullptr);

    slave_listener.mapmutex.lock();
	ASSERT_EQ(slave_listener.topicNtypes.size(),1);
    slave_listener.mapmutex.unlock();

    Participant *my_participant2;
    Publisher *my_publisher2;
    ParticipantAttributes p_attr2;
    PublisherAttributes pub_attr2;
    pub_dummy_listener my_dummy_listener2;
    p_attr2.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
    my_participant2 = Domain::createParticipant(p_attr2);

    ASSERT_NE(my_participant2, nullptr);
    ASSERT_EQ(Domain::registerType(my_participant2, &my_type), true);


    pub_attr2.topic.topicKind = NO_KEY;
    pub_attr2.topic.topicDataType = "HelloWorldType";
    std::ostringstream t2;
    t2 << std::string(test_info_->test_case_name() + std::string("_") + test_info_->name())
        << "_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
    pub_attr2.topic.topicName = t2.str();
    pub_attr2.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
    my_publisher2 = Domain::createPublisher(my_participant2, pub_attr2, &my_dummy_listener2);
    ASSERT_NE(my_publisher2, nullptr);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    slave_listener.mapmutex.lock();
    ASSERT_EQ(slave_listener.topicNtypes.size(),1);
    slave_listener.mapmutex.unlock();
    EDP_Readers.second->setListener(nullptr);

    eprosima::fastrtps::Domain::removeParticipant(my_participant);
    eprosima::fastrtps::Domain::removeParticipant(my_participant2);
}

TEST(ros2features, SlaveListenerCallbackWithOneParticipant_StaticMode){
	Participant *my_participant;
	Publisher *my_publisher;
	ParticipantAttributes p_attr;
	PublisherAttributes pub_attr;
	HelloWorldType my_type;
	gettopicnamesandtypesReaderListener slave_listener;
	bool result;	
	p_attr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
	my_participant = Domain::createParticipant(p_attr);
	ASSERT_NE(my_participant, nullptr);

	std::pair<StatefulReader*,StatefulReader*> EDP_Readers = my_participant->getEDPReaders();
	result = EDP_Readers.second->setListener(&slave_listener);
	ASSERT_EQ(result,true);

	ASSERT_EQ(Domain::registerType(my_participant, &my_type), true);

	pub_attr.topic.topicKind = NO_KEY;
	pub_attr.topic.topicDataType = "HelloWorldType";
	pub_attr.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
	my_publisher = Domain::createPublisher(my_participant, pub_attr, nullptr);
	ASSERT_NE(my_publisher, nullptr);

	slave_listener.mapmutex.lock();
	ASSERT_EQ(slave_listener.topicNtypes.size(),1);
	slave_listener.mapmutex.unlock();

	Publisher *my_publisher2;
	PublisherAttributes pub_attr2;

	pub_attr2.topic.topicKind = NO_KEY;
	pub_attr2.topic.topicDataType = "HelloWorldType";
    pub_attr2.topic.topicName = "OtherTopic";
	pub_attr2.historyMemoryPolicy = PREALLOCATED_MEMORY_MODE;
	my_publisher2 = Domain::createPublisher(my_participant, pub_attr2, nullptr);
	ASSERT_NE(my_publisher2, nullptr);

	slave_listener.mapmutex.lock();
	ASSERT_EQ(slave_listener.topicNtypes.size(), 2);
	slave_listener.mapmutex.unlock();

	EDP_Readers.second->setListener(nullptr);

	eprosima::fastrtps::Domain::removeParticipant(my_participant);
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    return result;
}
