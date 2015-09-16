/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReqRepHelloWorldReplier.cpp
 *
 */

#include "ReqRepHelloWorldReplier.hpp"

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/publisher/Publisher.h>

#include <gtest/gtest.h>

ReqRepHelloWorldReplier::ReqRepHelloWorldReplier(): request_listener_(*this), reply_listener_(*this),
    participant_(nullptr), request_subscriber_(nullptr), reply_publisher_(nullptr),
    initialized_(false), matched_(0)
{
}

ReqRepHelloWorldReplier::~ReqRepHelloWorldReplier()
{
    if(participant_ != nullptr)
        Domain::removeParticipant(participant_);
}

void ReqRepHelloWorldReplier::init()
{
	ParticipantAttributes pattr;
	participant_ = Domain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

    // Register type
	ASSERT_EQ(Domain::registerType(participant_,&type_), true);

	//Create subscriber
	SubscriberAttributes sattr;
	sattr.topic.topicKind = NO_KEY;
	sattr.topic.topicDataType = "HelloWorldType";
	sattr.topic.topicName = "HelloWorldTopicRequest";
    configSubscriber(sattr);
	request_subscriber_ = Domain::createSubscriber(participant_, sattr, &request_listener_);
    ASSERT_NE(request_subscriber_, nullptr);

	//Create publisher
	PublisherAttributes puattr;
	puattr.topic.topicKind = NO_KEY;
	puattr.topic.topicDataType = "HelloWorldType";
	puattr.topic.topicName = "HelloWorldTopicReply";
    configPublisher(puattr);
	reply_publisher_ = Domain::createPublisher(participant_, puattr, &reply_listener_);
    ASSERT_NE(reply_publisher_, nullptr);

    initialized_ = true;
}

void ReqRepHelloWorldReplier::newNumber(SampleIdentity sample_identity, uint16_t number)
{
    waitDiscovery();

    WriteParams wparams;
    HelloWorld hello;
    hello.index(number);
    hello.message("GoodBye");
    wparams.related_sample_identity(sample_identity);
    ASSERT_EQ(reply_publisher_->write((void*)&hello, wparams), true);
}

void ReqRepHelloWorldReplier::waitDiscovery()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    if(matched_ < 2)
        cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_GE(matched_, 2u);
}

void ReqRepHelloWorldReplier::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if(matched_ >= 2)
        cvDiscovery_.notify_one();
}

void ReqRepHelloWorldReplier::ReplyListener::onNewDataMessage(Subscriber *sub)
{
    ASSERT_NE(sub, nullptr);

    HelloWorld hello;
    SampleInfo_t info;

	if(sub->takeNextData((void*)&hello, &info))
	{
		if(info.sampleKind == ALIVE)
		{
            ASSERT_EQ(hello.message().compare("HelloWorld"), 0);
            replier_.newNumber(info.sample_identity, hello.index());
		}
	}
}
