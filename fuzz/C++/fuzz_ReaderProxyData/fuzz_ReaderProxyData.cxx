#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>

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

    ReaderProxyData pdata(10, 10);

    CDRMessage_t msg(0);
    msg.wraps = true;
    msg.buffer = const_cast<octet*>(data);
    msg.length = static_cast<uint32_t>(size);
    msg.max_size = static_cast<uint32_t>(size);
    msg.reserved_size = static_cast<uint32_t>(size);

    // bool read_from_cdr_message(CDRMessage_t* msg, fastdds::rtps::VendorId_t source_vendor_id = c_VendorId_eProsima);
    pdata.read_from_cdr_message(&msg);

    return 0;
}
