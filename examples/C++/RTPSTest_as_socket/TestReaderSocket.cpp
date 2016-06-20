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
