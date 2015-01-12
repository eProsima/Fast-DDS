/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestWriterSocket.cpp
 *
 */

#include "TestWriterSocket.h"

#include "fastrtps/rtps/writer/RTPSWriter.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"
#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"
#include "fastrtps/rtps/attributes/WriterAttributes.h"
#include "fastrtps/rtps/attributes/HistoryAttributes.h"

#include "fastrtps/rtps/history/WriterHistory.h"


TestWriterSocket::TestWriterSocket():
mp_participant(nullptr),
mp_writer(nullptr),
mp_history(nullptr)
{


}

TestWriterSocket::~TestWriterSocket()
{
	RTPSDomain::removeRTPSParticipant(mp_participant);
	delete(mp_history);
}

bool TestWriterSocket::init(std::string ip, uint32_t port)
{
	//CREATE PARTICIPANT
	RTPSParticipantAttributes PParam;
	PParam.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = false;
	PParam.builtin.use_WriterLivelinessProtocol = false;
	mp_participant = RTPSDomain::createParticipant(PParam);
	if(mp_participant==nullptr)
		return false;

	//CREATE WRITERHISTORY
	HistoryAttributes hatt;
	hatt.payloadMaxSize = 255;
	mp_history = new WriterHistory(hatt);

	//CREATE WRITER
	WriterAttributes watt;
	watt.endpoint.reliabilityKind = BEST_EFFORT;
	mp_writer = RTPSDomain::createRTPSWriter(mp_participant,watt,mp_history);
	if(mp_writer == nullptr)
		return false;

	//ADD REMOTE READER (IN THIS CASE A READER IN THE SAME MACHINE)
	RemoteReaderAttributes ratt;
	Locator_t loc;
	loc.set_IP4_address(ip);
	loc.port = port;
	ratt.endpoint.multicastLocatorList.push_back(loc);
	mp_writer->matched_reader_add(ratt);
	return true;
}

void TestWriterSocket::run(uint16_t nmsgs)
{
	for(int i = 0;i<nmsgs;++i )
	{
		CacheChange_t * ch = mp_writer->new_change(ALIVE);
#if defined(_WIN32)
		ch->serializedPayload.length =
			sprintf_s((char*)ch->serializedPayload.data,255, "My example string %d", i)+1;
#else
		ch->serializedPayload.length =
			sprintf((char*)ch->serializedPayload.data,"My example string %d",i);
#endif
		printf("Sending: %s\n",(char*)ch->serializedPayload.data);
		mp_history->add_change(ch);
	}
}
