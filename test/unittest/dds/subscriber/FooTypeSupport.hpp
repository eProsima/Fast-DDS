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

#include <fastcdr/Cdr.h>

#include <fastdds/dds/topic/TopicDataType.hpp>

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
        setName("FooType");
        m_typeSize = 4u + 4u + 256u; // encapsulation + index + message
        m_isGetKeyDefined = true;
    }

    bool serialize(
            void* data,
            fastrtps::rtps::SerializedPayload_t* payload) override
    {
        FooType* p_type = static_cast<FooType*>(data);

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fb(reinterpret_cast<char*>(payload->data), payload->max_size);
        // Object that serializes the data.
        eprosima::fastcdr::Cdr ser(fb, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
        payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        // Serialize encapsulation
        ser.serialize_encapsulation();

        try
        {
            // Serialize the object.
            p_type->serialize(ser);
        }
        catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            return false;
        }

        // Get the serialized length
        payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
        return true;
    }

    bool deserialize(
            fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override
    {
        //Convert DATA to pointer of your type
        FooType* p_type = static_cast<FooType*>(data);

        // Object that manages the raw buffer.
        eprosima::fastcdr::FastBuffer fb(reinterpret_cast<char*>(payload->data), payload->length);

        // Object that deserializes the data.
        eprosima::fastcdr::Cdr deser(fb, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

        // Deserialize encapsulation.
        deser.read_encapsulation();
        payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

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

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/) override
    {
        return [this]
               {
                   return m_typeSize;
               };
    }

    void* createData() override
    {
        return static_cast<void*>(new FooType());
    }

    void deleteData(
            void* data) override
    {
        FooType* p_type = static_cast<FooType*>(data);
        delete p_type;
    }

    bool getKey(
            void* data,
            fastrtps::rtps::InstanceHandle_t* handle,
            bool force_md5) override
    {
        FooType* p_type = static_cast<FooType*>(data);
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
            md5.update(key_buf, static_cast<unsigned int>(ser.getSerializedDataLength()));
            md5.finalize();
            for (uint8_t i = 0; i < 16; ++i)
            {
                handle->value[i] = md5.digest[i];
            }
        }
        else
        {
            for (uint8_t i = 0; i < 16; ++i)
            {
                handle->value[i] = key_buf[i];
            }
        }
        return true;
    }

    inline bool is_bounded() const override
    {
        return true;
    }

    inline bool is_plain() const override
    {
        return true;
    }

    inline bool construct_sample(
            void* memory) const override
    {
        new (memory) FooType();
        return true;
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // _TEST_UNITTEST_DDS_SUBSCRIBER_FOOTYPESUPPORT_HPP_
