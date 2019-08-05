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

#include <fastdds/rtps/security/common/SharedSecretHandle.h>

using namespace eprosima::fastrtps::rtps::security;

const char* const SharedSecret::class_id_ = "SharedSecret";

std::vector<uint8_t>* SharedSecretHelper::find_data_value(SharedSecret& sharedsecret, const std::string& name)
{
    std::vector<uint8_t>* returnedValue = nullptr;

    for(auto property = sharedsecret.data_.begin(); property != sharedsecret.data_.end(); ++property)
    {
        if(property->name().compare(name) == 0)
        {
            returnedValue = &property->value();
            break;
        }
    }

    return returnedValue;
}

const std::vector<uint8_t>* SharedSecretHelper::find_data_value(const SharedSecret& sharedsecret, const std::string& name)
{
    const std::vector<uint8_t>* returnedValue = nullptr;

    for(auto property = sharedsecret.data_.begin(); property != sharedsecret.data_.end(); ++property)
    {
        if(property->name().compare(name) == 0)
        {
            returnedValue = &property->value();
            break;
        }
    }

    return returnedValue;
}
