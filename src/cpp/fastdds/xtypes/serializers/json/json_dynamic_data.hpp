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

#ifndef FASTDDS_XTYPES_SERIALIZERS_JSON__JSON_DYNAMIC_DATA_HPP
#define FASTDDS_XTYPES_SERIALIZERS_JSON__JSON_DYNAMIC_DATA_HPP

#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include <fastdds/dds/xtypes/utils.hpp>

#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "../../dynamic_types/DynamicDataImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

//////////////////////////////////////////
// JSON to Dynamic Data deserialization //
//////////////////////////////////////////

ReturnCode_t json_deserialize(
        const nlohmann::json& j,
        const traits<DynamicTypeImpl>::ref_type& dynamic_type,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept;

ReturnCode_t json_deserialize_aggregate(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

ReturnCode_t json_deserialize_member(
        const nlohmann::json& j,
        const traits<DynamicTypeMember>::ref_type& type_member,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

ReturnCode_t json_deserialize_member(
        const nlohmann::json& j,
        const MemberId& member_id,
        const TypeKind& member_kind,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

ReturnCode_t json_deserialize_basic_member(
        const nlohmann::json& j,
        const MemberId& member_id,
        const TypeKind& member_kind,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

ReturnCode_t json_deserialize_enum_member(
        const nlohmann::json& j,
        const MemberId& member_id,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

// WARNING: Enforcing noexcept here throws a warning in C++11, and in fact causes no effect (i.e. compilation does not
// fail when executing a method with this signature from a noexcept one) -> manually ensure all deserializers are noexcept
using MemberDeserializer = ReturnCode_t (*)(
    const nlohmann::json& j,
    DynamicDataJsonFormat format,
    traits<DynamicDataImpl>::ref_type& data);

ReturnCode_t json_deserialize_member_with_loan(
        const nlohmann::json& j,
        const MemberId& member_id,
        const std::string& kind_str,
        MemberDeserializer member_deserializer,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

ReturnCode_t json_deserialize_union(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

ReturnCode_t json_deserialize_collection(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

ReturnCode_t json_deserialize_array(
        const nlohmann::json& j,
        TypeKind element_kind,
        unsigned int& index,
        const std::vector<unsigned int>& bounds,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

ReturnCode_t json_deserialize_map(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

ReturnCode_t json_deserialize_bitmask(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        traits<DynamicDataImpl>::ref_type& data) noexcept;

template<class Target>
Target numeric_get(
        const nlohmann::json& j);

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_SERIALIZERS_JSON__JSON_DYNAMIC_DATA_HPP
