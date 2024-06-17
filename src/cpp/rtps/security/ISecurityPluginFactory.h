// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ISecurityPluginFactory.h
 */
#ifndef _RTPS_SECURITY_ISECURITYPLUGINFACTORY_H_
#define _RTPS_SECURITY_ISECURITYPLUGINFACTORY_H_

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <rtps/security/accesscontrol/AccessControl.h>
#include <rtps/security/authentication/Authentication.h>
#include <rtps/security/cryptography/Cryptography.h>
#include <rtps/security/logging/Logging.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class ISecurityPluginFactory
{
public:

    /*!
     * @brief Create an Authentication plugin described in the PropertyPolicy.
     * @param property_policy PropertyPolicy containing the definition of the Authentication
     * plugin that has to be created.
     * @param Pointer to the new Authentication plugin. In case of error nullptr will be returned.
     */
    virtual Authentication* create_authentication_plugin(
            const PropertyPolicy& property_policy) = 0;

    /*!
     * @brief Create an AccessControl plugin described in the PropertyPolicy.
     * @param property_policy PropertyPolicy containing the definition of the AccessControl
     * plugin that has to be created.
     * @param Pointer to the new AccessControl plugin. In case of error nullptr will be returned.
     */
    virtual AccessControl* create_access_control_plugin(
            const PropertyPolicy& property_policy) = 0;

    /*!
     * @brief Create an Cryptographic plugin described in the PropertyPolicy.
     * @param property_policy PropertyPolicy containing the definition of the Cryptographic
     * plugin that has to be created.
     * @param Pointer to the new Cryptographic plugin. In case of error nullptr will be returned.
     */
    virtual Cryptography* create_cryptography_plugin(
            const PropertyPolicy& property_policy) = 0;

    /**
     * @brief Create a Logging plugin described in the PropertyPolicy.
     * @param property_policy PropertyPolicy containing the definition of the Logging
     * plugin that has to be created.
     * @return Pointer to the new Logging plugin. In case of error nullptr will be returned.
     */
    virtual Logging* create_logging_plugin(
            const PropertyPolicy& property_policy) = 0;
};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _RTPS_SECURITY_SECURITYPLUGINFACTORY_H_
