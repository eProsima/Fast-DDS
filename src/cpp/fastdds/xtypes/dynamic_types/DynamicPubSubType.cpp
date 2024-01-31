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
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/common/SerializedPayload.h>

#include "DynamicDataImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

DynamicPubSubType::DynamicPubSubType(
        traits<DynamicType>::ref_type type)
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
}

traits<DynamicType>::ref_type DynamicPubSubType::GetDynamicType() const
{
    return dynamic_type_;
}

ReturnCode_t DynamicPubSubType::SetDynamicType(
        traits<DynamicData>::ref_type data)
{
    if (!dynamic_type_)
    {
        return SetDynamicType(data->type());
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error Setting the dynamic type. There is already a registered type");
        return RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicPubSubType::SetDynamicType(
        traits<DynamicType>::ref_type type)
{
    if (!dynamic_type_)
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
    if (!dynamic_type_)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "DynamicPubSubType cannot create data. Unspecified type.");
        return nullptr;
    }
    else
    {
        traits<DynamicDataImpl>::ref_type* ret_val = new traits<DynamicDataImpl>::ref_type();
        *ret_val =
                traits<DynamicData>::narrow<DynamicDataImpl>(DynamicDataFactory::get_instance()->create_data(
                            dynamic_type_));
        return ret_val;
    }
}

void DynamicPubSubType::deleteData(
        void* data)
{
    traits<DynamicDataImpl>::ref_type* data_ptr = static_cast<traits<DynamicDataImpl>::ref_type*>(data);
    DynamicDataFactory::get_instance()->delete_data(*data_ptr);
    delete data_ptr;
}

bool DynamicPubSubType::deserialize(
        eprosima::fastrtps::rtps::SerializedPayload_t* payload,
        void* data)
{
    traits<DynamicDataImpl>::ref_type* data_ptr = static_cast<traits<DynamicDataImpl>::ref_type*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length); // Object that manages the raw buffer.
    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN); // Object that deserializes the data.

    try
    {
        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        deser >> *data_ptr;
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
    traits<DynamicDataImpl>::ref_type* data_ptr = static_cast<traits<DynamicDataImpl>::ref_type*>(data);
    size_t keyBufferSize = static_cast<uint32_t>(DynamicDataImpl::get_key_max_cdr_serialized_size(dynamic_type_));

    if (m_keyBuffer == nullptr)
    {
        m_keyBuffer = (unsigned char*)malloc(keyBufferSize > 16 ? keyBufferSize : 16);
        memset(m_keyBuffer, 0, keyBufferSize > 16 ? keyBufferSize : 16);
    }

    eprosima::fastcdr::FastBuffer fastbuffer((char*)m_keyBuffer, keyBufferSize);
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS,
            eprosima::fastdds::rtps::DEFAULT_XCDR_VERSION);  // Object that serializes the data.
    (*data_ptr)->serialize_key(ser);
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
    return getSerializedSizeProvider(data, DEFAULT_DATA_REPRESENTATION);
}

std::function<uint32_t()> DynamicPubSubType::getSerializedSizeProvider(
        void* data,
        DataRepresentationId_t data_representation)
{
    traits<DynamicDataImpl>::ref_type* data_ptr = static_cast<traits<DynamicDataImpl>::ref_type*>(data);

    return [data_ptr, data_representation]() -> uint32_t
           {
               try
               {
                   eprosima::fastcdr::CdrSizeCalculator calculator(
                       data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
                       eprosima::fastcdr::CdrVersion::XCDRv1 :eprosima::fastcdr::CdrVersion::XCDRv2);
                   size_t current_alignment {0};
                   return static_cast<uint32_t>(calculator.calculate_serialized_size(
                              *data_ptr, current_alignment)) + 4u /*encapsulation*/;

               }
               catch (eprosima::fastcdr::exception::Exception& /*exception*/)
               {
                   return 0;
               }
           };
}

bool DynamicPubSubType::serialize(
        void* data,
        eprosima::fastrtps::rtps::SerializedPayload_t* payload,
        fastdds::dds::DataRepresentationId_t data_representation)
{
    traits<DynamicDataImpl>::ref_type* data_ptr = static_cast<traits<DynamicDataImpl>::ref_type*>(data);
    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);

    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            data_representation ==
            fastdds::dds::DataRepresentationId_t::XCDR_DATA_REPRESENTATION ? eprosima::fastcdr::CdrVersion::
                    XCDRv1 : eprosima::fastcdr::CdrVersion::XCDRv2);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    //TODO (richiware)
    ser.set_encoding_flag(
        data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR  :
        eprosima::fastcdr::EncodingAlgorithmFlag::DELIMIT_CDR2);

    try
    {
        // Serialize encapsulation
        ser.serialize_encapsulation();

        ser << *data_ptr;
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

    m_typeSize = static_cast<uint32_t>(DynamicDataImpl::get_max_cdr_serialized_size(dynamic_type_) + 4);
    setName(dynamic_type_->get_name());

    //TODO(richiware) m_isGetKeyDefined = dynamic_type_->key_annotation();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
