#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>

#include <MessageReceiver.h>

#define MIN_SIZE RTPSMESSAGE_HEADER_SIZE
#define MAX_SIZE 64000

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

static const Locator_t remoteLocator;
static const Locator_t recvLocator;

extern "C" int LLVMFuzzerTestOneInput(
        const uint8_t* data,
        size_t size)
{
    if (size < MIN_SIZE || size > MAX_SIZE)
    {
        return 0;
    }

    MessageReceiver* rcv = new MessageReceiver(NULL, MAX_SIZE);

    CDRMessage_t msg(0);
    msg.wraps = true;
    msg.buffer = const_cast<octet*>(data);
    msg.length = size;
    msg.max_size = size;
    msg.reserved_size = size;

    // TODO: Should we unlock in case UnregisterReceiver is called from callback ?
    rcv->processCDRMsg(remoteLocator, recvLocator, &msg);
    delete rcv;

    return 0;
}
