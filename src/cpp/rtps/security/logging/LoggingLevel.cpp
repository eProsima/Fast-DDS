// Copyright 2024 Canonical ltd.
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
 * @file LoggingLevel.cpp
 */
#include <rtps/security/logging/LoggingLevel.h>

#include <rtps/security/exceptions/SecurityException.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

bool string_to_LogLevel(
        const std::string& s,
        LoggingLevel& l,
        SecurityException& e)
{
    //TODO(artivis): use an array of const char to avoid strings?
    bool convert = true;
    if (!s.compare("0") || !s.compare("EMERGENCY_LEVEL"))
    {
        l = LoggingLevel::EMERGENCY_LEVEL;
    }
    else if (!s.compare("1") || !s.compare("ALERT_LEVEL"))
    {
        l = LoggingLevel::ALERT_LEVEL;
    }
    else if (!s.compare("2") || !s.compare("CRITICAL_LEVEL"))
    {
        l = LoggingLevel::CRITICAL_LEVEL;
    }
    else if (!s.compare("3") || !s.compare("ERROR_LEVEL"))
    {
        l = LoggingLevel::ERROR_LEVEL;
    }
    else if (!s.compare("4") || !s.compare("WARNING_LEVEL"))
    {
        l = LoggingLevel::WARNING_LEVEL;
    }
    else if (!s.compare("5") || !s.compare("NOTICE_LEVEL"))
    {
        l = LoggingLevel::NOTICE_LEVEL;
    }
    else if (!s.compare("6") || !s.compare("INFORMATIONAL_LEVEL"))
    {
        l = LoggingLevel::INFORMATIONAL_LEVEL;
    }
    else if (!s.compare("7") || !s.compare("DEBUG_LEVEL"))
    {
        l = LoggingLevel::DEBUG_LEVEL;
    }
    else
    {
        e = SecurityException("Unknown LoggingLevel");
        convert = false;
    }

    return convert;
}

bool LogLevel_to_string(
        const LoggingLevel l,
        std::string& s,
        SecurityException& e)
{
    bool convert = true;
    switch (l)
    {
        case LoggingLevel::EMERGENCY_LEVEL:
            s = "EMERGENCY";
            break;
        case LoggingLevel::ALERT_LEVEL:
            s = "ALERT";
            break;
        case LoggingLevel::CRITICAL_LEVEL:
            s = "CRITICAL";
            break;
        case LoggingLevel::ERROR_LEVEL:
            s = "ERROR";
            break;
        case LoggingLevel::WARNING_LEVEL:
            s = "WARNING";
            break;
        case LoggingLevel::NOTICE_LEVEL:
            s = "NOTICE";
            break;
        case LoggingLevel::INFORMATIONAL_LEVEL:
            s = "INFORMATIONAL";
            break;
        case LoggingLevel::DEBUG_LEVEL:
            s = "DEBUG";
            break;
        default:
            s = "UNKNOWN";
            convert = false;
            e = SecurityException("Unknown LoggingLevel");
            break;
    }

    return convert;
}

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima