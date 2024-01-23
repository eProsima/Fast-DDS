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
 * @file DDSFilterValue.cpp
 */

#include "DDSFilterValue.hpp"

#include <cassert>
#include <cstring>
#include <string>
#include <regex>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace DDSSQLFilter {

template<typename T>
int compare_values(
        T lvalue,
        T rvalue)
{
    return lvalue < rvalue ? -1 :
           lvalue > rvalue ? 1 : 0;
}

/**
 * Check if a value is negative.
 * Only used during promotion to UNSIGNED_INTEGER.
 */
static bool is_negative(
        const DDSFilterValue& value)
{
    switch (value.kind)
    {
        case DDSFilterValue::ValueKind::BOOLEAN:
            return false;

        case DDSFilterValue::ValueKind::ENUM:
        case DDSFilterValue::ValueKind::SIGNED_INTEGER:
            return value.signed_integer_value < 0;

        // The rest of the types shall never be promoted to UNSIGNED_INTEGER
        default:
            assert(false);
    }

    return false;
}

/**
 * Performs promotion to SIGNED_INTEGER
 */
static int64_t to_signed_integer(
        const DDSFilterValue& value)
{
    switch (value.kind)
    {
        case DDSFilterValue::ValueKind::BOOLEAN:
            return value.boolean_value ? 1 : 0;

        case DDSFilterValue::ValueKind::ENUM:
            return value.signed_integer_value;

        // The rest of the types shall never be promoted to SIGNED_INTEGER
        default:
            assert(false);
    }

    return 0;
}

/**
 * Performs promotion to UNSIGNED_INTEGER
 */
static uint64_t to_unsigned_integer(
        const DDSFilterValue& value)
{
    switch (value.kind)
    {
        case DDSFilterValue::ValueKind::BOOLEAN:
            return value.boolean_value ? 1 : 0;

        case DDSFilterValue::ValueKind::ENUM:
        case DDSFilterValue::ValueKind::SIGNED_INTEGER:
            return static_cast<uint64_t>(value.signed_integer_value);

        // The rest of the types shall never be promoted to UNSIGNED_INTEGER
        default:
            assert(false);
    }

    return 0;
}

/**
 * Performs promotion to FLOAT
 */
static long double to_float(
        const DDSFilterValue& value)
{
    switch (value.kind)
    {
        case DDSFilterValue::ValueKind::ENUM:
        case DDSFilterValue::ValueKind::SIGNED_INTEGER:
            return static_cast<long double>(value.signed_integer_value);

        case DDSFilterValue::ValueKind::UNSIGNED_INTEGER:
            return static_cast<long double>(value.unsigned_integer_value);

        case DDSFilterValue::ValueKind::FLOAT_CONST:
        case DDSFilterValue::ValueKind::FLOAT_FIELD:
        case DDSFilterValue::ValueKind::DOUBLE_FIELD:
        case DDSFilterValue::ValueKind::LONG_DOUBLE_FIELD:
            return value.float_value;

        // The rest of the types shall never be promoted to FLOAT
        default:
            assert(false);
    }

    return 0;
}

/**
 * Performs promotion to STRING
 */
static void to_string_value(
        const DDSFilterValue& in,
        eprosima::fastcdr::string_255& out)
{
    assert(DDSFilterValue::ValueKind::CHAR == in.kind);
    out.assign(&in.char_value, 1);
}

void DDSFilterValue::copy_from(
        const DDSFilterValue& other,
        bool copy_regular_expression) noexcept
{
    kind = other.kind;
    switch (kind)
    {
        case ValueKind::BOOLEAN:
            boolean_value = other.boolean_value;
            break;

        case ValueKind::CHAR:
            char_value = other.char_value;
            break;

        case ValueKind::ENUM:
        case ValueKind::SIGNED_INTEGER:
            signed_integer_value = other.signed_integer_value;
            break;

        case ValueKind::UNSIGNED_INTEGER:
            unsigned_integer_value = other.unsigned_integer_value;
            break;

        case ValueKind::FLOAT_CONST:
        case ValueKind::FLOAT_FIELD:
        case ValueKind::DOUBLE_FIELD:
        case ValueKind::LONG_DOUBLE_FIELD:
            float_value = other.float_value;
            break;

        case ValueKind::STRING:
            string_value = other.string_value;
            break;

        default:
            assert(false);
    }

    if (copy_regular_expression)
    {
        regular_expr_kind_ = other.regular_expr_kind_;
        if (has_value())
        {
            value_has_changed();
        }
    }
}

