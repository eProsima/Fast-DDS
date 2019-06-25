// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fastrtps/rtps/security/exceptions/SecurityException.h>
#include <cstring>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

RTPS_DllAPI SecurityException::SecurityException() throw()
{
    std::memset(msg_, 0, sizeof(msg_));
}

RTPS_DllAPI SecurityException& SecurityException::set_msg(
        const char* const msg) throw()
{
    snprintf(msg_, SECURITY_ERROR_MAX_LENGTH, "%s", msg);
    return *this;
}

RTPS_DllAPI SecurityException& SecurityException::set_msg(
        const char* const msg,
        unsigned long code) throw()
{
    snprintf(msg_, SECURITY_ERROR_MAX_LENGTH, "%s (%lu)", msg, code);
    return *this;
}

RTPS_DllAPI SecurityException& SecurityException::append_msg(
        const char* const msg) throw()
{
    snprintf(msg_, SECURITY_ERROR_MAX_LENGTH, "%s%s", msg_, msg);
    return *this;
}

} /* namespace security */
} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
