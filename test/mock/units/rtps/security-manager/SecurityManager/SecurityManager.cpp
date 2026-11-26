// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of SecurityManager: replaces src/cpp/rtps/security/SecurityManager.cpp. Remote entities are
// not validated, no token is provided and no builtin secure endpoint is announced. Its construction and destruction,
// done by the participant owning it, initialize nothing else than its members.

#include <fastdds/config.hpp>

#if HAVE_SECURITY

#include <rtps/security/SecurityManager.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

SecurityManager::SecurityManager(
        RTPSParticipantImpl* participant,
        const RTPSParticipantAttributes& pattr,
        ISecurityPluginFactory& plugin_factory)
    : participant_stateless_message_listener_(*this)
    , participant_volatile_message_secure_listener_(*this)
    , participant_(participant)
    , factory_(plugin_factory)
    , domain_id_(0)
    , max_discovered_participants_(pattr.allocation.participants.maximum)
    , auth_last_sequence_number_(1)
    , crypto_last_sequence_number_(1)
    , temp_reader_proxies_({
                pattr.allocation.locators.max_unicast_locators,
                pattr.allocation.locators.max_multicast_locators,
                pattr.allocation.data_limits,
                pattr.allocation.content_filter})
    , temp_writer_proxies_({
                pattr.allocation.locators.max_unicast_locators,
                pattr.allocation.locators.max_multicast_locators,
                pattr.allocation.data_limits})
{
}

SecurityManager::~SecurityManager()
{
}

SecurityManager::AuthenticationHandshakeProperties::AuthenticationHandshakeProperties()
    : max_handshake_requests_(10)
    , initial_handshake_resend_period_ms_(125)
    , handshake_resend_period_gain_(1.5)
{
}

bool SecurityManager::on_revoke_identity(
        Authentication&,
        const IdentityHandle&,
        SecurityException&)
{
    return true;
}

bool SecurityManager::on_revoke_permissions(
        AccessControl&,
        const PermissionsHandle&,
        SecurityException&)
{
    return true;
}

void SecurityManager::on_writer_change_received_by_all(
        RTPSWriter*,
        CacheChange_t*)
{
}

void SecurityManager::ParticipantStatelessMessageListener::on_new_cache_change_added(
        RTPSReader*,
        const CacheChange_t* const)
{
}

void SecurityManager::ParticipantVolatileMessageListener::on_new_cache_change_added(
        RTPSReader*,
        const CacheChange_t* const)
{
}

bool SecurityManager::discovered_reader(
        const GUID_t&,
        const GUID_t&,
        ReaderProxyData&,
        const EndpointSecurityAttributes&)
{
    return false;
}

void SecurityManager::remove_reader(
        const GUID_t&,
        const GUID_t&,
        const GUID_t&)
{
}

bool SecurityManager::discovered_writer(
        const GUID_t&,
        const GUID_t&,
        WriterProxyData&,
        const EndpointSecurityAttributes&)
{
    return false;
}

void SecurityManager::remove_writer(
        const GUID_t&,
        const GUID_t&,
        const GUID_t&)
{
}

bool SecurityManager::check_guid_comes_from(
        const GUID_t&,
        const GUID_t&) const
{
    return false;
}

bool SecurityManager::get_identity_token(
        IdentityToken**) const
{
    return false;
}

bool SecurityManager::return_identity_token(
        IdentityToken*) const
{
    return false;
}

bool SecurityManager::get_permissions_token(
        PermissionsToken**) const
{
    return false;
}

bool SecurityManager::return_permissions_token(
        PermissionsToken*) const
{
    return false;
}

void SecurityManager::remove_participant(
        const ParticipantProxyData&)
{
}

uint32_t SecurityManager::builtin_endpoints() const
{
    return 0;
}

} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // HAVE_SECURITY
