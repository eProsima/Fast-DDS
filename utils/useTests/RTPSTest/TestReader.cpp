/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestReader.cpp
 *
 */

#include "TestReader.h"

#include "fastrtps/rtps/reader/RTPSReader.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"
#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"
#include "fastrtps/rtps/attributes/ReaderAttributes.h"
#include "fastrtps/rtps/attributes/HistoryAttributes.h"

#include "fastrtps/rtps/history/ReaderHistory.h"


TestReader::TestReader():
mp_participant(nullptr),
mp_reader(nullptr),
mp_history(nullptr)
{


}

TestReader::~TestReader()
{
	RTPSDomain::removeRTPSParticipant(mp_participant);
	delete(mp_history);
}

bool TestReader::init()
{
	//CREATE PARTICIPANT
	RTPSParticipantAttributes PParam;
	PParam.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = false;
	PParam.builtin.use_WriterLivelinessProtocol = false;
	mp_participant = RTPSDomain::createParticipant(PParam);
	if(mp_participant==nullptr)
		return false;
	//CREATE READERHISTORY
	HistoryAttributes hatt;
	hatt.payloadMaxSize = 255;
	mp_history = new ReaderHistory(hatt);

	ReaderAttributes ratt;
	Locator_t loc(22222);
	ratt.endpoint.unicastLocatorList.push_back(loc);
	mp_reader = RTPSDomain::createRTPSReader(mp_participant,ratt,mp_history,&m_listener);
	if(mp_reader == nullptr)
		return false;

	return true;
}

void TestReader::run()
{
	printf("Enter number to stop reader.\n");
	int aux;
	std::cin >> aux;
}

void TestReader::MyListener::onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const change)
{
	printf("Received: %s\n",change->serializedPayload.data);
	reader->getHistory()->remove_change((CacheChange_t*)change);
}
