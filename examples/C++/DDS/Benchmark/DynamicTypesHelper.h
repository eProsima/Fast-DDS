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

/**
 * @file BenchMarkPublisher.h
 *
 */

#ifndef DYNAMIC_TYPES_HELPER_H_
#define DYNAMIC_TYPES_HELPER_H_

#include <fastrtps/fastrtps_fwd.h>

namespace eprosima
{
    namespace fastrtps
    {
        namespace types
        {
            class DynamicData;
            class DynamicType_ptr;
        }
    }
}

class DynamicTypesHelper
{
public:
    static eprosima::fastrtps::types::DynamicData* CreateSmallData();
    static eprosima::fastrtps::types::DynamicData* CreateData();
    static eprosima::fastrtps::types::DynamicData* CreateMediumData();
    static eprosima::fastrtps::types::DynamicData* CreateBigData();
protected:

    static eprosima::fastrtps::types::DynamicType_ptr GetMyMiniArrayType();
    static eprosima::fastrtps::types::DynamicType_ptr GetBSAlias5Type();
    static eprosima::fastrtps::types::DynamicType_ptr GetBasicStructType();
    static eprosima::fastrtps::types::DynamicType_ptr GetMyOctetArray500Type();
    static eprosima::fastrtps::types::DynamicType_ptr GetMySequenceLongType();
    static eprosima::fastrtps::types::DynamicType_ptr GetMyEnumType();
    static eprosima::fastrtps::types::DynamicType_ptr GetMyAliasEnumType();
    static eprosima::fastrtps::types::DynamicType_ptr GetMA3Type();
    static eprosima::fastrtps::types::DynamicType_ptr GetMyAliasEnum3Type();
    static eprosima::fastrtps::types::DynamicType_ptr GetMyAliasEnum2Type();
    static eprosima::fastrtps::types::DynamicType_ptr GetUnionSwitchType();
    static eprosima::fastrtps::types::DynamicType_ptr GetUnion2SwitchType();
    static eprosima::fastrtps::types::DynamicType_ptr GetComplexStructType();


};



#endif /* DYNAMIC_TYPES_HELPER_H_ */
