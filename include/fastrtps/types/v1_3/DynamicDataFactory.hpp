// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_1_3_DYNAMIC_DATA_FACTORY_HPP
#define TYPES_1_3_DYNAMIC_DATA_FACTORY_HPP

#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicType;
class DynamicData;

class RTPS_DllAPI DynamicDataFactory final
{
    DynamicDataFactory() = default;

public:

    ~DynamicDataFactory() = default;

    static DynamicDataFactory& get_instance();

    static ReturnCode_t delete_instance();

    DynamicData* create_data(
            const DynamicType& type);

    DynamicData* create_copy(
            const DynamicData& data);

    ReturnCode_t delete_data(
            DynamicData* pData);
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_DATA_FACTORY_HPP
