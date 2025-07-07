#ifndef FASTDDS_UTILS__DLOPEN_HPP
#define FASTDDS_UTILS__DLOPEN_HPP

#include "../fastdds_dll.hpp"

#include <string>

namespace eprosima {
namespace fastdds {

/**
 * @ingroup UTILITIES_MODULE
 */
class dlopen
{
public:

    FASTDDS_EXPORTED_API static void* open_library(
            const std::string& library);

    FASTDDS_EXPORTED_API static void* open_symbol(
            void* library,
            const std::string& symbol);
};

} /* namespace fastdds */
} /* namespace eprosima */

#endif // FASTDDS_UTILS__DLOPEN_HPP
