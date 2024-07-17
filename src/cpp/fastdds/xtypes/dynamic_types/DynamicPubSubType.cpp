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

#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>

#include <fastcdr/Cdr.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>

#include "common.hpp"
#include "DynamicDataImpl.hpp"
#include "DynamicTypeImpl.hpp"
#include <rtps/RTPSDomainImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

//{{{ Public functions

DynamicPubSubType::DynamicPubSubType(
        traits<DynamicType>::ref_type type)
    : dynamic_type_(type)
{
    update_dynamic_type();
}

DynamicPubSubType::DynamicPubSubType(
        traits<DynamicType>::ref_type type,
        const xtypes::TypeInformation& type_information)
    : DynamicPubSubType(type)
{
    xtypes::TypeIdentifierPair pair;

    if (xtypes::TK_NONE == type_information.complete().typeid_with_size().type_id()._d())
    {
        // If the complete type is not set, we use the minimal type as type_identifier1, leaving
        // type_identifier2 empty.
        pair.type_identifier1(type_information.minimal().typeid_with_size().type_id());
        pair.type_identifier2().no_value({});
    }
    else
    {
        pair.type_identifier1(type_information.complete().typeid_with_size().type_id());
        pair.type_identifier2(type_information.minimal().typeid_with_size().type_id());
    }

    type_identifiers_ = pair;
}

DynamicPubSubType::~DynamicPubSubType()
{
    if (key_buffer_ != nullptr)
    {
        free(key_buffer_);
    }
}

void* DynamicPubSubType::create_data()
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

void DynamicPubSubType::delete_data(
        void* data)
{
    traits<DynamicData>::ref_type* data_ptr = static_cast<traits<DynamicData>::ref_type*>(data);
    DynamicDataFactory::get_instance()->delete_data(*data_ptr);
    delete data_ptr;
}

bool DynamicPubSubType::deserialize(
        eprosima::fastdds::rtps::SerializedPayload_t& payload,
        void* data)
{
    traits<DynamicDataImpl>::ref_type* data_ptr = static_cast<traits<DynamicDataImpl>::ref_type*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.length); // Object that manages the raw buffer.
    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN); // Object that deserializes the data.

    try
    {
        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        deser >> *data_ptr;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }
    return true;
}

traits<DynamicType>::ref_type DynamicPubSubType::get_dynamic_type() const noexcept
{
    return dynamic_type_;
}

bool DynamicPubSubType::compute_key(
        eprosima::fastdds::rtps::SerializedPayload_t& payload,
        eprosima::fastdds::rtps::InstanceHandle_t& handle,
        bool force_md5)
{
    if (!dynamic_type_ || !is_compute_key_provided)
    {
        return false;
    }

    traits<DynamicDataImpl>::ref_type temp_val {traits<DynamicData>::narrow<DynamicDataImpl>(
                                                    DynamicDataFactory::get_instance()->create_data(
                                                        dynamic_type_))};
    if (deserialize(payload, static_cast<void*>(&temp_val)))
    {
        return compute_key(static_cast<void*>(&temp_val), handle, force_md5);
    }

    return false;
}

bool DynamicPubSubType::compute_key(
        const void* const data,
        eprosima::fastdds::rtps::InstanceHandle_t& handle,
        bool force_md5)
{
    if (!dynamic_type_ || !is_compute_key_provided)
    {
        return false;
    }
    const traits<DynamicDataImpl>::ref_type* data_ptr = static_cast<const traits<DynamicDataImpl>::ref_type*>(data);
    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment {0};
    size_t keyBufferSize =
            static_cast<uint32_t>((*data_ptr)->calculate_key_serialized_size(calculator, current_alignment));

    if (nullptr == key_buffer_)
    {
        key_buffer_ = reinterpret_cast<unsigned char*>(malloc(keyBufferSize > 16 ? keyBufferSize : 16));
        memset(key_buffer_, 0, keyBufferSize > 16 ? keyBufferSize : 16);
    }

    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(key_buffer_), keyBufferSize);
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS,
            eprosima::fastcdr::CdrVersion::XCDRv2);  // Object that serializes the data.
    ser.set_encoding_flag(eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2);
    (*data_ptr)->serialize_key(ser);
    if (force_md5 || keyBufferSize > 16)
    {
        md5_.init();
        md5_.update(key_buffer_, (unsigned int)ser.get_serialized_data_length());
        md5_.finalize();
        for (uint8_t i = 0; i < 16; ++i)
        {
            handle.value[i] = md5_.digest[i];
        }
    }
    else
    {
        for (uint8_t i = 0; i < 16; ++i)
        {
            handle.value[i] = key_buffer_[i];
        }
    }
    return true;
}

