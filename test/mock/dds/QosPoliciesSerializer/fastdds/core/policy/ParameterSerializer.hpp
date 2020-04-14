// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file QosPoliciesSerializer.hpp
 *
 */

#ifndef FASTDDS_CORE_POLICY__PARAMETERSERIALIZER_HPP_
#define FASTDDS_CORE_POLICY__PARAMETERSERIALIZER_HPP_

#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

template <typename Parameter>
class ParameterSerializer
{
public:

    static bool add_to_cdr_message(
            const Parameter&,
            fastrtps::rtps::CDRMessage_t*)
    {
        return true;
    }

    static bool read_from_cdr_message(
            Parameter&,
            fastrtps::rtps::CDRMessage_t*,
            const uint16_t)
    {
        return true;
    }

    static uint32_t cdr_serialized_size(
            const Parameter&)
    {
        return 0;
    }

    static inline uint32_t cdr_serialized_size(
            const fastrtps::string_255&)
    {
        return 0;
    }

    static inline uint32_t cdr_serialized_size(
            const fastrtps::rtps::Token&)
    {
        return 0;
    }

    static bool add_parameter_sentinel(
            fastrtps::rtps::CDRMessage_t*)
    {
        return true;
    }

};

}
}
}

#endif // FASTDDS_CORE_POLICY__PARAMETERSERIALIZER_HPP_
