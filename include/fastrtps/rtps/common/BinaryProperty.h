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
            _name(property._name),
            _value(property._value),
            _propagate(property._propagate) {}

        BinaryProperty(BinaryProperty&& property) :
            _name(std::move(property._name)),
            _value(std::move(property._value)),
            _propagate(property._propagate) {}

        void name(const std::string& name)
        {
            _name = name;
        }

        void name(std::string&& name)
        {
            _name = std::move(name);
        }

        const std::string& name() const
        {
            return _name;
        }

        std::string& name()
        {
            return _name;
        }

        void value(const std::vector<uint8_t>& value)
        {
            _value = value;
        }

        void value(std::vector<uint8_t>&& value)
        {
            _value = std::move(value);
        }

        const std::vector<uint8_t>& value() const
        {
            return _value;
        }

        std::vector<uint8_t>& value()
        {
            return _value;
        }

        void propagate(bool propagate)
        {
            _propagate = propagate;
        }

        bool propagate() const
        {
            return _propagate;
        }

        bool& propagate()
        {
            return _propagate;
        }

    private:

        std::string _name;

        std::vector<uint8_t> _value;

        bool _propagate;
};

typedef std::vector<BinaryProperty> BinaryPropertySeq;

} //namespace eprosima
} //namespace fastrtps
} //namespace rtps

#endif // _RTPS_COMMON_BINARYPROPERTYQOS_H_