uint32_t DynamicPubSubType::calculate_serialized_size(
        const void* const data,
        DataRepresentationId_t data_representation)
{
    const traits<DynamicDataImpl>::ref_type* data_ptr = static_cast<const traits<DynamicDataImpl>::ref_type*>(data);

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
}

bool DynamicPubSubType::serialize(
        const void* const data,
        eprosima::fastdds::rtps::SerializedPayload_t& payload,
        fastdds::dds::DataRepresentationId_t data_representation)
{
    const traits<DynamicDataImpl>::ref_type* data_ptr = static_cast<const traits<DynamicDataImpl>::ref_type*>(data);
    // Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.max_size);

    // Object that serializes the data.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            data_representation ==
            fastdds::dds::DataRepresentationId_t::XCDR_DATA_REPRESENTATION ? eprosima::fastcdr::CdrVersion::
                    XCDRv1 : eprosima::fastcdr::CdrVersion::XCDRv2);
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    auto type_impl = traits<DynamicType>::narrow<DynamicTypeImpl>(dynamic_type_);
    ser.set_encoding_flag(get_fastcdr_encoding_flag(type_impl->get_descriptor().extensibility_kind(),
            fastdds::dds::DataRepresentationId_t::XCDR_DATA_REPRESENTATION == data_representation?
            eprosima::fastcdr::CdrVersion:: XCDRv1 :
            eprosima::fastcdr::CdrVersion::XCDRv2));

    try
    {
        // Serialize encapsulation
        ser.serialize_encapsulation();

        ser << *data_ptr;
    }
    catch (eprosima::fastcdr::exception::Exception& /*exception*/)
    {
        return false;
    }

    payload.length = (uint32_t)ser.get_serialized_data_length(); //Get the serialized length
    return true;
}

ReturnCode_t DynamicPubSubType::set_dynamic_type(
        traits<DynamicType>::ref_type type)
{
    if (!dynamic_type_)
    {
        dynamic_type_ = type;
        update_dynamic_type();
        return RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error Setting the dynamic type. There is already a registered type");
    }

    return RETCODE_BAD_PARAMETER;
}

void DynamicPubSubType::register_type_object_representation()
{
    if (dynamic_type_)
    {
        // Only register the type object representation if the type identifiers are not previously set.
        // If they are set, it means that the type object representation is already registered via remote
        // type discovery.
        bool type_identifiers_are_set =
                (TK_NONE != type_identifiers_.type_identifier1()._d()) ||
                (TK_NONE != type_identifiers_.type_identifier2()._d());

        if (!type_identifiers_are_set)
        {
            if (RETCODE_OK != fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer().
                            register_typeobject_w_dynamic_type(dynamic_type_, type_identifiers_))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error registering DynamicType TypeObject representation.");
            }
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES,
                "Error registering DynamicType TypeObject representation: DynamicType not initialized");
    }
}

//}}}

void DynamicPubSubType::update_dynamic_type()
{
    is_compute_key_provided = false;

    if (nullptr == dynamic_type_)
    {
        return;
    }

    max_serialized_type_size = static_cast<uint32_t>(DynamicDataImpl::calculate_max_serialized_size(dynamic_type_) + 4);
    set_name(dynamic_type_->get_name().to_string());

    if (TK_STRUCTURE == dynamic_type_->get_kind())
    {
        auto type_impl = traits<DynamicType>::narrow<DynamicTypeImpl>(dynamic_type_);
        for (auto& member : type_impl->get_all_members_by_index())
        {
            auto member_impl = traits<DynamicTypeMember>::narrow<DynamicTypeMemberImpl>(member);
            if (member_impl->get_descriptor().is_key())
            {
                is_compute_key_provided = true;
                break;
            }
        }
    }
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
