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
 * @file TestReaderRegistered.cpp
 *
 */

#include "TestReaderRegistered.h"

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/TopicAttributes.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/RTPSDomain.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TestReaderRegistered::TestReaderRegistered()
    : mp_participant(nullptr)
    , mp_reader(nullptr)
    , mp_history(nullptr)
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
    PParam.builtin.discovery_config.discoveryProtocol = eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE;
    PParam.builtin.use_WriterLivelinessProtocol = true;
    mp_participant = RTPSDomain::createParticipant(0, PParam);
    if (mp_participant == nullptr)
    {
        return false;
    }
    //CREATE READERHISTORY
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 255;
    mp_history = new ReaderHistory(hatt);

    //CREATE READER
    ReaderAttributes ratt;
    Locator_t loc(22222);
    ratt.endpoint.unicastLocatorList.push_back(loc);
    mp_reader = RTPSDomain::createRTPSReader(mp_participant, ratt, mp_history, &m_listener);
    if (mp_reader == nullptr)
    {
        return false;
    }

    return true;
}

bool TestReaderRegistered::reg()
{
    std::cout << "Registering Reader" << std::endl;
    TopicAttributes Tatt;
    Tatt.topicKind = NO_KEY;
    Tatt.topicDataType = "string";
    Tatt.topicName = "exampleTopic";
    eprosima::fastdds::dds::ReaderQos Rqos;
    return mp_participant->registerReader(mp_reader, Tatt, Rqos);
}

void TestReaderRegistered::run()
{
    printf("Press Enter to stop the Reader.\n");
    std::cin.ignore();
}

void TestReaderRegistered::MyListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change)
{
    printf("Received: %s\n", change->serializedPayload.data);
    reader->get_history()->remove_change((CacheChange_t*)change);
    n_received++;
}
