// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/utils/TypePropagation.hpp>

#include <string>

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace utils {

TypePropagation to_type_propagation(
        const PropertyPolicyQos& property_qos)
{
    TypePropagation type_propagation;

    const std::string* property_value = rtps::PropertyPolicyHelper::find_property(
        property_qos, parameter_policy_type_propagation);

    if (nullptr == property_value)
    {
        type_propagation = TypePropagation::TYPEPROPAGATION_ENABLED;
    }
    else if (*property_value == "disabled")
    {
        type_propagation = TypePropagation::TYPEPROPAGATION_DISABLED;
    }
    else if (*property_value == "enabled")
    {
        type_propagation = TypePropagation::TYPEPROPAGATION_ENABLED;
    }
    else if (*property_value == "minimal_bandwidth")
    {
        type_propagation = TypePropagation::TYPEPROPAGATION_MINIMAL_BANDWIDTH;
    }
    else if (*property_value == "registration_only")
    {
        type_propagation = TypePropagation::TYPEPROPAGATION_REGISTRATION_ONLY;
    }
    else
    {
        type_propagation = TypePropagation::TYPEPROPAGATION_UNKNOWN;
    }

    return type_propagation;
}

std::ostream& operator <<(
        std::ostream& output,
        TypePropagation value)
{
    switch (value)
    {
        case TypePropagation::TYPEPROPAGATION_UNKNOWN:
            output << "TYPEPROPAGATION_UNKNOWN";
            break;
        case TypePropagation::TYPEPROPAGATION_DISABLED:
            output << "TYPEPROPAGATION_DISABLED";
            break;
        case TypePropagation::TYPEPROPAGATION_ENABLED:
            output << "TYPEPROPAGATION_ENABLED";
            break;
        case TypePropagation::TYPEPROPAGATION_MINIMAL_BANDWIDTH:
            output << "TYPEPROPAGATION_MINIMAL_BANDWIDTH";
            break;
        case TypePropagation::TYPEPROPAGATION_REGISTRATION_ONLY:
            output << "TYPEPROPAGATION_REGISTRATION_ONLY";
            break;
    }

    return output;
}

} // namespace utils
} // namespace dds
} // namespace fastdds
} // namespace eprosima
