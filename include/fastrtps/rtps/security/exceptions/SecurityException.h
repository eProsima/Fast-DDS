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

#ifndef _RTPS_SECURITY_EXCEPTIONS_SECURITYEXCEPTION_H_
#define _RTPS_SECURITY_EXCEPTIONS_SECURITYEXCEPTION_H_

#include "fastrtps/fastrtps_dll.h"

#include <string>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

/**
 * @brief This class is thrown as an exception when there is an error in security module.
 * @ingroup EXCEPTIONMODULE
 */
class SecurityException
{

public:

    RTPS_DllAPI SecurityException() {}

    /// \brief Default constructor
    virtual RTPS_DllAPI ~SecurityException() throw() = default;

    /**
     * @brief Default copy constructor.
     * @param ex SecurityException that will be copied.
     */
    SecurityException(const SecurityException &ex) = delete;

    /**
     * @brief Default move constructor.
     * @param ex SecurityException that will be moved.
     */
    SecurityException(SecurityException&& ex) = delete;

    /**
     * @brief Assigment operation.
     * @param ex SecurityException that will be copied.
     */
    SecurityException& operator=(const SecurityException &ex) = delete;

    /**
     * @brief Assigment operation.
     * @param ex SecurityException that will be moved.
     */
    SecurityException& operator=(SecurityException&& ex) = delete;

    /**
     * @brief 
     */
    RTPS_DllAPI void set_msg(
            const std::string& msg)
    {
        msg_ = msg;
    }

    /**
     * @brief This function returns the error message.
     * @return The error message.
     */
    RTPS_DllAPI const char* const what() const throw()
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

} // namespace security
} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_SECURITY_EXCEPTIONS_SECURITYEXCEPTION_H_
