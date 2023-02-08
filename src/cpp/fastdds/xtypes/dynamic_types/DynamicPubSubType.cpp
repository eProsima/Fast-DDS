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

#include <fastcdr/Cdr.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/common/SerializedPayload.h>

namespace eprosima {
namespace fastdds {
namespace dds {

DynamicPubSubType::DynamicPubSubType(
        const DynamicType& type)
    : dynamic_type_(DynamicTypeBuilderFactory::get_instance().create_copy(type))
{
    UpdateDynamicTypeInfo();
}

DynamicPubSubType::DynamicPubSubType(
        const DynamicType* type)
    : dynamic_type_(type)
{
    UpdateDynamicTypeInfo();
}

DynamicPubSubType::~DynamicPubSubType()
{
    if (m_keyBuffer != nullptr)
    {
        free(m_keyBuffer);
    }

    CleanDynamicType();
}

void DynamicPubSubType::CleanDynamicType()
{
    if (dynamic_type_ != nullptr)
    {
        DynamicTypeBuilderFactory::get_instance().delete_type(dynamic_type_);
    }

    dynamic_type_ = nullptr;
}

const DynamicType* DynamicPubSubType::GetDynamicType() const
{
    return nullptr == dynamic_type_ ? nullptr :
           DynamicTypeBuilderFactory::get_instance().create_copy(*dynamic_type_);
}

ReturnCode_t DynamicPubSubType::SetDynamicType(
        const DynamicData& data)
{
    if (nullptr == dynamic_type_)
    {
        return SetDynamicType(data.get_type());
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error Setting the dynamic type. There is already a registered type");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicPubSubType::SetDynamicType(
        const DynamicType& type)
{
    if (nullptr == dynamic_type_)
    {
        return SetDynamicType(DynamicTypeBuilderFactory::get_instance().create_copy(type));
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error Setting the dynamic type. There is already a registered type");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicPubSubType::SetDynamicType(
        const DynamicType* type)
{
    if (nullptr == dynamic_type_)
    {
        dynamic_type_ = type;
        UpdateDynamicTypeInfo();
        return RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error Setting the dynamic type. There is already a registered type");
        return RETCODE_BAD_PARAMETER;
    }
}

void* DynamicPubSubType::createData()
{
    if (nullptr == dynamic_type_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "DynamicPubSubType cannot create data. Unspecified type.");
        return nullptr;
    }
    else
    {
        return DynamicDataFactory::get_instance().create_data(*dynamic_type_);
    }
}

void DynamicPubSubType::deleteData(
        void* data)
{
    DynamicDataFactory::get_instance().delete_data(static_cast<DynamicData*>(data));
}

bool DynamicPubSubType::deserialize(
        eprosima::fastrtps::rtps::SerializedPayload_t* payload,
        void* data)
{
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length); // Object that manages the raw buffer.
    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN); // Object that deserializes the data.
    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try
    {
        ((DynamicData*)data)->deserialize(deser); //Deserialize the object:
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }
    return true;
}

bool DynamicPubSubType::getKey(
        void* data,
        eprosima::fastrtps::rtps::InstanceHandle_t* handle,
        bool force_md5)
{
    if (dynamic_type_ != nullptr || !m_isGetKeyDefined)
    {
        return false;
    }
    DynamicData* pDynamicData = (DynamicData*)data;
    size_t keyBufferSize = static_cast<uint32_t>(DynamicData::getKeyMaxCdrSerializedSize(*dynamic_type_));

    if (m_keyBuffer == nullptr)
    {
        m_keyBuffer = (unsigned char*)malloc(keyBufferSize > 16 ? keyBufferSize : 16);
        memset(m_keyBuffer, 0, keyBufferSize > 16 ? keyBufferSize : 16);
    }

    eprosima::fastcdr::FastBuffer fastbuffer((char*)m_keyBuffer, keyBufferSize);
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS,
            eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);  // Object that serializes the data.
    pDynamicData->serializeKey(ser);
    if (force_md5 || keyBufferSize > 16)
    {
        m_md5.init();
        m_md5.update(m_keyBuffer, (unsigned int)ser.get_serialized_data_length());
        m_md5.finalize();
        for (uint8_t i = 0; i < 16; ++i)
        {
            handle->value[i] = m_md5.digest[i];
        }
    }
    else
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            handle->value[i] = m_keyBuffer[i];
        }
    }
    return true;
}

std::function<uint32_t()> DynamicPubSubType::getSerializedSizeProvider(
        void* data)
{
    assert(data);

    return [data]() -> uint32_t
           {
               return (uint32_t)DynamicData::getCdrSerializedSize(*(DynamicData*)data) + 4 /*encapsulation*/;
           };
}

bool DynamicPubSubType::serialize(
        void* data,
        eprosima::fastrtps::rtps::SerializedPayload_t* payload,
        fastdds::dds::DataRepresentationId_t data_representation)
{
    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);

    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            data_representation ==
            fastdds::dds::DataRepresentationId_t::XCDR_DATA_REPRESENTATION ? eprosima::fastcdr::CdrVersion::
                    XCDRv1 : eprosima::fastcdr::CdrVersion::XCDRv2);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    // Serialize encapsulation
    ser.serialize_encapsulation();

    try
    {
        ((DynamicData*)data)->serialize(ser); // Serialize the object:
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    payload->length = (uint32_t)ser.get_serialized_data_length(); //Get the serialized length
    return true;
}

void DynamicPubSubType::UpdateDynamicTypeInfo()
{
    m_isGetKeyDefined = false;

    if (nullptr == dynamic_type_)
    {
        return;
    }

    m_typeSize = static_cast<uint32_t>(DynamicData::getMaxCdrSerializedSize(*dynamic_type_) + 4);
    setName(dynamic_type_->get_name());

    m_isGetKeyDefined = dynamic_type_->key_annotation();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
