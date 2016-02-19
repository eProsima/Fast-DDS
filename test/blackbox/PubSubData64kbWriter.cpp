/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubData64kbWriter.cpp
 *
 */

#include "PubSubData64kbWriter.hpp"

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <fastrtps/publisher/Publisher.h>

#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <gtest/gtest.h>

PubSubData64kbWriter::PubSubData64kbWriter(): listener_(*this), participant_(nullptr),
    publisher_(nullptr), initialized_(false), matched_(0)
{
}

PubSubData64kbWriter::~PubSubData64kbWriter()
{
    if(participant_ != nullptr)
        Domain::removeParticipant(participant_);
}

void PubSubData64kbWriter::init()
{
	//Create participant
	ParticipantAttributes pattr;
    pattr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
	participant_ = Domain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

    // Register type
    Domain::registerType(participant_, &type_);

	//Create publisher
	PublisherAttributes puattr;
	puattr.topic.topicKind = NO_KEY;
	puattr.topic.topicDataType = "Data64kbType";
    configPublisher(puattr);
	publisher_ = Domain::createPublisher(participant_, puattr, &listener_);
    ASSERT_NE(publisher_, nullptr);

    initialized_ = true;
}

void PubSubData64kbWriter::destroy()
{
    if(participant_ != nullptr)
    {
        Domain::removeParticipant(participant_);
        participant_ = nullptr;
    }
}

void PubSubData64kbWriter::send(const std::list<uint16_t> &msgs)
{
    waitDiscovery();

    Data64kb data;
    for(int i = 0; i < 63996; ++i)
        data.data().push_back(i);

	for(std::list<uint16_t>::const_iterator it = msgs.begin(); it != msgs.end(); ++it)
	{
        data.data()[0] = *it;
        ASSERT_EQ(publisher_->write((void*)&data), true);
	}
}

void PubSubData64kbWriter::matched()
{
    std::unique_lock<std::mutex> lock(mutex_);
    ++matched_;
    cv_.notify_one();
}

void PubSubData64kbWriter::unmatched()
{
    std::unique_lock<std::mutex> lock(mutex_);
    --matched_;
    cv_.notify_one();
}

void PubSubData64kbWriter::waitDiscovery()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if(matched_ == 0)
        cv_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_NE(matched_, 0u);
}

void PubSubData64kbWriter::waitRemoval()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if(matched_ != 0)
        cv_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_EQ(matched_, 0u);
}
