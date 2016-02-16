/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubHelloWorldWriter.cpp
 *
 */

#include "PubSubHelloWorldWriter.hpp"

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <fastrtps/publisher/Publisher.h>

#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <gtest/gtest.h>

PubSubHelloWorldWriter::PubSubHelloWorldWriter(): listener_(*this), participant_(nullptr),
    publisher_(nullptr), initialized_(false), matched_(0)
{
}

PubSubHelloWorldWriter::~PubSubHelloWorldWriter()
{
    if(participant_ != nullptr)
        Domain::removeParticipant(participant_);
}

void PubSubHelloWorldWriter::init()
{
	//Create participant
	ParticipantAttributes pattr;
    pattr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id();
	participant_ = Domain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

    // Register type
    Domain::registerType(participant_, &type_);

	//Create publisher
	PublisherAttributes puattr;
	puattr.topic.topicKind = NO_KEY;
	puattr.topic.topicDataType = "HelloWorldType";
    configPublisher(puattr);
	publisher_ = Domain::createPublisher(participant_, puattr, &listener_);
    ASSERT_NE(publisher_, nullptr);

    initialized_ = true;
}

void PubSubHelloWorldWriter::send(const std::list<uint16_t> &msgs)
{
    waitDiscovery();

	for(std::list<uint16_t>::const_iterator it = msgs.begin(); it != msgs.end(); ++it)
	{
        HelloWorld hello;
        hello.index(*it);
        hello.message("HelloWorld");
        ASSERT_EQ(publisher_->write((void*)&hello), true);
	}
}

void PubSubHelloWorldWriter::matched()
{
    std::unique_lock<std::mutex> lock(mutex_);
    ++matched_;
    cv_.notify_one();
}

void PubSubHelloWorldWriter::waitDiscovery()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if(matched_ == 0)
        cv_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_NE(matched_, 0u);
}
