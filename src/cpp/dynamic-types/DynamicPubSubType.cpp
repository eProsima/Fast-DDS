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

#include <fastrtps/types/DynamicPubSubType.h>

using namespace eprosima::fastrtps::types;

DynamicPubSubType::DynamicPubSubType(
        v1_1::DynamicType_ptr pDynamicType)
    : v1_1::internal::DynamicPubSubType(pDynamicType)
    , active_(version::v1_1)
{
}

DynamicPubSubType::DynamicPubSubType(
        v1_3::DynamicType_ptr pDynamicType)
    : v1_3::DynamicPubSubType(*pDynamicType)
    , active_(version::v1_3)
{
}

ReturnCode_t DynamicPubSubType::SetDynamicType(
        v1_1::DynamicType_ptr pType)
{
    CleanDynamicType();
    active_ = version::v1_1;
    return v1_1::internal::DynamicPubSubType::SetDynamicType(pType);
}

ReturnCode_t DynamicPubSubType::SetDynamicType(
        v1_3::DynamicType_ptr pType)
{
    CleanDynamicType();
    active_ = version::v1_3;
    return v1_3::DynamicPubSubType::SetDynamicType(*pType);
}

void DynamicPubSubType::CleanDynamicType()
{
    v1_1::internal::DynamicPubSubType::CleanDynamicType();
    v1_3::DynamicPubSubType::CleanDynamicType();
    active_ = version::none;
}

ReturnCode_t DynamicPubSubType::GetDynamicType(
        v1_3::DynamicType_ptr& p) const
{
    if (version::v1_3 == active_)
    {
        p = v1_3::DynamicPubSubType::GetDynamicType();
        return ReturnCode_t::RETCODE_OK;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicPubSubType::GetDynamicType(
        v1_1::DynamicType_ptr& p) const
{
    if (version::v1_1 == active_)
    {
        p = v1_1::internal::DynamicPubSubType::GetDynamicType();
        return ReturnCode_t::RETCODE_OK;
    }

    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

void* DynamicPubSubType::createData()
{
    switch (active_)
    {
        case version::v1_1:
            return v1_1::internal::DynamicPubSubType::createData();
        case version::v1_3:
            return v1_3::DynamicPubSubType::createData();
        default:
            break;
    }

    return nullptr;
}

void DynamicPubSubType::deleteData (
        void* data)
{
    switch (active_)
    {
        case version::v1_1:
            v1_1::internal::DynamicPubSubType::deleteData(data);
            break;
        case version::v1_3:
            v1_3::DynamicPubSubType::deleteData(data);
            break;
        default:
            break;
    }
}

bool DynamicPubSubType::serialize(
        void* data,
        eprosima::fastrtps::rtps::SerializedPayload_t* payload)
{
    switch (active_)
    {
        case version::v1_1:
            return v1_1::internal::DynamicPubSubType::serialize(data, payload);
        case version::v1_3:
            return v1_3::DynamicPubSubType::serialize(data, payload);
        default:
            break;
    }

    return false;
}

bool DynamicPubSubType::deserialize(
        eprosima::fastrtps::rtps::SerializedPayload_t* payload,
        void* data)
{
    switch (active_)
    {
        case version::v1_1:
            return v1_1::internal::DynamicPubSubType::deserialize(payload, data);
        case version::v1_3:
            return v1_3::DynamicPubSubType::deserialize(payload, data);
        default:
            break;
    }

    return false;
}

bool DynamicPubSubType::getKey(
        void* data,
        eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
        bool force_md5 /*= false*/)
{
    switch (active_)
    {
        case version::v1_1:
            return v1_1::internal::DynamicPubSubType::getKey(data, ihandle, force_md5);
        case version::v1_3:
            return v1_3::DynamicPubSubType::getKey(data, ihandle, force_md5);
        default:
            break;
    }

    return false;
}

std::function<uint32_t()> DynamicPubSubType::getSerializedSizeProvider(
        void* data)
{
    switch (active_)
    {
        case version::v1_1:
            return v1_1::internal::DynamicPubSubType::getSerializedSizeProvider(data);
        case version::v1_3:
            return v1_3::DynamicPubSubType::getSerializedSizeProvider(data);
        default:
            break;
    }

    return {};
}
