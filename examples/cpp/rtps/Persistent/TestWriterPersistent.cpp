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
 * @file TestWriterPersistent.cpp
 *
 */

#include "TestWriterPersistent.h"

#include <chrono>
#include <thread>

#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/TopicAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;
using namespace std;

TestWriterPersistent::TestWriterPersistent()
    : mp_participant(nullptr)
    , mp_writer(nullptr)
    , mp_history(nullptr)
{


}

TestWriterPersistent::~TestWriterPersistent()
{
    RTPSDomain::removeRTPSParticipant(mp_participant);
    delete(mp_history);
}

bool TestWriterPersistent::init()
{
    //CREATE PARTICIPANT
    RTPSParticipantAttributes PParam;
    PParam.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SIMPLE;
    PParam.builtin.use_WriterLivelinessProtocol = true;
    mp_participant = RTPSDomain::createParticipant(0, PParam);
    if (mp_participant == nullptr)
    {
        return false;
    }

    //CREATE WRITERHISTORY
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 255;
    hatt.maximumReservedCaches = 50;
    mp_history = new WriterHistory(hatt);

    PropertyPolicy property_policy;
    property_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    property_policy.properties().emplace_back("dds.persistence.sqlite3.filename", "test.db");

    //CREATE WRITER
    WriterAttributes watt;
    watt.endpoint.reliabilityKind = BEST_EFFORT;
    watt.endpoint.durabilityKind = TRANSIENT;
    watt.endpoint.persistence_guid.guidPrefix.value[11] = 1;
    watt.endpoint.persistence_guid.entityId.value[3] = 1;
    watt.endpoint.properties = property_policy;
    std::cout << "PID: " << watt.endpoint.persistence_guid << std::endl;
    mp_writer = RTPSDomain::createRTPSWriter(mp_participant, watt, mp_history, &m_listener);
    if (mp_writer == nullptr)
    {
        return false;
    }

    return true;
}

bool TestWriterPersistent::reg()
{
    cout << "Registering Writer" << endl;
    TopicAttributes Tatt;
    Tatt.topicKind = NO_KEY;
    Tatt.topicDataType = "string";
    Tatt.topicName = "exampleTopic";
    eprosima::fastdds::dds::WriterQos Wqos;
    return mp_participant->registerWriter(mp_writer, Tatt, Wqos);
}

void TestWriterPersistent::run(
        uint16_t samples)
{
    cout << "Waiting for matched Readers" << endl;
    while (m_listener.n_matched == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    for (int i = 0; i < samples; ++i )
    {
        CacheChange_t* ch = mp_history->create_change(255, ALIVE);
        if (!ch)     // In the case history is full, remove some old changes
        {
            std::cout << "cleaning history...";
            mp_history->remove_min_change();
            ch = mp_history->create_change(255, ALIVE);
        }

#if defined(_WIN32)
        ch->serializedPayload.length =
                sprintf_s((char*)ch->serializedPayload.data, 255, "My example string %d", i) + 1;
#else
        ch->serializedPayload.length =
                snprintf((char*)ch->serializedPayload.data, 255, "My example string %d", i) + 1;
#endif // if defined(_WIN32)
        printf("Sending: %s\n", (char*)ch->serializedPayload.data);
        mp_history->add_change(ch);
    }
}
