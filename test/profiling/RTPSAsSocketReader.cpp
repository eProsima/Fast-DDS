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

#include <asio.hpp>

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
    pattr.builtin.domainId = (uint32_t)GET_PID() % 230;
    pattr.participantID = 1;
	participant_ = RTPSDomain::createParticipant(pattr);

	//Create readerhistory
	HistoryAttributes hattr;
	hattr.payloadMaxSize = 255;
	history_ = new ReaderHistory(hattr);

	//Create reader
	ReaderAttributes rattr;
	Locator_t loc;
	loc.set_IP4_address(ip);
	loc.port = port;
	rattr.endpoint.multicastLocatorList.push_back(loc);
    configReader(rattr);
	reader_ = RTPSDomain::createRTPSReader(participant_, rattr, history_, &listener_);

	//Add remote writer (in this case a reader in the same machine)
    GUID_t guid = participant_->getGuid();
    addRemoteWriter(reader_, ip, port, guid);

    // Initialize list of msgs
    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        msgs_.push_back(count);
    }

    text_ = getText();
    domainId_ = (uint32_t)GET_PID();
    hostname_ = asio::ip::host_name();

    std::ostringstream word;
    word << text_ << "_" << hostname_ << "_" << domainId_;
    word_ = word.str();

    initialized_ = true;
}

void RTPSAsSocketReader::newNumber(uint16_t number)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::list<uint16_t>::iterator it = std::find(msgs_.begin(), msgs_.end(), number);
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

	ReaderHistory *history = reader->getHistory();

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
