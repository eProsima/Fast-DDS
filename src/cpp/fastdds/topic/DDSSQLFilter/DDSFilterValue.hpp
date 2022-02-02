// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DDSFilterValue.hpp
 */

#ifndef _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERVALUE_HPP_
#define _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERVALUE_HPP_

#include <fastrtps/utils/fixed_size_string.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

struct DDSFilterPredicate;

/**
 * Represents a value (either constant, parameter or fieldname) on a filter expression.
 */
struct DDSFilterValue
{
    friend struct DDSFilterPredicate;

    /**
     * The high-level kind of a DDSFilterValue.
     */
    enum class ValueKind
    {
        BOOLEAN,            ///< Value is a bool
        CHAR,               ///< Value is a char
        SIGNED_INTEGER,     ///< Value is a int16_t, int32_t, or int64_t
        UNSIGNED_INTEGER,   ///< Value is a uint8_t, uint16_t, uint32_t, or uint64_t
        FLOAT,              ///< Value is a float, double, or long double
        STRING,             ///< Value is a string
        ENUM                ///< Value is an int32_t with the value of an enumeration
    };

    /// The kind of value held by this DDSFilterValue
    ValueKind kind;

    union
    {
        bool boolean_value;                            ///< Value when kind == BOOL
        char char_value;                               ///< Value when kind == CHAR
        int64_t signed_integer_value;                  ///< Value when kind == SIGNED_INTEGER / ENUM
        uint64_t unsigned_integer_value;               ///< Value when kind == UNSIGNED_INTEGER
        long double float_value;                       ///< Value when kind == FLOAT
        eprosima::fastrtps::string_255 string_value;   ///< Value when kind == STRING
    };

    /**
     * Default constructor.
     * Constructs an empty string DDSFilterValue
     */
    DDSFilterValue() noexcept
        : kind(ValueKind::STRING)
        , string_value()
    {
    }

    /**
     * Explicit kind constructor.
     * Constructs a zero-valued, specific kind DDSFilterValue.
     *
     * @param[in] kind  The kind with which to construct the DDSFilterValue.
     */
    explicit DDSFilterValue(
            ValueKind data_kind) noexcept
        : kind(data_kind)
        , string_value()
    {
    }

    virtual ~DDSFilterValue() = default;

    /**
     * This method is used by a DDSFilterPredicate to check if this DDSFilterValue can be used.
     * Constants and parameters will always have a value, but fieldname-based values can only be
     * used after deserialization.
     *
     * @return whether this DDSFilterValue has a value that can be used on a predicate.
     */
    virtual bool has_value() const noexcept
    {
        return true;
    }

    /**
     * Instruct this value to reset.
     * Will only have effect on fieldname-based values.
     */
    virtual void reset() noexcept
    {
    }

protected:

    virtual void add_parent(
            DDSFilterPredicate* parent)
    {
        static_cast<void>(parent);
    }

};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERVALUE_HPP_
