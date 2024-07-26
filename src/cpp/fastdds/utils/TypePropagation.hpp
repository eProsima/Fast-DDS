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

/**
 * @file TypePropagation.hpp
 */

#ifndef FASTDDS_UTILS__TYPEPROPAGATION_HPP
#define FASTDDS_UTILS__TYPEPROPAGATION_HPP

#include <cstdint>
#include <ostream>

#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace utils {

/**
 * @brief Enum class TypePropagation.
 */
enum class TypePropagation : std::uint8_t
{
    //! Invalid value.
    TYPEPROPAGATION_UNKNOWN,

    //! Disable the propagation and registration of Type Information.
    TYPEPROPAGATION_DISABLED,

    //! Enable the propagation and registration of Type Information (EK_COMPLETE & EK_MINIMAL).
    TYPEPROPAGATION_ENABLED,

    //! Only propagation of EK_MINIMAL.
    TYPEPROPAGATION_MINIMAL_BANDWIDTH,

    //! Only registration of Type Information.
    TYPEPROPAGATION_REGISTRATION_ONLY
};

std::ostream& operator <<(
        std::ostream& output,
        TypePropagation value);

/**
 * @brief Converts a PropertyPolicyQos to a TypePropagation.
 *
 * @param property_qos PropertyPolicyQos to convert.
 * @return TypePropagation.
 */
TypePropagation to_type_propagation(
        const PropertyPolicyQos& property_qos);

} // namespace utils
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_UTILS__TYPEPROPAGATION_HPP
