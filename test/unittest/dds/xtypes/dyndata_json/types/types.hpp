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

#ifndef TYPES_HPP
#define TYPES_HPP

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::dds::xtypes;

enum class DataTypeKind
{
    COMPREHENSIVE_TYPE,
};

template <DataTypeKind Data>
traits<DynamicType>::ref_type create_dynamic_type();

template <DataTypeKind Data>
traits<DynamicData>::ref_type create_dynamic_data(
        traits<DynamicType>::ref_type& dynamic_type,
        const bool& filled,
        const unsigned int& index = 0);

template <DataTypeKind Data>
traits<DynamicData>::ref_type fill_dyn_data(
        traits<DynamicData>::ref_type& dyn_data,
        const unsigned int& index);


#endif // TYPES_HPP
