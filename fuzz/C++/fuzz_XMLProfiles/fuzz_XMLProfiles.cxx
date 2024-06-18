#include "fuzz_utils.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

using namespace eprosima;
using namespace eprosima::fastdds;

static bool initialized = false;

extern "C" int LLVMFuzzerTestOneInput(
        const uint8_t* data,
        size_t size)
{
    if (!initialized)
    {
        ignore_stdout();
        initialized = true;
    }

    if (!size)
    {
        return EXIT_FAILURE;
    }

    fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_string(reinterpret_cast<const char*>(data), size);

    return 0;
}
