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

#include "SecurityPluginFactory.h"

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::rtps::security;

Authentication* SecurityPluginFactory::auth_plugin_ = nullptr;

AccessControl* SecurityPluginFactory::access_plugin_ = nullptr;

Cryptography* SecurityPluginFactory::crypto_plugin_ = nullptr;

Logging* SecurityPluginFactory::logging_plugin_ = nullptr;

Authentication* SecurityPluginFactory::create_authentication_plugin(
        const PropertyPolicy& /*property_policy*/)
{
    Authentication* ret =  auth_plugin_;
    auth_plugin_ = nullptr;
    return ret;
}

void SecurityPluginFactory::set_auth_plugin(
        Authentication* plugin)
{
    auth_plugin_ = plugin;
}

void SecurityPluginFactory::release_auth_plugin()
{
    if (auth_plugin_ != nullptr)
    {
        delete auth_plugin_;
        auth_plugin_ = nullptr;
    }
}

AccessControl* SecurityPluginFactory::create_access_control_plugin(
        const PropertyPolicy& /*property_policy*/)
{
    AccessControl* ret =  access_plugin_;
    access_plugin_ = nullptr;
    return ret;
}

void SecurityPluginFactory::set_access_control_plugin(
        AccessControl* plugin)
{
    access_plugin_ = plugin;
}

void SecurityPluginFactory::release_access_control_plugin()
{
    if (access_plugin_ != nullptr)
    {
        delete access_plugin_;
        access_plugin_ = nullptr;
    }
}

Cryptography* SecurityPluginFactory::create_cryptography_plugin(
        const PropertyPolicy& /*property_policy*/)
{
    Cryptography* ret =  crypto_plugin_;
    crypto_plugin_ = nullptr;
    return ret;
}

void SecurityPluginFactory::set_crypto_plugin(
        Cryptography* plugin)
{
    crypto_plugin_ = plugin;
}

void SecurityPluginFactory::release_crypto_plugin()
{
    if (crypto_plugin_ != nullptr)
    {
        delete crypto_plugin_;
        crypto_plugin_ = nullptr;
    }
}

Logging* SecurityPluginFactory::create_logging_plugin(
        const PropertyPolicy& /*property_policy*/)
{
    Logging* ret =  logging_plugin_;
    logging_plugin_ = nullptr;
    return ret;
}

void SecurityPluginFactory::set_logging_plugin(
        Logging* plugin)
{
    logging_plugin_ = plugin;
}

void SecurityPluginFactory::release_logging_plugin()
{
    if (logging_plugin_ != nullptr)
    {
        delete logging_plugin_;
        logging_plugin_ = nullptr;
    }
}
