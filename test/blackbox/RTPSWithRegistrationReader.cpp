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

/**
 * @file RTPSWithRegistrationReader.cpp
 *
 */

#include "RTPSWithRegistrationReader.hpp"

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>

#include <fastrtps/utils/IPFinder.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/history/ReaderHistory.h>

#include <boost/interprocess/detail/os_thread_functions.hpp>
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
    pattr.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
	participant_ = RTPSDomain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

	//Create readerhistory
	HistoryAttributes hattr;
	hattr.payloadMaxSize = 255;
	history_ = new ReaderHistory(hattr);
    ASSERT_NE(history_, nullptr);

	//Create reader
	ReaderAttributes rattr;
    eprosima::fastrtps::ReaderQos Rqos;
   
   LocatorList_t locators;
   IPFinder::getIP4Address(&locators);

   for(auto& locator : locators)
   {
      locator.port = port;
	   rattr.endpoint.unicastLocatorList.push_back(locator);
   }

   Locator_t multicastLocator;
   multicastLocator.port = port;
   multicastLocator.set_IP4_address(239,255,1,4);
   rattr.endpoint.multicastLocatorList.push_back(multicastLocator);

    configReader(rattr, Rqos);
	reader_ = RTPSDomain::createRTPSReader(participant_, rattr, history_, &listener_);
    ASSERT_NE(reader_, nullptr);

    eprosima::fastrtps::TopicAttributes tattr;
	tattr.topicKind = NO_KEY;
	tattr.topicDataType = "string";
    configTopic(tattr);
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
    if(!msgs_.empty() && lastvalue_ == *msgs_.rbegin())
        cv_.wait_for(lock, seconds);
}

void RTPSWithRegistrationReader::waitDiscovery()
{
    std::unique_lock<std::mutex> lock(mutexDiscovery_);

    if(matched_ == 0)
        cvDiscovery_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_NE(matched_, 0u);
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

    // Check order of changes.
    ASSERT_LT(reader_.last_seq, change->sequenceNumber);
    reader_.last_seq = change->sequenceNumber;

    uint16_t number;
#ifdef WIN32
    ASSERT_EQ(sscanf_s((char*)change->serializedPayload.data, "My example string %hu", &number), 1);
#else
    ASSERT_EQ(sscanf((char*)change->serializedPayload.data, "My example string %hu", &number), 1);
#endif
    reader_.newNumber(number);

	ReaderHistory *history = reader->getHistory();
    ASSERT_NE(history, nullptr);

    history->remove_change((CacheChange_t*)change);
}
