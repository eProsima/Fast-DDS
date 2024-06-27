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
 * @file PropertyPolicy.hpp
 */
#ifndef FASTDDS_RTPS_ATTRIBUTES__PROPERTYPOLICY_HPP
#define FASTDDS_RTPS_ATTRIBUTES__PROPERTYPOLICY_HPP

#include <fastdds/rtps/common/Property.hpp>
#include <fastdds/rtps/common/BinaryProperty.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class PropertyPolicy
{
public:

    FASTDDS_EXPORTED_API PropertyPolicy()
    {
    }

    FASTDDS_EXPORTED_API PropertyPolicy(
            const PropertyPolicy& property_policy)
        : properties_(property_policy.properties_)
        , binary_properties_(property_policy.binary_properties_)
    {
    }

    FASTDDS_EXPORTED_API PropertyPolicy(
            PropertyPolicy&& property_policy)
        : properties_(std::move(property_policy.properties_))
        , binary_properties_(std::move(property_policy.binary_properties_))
    {
    }

    FASTDDS_EXPORTED_API PropertyPolicy& operator =(
            const PropertyPolicy& property_policy)
    {
        properties_ = property_policy.properties_;
        binary_properties_ = property_policy.binary_properties_;
        return *this;
    }

    FASTDDS_EXPORTED_API PropertyPolicy& operator =(
            PropertyPolicy&& property_policy)
    {
        properties_ = std::move(property_policy.properties_);
        binary_properties_ = std::move(property_policy.binary_properties_);
        return *this;
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const PropertyPolicy& b) const
    {
        return (this->properties_ == b.properties_) &&
               (this->binary_properties_ == b.binary_properties_);
    }

    FASTDDS_EXPORTED_API bool operator !=(
            const PropertyPolicy& b) const
    {
        return !(*this == b);
    }

    //!Get properties
    FASTDDS_EXPORTED_API const PropertySeq& properties() const
    {
        return properties_;
    }

    //!Set properties
    FASTDDS_EXPORTED_API PropertySeq& properties()
    {
        return properties_;
    }

    //!Get binary_properties
    FASTDDS_EXPORTED_API const BinaryPropertySeq& binary_properties() const
    {
        return binary_properties_;
    }

    //!Set binary_properties
    FASTDDS_EXPORTED_API BinaryPropertySeq& binary_properties()
    {
        return binary_properties_;
    }

private:

    PropertySeq properties_;

    BinaryPropertySeq binary_properties_;
};

class PropertyPolicyHelper
{
public:

    /*!
     * @brief Returns only the properties whose name starts with the prefix.
     * Prefix is removed in returned properties.
     * @param property_policy PropertyPolicy where properties will be searched.
     * @param prefix Prefix used to search properties.
     * @return A copy of properties whose name starts with the prefix.
     */
    FASTDDS_EXPORTED_API static PropertyPolicy get_properties_with_prefix(
            const PropertyPolicy& property_policy,
            const std::string& prefix);

    //!Get the length of the property_policy
    FASTDDS_EXPORTED_API static size_t length(
            const PropertyPolicy& property_policy);

    //!Look for a property_policy by name
    FASTDDS_EXPORTED_API static std::string* find_property(
            PropertyPolicy& property_policy,
            const std::string& name);

    //!Retrieves a property_policy by name
    FASTDDS_EXPORTED_API static const std::string* find_property(
            const PropertyPolicy& property_policy,
            const std::string& name);

    /**
     * @brief Retrieves a property by name
     * @param property_policy PropertyPolicy where the property will be searched.
     * @param name Name of the property to be searched.
     * @return A pointer to the property if found, nullptr otherwise.
     */
    FASTDDS_EXPORTED_API static const Property* get_property(
            const PropertyPolicy& property_policy,
            const std::string& name);
};

} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_RTPS_ATTRIBUTES__PROPERTYPOLICY_HPP
