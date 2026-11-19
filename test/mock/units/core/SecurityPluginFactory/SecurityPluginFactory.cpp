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

// Link-seam implementation of SecurityPluginFactory: replaces src/cpp/rtps/security/SecurityPluginFactory.cpp. Each
// factory method hands out the plugin set with SecurityPluginFactoryMock, and no builtin plugin is created.

#include <rtps/security/SecurityPluginFactory.h>

#include <mock/rtps/security/SecurityPluginFactoryMock.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

Authentication* SecurityPluginFactoryMock::auth_plugin = nullptr;
AccessControl* SecurityPluginFactoryMock::access_plugin = nullptr;
Cryptography* SecurityPluginFactoryMock::crypto_plugin = nullptr;

template<class Plugin>
static Plugin* take(
        Plugin*& plugin)
{
    Plugin* ret = plugin;
    plugin = nullptr;
    return ret;
}

template<class Plugin>
static void release(
        Plugin*& plugin)
{
    delete plugin;
    plugin = nullptr;
}

void SecurityPluginFactoryMock::set_auth_plugin(
        Authentication* plugin)
{
    auth_plugin = plugin;
}

void SecurityPluginFactoryMock::release_auth_plugin()
{
    release(auth_plugin);
}

void SecurityPluginFactoryMock::set_access_control_plugin(
        AccessControl* plugin)
{
    access_plugin = plugin;
}

void SecurityPluginFactoryMock::release_access_control_plugin()
{
    release(access_plugin);
}

void SecurityPluginFactoryMock::set_crypto_plugin(
        Cryptography* plugin)
{
    crypto_plugin = plugin;
}

void SecurityPluginFactoryMock::release_crypto_plugin()
{
    release(crypto_plugin);
}

Authentication* SecurityPluginFactory::create_authentication_plugin(
        const PropertyPolicy&)
{
    return take(SecurityPluginFactoryMock::auth_plugin);
}

AccessControl* SecurityPluginFactory::create_access_control_plugin(
        const PropertyPolicy&)
{
    return take(SecurityPluginFactoryMock::access_plugin);
}

Cryptography* SecurityPluginFactory::create_cryptography_plugin(
        const PropertyPolicy&)
{
    return take(SecurityPluginFactoryMock::crypto_plugin);
}

Logging* SecurityPluginFactory::create_logging_plugin(
        const PropertyPolicy&)
{
    return nullptr;
}

Authentication* SecurityPluginFactory::create_builtin_authentication_plugin()
{
    return nullptr;
}

AccessControl* SecurityPluginFactory::create_builtin_access_control_plugin()
{
    return nullptr;
}

Cryptography* SecurityPluginFactory::create_builtin_cryptography_plugin()
{
    return nullptr;
}

Logging* SecurityPluginFactory::create_builtin_logging_plugin()
{
    return nullptr;
}

} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
