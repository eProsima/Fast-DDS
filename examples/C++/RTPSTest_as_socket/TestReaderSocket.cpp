/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestReaderSocket.cpp
 *
 */

#include "TestReaderSocket.h"

#include "fastrtps/rtps/reader/RTPSReader.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"
#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"
#include "fastrtps/rtps/attributes/ReaderAttributes.h"
#include "fastrtps/rtps/attributes/HistoryAttributes.h"

#include "fastrtps/rtps/history/ReaderHistory.h"


TestReaderSocket::TestReaderSocket():
mp_participant(nullptr),
mp_reader(nullptr),
mp_history(nullptr)
{


}

TestReaderSocket::~TestReaderSocket()
{
	RTPSDomain::removeRTPSParticipant(mp_participant);
	delete(mp_history);
}

bool TestReaderSocket::init(std::string ip, uint32_t port)
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

	//CREATE READER
	ReaderAttributes ratt;
	Locator_t loc;
	loc.set_IP4_address(ip);
	loc.port = port;
	ratt.endpoint.multicastLocatorList.push_back(loc);
	mp_reader = RTPSDomain::createRTPSReader(mp_participant,ratt,mp_history,&m_listener);
	if(mp_reader == nullptr)
		return false;

	return true;
}

void TestReaderSocket::run()
{
	printf("Enter number to stop reader.\n");
	int aux;
	std::cin >> aux;
}

void TestReaderSocket::MyListener::onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const change)
{
	printf("Received: %s\n",change->serializedPayload.data);
	reader->getHistory()->remove_change((CacheChange_t*)change);
	m_received++;
}
