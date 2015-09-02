/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubHelloWorldReader.cpp
 *
 */

#include "PubSubHelloWorldReader.hpp"

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <gtest/gtest.h>

PubSubHelloWorldReader::PubSubHelloWorldReader(): listener_(*this), lastvalue_(std::numeric_limits<uint16_t>::max()),
    participant_(nullptr), subscriber_(nullptr), initialized_(false), matched_(0)
{
}

PubSubHelloWorldReader::~PubSubHelloWorldReader()
{
    if(participant_ != nullptr)
        Domain::removeParticipant(participant_);
}

// TODO Change api of  set_IP4_address to support const string.
void PubSubHelloWorldReader::init(uint16_t nmsgs)
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
	sattr.topic.topicName = "HelloWorldTopic";
    configSubscriber(sattr);
	subscriber_ = Domain::createSubscriber(participant_, sattr, &listener_);
    ASSERT_NE(subscriber_, nullptr);

    // Initialize list of msgs
    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        msgs_.push_back(count);
    }

    initialized_ = true;
}

void PubSubHelloWorldReader::newNumber(uint16_t number)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::list<uint16_t>::iterator it = std::find(msgs_.begin(), msgs_.end(), number);
    ASSERT_NE(it, msgs_.end());
    if(lastvalue_ == *it)
        cv_.notify_one();
    msgs_.erase(it);
}

std::list<uint16_t> PubSubHelloWorldReader::getNonReceivedMessages()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return msgs_;
}

void PubSubHelloWorldReader::block(uint16_t lastvalue, const std::chrono::seconds &seconds)
{
    std::unique_lock<std::mutex> lock(mutex_);
    lastvalue_ = lastvalue;
    if(lastvalue_ == *msgs_.rbegin())
        cv_.wait_for(lock, seconds);
}

void PubSubHelloWorldReader::waitDiscovery()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    if(matched_ == 0)
        cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_NE(matched_, 0u);
}

void PubSubHelloWorldReader::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    cvDiscovery_.notify_one();
}

void PubSubHelloWorldReader::Listener::onNewDataMessage(Subscriber *sub)
{
    ASSERT_NE(sub, nullptr);

    HelloWorld hello;
    SampleInfo_t info;

	if(sub->takeNextData((void*)&hello, &info))
	{
		if(info.sampleKind == ALIVE)
		{
            reader_.newNumber(hello.index());
		}
	}
}
