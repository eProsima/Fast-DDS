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

/*!
 * @file DistinguishedName.h
 */

#ifndef _SECURITY_ACCESSCONTROL_DISTINGUISHEDNAME_H_
#define _SECURITY_ACCESSCONTROL_DISTINGUISHEDNAME_H_

#include <array>
#include <string>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

/*
 * RFC 1779: https://datatracker.ietf.org/doc/html/rfc1779 specifies how to represent DistinguishedNames
 * in string format (obsolete in favor of rfc2253).
 *
 * RFC 2253: https://datatracker.ietf.org/doc/html/rfc2253 specifies how to represent DistinguishedNames
 * in string UTF-8 format.
 *
 * ASN.1 representation of DistinguishedNames
 * DistinguishedName ::= RDNSequence
 *
 * RDNSequence ::= SEQUENCE OF RelativeDistinguishedName
 *
 * RelativeDistinguishedName ::= SET SIZE (1..MAX) OF
 * AttributeTypeAndValue
 *
 * AttributeTypeAndValue ::= SEQUENCE {
 * type  AttributeType,
 * value AttributeValue }
 *
 * Each RDN in a RDNSequence is separated by ','.
 * Each AttributeTypeAndValue in a RDN is separated by '+'.
 * Attribute and Value are separated by '='.
 *
 */

/**
 * @brief class that represents a Distinguished Name
 *
 * @note so far this implementation works only as a string
 * @todo make this a real class with real methods that correctly implement RFC 2253:
 */
using DistinguishedName = std::string;

/**
 * @brief This function compares 2 strings representing DistinguishedNames.
 *
 * Follow RFC 2253: https://datatracker.ietf.org/doc/html/rfc2253
 * The idea here is that comparing 2 DistinguishedNames is not as trivial as comparing 2 strings,
 * nor comparing its values splitting the string.
 * The comparison must be a complex function that takes into account many specific detail.
 *
 * So far, Fast DDS gives 2 strings (in some format that we assume is the correct one) and this compares
 * whether the two strings refer to the same DistinguishedName.
 * It would be better (and easier) to store DistinguishedName in a specialized function instead of in a string,
 * and thus this comparison would be much easier and efficient.
 * But this will be left for future improvements.
 *
 * @warning this function does not correctly fulfilled all requirements of RFC 2253.
 * Requirements that are not fulfilled:
 * - multi-attribute
 * - types accept void
 * - assumes that input is a well formed DistinguishedName following rfc2253
 * @todo make this function complete (probably better by implementing DistinguishedName class).
 *
 * @param name1 first DistinguishedName in string format
 * @param name2 second DistinguishedName in string format
 *
 * @return true if both first strings represent the same DistinguishedName.
 * @return false otherwise
 */
bool rfc2253_string_compare(
        const DistinguishedName& name1,
        const DistinguishedName& name2);


namespace detail {

/**
 * @brief Struct that stores a const char* and a size, referencing a string without dynamic allocation.
 *
 * This struct is only an auxiliary class to implement \c rfc2253_string_compare .
 * This class is not meant to be used outside this function.
 *
 * The necessity of this class arises from the need to compare two key-value format strings without
 * allocating heap memory.
 * Thus, it implements string formatter functions (cut, trim, find) only using char ptr and size.
 */
struct Attribute
{
    Attribute() = default;

    Attribute(
            const DistinguishedName& name);

    Attribute(
            const char* name);

    bool is_set() const noexcept;

    size_t cut(
            size_t ini,
            size_t fin = 0) noexcept;

    size_t trim_blank_spaces();

    size_t trim_back_blank_spaces();

    size_t clear() noexcept;

    size_t find(
            char c) const noexcept;

    size_t find(
            char c,
            bool& result) const noexcept;

    bool found(
            const size_t& find_result) const noexcept;

    bool operator ==(
            const Attribute& other) const noexcept;

    bool operator !=(
            const Attribute& other) const noexcept;

    static Attribute cut(
            const Attribute& att,
            size_t ini,
            size_t fin = 0) noexcept;

    const char* value {nullptr};
    size_t length {0};
};

//! To string method for Attribute
std::ostream& operator <<(
        std::ostream& os,
        const Attribute& att);

/**
 * @brief This class is only used to compare 2 DistinguishedName.
 *
 * @warning be aware that this class does not store any memory, so using for something different than
 * comparison may lead to seg faults.
 */
class DistinguishedNameSpecialized
{
public:

    static bool compare(
            const DistinguishedName& name1,
            const DistinguishedName& name2) noexcept;

    enum class ErrorCase
    {
        OK,
        EMPTY,
        MAX_VALUE,
        NO_TYPE_VALUE_FORMAT,
    };

    using Type = Attribute;
    using Value = Attribute;

    DistinguishedNameSpecialized(
            const DistinguishedName& name) noexcept;

    ErrorCase find_and_add_type_values(
            const Attribute& input) noexcept;

    void add_type_values(
            const Attribute& type,
            const Attribute& value) noexcept;

    size_t size() const noexcept;

    Value get_attribute(
            const Type& type) const noexcept;

    bool operator ==(
            const DistinguishedNameSpecialized& other) const noexcept;

protected:

    static constexpr size_t MAX_VALUES_ = 20;

    std::array<Attribute, MAX_VALUES_> types_ {};
    std::array<Attribute, MAX_VALUES_> values_ {};
    bool ok_ {false};
    size_t size_ {0};
};

} //namespace detail

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _SECURITY_ACCESSCONTROL_DISTINGUISHEDNAME_H_
