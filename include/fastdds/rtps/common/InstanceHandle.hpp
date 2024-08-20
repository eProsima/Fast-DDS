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

/**
 * @file InstanceHandle.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__INSTANCEHANDLE_HPP
#define FASTDDS_RTPS_COMMON__INSTANCEHANDLE_HPP

#include <array>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/common/Guid.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

constexpr const uint8_t RTPS_KEY_HASH_SIZE = 16;

using KeyHash_t = std::array<octet, RTPS_KEY_HASH_SIZE>;

struct FASTDDS_EXPORTED_API InstanceHandleValue_t
{
    /**
     * Write access indexing operator.
     *
     * Provides a reference to the byte value at position @c i.
     *
     * @param [in] i index of the byte to return.
     *
     * @post Method has_been_set() returns @c true.
     *
     * @remark Do not use this method to check if this value has been set.
     *         Use method has_been_set() instead.
     */
    template<typename T>
    octet& operator [] (
            T i) noexcept
    {
        has_been_set_ = true;
        return value_[i];
    }

    /**
     * Read access indexing operator.
     *
     * Provides the byte value at position @c i.
     *
     * @param [in] i index of the byte to return.
     *
     * @remark Do not use this method to check if this value has been set.
     *         Use method has_been_set() instead.
     */
    template<typename T>
    octet operator [] (
            T i) const noexcept
    {
        return value_[i];
    }

    /**
     * Write access pointer cast operator.
     *
     * Provides a pointer to the start of the raw data.
     *
     * @post Method has_been_set() returns @c true.
     *
     * @remark Do not use this method to check if this value has been set.
     *         Use method has_been_set() instead.
     */
    operator octet* () noexcept
    {
        has_been_set_ = true;
        return value_.data();
    }

    /**
     * Read access pointer cast operator.
     *
     * Provides a pointer to the start of the raw data.
     *
     * @remark Do not use this method to check if this value has been set.
     *         Use method has_been_set() instead.
     */
    operator const octet* () const noexcept
    {
        return value_.data();
    }

    /**
     * Return whether any of the write access operators of this value has been used.
     */
    bool has_been_set() const noexcept
    {
        return has_been_set_;
    }

    void clear() noexcept
    {
        value_.fill(0);
        has_been_set_ = false;
    }

    /**
     * Equality comparison operator.
     */
    bool operator == (
            const InstanceHandleValue_t& other) const noexcept
    {
        return (has_been_set_ == other.has_been_set_) && (value_ == other.value_);
    }

    /**
     * Less than comparisor operator.
     */
    bool operator < (
            const InstanceHandleValue_t& other) const noexcept
    {
        if (has_been_set_)
        {
            return other.has_been_set_ && value_ < other.value_;
        }

        return other.has_been_set_;
    }

private:

    //! Hash value
    KeyHash_t value_ {};
    //! Flag indicating if value_ has been modified since the creation of this object
    bool has_been_set_ = false;
};

/**
 * Struct InstanceHandle_t, used to contain the key for WITH_KEY topics.
 * @ingroup COMMON_MODULE
 */
struct FASTDDS_EXPORTED_API InstanceHandle_t
{
    //!Value
    InstanceHandleValue_t value;

    InstanceHandle_t() noexcept = default;

    InstanceHandle_t(
            const InstanceHandle_t& ihandle) noexcept = default;

    InstanceHandle_t(
            const GUID_t& guid) noexcept
    {
        *this = guid;
    }

    /**
     * Assignment operator
     * @param ihandle Instance handle to copy the data from
     */
    InstanceHandle_t& operator =(
            const InstanceHandle_t& ihandle) noexcept = default;

    /**
     * Assignment operator
     * @param guid GUID to copy the data from
     */
    InstanceHandle_t& operator =(
            const GUID_t& guid) noexcept
    {
        octet* dst = value;
        memcpy(dst, guid.guidPrefix.value, 12);
        memcpy(&dst[12], guid.entityId.value, 4);
        return *this;
    }

    /**
     * Know if the instance handle is defined
     * @return True if the values are not zero.
     */
    bool isDefined() const noexcept
    {
        return value.has_been_set();
    }

