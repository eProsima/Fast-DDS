// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file
 * This file is based on the DDS-PSM-CXX Exception.hpp file.
 */

#ifndef _FASTDDS_DDS_XTYPES_EXCEPTION_EXCEPTION_HPP_
#define _FASTDDS_DDS_XTYPES_EXCEPTION_EXCEPTION_HPP_

#include <stdexcept>
#include <string>

#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes1_3 {

/**
 * @brief Exception: base class for specified DDS Exceptions.
 */
class Exception
{
protected:

    Exception() = default;

public:

    RTPS_DllAPI virtual ~Exception() throw() = default;

    /**
     * @brief Retrieve information about the exception that was thrown.
     *
     * @return Exception information.
     */
    RTPS_DllAPI virtual const char* what() const throw() = 0;

};

/**
 * @brief Application is passing an invalid argument.
 */
class InvalidArgumentError : public Exception, public std::invalid_argument
{
public:

    RTPS_DllAPI explicit InvalidArgumentError(
            const std::string& msg);

    RTPS_DllAPI InvalidArgumentError(
            const InvalidArgumentError& src);

    RTPS_DllAPI virtual ~InvalidArgumentError() throw();

    RTPS_DllAPI virtual const char* what() const throw();

};

} // xtypes1_3
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_EXCEPTION_EXCEPTION_HPP_
