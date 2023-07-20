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

#ifndef TYPES_DYNAMIC_PUB_SUB_TYPE_H
#define TYPES_DYNAMIC_PUB_SUB_TYPE_H

#include <fastrtps/types/DynamicTypePtr.h>

namespace eprosima {
namespace fastrtps {
namespace types {

namespace v1_3 {

class DynamicType;

} // v1_3

// DynamicPubSubType collides with v1_1::DynamicPubSubType because namespace v1_1 is inline
class DynamicPubSubType
{
public:

    enum class version
    {
        none,
        v1_1,
        v1_3
    };

    DynamicPubSubType() = default;

    DynamicPubSubType(
            v1_1::DynamicType_ptr )
    {
    }

    DynamicPubSubType(
            const v1_3::DynamicType* )
    {
    }

    DynamicPubSubType(
            const v1_3::DynamicType& )
    {
    }

    ReturnCode_t SetDynamicType(
            v1_1::DynamicType_ptr )
    {
        return {};
    }

    ReturnCode_t SetDynamicType(
            const v1_3::DynamicType* )
    {
        return {};
    }

    ReturnCode_t SetDynamicType(
            const v1_3::DynamicType& )
    {
        return {};
    }

    void CleanDynamicType()
    {
    }

    version GetDynamicTypeVersion() const
    {
        return version::none;
    }

    ReturnCode_t GetDynamicType(
            const v1_3::DynamicType*& ) const
    {
        return {};
    }

    ReturnCode_t GetDynamicType(
            v1_1::DynamicType_ptr& ) const
    {
        return {};
    }

    void* createData()
    {
        return nullptr;
    }

    void deleteData(
            void* )
    {
    }

    bool serialize(
            void* ,
            eprosima::fastrtps::rtps::SerializedPayload_t* )
    {
        return true;
    }

    bool deserialize (
            eprosima::fastrtps::rtps::SerializedPayload_t*,
            void* )
    {
        return true;
    }

    bool getKey(
            void* ,
            eprosima::fastrtps::rtps::InstanceHandle_t*,
            bool )
    {
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* )
    {
        return {};
    }
};


} // eprosima
} // fastrtps
} // types

#endif // TYPES_DYNAMIC_PUB_SUB_TYPE_H
