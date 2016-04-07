/*************************************************************************
 * Copyright (c) 2014 eProsima. AllTransient rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubKeepAllTransientReader.cpp
 *
 */

#include "PubSubKeepAllTransientReader.hpp"

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <boost/asio.hpp>
#include <gtest/gtest.h>

PubSubKeepAllTransientReader::PubSubKeepAllTransientReader(): listener_(*this), lastvalue_(std::numeric_limits<uint16_t>::max()),
    participant_(nullptr), subscriber_(nullptr), initialized_(false), matched_(0), data_received_(0)
{
}

PubSubKeepAllTransientReader::~PubSubKeepAllTransientReader()
{
    if(participant_ != nullptr)
        Domain::removeParticipant(participant_);
}

// TODO Change api of  set_IP4_address to support const string.
void PubSubKeepAllTransientReader::init(uint16_t nmsgs)
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
    std::ostringstream t;
    t << "PubSubAsReliableHelloworld_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();
    sattr.topic.topicName = t.str();
    sattr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
    sattr.topic.historyQos.kind = eprosima::fastrtps::KEEP_ALL_HISTORY_QOS;
    sattr.topic.resourceLimitsQos.max_samples = 2;
	subscriber_ = Domain::createSubscriber(participant_, sattr, &listener_);
    ASSERT_NE(subscriber_, nullptr);

    // Initialize list of msgs
    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        msgs_.push_back(count);
    }

    initialized_ = true;
}

void PubSubKeepAllTransientReader::destroy()
{
    if(participant_ != nullptr)
    {
        Domain::removeParticipant(participant_);
        participant_ = nullptr;
    }
}

void PubSubKeepAllTransientReader::newNumber(uint16_t number)
{
    //std::unique_lock<std::mutex> lock(mutex_);
    std::list<uint16_t>::iterator it = std::find(msgs_.begin(), msgs_.end(), number);
    ASSERT_NE(it, msgs_.end());
    if(lastvalue_ == *it)
        cv_.notify_one();
    msgs_.erase(it);
}

std::list<uint16_t> PubSubKeepAllTransientReader::getNonReceivedMessages()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return msgs_;
}

void PubSubKeepAllTransientReader::read(uint16_t lastvalue, const std::chrono::seconds &seconds)
{
    std::unique_lock<std::mutex> lock(mutex_);
    lastvalue_ = lastvalue;
    while(!msgs_.empty() && lastvalue_ == *msgs_.rbegin())
    {
        if(data_received_)
        {
            HelloWorld hello;
            SampleInfo_t info;

            if(subscriber_->takeNextData((void*)&hello, &info))
            {
                if(info.sampleKind == ALIVE)
                {
                    newNumber(hello.index());
                }
            }

            --data_received_;
        }
        else
        {
            if(cv_.wait_for(lock, seconds) == std::cv_status::timeout)
                break;
        }
    }
}

void PubSubKeepAllTransientReader::waitDiscovery()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    if(matched_ == 0)
        cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_NE(matched_, 0u);
}

void PubSubKeepAllTransientReader::waitRemoval()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    if(matched_ != 0)
        cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_EQ(matched_, 0u);
}

void PubSubKeepAllTransientReader::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    cvDiscovery_.notify_one();
}

void PubSubKeepAllTransientReader::unmatched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    --matched_;
    cvDiscovery_.notify_one();
}

void PubSubKeepAllTransientReader::data_received()
{
    std::unique_lock<std::mutex> lock(mutex_);
    ++data_received_;
    cv_.notify_one();
}

void PubSubKeepAllTransientReader::Listener::onNewDataMessage(Subscriber *sub)
{
    ASSERT_NE(sub, nullptr);
    reader_.data_received();
}
