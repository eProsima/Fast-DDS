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

constexpr std::size_t SECURITY_ERROR_MAX_LENGTH = 512;

/**
 * @brief This class is thrown as an exception when there is an error in security module.
 * @ingroup EXCEPTIONMODULE
 */
class SecurityException
{

public:

    /**
     * @brief Default constructor
     */
    RTPS_DllAPI SecurityException() throw();

    /**
     * @brief Default destructor
     */
    virtual RTPS_DllAPI ~SecurityException() throw() = default;

    /**
     * @brief Default copy constructor (deleted).
     * @param ex SecurityException that will be copied.
     */
    SecurityException(const SecurityException& ex) = delete;

    /**
     * @brief Default move constructor (deleted).
     * @param ex SecurityException that will be moved.
     */
    SecurityException(SecurityException&& ex) = delete;

    /**
     * @brief Assigment operation (deleted).
     * @param ex SecurityException that will be copied.
     */
    SecurityException& operator=(const SecurityException& ex) = delete;

    /**
     * @brief Assigment operation (deleted).
     * @param ex SecurityException that will be moved.
     */
    SecurityException& operator=(SecurityException&& ex) = delete;

    /**
     * @brief Set message value.
     * @param msg String with new message value.
     * @return own reference.
     */
    RTPS_DllAPI SecurityException& set_msg(
            const char* const msg) throw();

    /**
     * @brief Set message value.
     * @param msg String with new message value.
     * @param code Error code.
     * @return own reference.
     */
    RTPS_DllAPI SecurityException& set_msg(
            const char* const msg,
            unsigned long code) throw();

    /**
     * @brief Set message value.
     * @param msg String with message to append.
     * @return own reference.
     */
    RTPS_DllAPI SecurityException& append_msg(
            const char* const msg) throw();

    /**
     * @brief This function returns the error message.
     * @return The error message.
     */
    RTPS_DllAPI const char* what() const throw()
    {
        return msg_;
    }

private:
    char msg_[SECURITY_ERROR_MAX_LENGTH];
};

} // namespace security
} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_SECURITY_EXCEPTIONS_SECURITYEXCEPTION_H_
