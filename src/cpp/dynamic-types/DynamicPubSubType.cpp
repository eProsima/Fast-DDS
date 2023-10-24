// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypeDescriptor.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicPubSubType::DynamicPubSubType()
    : dynamic_type_(nullptr)
    , m_keyBuffer(nullptr)
{
}

DynamicPubSubType::DynamicPubSubType(
        DynamicType_ptr pType)
    : dynamic_type_(pType)
    , m_keyBuffer(nullptr)
{
    UpdateDynamicTypeInfo();
}

DynamicPubSubType::~DynamicPubSubType()
{
    if (m_keyBuffer != nullptr)
    {
        free(m_keyBuffer);
    }
}

void DynamicPubSubType::CleanDynamicType()
{
    dynamic_type_ = nullptr;
}

DynamicType_ptr DynamicPubSubType::GetDynamicType() const
{
    return dynamic_type_;
}

ReturnCode_t DynamicPubSubType::SetDynamicType(
        DynamicData_ptr pData)
{
    if (dynamic_type_ == nullptr)
    {
        dynamic_type_ = pData->type_;
        UpdateDynamicTypeInfo();
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error Setting the dynamic type. There is already a registered type");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicPubSubType::SetDynamicType(
        DynamicType_ptr pType)
{
    if (dynamic_type_ == nullptr)
    {
        dynamic_type_ = pType;
        UpdateDynamicTypeInfo();
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error Setting the dynamic type. There is already a registered type");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

void* DynamicPubSubType::createData()
{
    return DynamicDataFactory::get_instance()->create_data(dynamic_type_);
}

void DynamicPubSubType::deleteData(
        void* data)
{
    DynamicDataFactory::get_instance()->delete_data((DynamicData*)data);
}

bool DynamicPubSubType::deserialize(
        eprosima::fastrtps::rtps::SerializedPayload_t* payload,
        void* data)
{
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length); // Object that manages the raw buffer.
    eprosima::fastcdr::Cdr deser(fastbuffer
#if FASTCDR_VERSION_MAJOR == 1
            , eprosima::fastcdr::Cdr::DEFAULT_ENDIAN
            , eprosima::fastcdr::Cdr::CdrType::DDS_CDR
#endif // FASTCDR_VERSION_MAJOR == 1
            );

    try
    {
        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        //Deserialize the object:
        ((DynamicData*)data)->deserialize(deser);
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
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
    if (dynamic_type_ == nullptr || !m_isGetKeyDefined)
    {
        return false;
    }
    DynamicData* pDynamicData = (DynamicData*)data;
    size_t keyBufferSize = static_cast<uint32_t>(DynamicData::getKeyMaxCdrSerializedSize(dynamic_type_));

    if (m_keyBuffer == nullptr)
    {
        m_keyBuffer = (unsigned char*)malloc(keyBufferSize > 16 ? keyBufferSize : 16);
        memset(m_keyBuffer, 0, keyBufferSize > 16 ? keyBufferSize : 16);
    }

    eprosima::fastcdr::FastBuffer fastbuffer((char*)m_keyBuffer, keyBufferSize);
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS,
            eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);                                                                            // Object that serializes the data.
    pDynamicData->serializeKey(ser);
    if (force_md5 || keyBufferSize > 16)
    {
        m_md5.init();
#if FASTCDR_VERSION_MAJOR == 1
        m_md5.update(m_keyBuffer, (unsigned int)ser.getSerializedDataLength());
#else
        m_md5.update(m_keyBuffer, (unsigned int)ser.get_serialized_data_length());
#endif // FASTCDR_VERSION_MAJOR == 1
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
        void* data,
        fastdds::dds::DataRepresentationId_t data_representation)
{
    static_cast<void>(data_representation);
    return [data]() -> uint32_t
           {
               return (uint32_t)DynamicData::getCdrSerializedSize((DynamicData*)data) + 4 /*encapsulation*/;
           };
}

bool DynamicPubSubType::serialize(
        void* data,
        eprosima::fastrtps::rtps::SerializedPayload_t* payload,
        fastdds::dds::DataRepresentationId_t data_representation)
{
    if (dynamic_type_ == nullptr)
    {
        dynamic_type_ = static_cast<DynamicData*>(data)->type_;
        UpdateDynamicTypeInfo();
    }

    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);

    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            data_representation ==
            fastdds::dds::DataRepresentationId_t::XCDR_DATA_REPRESENTATION ? eprosima::fastcdr::CdrVersion::
                    XCDRv1 : eprosima::fastcdr::CdrVersion::XCDRv2);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
#if FASTCDR_VERSION_MAJOR > 1
    if (data_representation == fastdds::dds::DataRepresentationId_t::XCDR_DATA_REPRESENTATION)
    {
        if (MUTABLE == extensibility_)
        {
            ser.set_encoding_flag(eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR);
        }
        else
        {
            ser.set_encoding_flag(eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR);
        }
    }
    else
    {
        if (MUTABLE == extensibility_)
        {
            ser.set_encoding_flag(eprosima::fastcdr::EncodingAlgorithmFlag::PL_CDR2);
        }
        else if (APPENDABLE == extensibility_)
        {
            ser.set_encoding_flag(eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2);
        }
        else
        {
            ser.set_encoding_flag(eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2);
        }
    }
#endif // FASTCDR_VERSION_MAJOR > 1

    try
    {
        // Serialize encapsulation
        ser.serialize_encapsulation();
        ((DynamicData*)data)->serialize(ser); // Serialize the object:
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

#if FASTCDR_VERSION_MAJOR == 1
    payload->length = (uint32_t)ser.getSerializedDataLength(); //Get the serialized length
#else
    payload->length = (uint32_t)ser.get_serialized_data_length(); //Get the serialized length
#endif // FASTCDR_VERSION_MAJOR == 1
    return true;
}

void DynamicPubSubType::UpdateDynamicTypeInfo()
{
    if (dynamic_type_ != nullptr)
    {
        m_isGetKeyDefined = dynamic_type_->key_annotation();

        std::map<MemberId, DynamicTypeMember*> membersMap;
        dynamic_type_->get_all_members(membersMap);
        for (auto it = membersMap.begin(); it != membersMap.end(); ++it)
        {
            m_isGetKeyDefined |= it->second->key_annotation();
        }

        m_typeSize = static_cast<uint32_t>(DynamicData::getMaxCdrSerializedSize(dynamic_type_) + 4);
        setName(dynamic_type_->get_name().c_str());

        // Retrieve extensibility.
        if (dynamic_type_->get_descriptor()->annotation_is_final())
        {
            extensibility_ = FINAL;
        }
        else if (dynamic_type_->get_descriptor()->annotation_is_mutable())
        {
            extensibility_ = MUTABLE;
        }
        else
        {
            extensibility_ = APPENDABLE;
        }
    }
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
