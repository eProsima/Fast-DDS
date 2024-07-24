// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _TEST_UNITTEST_DDS_SUBSCRIBER_FOOBOUNDEDTYPESUPPORT_HPP_
#define _TEST_UNITTEST_DDS_SUBSCRIBER_FOOBOUNDEDTYPESUPPORT_HPP_

#include <fastcdr/Cdr.h>

#include <fastdds/dds/topic/TopicDataType.hpp>

#include "./FooBoundedType.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

class FooBoundedTypeSupport : public TopicDataType
{
    using type = FooBoundedType;

public:

    FooBoundedTypeSupport()
        : TopicDataType()
    {
        set_name("type");
        max_serialized_type_size = 4u + 4u + 4u + 256u; // encapsulation + index + message len + message data
        is_compute_key_provided = false;
    }

    bool serialize(
            const void* const data,
            fastdds::rtps::SerializedPayload_t& payload,
            DataRepresentationId_t data_representation) override
    {
        const type* p_type = static_cast<const type*>(data);

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
        type* p_type = static_cast<type*>(data);

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
            const void* const data,
            DataRepresentationId_t /*data_representation*/) override
    {
        return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<const type*>(data))) +
               4u /*encapsulation*/;
    }

    void* create_data() override
    {
        return static_cast<void*>(new type());
    }

    void delete_data(
            void* data) override
    {
        type* p_type = static_cast<type*>(data);
        delete p_type;
    }

    bool compute_key(
            fastdds::rtps::SerializedPayload_t& /*payload*/,
            fastdds::rtps::InstanceHandle_t& /*handle*/,
            bool /*force_md5*/) override
    {
        return false;
    }

    bool compute_key(
            const void* const /*data*/,
            fastdds::rtps::InstanceHandle_t& /*handle*/,
            bool /*force_md5*/) override
    {
        return false;
    }

    inline bool is_bounded() const override
    {
        return true;
    }

    inline bool is_plain(
            eprosima::fastdds::dds::DataRepresentationId_t) const override
    {
        return false;
    }

    inline bool construct_sample(
            void* /*memory*/) const override
    {
        return false;
    }

private:

    using TopicDataType::is_plain;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // _TEST_UNITTEST_DDS_SUBSCRIBER_FOOBOUNDEDTYPESUPPORT_HPP_
