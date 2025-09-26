// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef __SECURITY_ACCESSCONTROL_LICENSEPARSER_H__
#define __SECURITY_ACCESSCONTROL_LICENSEPARSER_H__

#include <security/accesscontrol/CommonParser.h>

#include <string>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {


enum class VerifyResult
{
    kOk,
    kNoLicense,
    kUntrusted,
    kExpired,
    kInvalidSig,
    kBadPayload,
    kInternalError
};

struct LicenseInfo {
    std::string product_id;
    std::string license_id;
    std::string license_version;
    std::string holder;
    std::string issued_at; // ISO8601
    std::string not_before;
    std::string expires_at;
    std::string usage; // subscription/trial
    uint32_t running_time; // minutes;
};

/*
struct DomainAccessRules
{
    std::vector<DomainRule> rules;
};*/
/*struct LicenseAccessInfo
{
    std::vector<LicenseInfo> rules;
};*/

class LicenseParser
{
public:

    bool parse_stream(
            const char* stream,
            size_t stream_length);

    void swap(
            //LicenseAccessInfo& info);
            LicenseInfo& info);

private:

    LicenseInfo access_info_;
};

} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // __SECURITY_ACCESSCONTROL_LICENSEPARSER_H__
