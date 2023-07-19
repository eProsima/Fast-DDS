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

#ifndef TYPES_1_3_DYNAMIC_DATA_HPP
#define TYPES_1_3_DYNAMIC_DATA_HPP

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/v1_3/MemberId.hpp>

#include <cstdint>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class MemberDescriptor;

class DynamicData final
{

    DynamicData() noexcept = default;

    friend class DynamicDataImpl;

public:

    RTPS_DllAPI bool operator ==(
            const DynamicData& other) const noexcept;

    RTPS_DllAPI bool operator !=(
            const DynamicData& other) const noexcept;

    /**
     * Retrieve the @ref MemberDescriptor associated to a member according with (see [standard] 7.5.2.11.2)
     * @param [out] value @ref MemberDescriptor object to populate
     * @param [in] id identifier of the member to retrieve
     * @return standard DDS @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI ReturnCode_t get_descriptor(
            MemberDescriptor& value,
            MemberId id) const noexcept;

    /**
     * Retrieve the @ref DynamicType associated to a member according with (see [standard] 7.5.2.11.8)
     * @return @ref MemberDescriptor object to populate.
     * @attention There is no ownership transference.
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    RTPS_DllAPI const DynamicType& get_type() const noexcept;

    /**
     * Queries members by name
     * @param[in] name string
     * @return MemberId or MEMBER_ID_INVALID on failure
     */
    RTPS_DllAPI MemberId get_member_id_by_name(
            const char* name) const noexcept;

    /**
     * Queries members by index
     * @param[in] index uint32_t
     * @return MemberId or MEMBER_ID_INVALID on failure
     */
    RTPS_DllAPI MemberId get_member_id_at_index(
            uint32_t index) const noexcept;

    /**
     * Provides the @b item @b count of the data and depends on the type of object:
     * @li If the object is of a collection type, returns the number of elements currently in the collection.
     *     In the case of an array type, this value will always be equal to the product of the bounds of all array
     *     dimensions.
     * @li If the object is of a bitmask type, return the number of named flags that are currently set in the bitmask.
     * @li If the object is of a structure or annotation type, return the number of members in the object.
     *     This value may be different than the number of members in the corresponding @ref DynamicType (some optional
     *     members may be omitted.
     * @li If the object is of a union type, return the number of members in the object. This number will be two if the
     *     discriminator value selects a member and one otherwise.
     * @li if the object is of a primitive or enumerated type, it is atomic: return one.
     * @li if the object is of an alias type, return the value appropriate for the alias base type.
     * @return count as defined above
     */
    RTPS_DllAPI uint32_t get_item_count() const noexcept;

    /**
     * Compares two @ref DynamicData, equality requires:
     *     - Their respective type definitions are equal
     *     - All contained values are equal and occur in the same order
     *     - If the samples' type is an aggregated type, previous rule shall be amended as follows:
     *          -# Members shall be compared without regard to their order.
     *          -# One of the samples may omit a non-optional member that is present in the other if that
     *             member takes its default value in the latter sample.
     * @param [in] other @ref DynamicDataImpl object to compare to
     * @attention There is no ownership transference.
     * @return `true` on equality
     */
    RTPS_DllAPI bool equals(
            const DynamicData& other) const noexcept;

    /**
     * Clear all memory associated to the object
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t clear_all_values() noexcept;

    /**
     * Clear all memory not associated to the key
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t clear_nonkey_values() noexcept;

    /**
     * Clear all memory associated to a specific member
     * @param [in] id identifier of the member to purge
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t clear_value(
            MemberId id) noexcept;

    /**
     * \b Loans a @ref DynamicDataImpl object within the sample
     * @remarks This operation allows applications to visit values without allocating additional
     *         @ref DynamicDataImpl objects or copying values.
     * @remarks This loan shall be returned by the @ref DynamicData::return_loaned_value operation
     * @param [in] id identifier of the object to retrieve
     * @return @ref DynamicDataImpl object loaned or \b nil on outstanding loaned data
     * @attention There is ownership transference. The returned object should not be deleted but freed using
     *            returned_loaned_value
     */
    RTPS_DllAPI DynamicData* loan_value(
            MemberId id) noexcept;

