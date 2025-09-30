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
 * @file SecurityPluginFactory.cpp
 */

#include <rtps/security/SecurityPluginFactory.h>
#include <security/authentication/PKIDH.h>
#include <security/accesscontrol/Permissions.h>
#include <security/cryptography/AESGCMGMAC.h>
#include <security/logging/LogTopic.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace security;

Authentication* SecurityPluginFactory::create_authentication_plugin(
        const PropertyPolicy& property_policy)
{
    Authentication* plugin = nullptr;
    const std::string* auth_plugin_property = PropertyPolicyHelper::find_property(property_policy,
                    "dds.sec.auth.plugin");

    if (auth_plugin_property != nullptr)
    {
        if (auth_plugin_property->compare("builtin.PKI-DH") == 0)
        {
            plugin = create_builtin_authentication_plugin();
        }
    }

    return plugin;
}

AccessControl* SecurityPluginFactory::create_access_control_plugin(
        const PropertyPolicy& property_policy)
{
    AccessControl* plugin = nullptr;
    const std::string* access_plugin_property = PropertyPolicyHelper::find_property(property_policy,
                    "dds.sec.access.plugin");

    if (access_plugin_property != nullptr)
    {
        if (access_plugin_property->compare("builtin.Access-Permissions") == 0)
        {
            plugin = create_builtin_access_control_plugin();
        }
    }

    return plugin;
}

Cryptography* SecurityPluginFactory::create_cryptography_plugin(
        const PropertyPolicy& property_policy)
{
    Cryptography* plugin = nullptr;
    const std::string* crypto_plugin_property = PropertyPolicyHelper::find_property(property_policy,
                    "dds.sec.crypto.plugin");

    if (crypto_plugin_property != nullptr)
    {
        // Check it is builtin DDS:Auth:PKI-DH.
        if (crypto_plugin_property->compare("builtin.AES-GCM-GMAC") == 0)
        {
            plugin = create_builtin_cryptography_plugin();
        }
    }

    return plugin;
}

Logging* SecurityPluginFactory::create_logging_plugin(
        const PropertyPolicy& property_policy)
{
    Logging* plugin = nullptr;
    const std::string* logging_plugin_property = PropertyPolicyHelper::find_property(property_policy,
                    "dds.sec.log.plugin");

    if (logging_plugin_property != nullptr)
    {
        if (logging_plugin_property->compare("builtin.DDS_LogTopic") == 0)
        {
            plugin = create_builtin_logging_plugin();
        }
    }

    return plugin;
}

Authentication* SecurityPluginFactory::create_builtin_authentication_plugin()
{
    return new PKIDH();
}

AccessControl* SecurityPluginFactory::create_builtin_access_control_plugin()
{
    return new Permissions();
}

Cryptography* SecurityPluginFactory::create_builtin_cryptography_plugin()
{
    return new AESGCMGMAC();
}

Logging* SecurityPluginFactory::create_builtin_logging_plugin()
{
    return new LogTopic();
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
