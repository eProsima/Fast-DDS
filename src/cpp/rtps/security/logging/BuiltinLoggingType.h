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
 * @file BuiltinLoggingType.h
 */
#ifndef _FASTDDS_RTPS_SECURITY_LOGGING_BUILTINLOGGINGTYPE_H_
#define _FASTDDS_RTPS_SECURITY_LOGGING_BUILTINLOGGINGTYPE_H_

#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <rtps/security/logging/LoggingLevel.h>
#include <rtps/security/logging/NameValuePair.h>

#include <map>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

/**
 * @brief The BuiltinLoggingType struct
 *
 * @note Definition in DDS-Sec v1.1 9.6
 */
struct BuiltinLoggingType final
{
    octet facility;           // Set to 0x0A (10). Indicates sec/auth msgs
    LoggingLevel severity;
    rtps::Time_t timestamp;   // Since epoch 1970-01-01 00:00:00 +0000 (UTC)
    std::string hostname;     // IP host name of originator
    std::string hostip;       // IP address of originator
    std::string appname;      // Identify the device or application
    std::string procid;       // Process name/ID for syslog system
    std::string msgid;        // Identify the type of message
    std::string message;      // Free-form message
    // Note that certain string keys (SD-IDs) are reserved by IANA
    std::map<std::string, NameValuePairSeq> structured_data;
};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _FASTDDS_RTPS_SECURITY_LOGGING_BUILTINLOGGINGTYPE_H_
