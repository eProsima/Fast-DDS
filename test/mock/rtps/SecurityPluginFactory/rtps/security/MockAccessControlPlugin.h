// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file MockAccessControlPlugin.h
 */

#ifndef FASTDDS_RTPS_SECURITY__MOCKACCESSCONTROLPLUGIN_H
#define FASTDDS_RTPS_SECURITY__MOCKACCESSCONTROLPLUGIN_H

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/security/accesscontrol/AccessControl.h>
#include <gmock/gmock.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class MockAccessControlPlugin : public AccessControl
{
public:

    using AccessPermissionsHandle = HandleImpl<AccessPermissions, MockAccessControlPlugin>;

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD(PermissionsHandle*, validate_local_permissions, (
                Authentication & auth_plugin,
                const IdentityHandle& identity,
                const uint32_t domain_id,
                const RTPSParticipantAttributes& participant_attr,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, get_permissions_token, (
                PermissionsToken** permissions_token,
                const PermissionsHandle& handle,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, return_permissions_token, (
                PermissionsToken * token,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, get_permissions_credential_token, (
                PermissionsCredentialToken** permissions_credential_token,
                const PermissionsHandle& handle,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, return_permissions_credential_token, (
                PermissionsCredentialToken * token,
                SecurityException & exception), (override));

    MOCK_METHOD(PermissionsHandle*, validate_remote_permissions, (
                Authentication & auth_plugin,
                const IdentityHandle& local_identity_handle,
                const PermissionsHandle& local_permissions_handle,
                const IdentityHandle& remote_identity_handle,
                const PermissionsToken& remote_permissions_token,
                const PermissionsCredentialToken& remote_credential_token,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, check_create_participant, (
                const PermissionsHandle& local_handle,
                const uint32_t domain_id,
                const RTPSParticipantAttributes& qos,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, check_remote_participant, (
                const PermissionsHandle& remote_handle,
                const uint32_t domain_id,
                const ParticipantProxyData&,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, check_create_datawriter, (
                const PermissionsHandle& local_handle,
                const uint32_t domain_id,
                const std::string& topic_name,
                const std::vector<std::string>& partitions,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, check_create_datareader, (
                const PermissionsHandle& local_handle,
                const uint32_t domain_id,
                const std::string& topic_name,
                const std::vector<std::string>& partitions,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, check_remote_datawriter, (
                const PermissionsHandle& remote_handle,
                const uint32_t domain_id,
                const WriterProxyData& publication_data,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, check_remote_datareader, (
                const PermissionsHandle& remote_handle,
                const uint32_t domain_id,
                const ReaderProxyData& subscription_data,
                bool& relay_only,
                SecurityException & exception), (override));
    MOCK_METHOD(bool, get_participant_sec_attributes, (
                const PermissionsHandle& local_handle,
                ParticipantSecurityAttributes & attributes,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, get_datawriter_sec_attributes, (
                const PermissionsHandle& permissions_handle,
                const std::string& topic_name,
                const std::vector<std::string>& partitions,
                EndpointSecurityAttributes & attributes,
                SecurityException & exception), (override));

    MOCK_METHOD(bool, get_datareader_sec_attributes, (
                const PermissionsHandle& permissions_handle,
                const std::string& topic_name,
                const std::vector<std::string>& partitions,
                EndpointSecurityAttributes & attributes,
                SecurityException & exception), (override));
    // *INDENT-ON*

    PermissionsHandle* get_permissions_handle(
            SecurityException&) override
    {
        return new (std::nothrow) AccessPermissionsHandle();
    }

    bool return_permissions_handle(
            PermissionsHandle* permissions_handle,
            SecurityException&) override
    {
        delete dynamic_cast<AccessPermissionsHandle*>(permissions_handle);
        return true;
    }

};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_RTPS_SECURITY__MOCKACCESSCONTROLPLUGIN_H
