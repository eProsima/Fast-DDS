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

#include <memory>

#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/RTPSDomain.h>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/attributes/HistoryAttributes.h>

#include <fastdds/rtps/history/ReaderHistory.h>

#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/low-bandwidth/HeaderReductionTransportDescriptor.h>
#include <fastdds/rtps/transport/low-bandwidth/PayloadCompressionTransportDescriptor.h>
#include <fastdds/rtps/transport/low-bandwidth/SourceTimestampTransportDescriptor.h>

using namespace eprosima;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

TestReaderSocket::TestReaderSocket()
    : mp_participant(nullptr)
    , mp_reader(nullptr)
    , mp_history(nullptr)
{


}

TestReaderSocket::~TestReaderSocket()
{
    fastrtps::rtps::RTPSDomain::removeRTPSParticipant(mp_participant);
    delete(mp_history);
}

static int callback_num = 0;

bool TestReaderSocket::init(
        std::string ip,
        uint32_t port)
{
    fastrtps::rtps::RTPSParticipantAttributes PParam;

    //PREPARE TRANSPORTS

    // Lowest level is UDPv4
    auto udpTr = std::make_shared<UDPv4TransportDescriptor>();

    // Timestamp on top of UDPv4
    auto tsDesc = std::make_shared<SourceTimestampTransportDescriptor>(udpTr);
    tsDesc->callback_parameter = &callback_num;
    tsDesc->callback = [](void* p, int32_t sender_time, int32_t my_time, uint32_t packet_len)
            {
                int* pNum = (int*)p;
                *pNum = *pNum + 1;

                std::cout << "Packet no " << *pNum << " of " << packet_len << " bytes sent at " << sender_time <<
                    " and received at " << my_time << std::endl;
            };

#if HAVE_ZLIB || HAVE_BZIP2
    // Compression on top of timestamp
    auto zDesc = std::make_shared<PayloadCompressionTransportDescriptor>(tsDesc);
    PParam.properties.properties().emplace_back(fastrtps::rtps::Property("rtps.payload_compression.compression_library",
            "AUTOMATIC"));

    // Header reduction on top of compression
    auto hrDesc = std::make_shared<HeaderReductionTransportDescriptor>(zDesc);
#else
    // Header reduction on top of timestamp
    auto hrDesc = std::make_shared<HeaderReductionTransportDescriptor>(tsDesc);
#endif
    PParam.properties.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.remove_version",
            "true"));
    PParam.properties.properties().emplace_back(fastrtps::rtps::Property("rtps.header_reduction.remove_vendor_id",
            "true"));
    PParam.properties.properties().emplace_back(fastrtps::rtps::Property(
                "rtps.header_reduction.submessage.combine_id_and_flags",
                "true"));
    PParam.properties.properties().emplace_back(fastrtps::rtps::Property(
                "rtps.header_reduction.submessage.compress_entitiy_ids",
                "16,16"));

    // Activate custom transport chain
    PParam.useBuiltinTransports = false;
    PParam.userTransports.push_back(hrDesc);

    //CREATE PARTICIPANT
    mp_participant = fastrtps::rtps::RTPSDomain::createParticipant(0, PParam);
    if (mp_participant == nullptr)
    {
        return false;
    }
    //CREATE READERHISTORY
    fastrtps::rtps::HistoryAttributes hatt;
    hatt.payloadMaxSize = 1024;
    mp_history = new fastrtps::rtps::ReaderHistory(hatt);

    //CREATE READER
    fastrtps::rtps::ReaderAttributes ratt;
    fastrtps::rtps::Locator_t loc;
    fastrtps::rtps::IPLocator::setIPv4(loc, ip);
    loc.port = static_cast<uint16_t>(port);
    ratt.endpoint.multicastLocatorList.push_back(loc);
    mp_reader = fastrtps::rtps::RTPSDomain::createRTPSReader(mp_participant, ratt, mp_history, &m_listener);
    if (mp_reader == nullptr)
    {
        return false;
    }

    return true;
}

void TestReaderSocket::run()
{
    printf("Enter number to stop reader.\n");
    int aux;
    std::cin >> aux;
}

void TestReaderSocket::MyListener::onNewCacheChangeAdded(
        fastrtps::rtps::RTPSReader* reader,
        const fastrtps::rtps::CacheChange_t* const change)
{
    printf("Received %u bytes:", change->serializedPayload.length);
    for (uint32_t i = 0; i < change->serializedPayload.length; i++)
    {
        if ((i & 15) == 0)
        {
            printf("\n%06x  ", i);
        }
        printf(" %02x", change->serializedPayload.data[i]);
    }
    printf("\n");
    reader->getHistory()->remove_change((fastrtps::rtps::CacheChange_t*)change);
    m_received++;
}
