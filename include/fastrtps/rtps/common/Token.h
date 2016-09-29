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
 * @file Token.h	
 */
#ifndef _RTPS_COMMON_TOKEN_H_
#define _RTPS_COMMON_TOKEN_H_

#include "Property.h"
#include "BinaryProperty.h"

#include <string>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class DataHolder
{
    public:

        DataHolder(const DataHolder& data_holder) :
            _class_id(data_holder._class_id),
            _properties(data_holder._properties),
            _binary_properties(data_holder._binary_properties) {}

        DataHolder(DataHolder&& data_holder) :
            _class_id(data_holder._class_id),
            _properties(data_holder._properties),
            _binary_properties(data_holder._binary_properties) {}

        const std::string& class_id() const
        {
            return _class_id;
        }

        const PropertySeq& properties() const
        {
            return _properties;
        }

        PropertySeq& properties()
        {
            return _properties;
        }

        const BinaryPropertySeq& binary_properties() const
        {
            return _binary_properties;
        }

        BinaryPropertySeq& binary_properties()
        {
            return _binary_properties;
        }

    protected:

        std::string _class_id;

    private:

        PropertySeq _properties;

        BinaryPropertySeq _binary_properties;
};

typedef std::vector<DataHolder> DataHolderSeq;
typedef DataHolder Token;

} //namespace eprosima
} //namespace fastrtps
} //namespace rtps

#endif // _RTPS_COMMON_TOKEN_H_
