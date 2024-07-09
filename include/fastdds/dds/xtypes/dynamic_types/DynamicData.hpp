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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICDATA_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICDATA_HPP

#include <memory>
#include <string>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicData : public std::enable_shared_from_this<DynamicData>
{
public:

    using _ref_type = typename traits<DynamicData>::ref_type;

    /*!
     * Retrieve the @ref DynamicType reference associated to this @ref DynamicData
     * @return Non-nil @ref DynamicType reference
     */
    FASTDDS_EXPORTED_API virtual traits<DynamicType>::ref_type type() = 0;

    /*!
     * Retrieves the @ref MemberDescriptor associated to a member.
     * @param [inout] value Non-nil @ref MemberDescriptor reference where the information is copied.
     * @param [in] id Identifier of the member to be retrieved.
     * @retval RETCODE_OK when the copy was successful.
     * @retval RETCODE_BAD_PARAMETER when descriptor reference is nil or member identifier is not found.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_descriptor(
            traits<MemberDescriptor>::ref_type& value,
            MemberId id) = 0;

    /*!
     * Compares two @ref DynamicData, equality requires:
     *     - Their respective type definitions are equal
     *     - All contained values are equal and occur in the same order
     *     - If the samples' type is an aggregated type, previous rule shall be amended as follows:
     *          -# Members shall be compared without regard to their order.
     * @param [in] other @ref DynamicData reference to compare to
     * @return `true` on equality
     */
    FASTDDS_EXPORTED_API virtual bool equals(
            traits<DynamicData>::ref_type other) = 0;

    /*!
     * Queries @ref MemberId by name.
     * The query result depends on the type of the sample.
     * Only next types support accessing by name.
     * @li Aggregated type.
     * @li Map type.
     * @li Bitmask type.
     * @param [in] name string
     * @return MemberId or MEMBER_ID_INVALID on failure
     */
    FASTDDS_EXPORTED_API virtual MemberId get_member_id_by_name(
            const ObjectName& name) = 0;

    /*!
     * Queries @ref MemberId by index
     * The query result depends on the type of the sample.
     * Only next types support accessing by index.
     * @li Aggregated type.
     * @li Sequence type.
     * @li String type.
     * @li Map type.
     * @li Array type.
     * @li Bitmask type.
     * @param [in] index Index.
     * @return MemberId or MEMBER_ID_INVALID on failure
     */
    FASTDDS_EXPORTED_API virtual MemberId get_member_id_at_index(
            uint32_t index) = 0;

    /*!
     * Provides the @b item @b count of the data and depends on the type of object:
     * @li If the object is of a collection type, returns the number of elements currently in the collection.
     *     In the case of an array type, this value will always be equal to the product of the bounds of all array
     *     dimensions.
     * @li If the object is of a bitmask type, return the number of named flags that are currently set in the bitmask.
     * @li If the object is of a structure or annotation type, return the number of members in the object.
     *     This value may be different than the number of members in the corresponding @ref DynamicType.
     * @li If the object is of a union type, return the number of members in the object. This number will be two if the
     *     discriminator value selects a member and one otherwise.
     * @li if the object is of a primitive or enumerated type, it is atomic: return one.
     * @li if the object is of an alias type, return the value appropriate for the alias base type.
     * @return count as defined above
     */
    FASTDDS_EXPORTED_API virtual uint32_t get_item_count() = 0;

    /*!
     * Clear all members associated to the object.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the cleaning was successful.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t clear_all_values() = 0;

    /*!
     * Clear all members not associated to the key
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the cleaning was successful.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t clear_nonkey_values() = 0;

    /*!
     * Clear a member.
     * The meaning of "clearing" depends on the type of the sample:
     * @li If aggregated type, set it to its default value.
     * @li If variable-length collection type, remove the indicated element, shifting any subsequence elements to the
     * next-lowest index.
     * @li If array type, set the indicated element to its default value.
     * @li If bitmask type, clear the indicated bit.
     * @li If enumerated type, set it to the first value of the enumerated type.
     * @li If primitive type, set it to its default value.
     * @param [in] id Identifier of the member to purge
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the cleaning was successful.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t clear_value(
            MemberId id) = 0;

    /*!
     * \b Loans a @ref DynamicData reference within the sample
     * @remarks This loan shall be returned by the @ref DynamicData::return_loaned_value operation
     * @param [in] id identifier of the object to retrieve
     * @return @ref DynamicData reference loaned or \b nil on outstanding loaned data
     */
    FASTDDS_EXPORTED_API virtual traits<DynamicData>::ref_type loan_value(
            MemberId id) = 0;

    /*!
     * Returns a loan retrieved using @ref DynamicData::loan_value.
     * @param [in] value @ref DynamicData reference previously loaned
     * @retval RETCODE_OK when the loan was returned successfully.
     * @retval RETCODE_PRECONDITION_NOT_MET when the loan is invalid.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t return_loaned_value(
            traits<DynamicData>::ref_type value) = 0;

    /*!
     * Creates and returns a new data sample with the same contents as this one.
     * A comparison of this object and the clone using equals immediately following this call will return `true`.
     * @return @ref DynamicData reference
     */
    FASTDDS_EXPORTED_API virtual traits<DynamicData>::ref_type clone() = 0;

    /*!
     * Retrieves an \b int32 value associated to an identifier.
     * @param [inout] value \b int32 to populate
     * @param [in] id identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b int32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_int32_value(
            int32_t& value,
            MemberId id) = 0;

    /*!
     * Sets an \b int32 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b int32 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b int32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_int32_value(
            MemberId id,
            int32_t value) = 0;

    /*!
     * Retrieves an \b uint32 value associated to an identifier.
     * @param [inout] value \b uint32 to populate
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b uint32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_uint32_value(
            uint32_t& value,
            MemberId id) = 0;

    /*!
     * Sets an \b uint32 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b uint32 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b uint32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_uint32_value(
            MemberId id,
            uint32_t value) = 0;

    /*!
     * Retrieves an \b int8 value associated to an identifier.
     * @param [inout] value \b int8 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b int8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_int8_value(
            int8_t& value,
            MemberId id) = 0;

    /*!
     * Sets an \b int8 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b int8 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b int8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_int8_value(
            MemberId id,
            int8_t value) = 0;

    /*!
     * Retrieves an \b uint8 value associated to an identifier.
     * @param [inout] value \b uint8 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b uint8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_uint8_value(
            uint8_t& value,
            MemberId id) = 0;

    /*!
     * Sets an \b uint8 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b uint8 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b uint8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_uint8_value(
            MemberId id,
            uint8_t value) = 0;

    /*!
     * Retrieves an \b int16 value associated to an identifier.
     * @param [inout] value \b int16 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b int16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_int16_value(
            int16_t& value,
            MemberId id) = 0;

    /*!
     * Sets an \b int16 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b int16 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b int16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_int16_value(
            MemberId id,
            int16_t value) = 0;

    /*!
     * Retrieves an \b uint16 value associated to an identifier.
     * @param [inout] value \b uint16 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b uint16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_uint16_value(
            uint16_t& value,
            MemberId id) = 0;

    /*!
     * Sets an \b uint16 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b uint16 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b uint16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_uint16_value(
            MemberId id,
            uint16_t value) = 0;

    /*!
     * Retrieves an \b int64 value associated to an identifier.
     * @param [inout] value \b int64 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b int64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_int64_value(
            int64_t& value,
            MemberId id) = 0;

    /*!
     * Sets an \b int64 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b int64 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b int64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_int64_value(
            MemberId id,
            int64_t value) = 0;

    /*!
     * Retrieves an \b uint64 value associated to an identifier.
     * @param [inout] value \b uint64 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b uint64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_uint64_value(
            uint64_t& value,
            MemberId id) = 0;

    /*!
     * Sets an \b uint64 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b uint64 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b uint64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_uint64_value(
            MemberId id,
            uint64_t value) = 0;

    /*!
     * Retrieves an \b float32 value associated to an identifier.
     * @param [inout] value \b float32 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b float32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_float32_value(
            float& value,
            MemberId id) = 0;

    /*!
     * Sets an \b float32 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b float32 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b float32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_float32_value(
            MemberId id,
            float value) = 0;

    /*!
     * Retrieves an \b float64 value associated to an identifier.
     * @param [inout] value \b float64 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b float64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_float64_value(
            double& value,
            MemberId id) = 0;

    /*!
     * Sets an \b float64 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b float64 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b float64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_float64_value(
            MemberId id,
            double value) = 0;

    /*!
     * Retrieves an \b float128 value associated to an identifier.
     * @param [inout] value \b float128 to populate.
     * @param [in] id Identifier of the member to query.
     * @remarks Only available on platforms supporting long double
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b float128.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_float128_value(
            long double& value,
            MemberId id) = 0;

    /*!
     * Sets an \b float128 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b float128 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b float128.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_float128_value(
            MemberId id,
            long double value) = 0;

    /*!
     * Retrieves an \b char8 value associated to an identifier.
     * @param [inout] value \b char8 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b char8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_char8_value(
            char& value,
            MemberId id) = 0;

    /*!
     * Sets an \b char8 value associated to an identifier
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b char8 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b char8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_char8_value(
            MemberId id,
            char value) = 0;

    /*!
     * Retrieves an \b char16 value associated to an identifier.
     * @param [inout] value \b char16 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b char16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_char16_value(
            wchar_t& value,
            MemberId id) = 0;

    /*!
     * Sets an \b char16 value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b char16 to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b char16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_char16_value(
            MemberId id,
            wchar_t value) = 0;

    /*!
     * Retrieves an \b byte value associated to an identifier.
     * @param [inout] value \b byte to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b byte.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_byte_value(
            eprosima::fastdds::rtps::octet& value,
            MemberId id) = 0;

    /*!
     * Sets an \b byte value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b byte to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b byte.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_byte_value(
            MemberId id,
            eprosima::fastdds::rtps::octet value) = 0;

    /*!
     * Retrieves an \b bool value associated to an identifier.
     * @param [in] id Identifier of the member to query.
     * @param [inout] value \b bool to populate.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b bool.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_boolean_value(
            bool& value,
            MemberId id) = 0;

    /*!
     * Sets an \b bool value associated to an identifier
     * @param [in] id identifier of the member to set.
     * @param [in] value \b bool to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b bool.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_boolean_value(
            MemberId id,
            bool value) = 0;

    /*!
     * Retrieves an \b string value associated to an identifier.
     * @param [inout] value \b string to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b string.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_string_value(
            std::string& value,
            MemberId id) = 0;

    /*!
     * Sets an \b string value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b string to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b string
     * or the string length is greater than the string bound.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_string_value(
            MemberId id,
            const std::string& value) = 0;

    /*!
     * Retrieves an \b wstring value associated to an identifier.
     * @param [inout] value \b wstring to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b wstring.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_wstring_value(
            std::wstring& value,
            MemberId id) = 0;

    /*!
     * Sets an \b wstring value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value \b wstring to set.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b wstring
     * or the string length is greater than the string bound.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_wstring_value(
            MemberId id,
            const std::wstring& value) = 0;

    /*!
     * Retrieves a \b complex value associated to an identifier.
     * @param [inout] value @ref DynamicData reference to populate
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to \b complex.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_complex_value(
            traits<DynamicData>::ref_type& value,
            MemberId id) = 0;

    /*!
     * Sets a \b complex value associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value @ref DynamicData reference to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the value reference is nil or @ref MemberId is invalid or the member type is
     * not promotable to \b complex.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_complex_value(
            MemberId id,
            traits<DynamicData>::ref_type value) = 0;

    /*!
     * Retrieves a sequence of \b int32 values associated to an identifier.
     * @param [inout] value \b Sequence of \b int32 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b int32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_int32_values(
            Int32Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b int32 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b int32 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b int32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_int32_values(
            MemberId id,
            const Int32Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b uint32 values associated to an identifier.
     * @param [inout] value \b Sequence of \b uint32 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b uint32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_uint32_values(
            UInt32Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b uint32 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b uint32 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b uint32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_uint32_values(
            MemberId id,
            const UInt32Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b int8 values associated to an identifier.
     * @param [inout] value \b Sequence of \b int8 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b int8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_int8_values(
            Int8Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b int8 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b int8 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b int8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_int8_values(
            MemberId id,
            const Int8Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b uint8 values associated to an identifier.
     * @param [inout] value \b Sequence of \b uint8 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b uint8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_uint8_values(
            UInt8Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b uint8 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b uint8 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b uint8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_uint8_values(
            MemberId id,
            const UInt8Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b int16 values associated to an identifier.
     * @param [inout] value \b Sequence of \b int16 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b int16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_int16_values(
            Int16Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b int16 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b int16 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b int16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_int16_values(
            MemberId id,
            const Int16Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b uint16 values associated to an identifier.
     * @param [inout] value \b Sequence of \b uint16 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b uint16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_uint16_values(
            UInt16Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b uint16 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b uint16 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b uint16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_uint16_values(
            MemberId id,
            const UInt16Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b int64 values associated to an identifier.
     * @param [inout] value \b Sequence of \b int64 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b int64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_int64_values(
            Int64Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b int64 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b int64 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b int64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_int64_values(
            MemberId id,
            const Int64Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b uint64 values associated to an identifier.
     * @param [inout] value \b Sequence of \b uint64 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b uint64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_uint64_values(
            UInt64Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b uint64 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b uint64 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b uint64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_uint64_values(
            MemberId id,
            const UInt64Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b float32 values associated to an identifier.
     * @param [inout] value \b Sequence of \b float32 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b float32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_float32_values(
            Float32Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b float32 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b float32 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b float32.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_float32_values(
            MemberId id,
            const Float32Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b float64 values associated to an identifier.
     * @param [inout] value \b Sequence of \b float64 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b float64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_float64_values(
            Float64Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b float64 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b float64 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b float64.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_float64_values(
            MemberId id,
            const Float64Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b float128 values associated to an identifier.
     * @param [inout] value \b Sequence of \b float128 to populate.
     * @param [in] id Identifier of the member to query.
     * @remarks Only available on platforms supporting long double
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b
     * float128.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_float128_values(
            Float128Seq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b float128 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b float128 to set
     * @remarks Only available on platforms supporting long double
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b float128.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_float128_values(
            MemberId id,
            const Float128Seq& value) = 0;

    /*!
     * Retrieves a sequence of \b char8 values associated to an identifier.
     * @param [inout] value \b Sequence of \b char8 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b char8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_char8_values(
            CharSeq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b char8 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b char8 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b char8.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_char8_values(
            MemberId id,
            const CharSeq& value) = 0;

    /*!
     * Retrieves a sequence of \b char16 values associated to an identifier.
     * @param [inout] value \b Sequence of \b char16 to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b char16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_char16_values(
            WcharSeq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b char16 values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b char16 to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b char16.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_char16_values(
            MemberId id,
            const WcharSeq& value) = 0;

    /*!
     * Retrieves a sequence of \b byte values associated to an identifier.
     * @param [inout] value \b Sequence of \b byte to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b byte.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_byte_values(
            ByteSeq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b byte values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b byte to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b byte.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_byte_values(
            MemberId id,
            const ByteSeq& value) = 0;

    /*!
     * Retrieves a sequence of \b bool values associated to an identifier.
     * @param [inout] value \b Sequence of \b bool to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b bool.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_boolean_values(
            BooleanSeq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b bool values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b bool to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b bool.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_boolean_values(
            MemberId id,
            const BooleanSeq& value) = 0;

    /*!
     * Retrieves a sequence of \b string values associated to an identifier.
     * @param [inout] value \b Sequence of \b string to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b string.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_string_values(
            StringSeq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b string values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b string to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b string.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_string_values(
            MemberId id,
            const StringSeq& value) = 0;

    /*!
     * Retrieves a sequence of \b wstring values associated to an identifier.
     * @param [inout] value \b Sequence of \b wstring to populate.
     * @param [in] id Identifier of the member to query.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was retrieved successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b wstring.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t get_wstring_values(
            WstringSeq& value,
            MemberId id) = 0;

    /*!
     * Sets a sequence of \b wstring values associated to an identifier.
     * @param [in] id Identifier of the member to set.
     * @param [in] value Sequence of \b wstring to set
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the value was set successfully.
     * @retval RETCODE_BAD_PARAMETER when the @ref MemberId is invalid or the member type is not promotable to sequence of \b wstring.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t set_wstring_values(
            MemberId id,
            const WstringSeq& value) = 0;

protected:

    DynamicData() = default;

    virtual ~DynamicData() = default;

    traits<DynamicData>::ref_type _this();

private:

    DynamicData(
            const DynamicData&) = delete;

    DynamicData(
            DynamicData&&) = delete;

    DynamicData& operator =(
            const DynamicData&) = delete;

    DynamicData& operator =(
            DynamicData&&) = delete;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICDATA_HPP
