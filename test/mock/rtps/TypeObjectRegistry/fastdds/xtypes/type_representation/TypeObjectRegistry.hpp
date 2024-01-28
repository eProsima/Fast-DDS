// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


#ifndef _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_
#define _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_

#include <gmock/gmock.h>

#include <unordered_set>

#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

namespace std {
template<>
struct hash<eprosima::fastdds::dds::xtypes::TypeIdentifier>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes::TypeIdentifier& k) const
    {
        return (static_cast<size_t>(k.equivalence_hash()[0]) << 16) |
               (static_cast<size_t>(k.equivalence_hash()[1]) << 8) |
               (static_cast<size_t>(k.equivalence_hash()[2]));
    }

};

} // std

namespace eprosima {
namespace fastdds {
namespace dds {

namespace xtypes {

using ReturnCode_t = eprosima::fastdds::dds::ReturnCode_t;

class TypeObjectRegistry : public ITypeObjectRegistry
{

public:

    TypeObjectRegistry()
    {
    }

    ~TypeObjectRegistry()
    {
    }

    MOCK_METHOD2(register_type_object, ReturnCode_t(
                const std::string&, const CompleteTypeObject&));


    RTPS_DllAPI ReturnCode_t register_type_identifier(
            const std::string&,
            const TypeIdentifier&)
    {
        return RETCODE_OK;
    }

    RTPS_DllAPI ReturnCode_t get_type_objects(
            const std::string&,
            TypeObjectPair&)
    {
        return RETCODE_OK;
    }

    RTPS_DllAPI ReturnCode_t get_type_identifiers(
            const std::string&,
            TypeIdentifierPair&)
    {
        return RETCODE_OK;
    }

    RTPS_DllAPI ReturnCode_t get_type_object(
            const TypeIdentifier& id,
            TypeObject& obj)
    {
        return mock_get_type_object(id, obj);
    }

    MOCK_METHOD2(mock_get_type_object, ReturnCode_t(
                const TypeIdentifier&, const TypeObject&));

    MOCK_METHOD2(register_type_object, ReturnCode_t(
                const TypeIdentifier&, const TypeObject&));

    MOCK_METHOD2(get_type_dependencies, ReturnCode_t(
                const TypeIdentifierSeq&, std::unordered_set<TypeIdentfierWithSize>&));

    MOCK_METHOD1(is_type_identifier_known, bool(
                const TypeIdentfierWithSize&));

    MOCK_METHOD1(get_complementary_type_identifier, TypeIdentifier(
                const TypeIdentifier&));

    MOCK_METHOD1(is_builtin_annotation, bool(
                const TypeIdentifier&));

    MOCK_METHOD2(calculate_type_identifier, TypeIdentifier(
                const TypeObject&, uint32_t&));
};

} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_
