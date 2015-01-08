/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
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

#include "fastrtps/attributes/TopicAttributes.h"
#include "fastrtps/qos/WriterQos.h"

#include "fastrtps/utils/eClock.h"

using namespace eprosima;
using namespace fastrtps;


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
	PParam.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.builtin.use_WriterLivelinessProtocol = true;
	mp_participant = RTPSDomain::createParticipant(PParam);
	if(mp_participant==nullptr)
		return false;

	//CREATE WRITERHISTORY
	HistoryAttributes hatt;
	hatt.payloadMaxSize = 255;
	mp_history = new WriterHistory(hatt);

	//CREATE WRITER
	WriterAttributes watt;
	mp_writer = RTPSDomain::createRTPSWriter(mp_participant,watt,mp_history,&m_listener);
	if(mp_writer == nullptr)
		return false;

	return true;
}

bool TestWriter::reg()
{
	cout << "Registering Writer" << endl;
	TopicAttributes Tatt;
	Tatt.topicKind = NO_KEY;
	Tatt.topicDataType = "string";
	Tatt.topicName = "exampleTopic";
	WriterQos Wqos;
	return mp_participant->registerWriter(mp_writer, Tatt, Wqos);
}


void TestWriter::run()
{
	cout << "Waiting for matched Readers" << endl;
	while (m_listener.n_matched==0)
	{
		eClock::my_sleep(250);
	}

	for(int i = 0;i<10;++i )
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
