#include <fastrtps/utils/dlopen.hpp>

#include <dlfcn.h>

namespace eprosima {
namespace fastrtps {

void* dlopen::open_library(
        const std::string& library)
{
    return ::dlopen(library.c_str(), RTLD_LAZY);
}

void* dlopen::open_symbol(
        void* library,
        const std::string& symbol)
{
    return dlsym(library, symbol.c_str());
}

} // namespace fastrtps
} // namespace eprosima
