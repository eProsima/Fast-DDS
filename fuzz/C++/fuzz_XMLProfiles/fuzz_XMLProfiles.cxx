#include <fastrtps/Domain.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "fuzz_utils.h"

using namespace eprosima;
using namespace eprosima::fastrtps;

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

    const char* filename = buf_to_file(data, size);

    if (filename == NULL)
    {
        return EXIT_FAILURE;
    }

    // TODO change this to a func. taking buf + len (or C string)
    // to avoid using `buf_to_file`
    xmlparser::XMLProfileManager::loadXMLFile(filename);

    if (delete_file(filename) != 0)
    {
        return EXIT_FAILURE;
    }

    return 0;
}
