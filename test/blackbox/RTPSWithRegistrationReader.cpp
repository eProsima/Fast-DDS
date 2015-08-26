/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSWithRegistrationReader.cpp
 *
 */

#include "RTPSWithRegistrationReader.hpp"

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/qos/ReaderQos.h>

#include <gtest/gtest.h>

RTPSWithRegistrationReader::RTPSWithRegistrationReader(): listener_(*this), lastvalue_(std::numeric_limits<uint16_t>::max()),
    participant_(nullptr), reader_(nullptr), history_(nullptr), initialized_(false), matched_(0)
{
}

RTPSWithRegistrationReader::~RTPSWithRegistrationReader()
{
    if(participant_ != nullptr)
        RTPSDomain::removeRTPSParticipant(participant_);
    if(history_ != nullptr)
        delete(history_);
}

// TODO Change api of  set_IP4_address to support const string.
void RTPSWithRegistrationReader::init(uint32_t port, uint16_t nmsgs)
{
	RTPSParticipantAttributes pattr;
	pattr.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	pattr.builtin.use_WriterLivelinessProtocol = true;
	participant_ = RTPSDomain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

	//Create readerhistory
	HistoryAttributes hattr;
	hattr.payloadMaxSize = 255;
	history_ = new ReaderHistory(hattr);
    ASSERT_NE(history_, nullptr);

	//Create reader
	ReaderAttributes rattr;
	Locator_t loc(port);
	rattr.endpoint.unicastLocatorList.push_back(loc);
    configReader(rattr);
	reader_ = RTPSDomain::createRTPSReader(participant_, rattr, history_, &listener_);
    ASSERT_NE(reader_, nullptr);

    eprosima::fastrtps::TopicAttributes tattr;
	tattr.topicKind = NO_KEY;
	tattr.topicDataType = "string";
	tattr.topicName = "exampleTopic";
    eprosima::fastrtps::ReaderQos Rqos;
	ASSERT_EQ(participant_->registerReader(reader_, tattr, Rqos), true);

    // Initialize list of msgs
    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        msgs_.push_back(count);
    }

    initialized_ = true;
}

void RTPSWithRegistrationReader::newNumber(uint16_t number)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::list<uint16_t>::iterator it = std::find(msgs_.begin(), msgs_.end(), number);
    ASSERT_NE(it, msgs_.end());
    if(lastvalue_ == *it)
        cv_.notify_one();
    msgs_.erase(it);
}

std::list<uint16_t> RTPSWithRegistrationReader::getNonReceivedMessages()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return msgs_;
}

void RTPSWithRegistrationReader::block(uint16_t lastvalue, const std::chrono::seconds &seconds)
{
    std::unique_lock<std::mutex> lock(mutex_);
    lastvalue_ = lastvalue;
    if(lastvalue_ == *msgs_.rbegin())
        cv_.wait_for(lock, seconds);
}

void RTPSWithRegistrationReader::waitDiscovery()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    if(matched_ == 0)
        cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_NE(matched_, 0);
}

void RTPSWithRegistrationReader::matched()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);
    ++matched_;
    cvDiscovery_.notify_one();
}

void RTPSWithRegistrationReader::Listener::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change)
{
    ASSERT_NE(reader, nullptr);
    ASSERT_NE(change, nullptr);

	ReaderHistory *history = reader->getHistory();
    ASSERT_NE(history, nullptr);

    history->remove_change((CacheChange_t*)change);

    uint16_t number;
    ASSERT_EQ(sscanf((char*)change->serializedPayload.data, "My example string %hu", &number), 1);
    reader_.newNumber(number);
}