int DDSFilterValue::compare(
        const DDSFilterValue& lhs,
        const DDSFilterValue& rhs) noexcept
{
    if (lhs.kind == rhs.kind)
    {
        switch (lhs.kind)
        {
            case ValueKind::BOOLEAN:
                return lhs.boolean_value - rhs.boolean_value;

            case ValueKind::CHAR:
                return lhs.char_value - rhs.char_value;

            case ValueKind::SIGNED_INTEGER:
            case ValueKind::ENUM:
                return compare_values(lhs.signed_integer_value, rhs.signed_integer_value);

            case ValueKind::UNSIGNED_INTEGER:
                return compare_values(lhs.unsigned_integer_value, rhs.unsigned_integer_value);

            case ValueKind::FLOAT_FIELD:
                return compare_values(static_cast<float>(lhs.float_value), static_cast<float>(rhs.float_value));

            case ValueKind::DOUBLE_FIELD:
                return compare_values(static_cast<double>(lhs.float_value), static_cast<double>(rhs.float_value));

            case ValueKind::LONG_DOUBLE_FIELD:
                return compare_values(lhs.float_value, rhs.float_value);

            case ValueKind::STRING:
                return std::strcmp(lhs.string_value.c_str(), rhs.string_value.c_str());

            // As the grammar does not allow constant vs constant comparisons, FLOAT_CONST should never reach here
            case ValueKind::FLOAT_CONST:
            default:
                assert(false);
        }
    }
    else if (lhs.kind < rhs.kind)
    {
        // We want to always promote rhs
        return -compare(rhs, lhs);
    }
    else
    {
        /* Type promotion rules are enforced during the construction of the expression tree on
         * DDSFilterFactory. For the types on which promotion is supported, a corresponding to_xxx method exists
         * that will assert if an invalid promotion is performed.
         */
        switch (lhs.kind)
        {
            case ValueKind::ENUM:
            case ValueKind::SIGNED_INTEGER:
            {
                return compare_values(lhs.signed_integer_value, to_signed_integer(rhs));
            }

            case ValueKind::UNSIGNED_INTEGER:
                return is_negative(rhs) ? 1 : compare_values(lhs.unsigned_integer_value, to_unsigned_integer(rhs));

            case ValueKind::FLOAT_CONST:
            case ValueKind::FLOAT_FIELD:
                return compare_values(static_cast<float>(lhs.float_value), static_cast<float>(to_float(rhs)));

            case ValueKind::DOUBLE_FIELD:
                return compare_values(static_cast<double>(lhs.float_value), static_cast<double>(to_float(rhs)));

            case ValueKind::LONG_DOUBLE_FIELD:
                return compare_values(lhs.float_value, to_float(rhs));

            case ValueKind::STRING:
            {
                eprosima::fastcdr::string_255 rvalue;
                to_string_value(rhs, rvalue);
                return std::strcmp(lhs.string_value.c_str(), rvalue.c_str());
            }

            // Promotion to boolean or char are not allowed.
            case ValueKind::BOOLEAN:
            case ValueKind::CHAR:
            default:
                assert(false);
                break;
        }
    }

    return 0;
}

void DDSFilterValue::as_regular_expression(
        bool is_like_operand)
{
    regular_expr_kind_ = is_like_operand ? RegExpKind::LIKE : RegExpKind::MATCH;
    if (has_value())
    {
        value_has_changed();
    }
}

void DDSFilterValue::value_has_changed()
{
    if (RegExpKind::NONE != regular_expr_kind_)
    {
        std::string expr;

        switch (kind)
        {
            case ValueKind::CHAR:
                expr = char_value;
                break;

            case ValueKind::STRING:
                expr = string_value.c_str();
                break;

            default:
                assert(false);
        }

        if (RegExpKind::LIKE == regular_expr_kind_)
        {
            expr = std::regex_replace(expr, std::regex("\\*"), ".*");
            expr = std::regex_replace(expr, std::regex("\\?"), ".");
            expr = std::regex_replace(expr, std::regex("%"), ".*");
            expr = std::regex_replace(expr, std::regex("_"), ".");
        }

        regular_expr_.reset(new std::regex(expr));
    }
}

bool DDSFilterValue::is_like(
        const DDSFilterValue& other) const noexcept
{
    assert(other.regular_expr_);

    eprosima::fastcdr::string_255 char_string_value;

    switch (kind)
    {
        case ValueKind::CHAR:
            assert(ValueKind::STRING == other.kind);
            to_string_value(*this, char_string_value);
            return std::regex_match(char_string_value.c_str(), *other.regular_expr_);

        case ValueKind::STRING:
            switch (other.kind)
            {
                case ValueKind::CHAR:
                case ValueKind::STRING:
                    return std::regex_match(string_value.c_str(), *other.regular_expr_);

                default:
                    assert(false);
            }
            break;

        default:
            assert(false);
    }

    return false;
}

}  // namespace DDSSQLFilter
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
