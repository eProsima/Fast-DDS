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

#include <cstdint>
#include <iostream>
#include <string>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

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

FASTDDS_TODO_BEFORE(4, 0, "Remove this enum in favor of DynamicDataJsonMapping");
enum class DynamicDataJsonFormat
{
    OMG,
    EPROSIMA,
};

/*!
 * JSON mapping style used when serializing/deserializing @ref eprosima::fastdds::dds::DynamicData.
 *
 * @li @c OMG: Follows the OMG DDS Consolidated JSON Syntax specification
 *     (see https://www.omg.org/spec/DDS-JSON/1.0/About-DDS-JSON). Enums are
 *     emitted as their literal name string, and bitmasks as a single integer
 *     holding the bitmask value.
 * @li @c EPROSIMA: eProsima's own JSON mapping. Enums are emitted as a
 *     @c {"name", "value"} object, and bitmasks as a
 *     @c {"value", "binary", "active"} object exposing the numeric value,
 *     its binary string, and the list of active bit names. Other constructs
 *     match the OMG mapping.
 */
enum class DynamicDataJsonMapping
{
    OMG,
    EPROSIMA,
};

/*!
 * Options controlling JSON serialization of @ref DynamicData.
 */
struct FormatOptions
{
    /// JSON serialization mapping (OMG or EPROSIMA).
    DynamicDataJsonMapping mapping = DynamicDataJsonMapping::EPROSIMA;

    /// Maximum number of items to emit per sequence/array. Collections that
    /// exceed this limit are replaced with a compact label of the form
    /// @c "<sequence: N <idl-type>>" for sequences and
    /// @c "<array: <idl-type>[d1,d2,...]>" for arrays (where @c d1,d2,... are
    /// the array bounds and @c <idl-type> is the IDL keyword for the element
    /// type, e.g. @c long, @c "long long", @c octet, @c string). 0 means no
    /// limit (default).
    /// Note: only applies to sequences and arrays; maps are always emitted in
    /// full.
    /// @warning Truncation is destructive and display only: the emitted label
    /// is a JSON string where a JSON array would be required by the schema,
    /// so a truncated document cannot be round tripped, @ref json_deserialize
    /// will fail with @c RETCODE_BAD_PARAMETER on any member whose value is a
    /// truncation label. Use a non-zero @c max_collection_items only for
    /// human-facing output.
    uint32_t max_collection_items = 0;
};

FASTDDS_DEPRECATED_UNTIL(4, json_serialize, "Use json_serialize with FormatOptions instead")
/*!
 * Serializes a @ref DynamicData into a JSON object, which is then dumped into an \c std::ostream.
 * Equivalent to calling the @ref FormatOptions overload with default options
 * and the given @p format.
 * @param[in] data @ref DynamicData reference to be serialized.
 * @param[in] format @ref DynamicDataJsonFormat JSON serialization format.
 * @param[in,out] output \c std::ostream reference where the JSON object is dumped.
 * @retval RETCODE_OK when serialization fully succeeds, and inner (member serialization) failing code otherwise.
 * @warning Deprecated. Use the @ref FormatOptions overload instead.
 */
FASTDDS_EXPORTED_API ReturnCode_t json_serialize(
        const DynamicData::_ref_type& data,
        DynamicDataJsonFormat format,
        std::ostream& output) noexcept;

/*!
 * Serializes a @ref DynamicData into a JSON object, which is then dumped into an \c std::ostream.
 * See @ref FormatOptions for the available controls (JSON style, collection truncation).
 * @param[in] data @ref DynamicData reference to be serialized.
 * @param[in] format @ref FormatOptions controlling format and collection truncation.
 * @param[in,out] output \c std::ostream reference where the JSON object is dumped.
 * @retval RETCODE_OK when serialization fully succeeds, and inner (member serialization) failing code otherwise.
 */
FASTDDS_EXPORTED_API ReturnCode_t json_serialize(
        const DynamicData::_ref_type& data,
        const FormatOptions& format,
        std::ostream& output) noexcept;

FASTDDS_DEPRECATED_UNTIL(4, json_deserialize, "Use json_deserialize with FormatOptions instead")
/*!
 * Deserializes a JSON object string into a @ref DynamicData.
 * @param[in] input JSON object string to be deserialized.
 * @param[in] dynamic_type @ref DynamicType corresponding to the @ref DynamicData where the JSON string is deserialized.
 * @param[in] format @ref DynamicDataJsonFormat JSON serialization format.
 * @param[in,out] data @ref DynamicData reference where the JSON string is deserialized. Must be null.
 * @retval RETCODE_OK when deserialization fully succeeds, RETCODE_BAD_PARAMETER when parsing fails or preconditions are not met, and inner (member deserialization) failing code otherwise.
 * @warning Deprecated. Use the @ref FormatOptions overload instead.
 */
FASTDDS_EXPORTED_API ReturnCode_t json_deserialize(
        const std::string& input,
        const DynamicType::_ref_type& dynamic_type,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept;

/*!
 * Deserializes a JSON object string into a @ref DynamicData.
 *
 * Documents produced by @ref json_serialize with a non-zero
 * @c max_collection_items that triggered truncation cannot be deserialized
 * back: truncation replaces a JSON array with a JSON string label, which
 * violates the schema expected for sequence/array members. Such inputs are
 * rejected with @c RETCODE_BAD_PARAMETER.
 *
 * @param[in] input JSON object string to be deserialized.
 * @param[in] dynamic_type @ref DynamicType corresponding to the @ref DynamicData where the JSON string is deserialized.
 * @param[in] format @ref FormatOptions, only the JSON mapping is used.
 * @param[in,out] data @ref DynamicData reference where the JSON string is deserialized. Must be null.
 * @retval RETCODE_OK when deserialization fully succeeds, RETCODE_BAD_PARAMETER when parsing fails or preconditions are not met (including truncated-collection labels in @p input), and inner (member deserialization) failing code otherwise.
 */
FASTDDS_EXPORTED_API ReturnCode_t json_deserialize(
        const std::string& input,
        const DynamicType::_ref_type& dynamic_type,
        const FormatOptions& format,
        DynamicData::_ref_type& data) noexcept;

} // dds
} // fastdds
} // eprosima

#endif // FASTDDS_DDS_XTYPES__UTILS_HPP
