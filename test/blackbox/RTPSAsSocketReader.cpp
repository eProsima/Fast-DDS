/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSAsSocketReader.cpp
 *
 */

#include "RTPSAsSocketReader.hpp"

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/history/ReaderHistory.h>

#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <boost/asio.hpp>
#include <gtest/gtest.h>

RTPSAsSocketReader::RTPSAsSocketReader(): listener_(*this), lastvalue_(std::numeric_limits<uint16_t>::max()),
    participant_(nullptr), reader_(nullptr), history_(nullptr), initialized_(false) 
{
}

RTPSAsSocketReader::~RTPSAsSocketReader()
{
    if(participant_ != nullptr)
        RTPSDomain::removeRTPSParticipant(participant_);
    if(history_ != nullptr)
        delete(history_);
}

// TODO Change api of  set_IP4_address to support const string.
void RTPSAsSocketReader::init(std::string &ip, uint32_t port, uint16_t nmsgs)
{
	RTPSParticipantAttributes pattr;
	pattr.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = false;
	pattr.builtin.use_WriterLivelinessProtocol = false;
    pattr.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id();
    pattr.participantID = 1;
	participant_ = RTPSDomain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

	//Create readerhistory
	HistoryAttributes hattr;
	hattr.payloadMaxSize = 255;
	history_ = new ReaderHistory(hattr);
    ASSERT_NE(history_, nullptr);

	//Create reader
	ReaderAttributes rattr;
	Locator_t loc;
	loc.set_IP4_address(ip);
	loc.port = port;
	rattr.endpoint.multicastLocatorList.push_back(loc);
    configReader(rattr);
	reader_ = RTPSDomain::createRTPSReader(participant_, rattr, history_, &listener_);
    ASSERT_NE(reader_, nullptr);

	//Add remote writer (in this case a reader in the same machine)
    GUID_t guid = participant_->getGuid();
    addRemoteWriter(reader_, ip, port, guid);

    // Initialize list of msgs
    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        msgs_.push_back(count);
    }

    text_ = getText();
    domainId_ = pattr.builtin.domainId;
    hostname_ = boost::asio::ip::host_name();

    std::ostringstream word;
    word << text_ << "_" << domainId_ << "_" << hostname_;
    word_ = word.str();

    initialized_ = true;
}

void RTPSAsSocketReader::newNumber(uint16_t number)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::list<uint16_t>::iterator it = std::find(msgs_.begin(), msgs_.end(), number);
    ASSERT_NE(it, msgs_.end());
    if(lastvalue_ == *it)
        cv_.notify_one();
    msgs_.erase(it);
}

std::list<uint16_t> RTPSAsSocketReader::getNonReceivedMessages()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return msgs_;
}

void RTPSAsSocketReader::block(uint16_t lastvalue, const std::chrono::seconds &seconds)
{
    std::unique_lock<std::mutex> lock(mutex_);
    lastvalue_ = lastvalue;
    if(!msgs_.empty() && lastvalue_ == *msgs_.rbegin())
        cv_.wait_for(lock, seconds);
}

void RTPSAsSocketReader::Listener::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change)
{
    ASSERT_NE(reader, nullptr);
    ASSERT_NE(change, nullptr);

	ReaderHistory *history = reader->getHistory();
    ASSERT_NE(history, nullptr);

    history->remove_change((CacheChange_t*)change);

    uint16_t number;
    std::istringstream input((char*)change->serializedPayload.data);
    std::string word;
    input >> word;

    if(word.compare(reader_.word_) == 0)
    {
        input >> number;
        reader_.newNumber(number);
    }
}
