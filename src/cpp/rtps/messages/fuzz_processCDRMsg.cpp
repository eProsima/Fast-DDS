#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <fastrtps/rtps/messages/MessageReceiver.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>

#define MIN_SIZE 256
#define MAX_SIZE 64000

extern "C" int LLVMFuzzerTestOneInput(
        const uint8_t* data,
        size_t size)
{
    if (size < MIN_SIZE || size > MAX_SIZE)
        return 0;

    const eprosima::fastrtps::rtps::Locator_t remoteLocator;
    const eprosima::fastrtps::rtps::Locator_t recvLocator;
    eprosima::fastrtps::rtps::MessageReceiver* rcv = new eprosima::fastrtps::rtps::MessageReceiver(NULL, MAX_SIZE);

    eprosima::fastrtps::rtps::CDRMessage_t msg(0);
    msg.wraps = true;
    msg.buffer = const_cast<eprosima::fastrtps::rtps::octet*>(data);
    msg.length = size;
    msg.max_size = size;
    msg.reserved_size = size;

    // TODO: Should we unlock in case UnregisterReceiver is called from callback ?
    rcv->processCDRMsg(remoteLocator, recvLocator, &msg);
    delete rcv;
    return 0;
}