    /**
     * Returns a loaned retrieved using @ref DynamicDataImpl::return_loaned_value
     * @param [in] value @ref DynamicDataImpl previously loaned
     * @attention There is ownership transference. It is not necessary to delete the returned object.
     */
    RTPS_DllAPI ReturnCode_t return_loaned_value(
            DynamicData* value) noexcept;

    /**
     * Create and return a new data sample with the same contents as this one.
     * A comparisson of this object and the clone using equals immediately following this call will return `true`.
     * @return @ref DynamicData
     * @attention There is ownership transference.
     */
    RTPS_DllAPI DynamicData* clone() const noexcept;

    /*
     * Retrieve an \b int32 value associated to an identifier
     * @param [out] value \b int32 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_int32_value(
            int32_t& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b int32 value associated to an identifier
     * @param [in] value \b int32 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_int32_value(
            int32_t value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b uint32 value associated to an identifier
     * @param [out] value \b uint32 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_uint32_value(
            uint32_t& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b uint32 value associated to an identifier
     * @param [in] value \b uint32 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_uint32_value(
            uint32_t value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b int16 value associated to an identifier
     * @param [out] value \b int16 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_int16_value(
            int16_t& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b int16 value associated to an identifier
     * @param [in] value \b int16 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_int16_value(
            int16_t value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b uint16 value associated to an identifier
     * @param [out] value \b uint16 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_uint16_value(
            uint16_t& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b uint16 value associated to an identifier
     * @param [in] value \b uint16 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_uint16_value(
            uint16_t value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b int64 value associated to an identifier
     * @param [out] value \b int64 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_int64_value(
            int64_t& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b int64 value associated to an identifier
     * @param [in] value \b int64 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_int64_value(
            int64_t value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b uint64 value associated to an identifier
     * @param [out] value \b uint64 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_uint64_value(
            uint64_t& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b uint64 value associated to an identifier
     * @param [in] value \b uint64 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_uint64_value(
            uint64_t value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b float32 value associated to an identifier
     * @param [out] value \b float32 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_float32_value(
            float& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b float32 value associated to an identifier
     * @param [in] value \b float32 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_float32_value(
            float value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b float64 value associated to an identifier
     * @param [out] value \b float64 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_float64_value(
            double& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b float64 value associated to an identifier
     * @param [in] value \b float64 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_float64_value(
            double value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b float128 value associated to an identifier
     * @param [out] value \b float128 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @remarks Only available on platforms supporting long double
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_float128_value(
            long double& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b float128 value associated to an identifier
     * @param [in] value \b float128 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_float128_value(
            long double value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b char8 value associated to an identifier
     * @param [out] value \b char8 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_char8_value(
            char& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b char8 value associated to an identifier
     * @param [in] value \b char8 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_char8_value(
            char value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b char16 value associated to an identifier
     * @param [out] value \b char16 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_char16_value(
            wchar_t& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b char16 value associated to an identifier
     * @param [in] value \b char16 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_char16_value(
            wchar_t value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b byte value associated to an identifier
     * @param [out] value \b byte to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_byte_value(
            octet& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b byte value associated to an identifier
     * @param [in] value \b byte to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_byte_value(
            octet value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b int8 value associated to an identifier
     * @param [out] value \b int8 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_int8_value(
            int8_t& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b int8 value associated to an identifier
     * @param [in] value \b int8 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_int8_value(
            int8_t value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b uint8 value associated to an identifier
     * @param [out] value \b uint8 to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_uint8_value(
            uint8_t& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b uint8 value associated to an identifier
     * @param [in] value \b uint8 to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_uint8_value(
            uint8_t value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b bool value associated to an identifier
     * @param [out] value \b bool to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_bool_value(
            bool& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b bool value associated to an identifier
     * @param [in] value \b bool to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_bool_value(
            bool value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Set an \b bool value associated to an identifier
     * @param [in] value \b bool to set
     * @param [in] name bitmask flags can be addressed by name
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_bool_value(
            bool value,
            const char* name);

    /*
     * Retrieve an \b string value associated to an identifier
     * @param [out] value \b string to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_string_value(
            const char*& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b string value associated to an identifier
     * @param [in] value \b string to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_string_value(
            const char* value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b wstring value associated to an identifier
     * @param [out] value \b wstring to populate
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_wstring_value(
            const wchar_t*& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set an \b wstring value associated to an identifier
     * @param [in] value \b wstring to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_wstring_value(
            const wchar_t* value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b enum value associated to an identifier
     * @param [out] value string because enumerations can be addressed by name
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_enum_value(
            const char*& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;
    /*
     * Set an \b enum value associated to an identifier
     * @param [in] value string because enumerations can be addressed by name
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_enum_value(
            const char* value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve an \b enum value associated to an identifier
     * @param [out] value uin32_t because enums are kept as \b DWORDs.
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_enum_value(
            uint32_t& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;
    /*
     * Set an \b enum value associated to an identifier
     * @param [in] value \b enum to set
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_enum_value(
            const uint32_t& value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Retrieve a bitmask object \b mask
     * @param [out] value uin64_t because bitmasks are kept as \b QWORDs.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_bitmask_value(
            uint64_t& value) const;

    /*
     * Convenient override to retrieve a bitmask object \b mask
     * @throws \@ref ReturnCode_t on failure
     * @return uint64 representing bitmask mask
     */
    RTPS_DllAPI uint64_t get_bitmask_value() const;

