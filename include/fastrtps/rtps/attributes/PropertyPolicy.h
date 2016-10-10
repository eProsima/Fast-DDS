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
 * @file PropertyPolicy.h
 */
#ifndef _RTPS_ATTRIBUTES_PROPERTYPOLICY_H_
#define _RTPS_ATTRIBUTES_PROPERTYPOLICY_H_

#include "../common/Property.h"
#include "../common/BinaryProperty.h"

namespace eprosima {
namespace fastrtps {
namespace rtps {

class PropertyPolicy
{
    public:

        PropertyPolicy() {}

        PropertyPolicy(const PropertyPolicy& property_policy) :
            properties_(property_policy.properties_),
            binary_properties_(property_policy.binary_properties_) {}

        PropertyPolicy(PropertyPolicy&& property_policy) :
            properties_(property_policy.properties_),
            binary_properties_(property_policy.binary_properties_) {}

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
        static PropertyPolicy get_properties_with_prefix(const PropertyPolicy& property_policy,
                const std::string& prefix);

        static size_t length(const PropertyPolicy& property_policy);
};

} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _RTPS_ATTRIBUTES_PROPERTYPOLICY_H_
