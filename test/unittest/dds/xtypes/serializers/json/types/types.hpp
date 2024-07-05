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

/**
 * @file types.hpp
 *
 */

#ifndef TEST_UNITTEST_DDS_XTYPES_SERIALIZERS_JSON_TYPES__TYPES_HPP
#define TEST_UNITTEST_DDS_XTYPES_SERIALIZERS_JSON_TYPES__TYPES_HPP

#include <string>

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

enum class DataTypeKind
{
    COMPREHENSIVE_TYPE,
};

template <DataTypeKind Data>
eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicType>::ref_type create_dynamic_type();

template <DataTypeKind Data>
eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type create_dynamic_data(
        const eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicType>::ref_type& dynamic_type,
        bool filled,
        const unsigned int& index = 0);

template <DataTypeKind Data>
eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type fill_dyn_data(
        eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicData>::ref_type& dyn_data,
        const unsigned int& index);

template <DataTypeKind Data>
std::string get_expected_json(
        const eprosima::fastdds::dds::DynamicDataJsonFormat& format,
        bool filled,
        const unsigned int& index = 0);

#endif // TEST_UNITTEST_DDS_XTYPES_SERIALIZERS_JSON_TYPES__TYPES_HPP
