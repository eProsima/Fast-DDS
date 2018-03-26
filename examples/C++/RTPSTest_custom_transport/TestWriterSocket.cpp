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
 * @file TestWriterSocket.cpp
 *
 */

#include "TestWriterSocket.h"

#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/RTPSDomain.h>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/builtin//data//ReaderProxyData.h>

#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/low-bandwidth/HeaderReductionTransportDescriptor.h>
#include <fastdds/rtps/transport/low-bandwidth/PayloadCompressionTransportDescriptor.h>
#include <fastdds/rtps/transport/low-bandwidth/SourceTimestampTransportDescriptor.h>

using namespace eprosima;
using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

TestWriterSocket::TestWriterSocket()
    : mp_participant(nullptr)
    , mp_writer(nullptr)
    , mp_history(nullptr)
{


}

TestWriterSocket::~TestWriterSocket()
{
    fastrtps::rtps::RTPSDomain::removeRTPSParticipant(mp_participant);
    delete(mp_history);
}

static int callback_num = 0;

bool TestWriterSocket::init(
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

    //CREATE WRITERHISTORY
    fastrtps::rtps::HistoryAttributes hatt;
    hatt.payloadMaxSize = 255;
    mp_history = new fastrtps::rtps::WriterHistory(hatt);

    //CREATE WRITER
    fastrtps::rtps::WriterAttributes watt;
    watt.endpoint.reliabilityKind = fastrtps::rtps::BEST_EFFORT;
    mp_writer = fastrtps::rtps::RTPSDomain::createRTPSWriter(mp_participant, watt, mp_history);
    if (mp_writer == nullptr)
    {
        return false;
    }

    //ADD REMOTE READER (IN THIS CASE A READER IN THE SAME MACHINE)
    fastrtps::rtps::ReaderProxyData ratt(4u, 1u);
    ratt.guid({fastrtps::rtps::c_GuidPrefix_Unknown, 0x304});
    fastrtps::rtps::Locator_t loc;
    fastrtps::rtps::IPLocator::setIPv4(loc, ip);
    loc.port = static_cast<uint16_t>(port);
    ratt.add_multicast_locator(loc);
    mp_writer->matched_reader_add(ratt);
    return true;
}

void TestWriterSocket::run(
        uint16_t nmsgs)
{
    for (int i = 0; i < nmsgs; ++i )
    {
        fastrtps::rtps::CacheChange_t* ch = mp_writer->new_change([]() -> uint32_t {
            return 255;
        }, fastrtps::rtps::ALIVE);
#if defined(_WIN32)
        ch->serializedPayload.length =
                sprintf_s((char*)ch->serializedPayload.data, 255, "My example string %d", i) + 1;
#else
        ch->serializedPayload.length =
                sprintf((char*)ch->serializedPayload.data, "My example string %d", i) + 1;
#endif
        printf("Sending: %s\n", (char*)ch->serializedPayload.data);
        mp_history->add_change(ch);
    }
}
