#include <fastrtps/utils/System.h>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

namespace eprosima {
namespace fastrtps {

int System::GetPID()
{
#if defined(__cplusplus_winrt)
    return (int)GetCurrentProcessId();
#elif defined(_WIN32)
    return (int)_getpid();
#else
    return (int)getpid();
#endif
}

}
}