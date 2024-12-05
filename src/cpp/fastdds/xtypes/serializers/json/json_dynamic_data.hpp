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
        const DynamicType::_ref_type& dynamic_type,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept;

ReturnCode_t json_deserialize_member(
        const nlohmann::json& j,
        const traits<DynamicTypeMember>::ref_type& type_member,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept;

ReturnCode_t json_deserialize_member(
        const nlohmann::json& j,
        const MemberId& member_id,
        const TypeKind& member_kind,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept;

ReturnCode_t json_deserialize_basic_member(
        const nlohmann::json& j,
        const MemberId& member_id,
        const TypeKind& member_kind,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept;

ReturnCode_t json_deserialize_collection(
        const nlohmann::json& j,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept;

ReturnCode_t json_deserialize_array(
        const nlohmann::json& j,
        TypeKind member_kind,
        unsigned int& index,
        const std::vector<unsigned int>& bounds,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_SERIALIZERS_JSON__JSON_DYNAMIC_DATA_HPP