    void clear() noexcept
    {
        value.clear();
    }

    // TODO Review this conversion once InstanceHandle_t is implemented as DDS standard defines
    explicit operator const GUID_t&() const noexcept
    {
        return *reinterpret_cast<const GUID_t*>(this);
    }

};

const InstanceHandle_t c_InstanceHandle_Unknown;

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Comparison operator
 * @param ihandle1 First InstanceHandle_t to compare
 * @param ihandle2 Second InstanceHandle_t to compare
 * @return True if equal
 */
inline bool operator ==(
        const InstanceHandle_t& ihandle1,
        const InstanceHandle_t& ihandle2) noexcept
{
    return ihandle1.value == ihandle2.value;
}

/**
 * @brief Comparison operator
 *
 * @param ihandle1 First InstanceHandle_t to compare
 * @param ihandle2 Second InstanceHandle_t to compare
 * @return True if not equal
 */
inline bool operator !=(
        const InstanceHandle_t& ihandle1,
        const InstanceHandle_t& ihandle2) noexcept
{
    return !(ihandle1 == ihandle2);
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Convert InstanceHandle_t to GUID
 * @param guid GUID to store the results
 * @param ihandle InstanceHandle_t to copy
 */
inline void iHandle2GUID(
        GUID_t& guid,
        const InstanceHandle_t& ihandle) noexcept
{
    const octet* value = ihandle.value;
    memcpy(guid.guidPrefix.value, value, 12);
    memcpy(guid.entityId.value, &value[12], 4);
}

/**
 * Convert GUID to InstanceHandle_t
 * @param ihandle InstanceHandle_t to store the results
 * @return GUID_t
 */
inline GUID_t iHandle2GUID(
        const InstanceHandle_t& ihandle) noexcept
{
    GUID_t guid;
    iHandle2GUID(guid, ihandle);
    return guid;
}

/**
 * @brief Comparison operator: checks if a InstanceHandle_t is less than another.
 *
 * @param h1 First InstanceHandle_t to compare.
 * @param h2 Second InstanceHandle_t to compare.
 * @return True if the first InstanceHandle_t is less than the second.
 */
inline bool operator <(
        const InstanceHandle_t& h1,
        const InstanceHandle_t& h2) noexcept
{
    return h1.value < h2.value;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * Stream operator: print an InstanceHandle_t.
 *
 * @param output Output stream.
 * @param iHandle InstanceHandle_t to print.
 * @return Stream operator.
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const InstanceHandle_t& iHandle)
{
    std::stringstream ss;
    ss << std::hex;
    for (uint8_t i = 0; i < 15; ++i)
    {
        ss << (int)iHandle.value[i] << ".";
    }
    ss << (int)iHandle.value[15u] << std::dec;
    return output << ss.str();
}

/**
 * Stream operator: retrieve an InstanceHandle_t.
 *
 * @param input Input stream.
 * @param iHandle InstanceHandle_t that will receive the input as its new value.
 * @return Stream operator.
 */
inline std::istream& operator >>(
        std::istream& input,
        InstanceHandle_t& iHandle)
{
    std::istream::sentry s(input);

    if (s)
    {
        char point;
        unsigned short hex;
        std::ios_base::iostate excp_mask = input.exceptions();

        try
        {
            input.exceptions(excp_mask | std::ios_base::failbit | std::ios_base::badbit);
            input >> std::hex >> hex;

            if (hex > 255)
            {
                input.setstate(std::ios_base::failbit);
            }

            iHandle.value[0u] = static_cast<octet>(hex);

            for (uint8_t i = 1; i < 16; ++i)
            {
                input >> point >> hex;
                if ( point != '.' || hex > 255 )
                {
                    input.setstate(std::ios_base::failbit);
                }
                iHandle.value[i] = static_cast<octet>(hex);
            }

            input >> std::dec;
        }
        catch (std::ios_base::failure& )
        {
        }

        input.exceptions(excp_mask);
    }

    return input;
}

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__INSTANCEHANDLE_HPP
