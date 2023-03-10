#ifndef _FASTRTPS_UTILS_DLOPEN_
#define _FASTRTPS_UTILS_DLOPEN_

#include "../fastrtps_dll.h"

#include <string>

namespace eprosima {
namespace fastrtps {

/**
 * @ingroup UTILITIES_MODULE
 */
class dlopen
{
public:

    RTPS_DllAPI static void* open_library(
            const std::string& library);

    RTPS_DllAPI static void* open_symbol(
            void* library,
            const std::string& symbol);
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif // _FASTRTPS_UTILS_DLOPEN_
