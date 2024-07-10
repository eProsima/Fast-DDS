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

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <memory>
#include <regex>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

class DDSFilterPredicate;

/**
 * Represents a value (either constant, parameter or fieldname) on a filter expression.
 */
class DDSFilterValue
{

public:

    // DDSFilterPredicate needs to call protected method add_parent
    friend class DDSFilterPredicate;

    /**
     * The high-level kind of a DDSFilterValue.
     * Please note that the constants here should follow the promotion order.
     */
    enum class ValueKind
    {
        BOOLEAN,            ///< Value is a bool
        ENUM,               ///< Value is an int32_t with the value of an enumeration
        SIGNED_INTEGER,     ///< Value is a int16_t, int32_t, or int64_t
        UNSIGNED_INTEGER,   ///< Value is a uint8_t, uint16_t, uint32_t, or uint64_t
        FLOAT_CONST,        ///< Value is a long double constant
        FLOAT_FIELD,        ///< Value is a float field
        DOUBLE_FIELD,       ///< Value is a double field
        LONG_DOUBLE_FIELD,  ///< Value is a long double field
        CHAR,               ///< Value is a char
        STRING              ///< Value is a string
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
        eprosima::fastcdr::string_255 string_value;   ///< Value when kind == STRING
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
     * @param [in] kind  The kind with which to construct the DDSFilterValue.
     */
    explicit DDSFilterValue(
            ValueKind data_kind) noexcept
        : kind(data_kind)
        , string_value()
    {
    }

    // *INDENT-OFF*
    DDSFilterValue(const DDSFilterValue&) = delete;
    DDSFilterValue& operator=(const DDSFilterValue&) = delete;
    DDSFilterValue(DDSFilterValue&&) = default;
    DDSFilterValue& operator=(DDSFilterValue&&) = default;
    // *INDENT-ON*

    virtual ~DDSFilterValue() = default;

    /**
     * Copy the state of this object from another one.
     *
     * @param [in] other                    The DDSFilterValue from where to copy the state.
     * @param [in] copy_regular_expression  Whether the regular expression state should be copied or not
     */
    void copy_from(
            const DDSFilterValue& other,
            bool copy_regular_expression) noexcept;

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

    /**
     * Mark that this value should be handled as a regular expression.
     *
     * @param [in] is_like_operand  Whether this value is used on a LIKE or MATCH operation.
     */
    void as_regular_expression(
            bool is_like_operand);

    /**
     * @name Comparison operations
     * Methods implementing the comparison operators of binary predicates.
     * Should only be called against a DDSFilterValue of a compatible kind,
     * according to the type promotion restrictions.
     */
    ///@{
    inline bool operator ==(
            const DDSFilterValue& other) const noexcept
    {
        return compare(*this, other) == 0;
    }

    inline bool operator !=(
            const DDSFilterValue& other) const noexcept
    {
        return compare(*this, other) != 0;
    }

    inline bool operator <(
            const DDSFilterValue& other) const noexcept
    {
        return compare(*this, other) < 0;
    }

    inline bool operator <=(
            const DDSFilterValue& other) const noexcept
    {
        return compare(*this, other) <= 0;
    }

    inline bool operator >(
            const DDSFilterValue& other) const noexcept
    {
        return compare(*this, other) > 0;
    }

    inline bool operator >=(
            const DDSFilterValue& other) const noexcept
    {
        return compare(*this, other) >= 0;
    }

    bool is_like(
            const DDSFilterValue& other) const noexcept;
    ///@}

protected:

    /**
     * Called when this DDSFilterValue is used on a DDSFilterPredicate.
     *
     * @param parent [in]  The DDSFilterPredicate referencing this DDSFilterValue.
     */
    virtual void add_parent(
            DDSFilterPredicate* parent)
    {
        static_cast<void>(parent);
    }

    /**
     * Called when the value of this DDSFilterValue has changed.
     * Will regenerate the regular expression object if as_regular_expression was called.
     */
    void value_has_changed();

private:

    enum class RegExpKind
    {
        NONE, LIKE, MATCH
    };

    RegExpKind regular_expr_kind_ = RegExpKind::NONE;
    std::unique_ptr<std::regex> regular_expr_;

    static int compare(
            const DDSFilterValue& lhs,
            const DDSFilterValue& rhs) noexcept;

};

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // _FASTDDS_TOPIC_DDSSQLFILTER_DDSFILTERVALUE_HPP_
