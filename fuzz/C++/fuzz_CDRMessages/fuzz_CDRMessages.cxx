#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <rtps/messages/CDRMessage.hpp>
#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/rtps/common/CacheChange.hpp>
#include <rtps/security/common/ParticipantGenericMessage.h>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 5)
    {
        return 0;
    }

    uint8_t choice = data[0] % 2;
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
        CacheChange_t change;
        uint32_t qos_size = 0;
        dds::ParameterList::updateCacheChangeFromInlineQos(change, &msg, qos_size);
    }
    else
    {
        security::ParticipantGenericMessage message;
        CDRMessage::readParticipantGenericMessage(&msg, message);
    }

    return 0;
}
