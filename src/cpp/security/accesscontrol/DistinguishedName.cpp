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
 * @file DistinguishedName.cpp
 */

#include <array>
#include <cstring>

#include <fastdds/dds/log/Log.hpp>

#include <security/accesscontrol/DistinguishedName.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

namespace detail {

Attribute::Attribute(
        const DistinguishedName& name)
    : value(name.c_str())
    , length(name.size())
{
    // Do nothing
}

Attribute::Attribute(
        const char* name)
    : value(name)
{
    size_t i = 0;
    while (name[i] != 0)
    {
        ++i;
    }
    length = i;
}

bool Attribute::is_set() const noexcept
{
    return length > 0;
}

size_t Attribute::cut(
        size_t ini,
        size_t fin /*= 0*/) noexcept
{
    // If cut greater than actual size, return empty string
    if (ini >= length || fin > length)
    {
        return clear();
    }

    // If fin is 0, cut to the end
    if (fin == 0)
    {
        value += ini;
        length -= ini;
        return ini;
    }

    // If fin is lower or equal ini, empty string
    if (fin <= ini)
    {
        return clear();
    }

    // Otherwise cut it
    value += ini;
    size_t aux = length;
    length = fin - ini;
    return aux - length;
}

size_t Attribute::trim_blank_spaces()
{
    size_t i = 0;
    while (i < length && value[i] == ' ')
    {
        i++;
    }
    return cut(i);
}

size_t Attribute::trim_back_blank_spaces()
{
    size_t i = length;
    // NOTE: using -1 because size_t cannot be lower than 0, so comparison i >= 0 make no sense
    while (i > 0 && value[i - 1] == ' ')
    {
        i--;
    }
    return cut(0, i);
}

size_t Attribute::clear() noexcept
{
    value = nullptr;
    size_t aux = length;
    length = 0;
    return aux;
}

size_t Attribute::find(
        char c) const noexcept
{
    size_t i = 0;
    bool in_quotes = false;
    while (i < length)
    {
        if (!in_quotes && value[i] == c)
        {
            return i;
        }

        if (value[i] == '"')
        {
            in_quotes = !in_quotes;
        }

        else if (value[i] == '\\')
        {
            i++;
        }

        i++;
    }
    return i;
}

size_t Attribute::find(
        char c,
        bool& result) const noexcept
{
    auto res = find(c);
    result = found(res);
    return res;
}

bool Attribute::found(
        const size_t& find_result) const noexcept
{
    return find_result < length;
}

bool Attribute::operator ==(
        const Attribute& other) const noexcept
{
    // Compare char to char
    // scaped chars with \ must be taken into account, and " must be ignored
    size_t index_this = 0;
    size_t index_other = 0;
    while (true)
    {
        while (
            index_this < this->length
            && (this->value[index_this] == '\\' || this->value[index_this] == '"'))
        {
            index_this++;
        }

        while (
            index_other < other.length
            && (other.value[index_other] == '\\' || other.value[index_other] == '"'))
        {
            index_other++;
        }

        if (index_this == this->length && index_other == other.length)
        {
            return true;
        }

        if (
            !(index_this < this->length)
            || !(index_other < other.length)
            || (this->value[index_this] != other.value[index_other]))
        {
            return false;
        }
        else
        {
            index_this++;
            index_other++;
        }
    }
}

bool Attribute::operator !=(
        const Attribute& other) const noexcept
{
    return !operator ==(other);
}

Attribute Attribute::cut(
        const Attribute& att,
        size_t ini,
        size_t fin /*= 0*/) noexcept
{
    Attribute res {att};
    res.cut(ini, fin);
    return res;
}

std::ostream& operator <<(
        std::ostream& os,
        const Attribute& att)
{
    os.write(att.value, att.length);
    return os;
}

bool DistinguishedNameSpecialized::compare(
        const DistinguishedName& name1,
        const DistinguishedName& name2) noexcept
{
    return DistinguishedNameSpecialized(name1) == DistinguishedNameSpecialized(name2);
}

DistinguishedNameSpecialized::DistinguishedNameSpecialized(
        const DistinguishedName& name) noexcept
{
    // Create it by recursive function
    auto res = find_and_add_type_values(name);

    switch (res)
    {
        case ErrorCase::MAX_VALUE:
            EPROSIMA_LOG_ERROR(
                SECURITY,
                "DistinguishedName " << name << " have more type-values attributes than allowed.");
            break;

        case ErrorCase::EMPTY:
            EPROSIMA_LOG_ERROR(
                SECURITY,
                "DistinguishedName " << name << " has an empty field.");
            break;

        case ErrorCase::NO_TYPE_VALUE_FORMAT:
            EPROSIMA_LOG_ERROR(
                SECURITY,
                "DistinguishedName " << name << " has incorrect format.");
            break;

        default:
            break;
    }
}

DistinguishedNameSpecialized::ErrorCase DistinguishedNameSpecialized::find_and_add_type_values(
        const Attribute& input) noexcept
{
    Attribute rest = input;
    rest.trim_blank_spaces();
    // If input has finished, stop
    if (!rest.is_set())
    {
        return ErrorCase::EMPTY;
    }

    // If this object is already full, failure (no exception to follow fast policy)
    if (size() >= MAX_VALUES_)
    {
        return ErrorCase::MAX_VALUE;
    }

    //////////////////////////////////////////
    // Variables
    bool found = false;
    size_t index = 0;

    //////////////////////////////////////////
    // Find first =
    index = rest.find('=', found);
    if (!found)
    {
        return ErrorCase::NO_TYPE_VALUE_FORMAT;
    }
    Attribute type = Attribute::cut(rest, 0, index);
    rest.cut(index + 1);

    //////////////////////////////////////////
    // Find first ,
    index = rest.find(',', found);
    if (!found)
    {
        // Last element found, add it and finish
        add_type_values(type, rest);
        return ErrorCase::OK;
    }

    // If it is not the last one, continue
    Attribute value = Attribute::cut(rest, 0, index);
    rest.cut(index + 1);

    // Add values
    add_type_values(type, value);

    //////////////////////////////////////////
    // Keep adding values
    return find_and_add_type_values(rest);
}

void DistinguishedNameSpecialized::add_type_values(
        const Attribute& type,
        const Attribute& value) noexcept
{
    // NOTE: storing values before trimming them avoids one copy

    //////////////////////////////////////////
    // Store new values
    types_[size_] = type;
    values_[size_] = value;

    //////////////////////////////////////////
    // Trim blank spaces
    types_[size_].trim_blank_spaces();
    types_[size_].trim_back_blank_spaces();
    values_[size_].trim_blank_spaces();
    values_[size_].trim_back_blank_spaces();

    size_++;
}

size_t DistinguishedNameSpecialized::size() const noexcept
{
    return size_;
}

DistinguishedNameSpecialized::Value DistinguishedNameSpecialized::get_attribute(
        const Type& type) const noexcept
{
    for (size_t i = 0; i < size_; ++i)
    {
        if (!types_[i].is_set())
        {
            // Finish if no more types
            break;
        }
        else if (types_[i] == type)
        {
            return values_[i];
        }
    }
    return Value();
}

bool DistinguishedNameSpecialized::operator ==(
        const DistinguishedNameSpecialized& other) const noexcept
{
    if (this->size() != other.size())
    {
        return false;
    }

    for (size_t i = 0; i < size_; ++i)
    {
        if (other.get_attribute(this->types_[i]) != this->values_[i])
        {
            return false;
        }
    }

    // If same size and all keys in A are in B, all keys in B are in A
    return true;
}

} //namespace detail

bool rfc2253_string_compare(
        const DistinguishedName& name1,
        const DistinguishedName& name2)
{
    return detail::DistinguishedNameSpecialized::compare(name1, name2);
}

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima
