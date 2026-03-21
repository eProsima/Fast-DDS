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
 * @file PropertyPolicy.cpp
 */

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>

#include <algorithm>

namespace eprosima {
namespace fastdds {
namespace rtps {

PropertyPolicy PropertyPolicyHelper::get_properties_with_prefix(
        const PropertyPolicy& property_policy,
        const std::string& prefix)
{
    PropertyPolicy returned_property_policy;

    // Search in properties
    std::for_each(property_policy.properties().begin(), property_policy.properties().end(),
            [&returned_property_policy, &prefix](const Property& property)
            {
                if (property.name().compare(0, prefix.size(), prefix) == 0)
                {
                    Property new_property(property);
                    new_property.name().erase(0, prefix.size());
                    returned_property_policy.properties().emplace_back(new_property);
                }
            });

    // Search in binary_properties
    std::for_each(property_policy.binary_properties().begin(), property_policy.binary_properties().end(),
            [&returned_property_policy, &prefix](const BinaryProperty& property)
            {
                if (property.name().compare(0, prefix.size(), prefix) == 0)
                {
                    BinaryProperty new_property(property);
                    new_property.name().erase(0, prefix.size());
                    returned_property_policy.binary_properties().emplace_back(new_property);
                }
            });

    return returned_property_policy;
}

size_t PropertyPolicyHelper::length(
        const PropertyPolicy& property_policy)
{
    return property_policy.properties().size() +
           property_policy.binary_properties().size();
}

std::string* PropertyPolicyHelper::find_property(
        PropertyPolicy& property_policy,
        const std::string& name)
{
    std::string* returnedValue = nullptr;

    for (auto property = property_policy.properties().begin(); property != property_policy.properties().end();
            ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &property->value();
            break;
        }
    }

    return returnedValue;
}

const std::string* PropertyPolicyHelper::find_property(
        const PropertyPolicy& property_policy,
        const std::string& name)
{
    const std::string* returnedValue = nullptr;

    for (auto property = property_policy.properties().begin(); property != property_policy.properties().end();
            ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &property->value();
            break;
        }
    }

    return returnedValue;
}

const Property* PropertyPolicyHelper::get_property(
        const PropertyPolicy& property_policy,
        const std::string& name)
{
    const Property* returnedValue = nullptr;

    for (auto property = property_policy.properties().begin(); property != property_policy.properties().end();
            ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &*property;
            break;
        }
    }

    return returnedValue;
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
