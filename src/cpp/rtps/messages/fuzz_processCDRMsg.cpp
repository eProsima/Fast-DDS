#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <fastrtps/rtps/messages/MessageReceiver.h>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>

extern "C" int LLVMFuzzerTestOneInput(
        const uint8_t* data,
        size_t size)
{
    const eprosima::fastrtps::rtps::Locator_t remoteLocator;
    const eprosima::fastrtps::rtps::Locator_t recvLocator;
    eprosima::fastrtps::rtps::MessageReceiver* rcv = new eprosima::fastrtps::rtps::MessageReceiver(NULL, 4096);

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
