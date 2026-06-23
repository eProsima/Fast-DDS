#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/network/NetworkFactory.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

extern "C" int LLVMFuzzerTestOneInput(
        const uint8_t* data,
        size_t size)
{
    if (size < 5)
    {
        return 0;
    }

    uint8_t choice = data[0] % 3;
    const uint8_t* fuzz_data = data + 1;
    size_t fuzz_size = size - 1;

    CDRMessage_t msg(0);
    msg.wraps = true;
    msg.buffer = const_cast<octet*>(fuzz_data);
    msg.length = static_cast<uint32_t>(fuzz_size);
    msg.max_size = static_cast<uint32_t>(fuzz_size);
    msg.reserved_size = static_cast<uint32_t>(fuzz_size);

    if (choice == 0)
    {
        RTPSParticipantAttributes PParam;
        NetworkFactory network(PParam);

        RTPSParticipantAllocationAttributes allocation;
        ParticipantProxyData pdata(allocation);

        // use_encapsulation = true will read first 4 bytes as encapsulation header
        pdata.read_from_cdr_message(&msg, true, network, false);
    }
    else if (choice == 1)
    {
        ReaderProxyData pdata(10, 10);
        pdata.read_from_cdr_message(&msg);
    }
    else
    {
        WriterProxyData pdata(10, 10);
        pdata.read_from_cdr_message(&msg);
    }

    return 0;
}
