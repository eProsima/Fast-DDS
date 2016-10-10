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

namespace eprosima {
namespace fastrtps {
namespace rtps {

class DataHolder
{
    public:

        DataHolder(const DataHolder& data_holder) :
            class_id_(data_holder.class_id_),
            properties_(data_holder.properties_),
            binary_properties_(data_holder.binary_properties_) {}

        DataHolder(DataHolder&& data_holder) :
            class_id_(data_holder.class_id_),
            properties_(data_holder.properties_),
            binary_properties_(data_holder.binary_properties_) {}

        const std::string& class_id() const
        {
            return class_id_;
        }

        const PropertySeq& properties() const
        {
            return properties_;
        }

        PropertySeq& properties()
        {
            return properties_;
        }

        const BinaryPropertySeq& binary_properties() const
        {
            return binary_properties_;
        }

        BinaryPropertySeq& binary_properties()
        {
            return binary_properties_;
        }

    protected:

        std::string class_id_;

    private:

        PropertySeq properties_;

        BinaryPropertySeq binary_properties_;
};

typedef std::vector<DataHolder> DataHolderSeq;
typedef DataHolder Token;
typedef Token IdentityToken;

} //namespace eprosima
} //namespace fastrtps
} //namespace rtps

#endif // _RTPS_COMMON_TOKEN_H_
