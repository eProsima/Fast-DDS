/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestReaderRegistered.cpp
 *
 */

#include "TestReaderRegistered.h"

#include "fastrtps/rtps/reader/RTPSReader.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"
#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"
#include "fastrtps/rtps/attributes/ReaderAttributes.h"
#include "fastrtps/rtps/attributes/HistoryAttributes.h"

#include "fastrtps/rtps/history/ReaderHistory.h"

#include "fastrtps/attributes/TopicAttributes.h"
#include "fastrtps/qos/ReaderQos.h"

using namespace eprosima;
using namespace fastrtps;

TestReaderRegistered::TestReaderRegistered():
mp_participant(nullptr),
mp_reader(nullptr),
mp_history(nullptr)
{


}

TestReaderRegistered::~TestReaderRegistered()
{
	RTPSDomain::removeRTPSParticipant(mp_participant);
	delete(mp_history);
}

bool TestReaderRegistered::init()
{
	//CREATE PARTICIPANT
	RTPSParticipantAttributes PParam;
	PParam.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.builtin.use_WriterLivelinessProtocol = true;
	mp_participant = RTPSDomain::createParticipant(PParam);
	if(mp_participant==nullptr)
		return false;
	//CREATE READERHISTORY
	HistoryAttributes hatt;
	hatt.payloadMaxSize = 255;
	mp_history = new ReaderHistory(hatt);

	//CREATE READER
	ReaderAttributes ratt;
	Locator_t loc(22222);
	ratt.endpoint.unicastLocatorList.push_back(loc);
	mp_reader = RTPSDomain::createRTPSReader(mp_participant,ratt,mp_history,&m_listener);
	if(mp_reader == nullptr)
		return false;

	return true;
}

bool TestReaderRegistered::reg()
{
	cout << "Registering Reader" << endl;
	TopicAttributes Tatt;
	Tatt.topicKind = NO_KEY;
	Tatt.topicDataType = "string";
	Tatt.topicName = "exampleTopic";
	ReaderQos Rqos;
	return mp_participant->registerReader(mp_reader, Tatt, Rqos);
}

void TestReaderRegistered::run()
{
	printf("Press Enter to stop the Reader.\n");
	std::cin.ignore();
}

void TestReaderRegistered::MyListener::onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const change)
{
	printf("Received: %s\n",change->serializedPayload.data);
	reader->getHistory()->remove_change((CacheChange_t*)change);
}