    /*
     * Set a \b mask value on a bitmask
     * @param [in] value \b mask to set
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_bitmask_value(
            uint64_t value);

    /*
     * Retrieve a \b complex value associated to an identifier
     * @param [out] value @ref DynamicDataImpl reference to populate
     * @attention There is ownership transference. The returned value must be released.
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t get_complex_value(
            const DynamicData*& value,
            MemberId id = MEMBER_ID_INVALID) const noexcept;

    /*
     * Set a \b complex value associated to an identifier
     * @param [in] value @ref DynamicDataImpl to set
     * @attention There is no ownership transference.
     * @param [in] id identifier of the member to set. \b MEMBER_ID_INVALID for primitives.
     * @return standard DDS @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t set_complex_value(
            const DynamicData& value,
            MemberId id = MEMBER_ID_INVALID) noexcept;

    /*
     * Convenient override to retrieve an \b int32 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b int32 queried
     */
    RTPS_DllAPI int32_t get_int32_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b uint32 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b uint32 queried
     */
    RTPS_DllAPI uint32_t get_uint32_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b int16 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b int16 queried
     */
    RTPS_DllAPI int16_t get_int16_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b uint16 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b uint16 queried
     */
    RTPS_DllAPI uint16_t get_uint16_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b int64 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b int64 queried
     */
    RTPS_DllAPI int64_t get_int64_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b uint64 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b uint64 queried
     */
    RTPS_DllAPI uint64_t get_uint64_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b float32 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b float32 queried
     */
    RTPS_DllAPI float get_float32_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b float64 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b float64 queried
     */
    RTPS_DllAPI double get_float64_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b float128 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b float128 queried
     */
    RTPS_DllAPI long double get_float128_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b char8 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b char8 queried
     */
    RTPS_DllAPI char get_char8_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b char16 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b char16 queried
     */
    RTPS_DllAPI wchar_t get_char16_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b byte associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b byte queried
     */
    RTPS_DllAPI octet get_byte_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b int8 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b int8 queried
     */
    RTPS_DllAPI int8_t get_int8_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b uint8 associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b uint8 queried
     */
    RTPS_DllAPI uint8_t get_uint8_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b bool associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b bool queried
     */
    RTPS_DllAPI bool get_bool_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b bool associated to an identifier
     * @param [in] name string because \b bitmask can be addressed by name
     * @throws \@ref ReturnCode_t on failure
     * @return \b bool queried
     */
    RTPS_DllAPI bool get_bool_value(
            const char* name) const;

