#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <rtps/messages/CDRMessage.hpp>
#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 4)
    {
        return 0;
    }

    CDRMessage_t msg(0);
    msg.wraps = true;
    msg.buffer = const_cast<octet*>(data);
    msg.length = static_cast<uint32_t>(size);
    msg.max_size = static_cast<uint32_t>(size);
    msg.reserved_size = static_cast<uint32_t>(size);

    CacheChange_t change;
    uint32_t qos_size = 0;
    
    dds::ParameterList::updateCacheChangeFromInlineQos(change, &msg, qos_size);

    return 0;
}
