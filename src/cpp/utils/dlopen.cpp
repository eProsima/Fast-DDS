#include <fastrtps/utils/dlopen.hpp>

#if HAVE_DLOPEN
#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif // defined(_WIN32)
#endif // HAVE_DLOPEN

namespace eprosima {
namespace fastrtps {

void* dlopen::open_library(
        const std::string& library)
{
#if HAVE_DLOPEN
#if defined(_WIN32)
    return LoadLibrary (library.c_str());
#else
    return ::dlopen(library.c_str(), RTLD_LAZY);
#endif // defined(_WIN32)
#else
    return nullptr
#endif // HAVE_DLOPEN
}

void* dlopen::open_symbol(
        void* library,
        const std::string& symbol)
{
#if HAVE_DLOPEN
#if defined(_WIN32)
    return (void*)(intptr_t)GetProcAddress ((HINSTANCE)library, symbol.c_str());
#else
    return dlsym(library, symbol.c_str());
#endif // defined(_WIN32)
#else
    return nullptr
#endif // HAVE_DLOPEN
}

} // namespace fastrtps
} // namespace eprosima
