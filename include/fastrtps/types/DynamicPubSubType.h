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

#include "v1_1/DynamicPubSubType.h"
#include "v1_3/DynamicPubSubType.h"

namespace eprosima {
namespace fastrtps {
namespace types {

// DynamicPubSubType collides with v1_1::DynamicPubSubType because namespace v1_1 is inline
class DynamicPubSubType
    : public virtual fastdds::dds::TopicDataType
    , protected v1_1::internal::DynamicPubSubType
    , protected v1_3::DynamicPubSubType
{
public:

    enum class version
    {
        v1_1,
        v1_3
    };

    RTPS_DllAPI DynamicPubSubType(
            v1_1::DynamicType_ptr pDynamicType);

    RTPS_DllAPI DynamicPubSubType(
            v1_3::DynamicType_ptr pDynamicType);

    RTPS_DllAPI version GetDynamicTypeVersion() const
    {
        return active_;
    }

    RTPS_DllAPI ReturnCode_t GetDynamicType(v1_3::DynamicType_ptr& p) const
    {
        if (version::v1_3 == active_)
        {
            p = v1_3::DynamicPubSubType::GetDynamicType();
            return ReturnCode_t::RETCODE_OK;
        }

        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    RTPS_DllAPI ReturnCode_t GetDynamicType(v1_1::DynamicType_ptr& p) const
    {
        if (version::v1_1 == active_)
        {
            p = v1_1::internal::DynamicPubSubType::GetDynamicType();
            return ReturnCode_t::RETCODE_OK;
        }

        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    // TopicDataType overrides
    RTPS_DllAPI void* createData() override;

    RTPS_DllAPI void deleteData(
            void* data) override;

    RTPS_DllAPI bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    RTPS_DllAPI bool deserialize (
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    RTPS_DllAPI bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    RTPS_DllAPI std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

private:

    version active_;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_PUB_SUB_TYPE_H
