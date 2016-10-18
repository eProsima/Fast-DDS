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
#include "../../security/authentication/PKIDH.h"

using namespace eprosima::fastrtps::rtps;
using namespace ::security;

Authentication* SecurityPluginFactory::create_authentication_plugin(const PropertyPolicy& property_policy)
{
    Authentication* plugin = nullptr;
    PropertyPolicy auth_properties = PropertyPolicyHelper::get_properties_with_prefix(property_policy, "dds.sec.auth.");

    if(PropertyPolicyHelper::length(auth_properties) > 0)
    {
        // Check it is builtin DDS:Auth:PKI-DH.
        PropertyPolicy pki_properties = PropertyPolicyHelper::get_properties_with_prefix(auth_properties, "builtin.PKI-DH.");

        if(PropertyPolicyHelper::length(pki_properties) > 0)
        {
            plugin = new PKIDH();
        }
    }

    return plugin;
}
