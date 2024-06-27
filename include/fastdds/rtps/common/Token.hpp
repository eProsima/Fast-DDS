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
 * @file Token.hpp
 */
#ifndef FASTDDS_RTPS_COMMON__TOKEN_HPP
#define FASTDDS_RTPS_COMMON__TOKEN_HPP

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Property.hpp>
#include <fastdds/rtps/common/BinaryProperty.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class DataHolder
{
public:

    DataHolder()
    {
    }

    DataHolder(
            const DataHolder& data_holder)
        : class_id_(data_holder.class_id_)
        , properties_(data_holder.properties_)
        , binary_properties_(data_holder.binary_properties_)
    {
    }

    DataHolder(
            DataHolder&& data_holder)
        : class_id_(data_holder.class_id_)
        , properties_(data_holder.properties_)
        , binary_properties_(data_holder.binary_properties_)
    {
    }

    DataHolder& operator =(
            const DataHolder& data_holder)
    {
        class_id_ = data_holder.class_id_;
        properties_ = data_holder.properties_;
        binary_properties_ = data_holder.binary_properties_;

        return *this;
    }

    DataHolder& operator =(
            DataHolder&& data_holder)
    {
        class_id_ = std::move(data_holder.class_id_);
        properties_ = std::move(data_holder.properties_);
        binary_properties_ = std::move(data_holder.binary_properties_);

        return *this;
    }

    bool is_nil() const
    {
        return class_id_.empty();
    }

    void class_id(
            const std::string& class_id)
    {
        class_id_ = class_id;
    }

    void class_id(
            std::string&& class_id)
    {
        class_id_ = std::move(class_id);
    }

    std::string& class_id()
    {
        return class_id_;
    }

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

private:

    std::string class_id_;

    PropertySeq properties_;

    BinaryPropertySeq binary_properties_;
};

typedef std::vector<DataHolder> DataHolderSeq;
typedef DataHolder Token;
typedef Token IdentityToken;
typedef Token IdentityStatusToken;
typedef Token PermissionsToken;
typedef Token AuthenticatedPeerCredentialToken;
typedef Token PermissionsCredentialToken;

class DataHolderHelper
{
public:

    static std::string* find_property_value(
            DataHolder& data_holder,
            const std::string& name);

    static const std::string* find_property_value(
            const DataHolder& data_holder,
            const std::string& name);

    static Property* find_property(
            DataHolder& data_holder,
            const std::string& name);

    static const Property* find_property(
            const DataHolder& data_holder,
            const std::string& name);

    static std::vector<uint8_t>* find_binary_property_value(
            DataHolder& data_holder,
            const std::string& name);

    static const std::vector<uint8_t>* find_binary_property_value(
            const DataHolder& data_holder,
            const std::string& name);

    static BinaryProperty* find_binary_property(
            DataHolder& data_holder,
            const std::string& name);

    static const BinaryProperty* find_binary_property(
            const DataHolder& data_holder,
            const std::string& name);

    static size_t serialized_size(
            const DataHolder& data_holder,
            size_t current_alignment = 0);

    static size_t serialized_size(
            const DataHolderSeq& data_holders,
            size_t current_alignment = 0);

private:

    inline static size_t alignment(
            size_t current_alignment,
            size_t dataSize)
    {
        return (dataSize - (current_alignment % dataSize)) & (dataSize - 1);
    }

};

} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_RTPS_COMMON__TOKEN_HPP
