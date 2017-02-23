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
 * @file TestWriterRegistered.cpp
 *
 */

#include "TestWriterRegistered.h"

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
#include "fastrtps/utils/TimeConversion.h"

using namespace eprosima;
using namespace fastrtps;


TestWriterRegistered::TestWriterRegistered():
mp_participant(nullptr),
mp_writer(nullptr),
mp_history(nullptr)
{


}

TestWriterRegistered::~TestWriterRegistered()
{
    RTPSDomain::removeRTPSParticipant(mp_participant);
    delete(mp_history);
}

bool TestWriterRegistered::init()
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
    watt.endpoint.reliabilityKind = RELIABLE;
    watt.times.heartbeatPeriod.seconds = 5;
    mp_writer = RTPSDomain::createRTPSWriter(mp_participant,watt,mp_history,&m_listener);
    if(mp_writer == nullptr)
        return false;

    return true;
}

bool TestWriterRegistered::reg()
{
    cout << "Registering Writer" << endl;
    TopicAttributes Tatt;
    Tatt.topicKind = NO_KEY;
    Tatt.topicDataType = "string";
    Tatt.topicName = "exampleTopic";
    WriterQos Wqos;
    Wqos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    return mp_participant->registerWriter(mp_writer, Tatt, Wqos);
}


void TestWriterRegistered::run(uint16_t samples)
{
    cout << "Waiting for matched Readers" << endl;
    while (m_listener.n_matched==0)
    {
        eClock::my_sleep(250);
    }
    std::cout << "Enter to send messages"<<std::endl;
    std::cin.ignore();
    for(int i = 0;i<samples;++i )
    {
        CacheChange_t * ch = mp_writer->new_change(ALIVE);
#if defined(_WIN32)
        ch->serializedPayload.length =
            sprintf_s((char*)ch->serializedPayload.data,255, "My example string %d", i)+1;
#else
        ch->serializedPayload.length =
            sprintf((char*)ch->serializedPayload.data,"My example string %d",i)+1;
#endif
        printf("Sending: %s\n",(char*)ch->serializedPayload.data);
        mp_history->add_change(ch);
        if(i == 3)
            mp_participant->loose_next_change();
    }
    std::cin.ignore();
}
