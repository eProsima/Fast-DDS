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

/*!
 * @file utils.hpp
 */

#ifndef FASTDDS_DDS_XTYPES__UTILS_HPP
#define FASTDDS_DDS_XTYPES__UTILS_HPP

#include <iostream>
#include <string>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

enum class DynamicDataJsonFormat
{
    OMG,
    EPROSIMA,
};

/**
 * @brief Serializes a @ref DynamicType into its IDL representation.
 *
 * @param [in] dynamic_type The @ref DynamicType to serialize.
 * @param [in,out] output \c std::ostream reference containing the IDL representation.
 * @retval RETCODE_OK when serialization fully succeeds, and inner (member serialization) failing code otherwise.
 */
FASTDDS_EXPORTED_API ReturnCode_t idl_serialize(
        const DynamicType::_ref_type& dynamic_type,
        std::ostream& output) noexcept;

/*!
 * Serializes a @ref DynamicData into a JSON object, which is then dumped into an \c std::ostream.
 * @param[in] data @ref DynamicData reference to be serialized.
 * @param[in] format @ref DynamicDataJsonFormat JSON serialization format.
 * @param[in,out] output \c std::ostream reference where the JSON object is dumped.
 * @retval RETCODE_OK when serialization fully succeeds, and inner (member serialization) failing code otherwise.
 */
FASTDDS_EXPORTED_API ReturnCode_t json_serialize(
        const DynamicData::_ref_type& data,
        DynamicDataJsonFormat format,
        std::ostream& output) noexcept;

} // dds
} // fastdds
} // eprosima

#endif // FASTDDS_DDS_XTYPES__UTILS_HPP
