// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file HelloWorldCommon.cpp
 *
 */

#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

#include "../../types.hpp"
#include "../gen/HelloWorld.hpp"
#include "../gen/HelloWorldTypeObjectSupport.hpp"

using namespace eprosima::fastdds::dds;

template <>
void register_type_object_representation_gen<DataTypeKind::HELLO_WORLD>()
{
    register_HelloWorld_type_objects();
}
