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
 * @file Property.h
 */
#ifndef _FASTDDS_RTPS_COMMON_PROPERTYQOS_H_
#define _FASTDDS_RTPS_COMMON_PROPERTYQOS_H_

#include <string>
#include <vector>

namespace eprosima {
namespace fastrtps {
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

} //namespace eprosima
} //namespace fastrtps
} //namespace rtps

#endif // _FASTDDS_RTPS_COMMON_PROPERTYQOS_H_
