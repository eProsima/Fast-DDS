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

/*!
 * @file Property.hpp
 */
#ifndef FASTDDS_RTPS_COMMON__PROPERTY_HPP
#define FASTDDS_RTPS_COMMON__PROPERTY_HPP

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace rtps {

class Property
{
public:

    Property()
    {
    }

    Property(
            const Property& property)
        : name_(property.name_)
        , value_(property.value_)
        , propagate_(property.propagate_)
    {
    }

    Property(
            Property&& property)
        : name_(std::move(property.name_))
        , value_(std::move(property.value_))
        , propagate_(property.propagate_)
    {
    }

    Property(
            const std::string& name,
            const std::string& value,
            bool propagate = false)
        : name_(name)
        , value_(value)
        , propagate_(propagate)
    {
    }

    Property(
            std::string&& name,
            std::string&& value,
            bool propagate = false)
        : name_(std::move(name))
        , value_(std::move(value))
        , propagate_(propagate)
    {
    }

    Property& operator =(
            const Property& property)
    {
        name_ = property.name_;
        value_ = property.value_;
        propagate_ = property.propagate_;
        return *this;
    }

    Property& operator =(
            Property&& property)
    {
        name_ = std::move(property.name_);
        value_ = std::move(property.value_);
        propagate_ = property.propagate_;
        return *this;
    }

    bool operator ==(
            const Property& b) const
    {
        return (this->name_ == b.name_) &&
               (this->value_ == b.value_);
    }

    void name(
            const std::string& name)
    {
        name_ = name;
    }

    void name(
            std::string&& name)
    {
        name_ = std::move(name);
    }

    const std::string& name() const
    {
        return name_;
    }

    std::string& name()
    {
        return name_;
    }

    void value(
            const std::string& value)
    {
        value_ = value;
    }

    void value(
            std::string&& value)
    {
        value_ = std::move(value);
    }

    const std::string& value() const
    {
        return value_;
    }

    std::string& value()
    {
        return value_;
    }

    void propagate(
            bool propagate)
    {
        propagate_ = propagate;
    }

    bool propagate() const
    {
        return propagate_;
    }

    bool& propagate()
    {
        return propagate_;
    }

private:

    std::string name_;

    std::string value_;

    bool propagate_ = false;
};

typedef std::vector<Property> PropertySeq;

class PropertyHelper
{
public:

    static size_t serialized_size(
            const Property& property,
            size_t current_alignment = 0)
    {
        if (property.propagate())
        {
            size_t initial_alignment = current_alignment;

            current_alignment += 4 + alignment(current_alignment, 4) + property.name().size() + 1;
            current_alignment += 4 + alignment(current_alignment, 4) + property.value().size() + 1;

            return current_alignment - initial_alignment;
        }
        else
        {
            return 0;
        }
    }

    static size_t serialized_size(
            const PropertySeq& properties,
            size_t current_alignment = 0)
    {
        size_t initial_alignment = current_alignment;

        current_alignment += 4 + alignment(current_alignment, 4);
        for (auto property = properties.begin(); property != properties.end(); ++property)
        {
            current_alignment += serialized_size(*property, current_alignment);
        }

        return current_alignment - initial_alignment;
    }

private:

    inline static size_t alignment(
            size_t current_alignment,
            size_t dataSize)
    {
        return (dataSize - (current_alignment % dataSize)) & (dataSize - 1);
    }

};

struct PropertyParser
{

    /**
     * @brief Parse a property value as an integer
     * @param property Property to parse
     * @param check_upper_bound If true, check that the value is lower than upper_bound
     * @param upper_bound Upper bound to check
     * @param check_lower_bound If true, check that the value is greater than lower_bound
     * @param lower_bound Lower bound to check
     * @param exception Exception to throw if the value is not a valid integer or if it is out of bounds
     * @return The parsed integer value
     *
     * @warning May throw an exception_t if the value is not a valid integer
     *  or if it is out of bounds.
     */
    template<typename exception_t>
    inline static int as_int(
            const Property& property,
            const bool& check_upper_bound,
            const int& upper_bound,
            const bool& check_lower_bound,
            const int& lower_bound,
            const exception_t& exception)
    {
        return parse_value(
            std::function<int(const Property& property)>(
                [](const Property& property)
                {
                    return std::stoi(property.value());
                }
                ),
            property,
            check_upper_bound,
            upper_bound,
            check_lower_bound,
            lower_bound,
            exception);
    }

    /**
     * @brief Parse a property value as a double
     * @param property Property to parse
     * @param check_upper_bound If true, check that the value is lower than upper_bound
     * @param upper_bound Upper bound to check
     * @param check_lower_bound If true, check that the value is greater than lower_bound
     * @param lower_bound Lower bound to check
     * @param exception Exception to throw if the value is not a valid double or if it is out of bounds
     * @return The parsed double value
     *
     * @warning May throw an exception_t if the value is not a valid double
     *  or if it is out of bounds.
     */
    template<typename exception_t>
    inline static double as_double(
            const Property& property,
            const bool& check_upper_bound,
            const double& upper_bound,
            const bool& check_lower_bound,
            const double& lower_bound,
            const exception_t& exception)
    {
        return parse_value(
            std::function<double(const Property& property)>(
                [](const Property& property)
                {
                    return std::stod(property.value());
                }
                ),
            property,
            check_upper_bound,
            upper_bound,
            check_lower_bound,
            lower_bound,
            exception);
    }

private:

    template <typename value_t,
            typename exception_t>
    inline static value_t parse_value(
            const std::function<value_t(const Property&)>& conversor,
            const Property& property,
            const bool& check_upper_bound,
            const value_t& upper_bound,
            const bool& check_lower_bound,
            const value_t& lower_bound,
            const exception_t& exception)
    {
        try
        {
            value_t converted_value = conversor(property);

            if (check_lower_bound && converted_value < lower_bound)
            {
                throw exception_t("Value '" + property.value() +
                              "' for " + property.name() + " must be greater or equal to " +
                              std::to_string(lower_bound));
            }

            if (check_upper_bound && converted_value > upper_bound)
            {
                throw exception_t("Value '" + property.value() +
                              "' for " + property.name() + " must be lower or equal to " +
                              std::to_string(upper_bound));
            }

            return converted_value;
        }
        catch (const std::invalid_argument&)
        {
            throw exception;
        }
        catch (const std::out_of_range&)
        {
            throw exception;
        }
    }

};

} //namespace eprosima
} //namespace fastdds
} //namespace rtps

#endif // FASTDDS_RTPS_COMMON__PROPERTY_HPP
