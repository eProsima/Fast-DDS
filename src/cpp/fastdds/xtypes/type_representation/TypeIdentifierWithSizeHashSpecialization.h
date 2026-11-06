// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeIdentifierWithSizeHashSpecialization.h
 *
 */

#include <functional>

#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#ifndef _FASTDDS_DDS_XTYPES_TYPE_IDENTIFIER_WITH_SIZE_HASH_SPECIALIZATION
#define _FASTDDS_DDS_XTYPES_TYPE_IDENTIFIER_WITH_SIZE_HASH_SPECIALIZATION

namespace std {

template<>
struct hash<eprosima::fastdds::dds::xtypes::TypeIdentfierWithSize>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes::TypeIdentfierWithSize& k) const
    {
        size_t hash {static_cast<size_t>(k.typeobject_serialized_size())};
        const eprosima::fastdds::dds::xtypes::TypeIdentifier& type_id {k.type_id()};
        // Hashing only the size makes types with equal serialized size collide, degrading the containers to O(n^2)
        if (eprosima::fastdds::dds::xtypes::EK_COMPLETE == type_id._d() ||
                eprosima::fastdds::dds::xtypes::EK_MINIMAL == type_id._d())
        {
            // The EquivalenceHash is MD5 based, so its bytes are already well distributed
            size_t equivalence_hash {0};
            for (size_t i = 0; i < sizeof(size_t); ++i)
            {
                equivalence_hash = (equivalence_hash << 8) | static_cast<size_t>(type_id.equivalence_hash()[i]);
            }
            hash ^= equivalence_hash;
        }
        return hash;
    }

};

} // namespace std

#endif // _FASTDDS_DDS_XTYPES_TYPE_IDENTIFIER_WITH_SIZE_HASH_SPECIALIZATION
