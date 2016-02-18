/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReqRepHelloWorldRequester.cpp
 *
 */

#include "ReqRepHelloWorldRequester.hpp"

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/publisher/Publisher.h>

#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <gtest/gtest.h>

ReqRepHelloWorldRequester::ReqRepHelloWorldRequester(): reply_listener_(*this), request_listener_(*this),
    current_number_(std::numeric_limits<uint16_t>::max()), number_received_(std::numeric_limits<uint16_t>::max()),
    participant_(nullptr), reply_subscriber_(nullptr), request_publisher_(nullptr),
    initialized_(false), matched_(0)
{
}

ReqRepHelloWorldRequester::~ReqRepHelloWorldRequester()
{
    if(participant_ != nullptr)
        Domain::removeParticipant(participant_);
}

void ReqRepHelloWorldRequester::init()
{
	ParticipantAttributes pattr;
    pattr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
	participant_ = Domain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

    // Register type
	ASSERT_EQ(Domain::registerType(participant_,&type_), true);

	//Create subscriber
	SubscriberAttributes sattr;
	sattr.topic.topicKind = NO_KEY;
	sattr.topic.topicDataType = "HelloWorldType";
    configSubscriber(sattr, "Reply");
	reply_subscriber_ = Domain::createSubscriber(participant_, sattr, &reply_listener_);
    ASSERT_NE(reply_subscriber_, nullptr);

	//Create publisher
	PublisherAttributes puattr;
	puattr.topic.topicKind = NO_KEY;
	puattr.topic.topicDataType = "HelloWorldType";
    configPublisher(puattr, "Request");
	request_publisher_ = Domain::createPublisher(participant_, puattr, &request_listener_);
    ASSERT_NE(request_publisher_, nullptr);

    initialized_ = true;
}

void ReqRepHelloWorldRequester::newNumber(SampleIdentity related_sample_identity, uint16_t number)
{
    std::unique_lock<std::mutex> lock(mutex_);
    ASSERT_EQ(related_sample_identity_, related_sample_identity);
    number_received_ = number;
    ASSERT_EQ(current_number_, number_received_);
    if(current_number_ == number_received_)
        cv_.notify_one();
}

void ReqRepHelloWorldRequester::block(const std::chrono::seconds &seconds)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if(current_number_ != number_received_)
    {
        ASSERT_EQ(cv_.wait_for(lock, seconds), std::cv_status::no_timeout);
    }

    ASSERT_EQ(current_number_, number_received_);
}

void ReqRepHelloWorldRequester::waitDiscovery()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    if(matched_ < 2)
        cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_GE(matched_, 2u);
}

void ReqRepHelloWorldRequester::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    if(matched_ >= 2)
        cvDiscovery_.notify_one();
}

void ReqRepHelloWorldRequester::ReplyListener::onNewDataMessage(Subscriber *sub)
{
    ASSERT_NE(sub, nullptr);

    HelloWorld hello;
    SampleInfo_t info;

	if(sub->takeNextData((void*)&hello, &info))
	{
		if(info.sampleKind == ALIVE)
		{
            ASSERT_EQ(hello.message().compare("GoodBye"), 0);
            requester_.newNumber(info.related_sample_identity, hello.index());
		}
	}
}

void ReqRepHelloWorldRequester::send(const uint16_t number)
{
    waitDiscovery();

    WriteParams wparams;
    HelloWorld hello;
    hello.index(number);
    hello.message("HelloWorld");

    std::unique_lock<std::mutex> lock(mutex_);

    ASSERT_EQ(request_publisher_->write((void*)&hello, wparams), true);
    related_sample_identity_ = wparams.sample_identity();
    current_number_ = number;
}