    /*
     * Convenient override to retrieve an \b string associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b string queried
     */
    RTPS_DllAPI const char* get_string_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b wstring associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b wstring queried
     */
    RTPS_DllAPI const wchar_t* get_wstring_value(
            MemberId id = MEMBER_ID_INVALID) const;

    /*
     * Convenient override to retrieve an \b enum associated to an identifier
     * @param [in] id identifier of the member to query. \b MEMBER_ID_INVALID for primitives.
     * @throws \@ref ReturnCode_t on failure
     * @return \b enum queried
     */
    RTPS_DllAPI const char* get_enum_value(
            MemberId id = MEMBER_ID_INVALID) const;

    RTPS_DllAPI ReturnCode_t get_union_label(
            uint64_t& value) const;

    RTPS_DllAPI uint64_t get_union_label() const;

    RTPS_DllAPI MemberId get_discriminator_value() const;

    RTPS_DllAPI ReturnCode_t get_discriminator_value(
            MemberId& id) const noexcept;

    RTPS_DllAPI ReturnCode_t set_discriminator_value(
            MemberId value) noexcept;

    //! Insert a new key in a map
    RTPS_DllAPI ReturnCode_t insert_map_data(
            const DynamicData& key,
            MemberId& outKeyId,
            MemberId& outValueId);

    //! Insert a new key-value pair in a map
    RTPS_DllAPI ReturnCode_t insert_map_data(
            const DynamicData& key,
            const DynamicData& value,
            MemberId& outKey,
            MemberId& outValue);

    //! Remove a key from a map
    RTPS_DllAPI ReturnCode_t remove_map_data(
            MemberId keyId);

    /*
     * Serialize the object into a given stream
     * @param cdr @ref eprosima::fastrtps::Cdr to fill in
     */
    RTPS_DllAPI void serialize(
            eprosima::fastcdr::Cdr& cdr) const;

    /*
     * Deserialize the object from the given payload
     * @param cdr @ref eprosima::fastrtps::Cdr to parse
     * @return bool specifying success
     */
    RTPS_DllAPI bool deserialize(
            eprosima::fastcdr::Cdr& cdr);

    /*
     * Serialize the object key into a given stream
     * @param cdr @ref eprosima::fastrtps::Cdr to fill in
     */
    RTPS_DllAPI void serializeKey(
            eprosima::fastcdr::Cdr& cdr) const;

    /*
     * Calculate an object serialized size
     * @param data @ref DynamicData object to serialize
     * @param current_alignment size_t
     * @return size_t calculated size in bytes
     */
    RTPS_DllAPI static size_t getCdrSerializedSize(
            const DynamicData& data,
            size_t current_alignment = 0);

    /*
     * Calculate an empty object serialized size
     * @param type @ref DynamicType object to serialize
     * @param current_alignment size_t
     * @return size_t calculated size in bytes
     */
    RTPS_DllAPI static size_t getEmptyCdrSerializedSize(
            const DynamicType& type,
            size_t current_alignment = 0);

    /*
     * Calculate maximum serialized size of the type's key
     * @param type @ref DynamicType object to serialize
     * @param current_alignment size_t
     * @return size_t calculated size in bytes
     * @remark returned value is a guidance, for example:
     *  a 100 sequence length is taken as reference
     */
    RTPS_DllAPI static size_t getKeyMaxCdrSerializedSize(
            const DynamicType& type,
            size_t current_alignment = 0);

    /*
     * Calculate maximum serialized size
     * @param type @ref DynamicType object to serialize
     * @param current_alignment size_t
     * @return size_t calculated size in bytes
     * @remark returned value is a guidance, for example:
     *  a 100 sequence length is taken as reference
     */
    RTPS_DllAPI static size_t getMaxCdrSerializedSize(
            const DynamicType& type,
            size_t current_alignment = 0);

};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_DATA_HPP
