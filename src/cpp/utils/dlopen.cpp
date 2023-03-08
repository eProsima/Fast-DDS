#include <fastrtps/utils/dlopen.hpp>

#include <dlfcn.h>

namespace eprosima {
namespace fastrtps {

void* dlopen::open_library(
        const std::string& library)
{
#if HAVE_DLOPEN
    return ::dlopen(library.c_str(), RTLD_LAZY);
#else
    return nullptr
#endif
}

void* dlopen::open_symbol(
        void* library,
        const std::string& symbol)
{
#if HAVE_DLOPEN
    return dlsym(library, symbol.c_str());
#else
    return nullptr
#endif
}

} // namespace fastrtps
} // namespace eprosima
