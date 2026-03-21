// Copyright 2020 Canonical ltd.
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
 * @file LoggingLevel.h
 */
#ifndef _FASTDDS_RTPS_SECURITY_LOGGING_LOGGINGLEVEL_H_
#define _FASTDDS_RTPS_SECURITY_LOGGING_LOGGINGLEVEL_H_

#include <string>

#include <rtps/security/exceptions/SecurityException.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class SecurityException;

/**
 * @brief The LoggingLevel enum
 *
 * @note Definition in DDS-Sec v1.1 9.6
 */
enum struct LoggingLevel : long
{
    EMERGENCY_LEVEL,    // System is unusable. Should not continue use.
    ALERT_LEVEL,        // Should be corrected immediately
    CRITICAL_LEVEL,     // A failure in primary application.
    ERROR_LEVEL,        // General error conditions
    WARNING_LEVEL,      // May indicate future error if action not taken.
    NOTICE_LEVEL,       // Unusual, but nor erroneous event or condition.
    INFORMATIONAL_LEVEL, // Normal operational. Requires no action.
    DEBUG_LEVEL
};

bool string_to_LogLevel(
        const std::string& s,
        LoggingLevel& l,
        SecurityException& e);

bool LogLevel_to_string(
        const LoggingLevel l,
        std::string& s,
        SecurityException& e);

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _FASTDDS_RTPS_SECURITY_LOGGING_LOGGINGLEVEL_H_
