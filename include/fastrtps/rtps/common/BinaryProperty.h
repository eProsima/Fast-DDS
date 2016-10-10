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
 * @file BinaryPropertyQos.h	
 */
#ifndef _RTPS_COMMON_BINARYPROPERTYQOS_H_
#define  _RTPS_COMMON_BINARYPROPERTYQOS_H_

#include <string>
#include <vector>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class BinaryProperty
{
    public:

        BinaryProperty(const BinaryProperty& property) :
            name_(property.name_),
            value_(property.value_),
            propagate_(property.propagate_) {}

        BinaryProperty(BinaryProperty&& property) :
            name_(std::move(property.name_)),
            value_(std::move(property.value_)),
            propagate_(property.propagate_) {}

        BinaryProperty(const std::string& name,
                const std::vector<uint8_t>& value) :
            name_(name), value_(value) {}

        BinaryProperty(std::string&& name,
                std::vector<uint8_t>&& value) :
            name_(std::move(name)), value_(std::move(value)) {}

        void name(const std::string& name)
        {
            name_ = name;
        }

        void name(std::string&& name)
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

        void value(const std::vector<uint8_t>& value)
        {
            value_ = value;
        }

        void value(std::vector<uint8_t>&& value)
        {
            value_ = std::move(value);
        }

        const std::vector<uint8_t>& value() const
        {
            return value_;
        }

        std::vector<uint8_t>& value()
        {
            return value_;
        }

        void propagate(bool propagate)
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

        std::vector<uint8_t> value_;

        bool propagate_;
};

typedef std::vector<BinaryProperty> BinaryPropertySeq;

} //namespace eprosima
} //namespace fastrtps
} //namespace rtps

#endif // _RTPS_COMMON_BINARYPROPERTYQOS_H_
