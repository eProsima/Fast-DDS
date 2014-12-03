/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestWriter.cpp
 *
 */

#include "TestWriter.h"

#include "fastrtps/rtps/writer/RTPSWriter.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"
#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"
#include "fastrtps/rtps/attributes/WriterAttributes.h"
#include "fastrtps/rtps/attributes/HistoryAttributes.h"

#include "fastrtps/rtps/history/WriterHistory.h"

TestWriter::TestWriter():
mp_participant(nullptr),
mp_writer(nullptr),
mp_history(nullptr)
{


}

TestWriter::~TestWriter()
{
	RTPSDomain::removeRTPSParticipant(mp_participant);
	delete(mp_history);
}

bool TestWriter::init()
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
	loc.set_IP4_address(127,0,0,1);
	loc.port = 22222; //MUSE BE THE SAME AS THE READER
	ratt.endpoint.unicastLocatorList.push_back(loc);
	mp_writer->matched_reader_add(ratt);
	return true;
}

void TestWriter::run()
{
	for(int i = 0;i<10;++i )
	{
		CacheChange_t * ch = mp_writer->new_change(ALIVE);
		ch->serializedPayload.length =
				sprintf((char*)ch->serializedPayload.data,"My example string %d",i);
		printf("Sending: %s\n",(char*)ch->serializedPayload.data);
		mp_history->add_change(ch);
	}
}
