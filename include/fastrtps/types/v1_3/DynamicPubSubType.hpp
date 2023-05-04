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

#ifndef TYPES_1_3_DYNAMIC_PUB_SUB_TYPE_H
#define TYPES_1_3_DYNAMIC_PUB_SUB_TYPE_H

#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/types/v1_3/DynamicDataPtr.hpp>
#include <fastrtps/utils/md5.h>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicPubSubType : public virtual eprosima::fastdds::dds::TopicDataType
{
protected:

    void UpdateDynamicTypeInfo();

    v1_3::DynamicType_ptr dynamic_type_;
    MD5 m_md5;
    unsigned char* m_keyBuffer = nullptr;

public:

    RTPS_DllAPI DynamicPubSubType() = default;

    RTPS_DllAPI DynamicPubSubType(
            v1_3::DynamicType_ptr pDynamicType);

    RTPS_DllAPI virtual ~DynamicPubSubType();

    RTPS_DllAPI void* createData() override;

    RTPS_DllAPI void deleteData (
            void* data) override;

    RTPS_DllAPI bool deserialize (
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    RTPS_DllAPI bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    RTPS_DllAPI std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    RTPS_DllAPI bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    RTPS_DllAPI void CleanDynamicType();

    RTPS_DllAPI v1_3::DynamicType_ptr GetDynamicType() const;

    RTPS_DllAPI ReturnCode_t SetDynamicType(
            v1_3::DynamicData_ptr pData);

    RTPS_DllAPI ReturnCode_t SetDynamicType(
            v1_3::DynamicType_ptr pType);
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_PUB_SUB_TYPE_H
