// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _TEST_UNITTEST_DDS_SUBSCRIBER_FOOTYPESUPPORT_HPP_
#define _TEST_UNITTEST_DDS_SUBSCRIBER_FOOTYPESUPPORT_HPP_


#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>

#include "./FooType.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

class FooTypeSupport : public TopicDataType
{
public:

    FooTypeSupport()
        : TopicDataType()
    {
        set_name("FooType");
        max_serialized_type_size = 4u + 4u + 256u; // encapsulation + index + message
        is_compute_key_provided = true;
    }

    bool serialize(
            const void* const data,
            fastdds::rtps::SerializedPayload_t& payload,
            DataRepresentationId_t data_representation) override
    {
        const FooType* p_type = static_cast<const FooType*>(data);

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fb(reinterpret_cast<char*>(payload.data), payload.max_size);
        // Object that serializes the data.
        eprosima::fastcdr::Cdr ser(fb, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
                eprosima::fastcdr::CdrVersion::XCDRv1 : eprosima::fastcdr::CdrVersion::XCDRv2);
        payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        ser.set_encoding_flag(
            data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR  :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2);

        try
        {
            // Serialize encapsulation
            ser.serialize_encapsulation();

            // Serialize the object.
            p_type->serialize(ser);
        }
        catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        // Get the serialized length
        payload.length = static_cast<uint32_t>(ser.get_serialized_data_length());
        return true;
    }

    bool deserialize(
            fastdds::rtps::SerializedPayload_t& payload,
            void* data) override
    {
        //Convert DATA to pointer of your type
        FooType* p_type = static_cast<FooType*>(data);

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fb(reinterpret_cast<char*>(payload.data), payload.length);

        // Object that deserializes the data.
        eprosima::fastcdr::Cdr deser(fb
                );

        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

        try
        {
            // Deserialize the object.
            p_type->deserialize(deser);
        }
        catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        return true;
    }

    uint32_t calculate_serialized_size(
            const void* const /*data*/,
            DataRepresentationId_t /*data_representation*/) override
    {
        return max_serialized_type_size;
    }

    void* create_data() override
    {
        return static_cast<void*>(new FooType());
    }

    void delete_data(
            void* data) override
    {
        FooType* p_type = static_cast<FooType*>(data);
        delete p_type;
    }

    bool compute_key(
            fastdds::rtps::SerializedPayload_t& payload,
            fastdds::rtps::InstanceHandle_t& handle,
            bool force_md5) override
    {
        FooType data;
        if (deserialize(payload, static_cast<void*>(&data)))
        {
            return compute_key(static_cast<void*>(&data), handle, force_md5);
        }

        return false;
    }

    bool compute_key(
            const void* const data,
            fastdds::rtps::InstanceHandle_t& handle,
            bool force_md5) override
    {
        const FooType* p_type = static_cast<const FooType*>(data);
        char key_buf[16]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fastbuffer(key_buf, 16);

        // Object that serializes the data.
        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
        p_type->serializeKey(ser);
        if (force_md5)
        {
            MD5 md5;
            md5.init();
            md5.update(key_buf, static_cast<unsigned int>(ser.get_serialized_data_length()));
            md5.finalize();
            for (uint8_t i = 0; i < 16; ++i)
            {
                handle.value[i] = md5.digest[i];
            }
        }
        else
        {
            for (uint8_t i = 0; i < 16; ++i)
            {
                handle.value[i] = key_buf[i];
            }
        }
        return true;
    }

    inline bool is_bounded() const override
    {
        return true;
    }

    inline bool is_plain(
            eprosima::fastdds::dds::DataRepresentationId_t) const override
    {
        return true;
    }

    inline bool construct_sample(
            void* memory) const override
    {
        new (memory) FooType();
        return true;
    }

private:

    using TopicDataType::is_plain;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // _TEST_UNITTEST_DDS_SUBSCRIBER_FOOTYPESUPPORT_HPP_
