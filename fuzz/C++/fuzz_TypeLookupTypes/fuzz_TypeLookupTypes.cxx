#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypesPubSubTypes.hpp>
#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds::builtin;
using namespace eprosima::fastdds::rtps;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 4)
    {
        return 0;
    }

    SerializedPayload_t payload(static_cast<uint32_t>(size));
    memcpy(payload.data, data, size);
    payload.length = static_cast<uint32_t>(size);

    // Fuzz different TypeLookup structures
    uint8_t choice = data[0] % 3;

    if (choice == 0)
    {
        TypeLookup_getTypes_In data_obj;
        TypeLookup_getTypes_InPubSubType pubsub_type;
        pubsub_type.deserialize(payload, &data_obj);
    }
    else if (choice == 1)
    {
        TypeLookup_getTypeDependencies_In data_obj;
        TypeLookup_getTypeDependencies_InPubSubType pubsub_type;
        pubsub_type.deserialize(payload, &data_obj);
    }
    else
    {
        TypeLookup_getTypes_Out data_obj;
        TypeLookup_getTypes_OutPubSubType pubsub_type;
        pubsub_type.deserialize(payload, &data_obj);
    }

    return 0;
}
