#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <fastrtps/rtps/messages/MessageReceiver.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include "fastrtps/rtps/reader/RTPSReader.h"
#include "fastrtps/rtps/writer/RTPSWriter.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"
#include "fastrtps/rtps/RTPSDomain.h"

#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"
#include "fastrtps/rtps/attributes/ReaderAttributes.h"
#include "fastrtps/rtps/attributes/HistoryAttributes.h"

#include "fastrtps/rtps/history/ReaderHistory.h"
#include "fastrtps/rtps/history/WriterHistory.h"
#include "fastrtps/utils/IPLocator.h"
#include "fastrtps/rtps/builtin/data/ReaderProxyData.h"

#define MIN_SIZE 256
#define MAX_SIZE 4096 // max size of the entire RTPS blob, including header
#define REMO_IP "127.0.0.3"
#define REMO_PORT 17410
#define RECV_IP "239.255.1.4"
#define RECV_PORT 7410

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#define EMULATE_WRITER

#ifdef EMULATE_WRITER
/***********************************************************************************************
 *
 * Writer (emulated)
 * 
 ************************************************************************************************/
static Locator_t writer_locator;
static RTPSWriter* writer;
static RTPSParticipant* writer_participant;
static WriterHistory* writer_history;
#endif

/***********************************************************************************************
 *
 * Reader
 * 
 ************************************************************************************************/
static RTPSParticipant *reader_participant;
static RTPSReader *reader;
static ReaderHistory *reader_history;
static Locator_t reader_locator;
static MessageReceiver *receiver;

extern "C" bool Init()
{
    using namespace eprosima::fastrtps;
    using namespace eprosima::fastrtps::rtps;

    // History
    HistoryAttributes hatt;
    hatt.payloadMaxSize = MAX_SIZE; // TODO should this be less than MAX_SIZE?

    /***********************************************************************************************
     *
     * Reader
     * 
     ************************************************************************************************/
    // Participant
    RTPSParticipantAttributes receiver_param;
    receiver_param.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::NONE;
    receiver_param.builtin.use_WriterLivelinessProtocol = false;
    reader_participant = RTPSDomain::createParticipant(0, receiver_param);
    if (reader_participant == nullptr)
    {
        return false;
    }

    // History
    reader_history = new ReaderHistory(hatt);

    // Reader
    ReaderAttributes ratt;
    ratt.endpoint.setEntityID(3);
    IPLocator::setIPv4(reader_locator, RECV_IP);
    reader_locator.port = static_cast<uint16_t>(RECV_PORT);
    ratt.endpoint.multicastLocatorList.push_back(reader_locator);
    reader = RTPSDomain::createRTPSReader(reader_participant, ratt, reader_history);
    reader->enableMessagesFromUnkownWriters(true);
    if (reader == nullptr)
    {
        return false;
    }

    // Receiver
    receiver = new MessageReceiver((
        RTPSParticipantImpl *)reader_participant, MAX_SIZE);

    if (receiver == nullptr)
    {
        return false;
    }

    return true;

#ifdef EMULATE_WRITER
    /***********************************************************************************************
     *
     * Writer (emulated)
     * 
     ************************************************************************************************/
    // Participant
    RTPSParticipantAttributes writer_param;
    writer_param.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::NONE;
    writer_param.builtin.use_WriterLivelinessProtocol = false;
    writer_participant = RTPSDomain::createParticipant(0, writer_param);
    if (writer_participant == nullptr)
    {
        return false;
    }

    // History
    writer_history = new WriterHistory(hatt);

    // Writer
    WriterAttributes watt;
    watt.endpoint.reliabilityKind = BEST_EFFORT;
    writer = RTPSDomain::createRTPSWriter(writer_participant, watt, writer_history);
    if (writer == nullptr)
    {
        return false;
    }

    //ADD REMOTE READER (IN THIS CASE A READER IN THE SAME MACHINE)
    ReaderProxyData pratt(4u, 1u);
    pratt.guid({c_GuidPrefix_Unknown, 0x304});
    IPLocator::setIPv4(writer_locator, REMO_IP);
    writer_locator.port = static_cast<uint16_t>(REMO_PORT);
    pratt.add_unicast_locator(writer_locator);
    writer->matched_reader_add(pratt);
#endif
}

#define MIN_SIZE 256
#define MAX_SIZE 64000

extern "C" int LLVMFuzzerTestOneInput(
    const uint8_t *data,
    size_t size)
{
    using namespace eprosima::fastrtps;
    using namespace eprosima::fastrtps::rtps;

    static bool Initialized = Init();

    if (size < MIN_SIZE || size > MAX_SIZE)
        return 0;

    CDRMessage_t msg(0);
    msg.wraps = true;
    msg.buffer = const_cast<octet *>(data);
    msg.length = size;
    msg.max_size = size;
    msg.reserved_size = size;

    receiver->processCDRMsg(writer_locator, reader_locator, &msg);

    return 0;
}
