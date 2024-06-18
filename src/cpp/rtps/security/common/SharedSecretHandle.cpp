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
 * @file SharedSecretHandle.cpp
 */

#include <rtps/security/common/SharedSecretHandle.h>

using namespace eprosima::fastdds::rtps::security;

const char* const SharedSecret::class_id_ = "SharedSecret";

const std::vector<uint8_t>* SharedSecretHelper::find_data_value(
        const SecretHandle& secret,
        const std::string& name)
{
    const std::vector<uint8_t>* returnedValue = nullptr;
    const SharedSecretHandle& sh = SharedSecretHandle::narrow(secret);

    // Check the right class is underneath the handle
    if (sh.nil())
    {
        return nullptr;
    }

    const SharedSecret& sharedsecret = **sh;

    for (auto property = sharedsecret.data_.begin(); property != sharedsecret.data_.end(); ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &property->value();
            break;
        }
    }

    return returnedValue;
}
