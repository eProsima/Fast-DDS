#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/network/NetworkFactory.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

extern "C" int LLVMFuzzerTestOneInput(
        const uint8_t* data,
        size_t size)
{
    if (size < 4)
    {
        return 0;
    }

    RTPSParticipantAttributes PParam;
    NetworkFactory network(PParam);

    RTPSParticipantAllocationAttributes allocation;
    ParticipantProxyData pdata(allocation);

    CDRMessage_t msg(0);
    msg.wraps = true;
    msg.buffer = const_cast<octet*>(data);
    msg.length = static_cast<uint32_t>(size);
    msg.max_size = static_cast<uint32_t>(size);
    msg.reserved_size = static_cast<uint32_t>(size);

    // use_encapsulation = true will read first 4 bytes as encapsulation header
    pdata.read_from_cdr_message(&msg, true, network, false);

    return 0;
}
