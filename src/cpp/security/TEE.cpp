#include <fastrtps/rtps/security/common/TEE.h>

using namespace eprosima::fastrtps;

TEE_Init tee;

void TEE_Init::instanceOpenFailLog(int32_t error_code)
{
    logError(SECURITY_AUTHENTICATION,
             "Fatal: could not create dsec instance. Error: " << error_code);
}

void TEE_Init::instanceCloseFailLog(int32_t error_code)
{
    logWarning(SECURITY_AUTHENTICATION,
               "Could not close dsec instance. Error: " << error_code);
}
