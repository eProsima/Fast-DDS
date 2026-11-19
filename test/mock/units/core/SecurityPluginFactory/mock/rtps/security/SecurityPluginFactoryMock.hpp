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

#ifndef FASTDDS_TEST_MOCK_RTPS_SECURITY__SECURITYPLUGINFACTORYMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_SECURITY__SECURITYPLUGINFACTORYMOCK_HPP

#include <rtps/security/SecurityPluginFactory.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

/**
 * Plugins handed out by every SecurityPluginFactory created with the mock implementation.
 *
 * Each create_*_plugin() method of the factory returns the plugin set here and forgets it, so the caller takes its
 * ownership. A plugin not taken is destroyed by the matching release method. create_logging_plugin() always returns
 * nullptr: destroying a Logging plugin here would need the rtps::security-logging unit, whose typeinfo UBSan checks.
 */
class SecurityPluginFactoryMock
{
public:

    /**
     * Set the plugin the next call to create_authentication_plugin() returns.
     *
     * @param plugin Plugin to hand out, whose ownership is taken.
     */
    static void set_auth_plugin(
            Authentication* plugin);

    /**
     * Destroy the authentication plugin set and not handed out, if any.
     */
    static void release_auth_plugin();

    /**
     * Set the plugin the next call to create_access_control_plugin() returns.
     *
     * @param plugin Plugin to hand out, whose ownership is taken.
     */
    static void set_access_control_plugin(
            AccessControl* plugin);

    /**
     * Destroy the access control plugin set and not handed out, if any.
     */
    static void release_access_control_plugin();

    /**
     * Set the plugin the next call to create_cryptography_plugin() returns.
     *
     * @param plugin Plugin to hand out, whose ownership is taken.
     */
    static void set_crypto_plugin(
            Cryptography* plugin);

    /**
     * Destroy the cryptography plugin set and not handed out, if any.
     */
    static void release_crypto_plugin();

    static Authentication* auth_plugin;

    static AccessControl* access_plugin;

    static Cryptography* crypto_plugin;
};

} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_SECURITY__SECURITYPLUGINFACTORYMOCK_HPP
