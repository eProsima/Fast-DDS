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
 * @file RTPSAsSocketWriter.cpp
 *
 */

#include "RTPSAsSocketWriter.hpp"

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/history/WriterHistory.h>

#include <boost/interprocess/detail/os_thread_functions.hpp>
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4005)
#endif  // _MSC_VER
#include <boost/asio.hpp>
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER
#include <gtest/gtest.h>
#include <inttypes.h>

RTPSAsSocketWriter::RTPSAsSocketWriter(): participant_(nullptr),
    writer_(nullptr), history_(nullptr), initialized_(false)
{
}

RTPSAsSocketWriter::~RTPSAsSocketWriter()
{
    if(participant_ != nullptr)
        RTPSDomain::removeRTPSParticipant(participant_);
    if(history_ != nullptr)
	delete(history_);
}

void RTPSAsSocketWriter::init(std::string ip, uint32_t port, bool async)
{
	//Create participant
	RTPSParticipantAttributes pattr;
	pattr.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = false;
	pattr.builtin.use_WriterLivelinessProtocol = false;
    pattr.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
    pattr.participantID = 2;
	participant_ = RTPSDomain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

	//Create writerhistory
	HistoryAttributes hattr;
	hattr.payloadMaxSize = 255;
	history_ = new WriterHistory(hattr);

	//Create writer
	WriterAttributes wattr;
	Locator_t loc;
	loc.set_IP4_address(ip);
	loc.port = port;
	wattr.endpoint.multicastLocatorList.push_back(loc);
    configWriter(wattr);

    // Asynchronous
    if(async)
        wattr.mode = ASYNCHRONOUS_WRITER;

	writer_ = RTPSDomain::createRTPSWriter(participant_, wattr, history_);
    ASSERT_NE(writer_, nullptr);

	//Add remote reader (in this case a reader in the same machine)
    GUID_t guid = participant_->getGuid();
	RemoteReaderAttributes rattr;
	loc.set_IP4_address(ip);
	loc.port = port;
	rattr.endpoint.multicastLocatorList.push_back(loc);
    configRemoteReader(rattr, guid);
	writer_->matched_reader_add(rattr);

    text_ = getText();
    domainId_ = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id();
    hostname_ = boost::asio::ip::host_name();

    initialized_ = true;
}

void RTPSAsSocketWriter::send(const std::list<uint16_t> &msgs)
{
	for(std::list<uint16_t>::const_iterator it = msgs.begin(); it != msgs.end(); ++it)
	{
		CacheChange_t * ch = writer_->new_change(ALIVE);

#if defined(_WIN32)
		ch->serializedPayload.length =
            (uint16_t)sprintf_s((char*)ch->serializedPayload.data, 255, "%s_%s_%I32u %hu", text_.c_str(), hostname_.c_str(), domainId_, *it) + 1;
#else
		ch->serializedPayload.length =
			snprintf((char*)ch->serializedPayload.data, 255, "%s_%s_%" PRIu32 " %hu", text_.c_str(), hostname_.c_str(), domainId_, *it) + 1;
#endif
		history_->add_change(ch);
	}
}
