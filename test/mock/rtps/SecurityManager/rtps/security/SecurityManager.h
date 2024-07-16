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

/*!
 * @file SecurityPluginFactory.h
 */
#ifndef FASTDDS_RTPS_SECURITY__SECURITYMANAGER_H
#define FASTDDS_RTPS_SECURITY__SECURITYMANAGER_H

#include <gmock/gmock.h>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class SecurityManager
{
public:

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD4(discovered_reader, bool(
                const GUID_t& writer_guid,
                const GUID_t& remote_participant,
                ReaderProxyData& remote_reader_data,
                const EndpointSecurityAttributes& security_attributes));

    MOCK_METHOD3(remove_reader, void(
                const GUID_t& writer_guid,
                const GUID_t& remote_participant,
                const GUID_t& remote_reader_guid));

    MOCK_METHOD4(discovered_writer, bool(
                const GUID_t& reader_guid,
                const GUID_t& remote_participant,
                WriterProxyData& remote_writer_guid,
                const EndpointSecurityAttributes& security_attributes));

    MOCK_METHOD3(remove_writer, void(
                const GUID_t& reader_guid,
                const GUID_t& remote_participant,
                const GUID_t& remote_writer_guid));

    MOCK_METHOD2(check_guid_comes_from, bool(
                const GUID_t& reader_guid,
                const GUID_t& remote_participant));

    MOCK_CONST_METHOD1(get_identity_token, bool(
                IdentityToken** identity_token));

    MOCK_CONST_METHOD1(return_identity_token, bool(
                IdentityToken* identity_token));

    MOCK_CONST_METHOD1(get_permissions_token, bool(
                PermissionsToken** permissions_token));

    MOCK_CONST_METHOD1(return_permissions_token, bool(
                PermissionsToken* permissions_token));

    MOCK_METHOD1(remove_participant, void(const ParticipantProxyData& participant_data));

    MOCK_CONST_METHOD0(builtin_endpoints, fastdds::rtps::BuiltinEndpointSet_t());

    // *INDENT-ON*
};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_RTPS_SECURITY__SECURITYMANAGER_H
