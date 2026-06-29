// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


#include <cstdint>
#include <limits>
#include <memory>

#include <gtest/gtest.h>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/dds/topic/TopicDataType.hpp>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <cstring>
#include <fastdds/utils/md5.hpp>

//{ Type and context definitions

namespace eprosima {
namespace fastdds {
namespace dds {
namespace custom_serialization_test {

/**
 * @brief CustomData type that will be used in the custom serialization tests.
 */
struct CustomData
{
    uint8_t data;
};

/**
 * @brief CustomContext type that will be used in the custom serialization tests.
 */
struct CustomContext : public TopicDataType::Context
{
    ~CustomContext() override = default;

    /**
     * @brief Returns a shared pointer to the singleton instance of CustomContext.
     */
    static std::shared_ptr<CustomContext> instance()
    {
        static std::shared_ptr<CustomContext> instance = std::make_shared<CustomContext>();
        return instance;
    }

    /**
     * @brief Checks that the given context is the same as the singleton instance of CustomContext.
     */
    static void check_context(
            const std::shared_ptr<TopicDataType::Context>& context)
    {
        EXPECT_EQ(context, instance());
    }

};

}  // namespace custom_serialization_test
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

//}

//{ Custom serialization methods

namespace eprosima {
namespace fastcdr {

namespace test = eprosima::fastdds::dds::custom_serialization_test;

template<>
size_t calculate_serialized_size(
        CdrSizeCalculator& calculator,
        const test::CustomData& data,
        size_t& current_alignment)
{
    auto ctx = calculator.get_context();
    test::CustomContext::check_context(std::dynamic_pointer_cast<test::CustomContext>(ctx));

    // We directly encode the data inside the representation header, so we don't need to add any size for the data.
    static_cast<void>(data);
    return current_alignment;
}

template<>
void serialize(
        Cdr& scdr,
        const test::CustomData& data)
{
    auto ctx = scdr.get_context();
    test::CustomContext::check_context(std::dynamic_pointer_cast<test::CustomContext>(ctx));

    // We directly encode the data inside the representation header, so we don't need to serialize any data.
    static_cast<void>(data);
}

template<>
void deserialize(
        Cdr& dcdr,
        test::CustomData& data)
{
    auto ctx = dcdr.get_context();
    test::CustomContext::check_context(std::dynamic_pointer_cast<test::CustomContext>(ctx));

    // We directly encode the data inside the representation header.
    auto options = dcdr.get_dds_cdr_options();
    data.data = options[0];
}

}  // namespace fastcdr
}  // namespace eprosima

//}

namespace eprosima {
namespace fastdds {
namespace dds {
namespace custom_serialization_test {

/**
 * @brief CustomTypeSupport class that implements the TopicDataType interface for the CustomData type.
 *
 * It checks that the operations without context are not called by calling the corresponding _ctx methods with a null context.
 * The CustomData objects serialization is done inside the representation header, so the serialized size of the data is 0.
 * XCDRv1 is considered plain so we can check invocation of the construct_sample method.
 * XCDRv2 is not considered plain, so we can check the invocation of create_data and delete_data methods.
 */
class CustomTypeSupport : public TopicDataType
{
public:

    typedef CustomData type;

    CustomTypeSupport()
    {
        set_name("CustomData");
        // Data is sent inside the representation header on this custom type support.
        max_serialized_type_size = rtps::SerializedPayload_t::representation_header_size;
        is_compute_key_provided = true;
    }

    bool serialize(
            const void* const data,
            rtps::SerializedPayload_t& payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override
    {
        return serialize_ctx(nullptr, data, payload, data_representation);
    }

    bool serialize_ctx(
            const std::shared_ptr<Context>& context,
            const void* const data,
            rtps::SerializedPayload_t& payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override
    {
        CustomContext::check_context(context);

        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.max_size);
        // Object that serializes the data.
        eprosima::fastcdr::Cdr ser(fastbuffer, context, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
                data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
                eprosima::fastcdr::CdrVersion::XCDRv1 : eprosima::fastcdr::CdrVersion::XCDRv2);
        payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        ser.set_encoding_flag(
            data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR :
            eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2);

        try
        {
            const CustomData* p_type = static_cast<const CustomData*>(data);

            // Serialize encapsulation
            ser.serialize_encapsulation();
            // Serialize the object.
            ser << *p_type;
            ser.set_dds_cdr_options({ p_type->data, 0 });
        }
        catch (eprosima::fastcdr::exception::Exception& /*exception*/)
        {
            return false;
        }

        // Get the serialized length
        payload.length = static_cast<uint32_t>(ser.get_serialized_data_length());
        return true;
    }

    bool deserialize(
            rtps::SerializedPayload_t& payload,
            void* data) override
    {
        return deserialize_ctx(nullptr, payload, data);
    }

    bool deserialize_ctx(
            const std::shared_ptr<Context>& context,
            rtps::SerializedPayload_t& payload,
            void* data) override
    {
        CustomContext::check_context(context);

        try
        {
            // Convert DATA to pointer of your type
            CustomData* p_type = static_cast<CustomData*>(data);

            // Object that manages the raw buffer.
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.length);

            // Object that deserializes the data.
            eprosima::fastcdr::Cdr deser(fastbuffer, context, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN);

            // Deserialize encapsulation.
            deser.read_encapsulation();
            payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

            // Deserialize the object.
            deser >> *p_type;

            // Check that the options are correct
            auto options = deser.get_dds_cdr_options();
            EXPECT_EQ(options[0], p_type->data);
        }
        catch (eprosima::fastcdr::exception::Exception& /*exception*/)
        {
            return false;
        }

        return true;
    }

    uint32_t calculate_serialized_size(
            const void* const data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override
    {
        return calculate_serialized_size_ctx(nullptr, data, data_representation);
    }

    uint32_t calculate_serialized_size_ctx(
            const std::shared_ptr<Context>& context,
            const void* const data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override
    {
        CustomContext::check_context(context);

        try
        {
            eprosima::fastcdr::CdrSizeCalculator calculator(
                data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
                eprosima::fastcdr::CdrVersion::XCDRv1 :eprosima::fastcdr::CdrVersion::XCDRv2);
            size_t current_alignment {0};
            const ::Data64kb* p_type =
                    static_cast<const ::Data64kb*>(data);
            auto calc_size = calculator.calculate_serialized_size(*p_type, current_alignment);
            return static_cast<uint32_t>(calc_size) + rtps::SerializedPayload_t::representation_header_size;
        }
        catch (eprosima::fastcdr::exception::Exception& /*exception*/)
        {
            return 0;
        }
    }

    void* create_data() override
    {
        return create_data_ctx(nullptr);
    }

    void* create_data_ctx(
            const std::shared_ptr<Context>& context)
    {
        CustomContext::check_context(context);
        return new CustomData();
    }

    void delete_data(
            void* data) override
    {
        delete_data_ctx(nullptr, data);
    }

    void delete_data_ctx(
            const std::shared_ptr<Context>& context,
            void* data)
    {
        CustomContext::check_context(context);
        delete static_cast<CustomData*>(data);
    }

    bool compute_key(
            rtps::SerializedPayload_t& payload,
            rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override
    {
        return compute_key_ctx(nullptr, payload, ihandle, force_md5);
    }

    bool compute_key_ctx(
            const std::shared_ptr<Context>& context,
            rtps::SerializedPayload_t& payload,
            rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override
    {
        CustomData data;
        if (deserialize_ctx(context, payload, &data))
        {
            return compute_key_ctx(context, &data, ihandle, force_md5);
        }
        return false;
    }

    bool compute_key(
            const void* const data,
            rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override
    {
        return compute_key_ctx(nullptr, data, ihandle, force_md5);
    }

    bool compute_key_ctx(
            const std::shared_ptr<Context>& context,
            const void* const data,
            rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override
    {
        CustomContext::check_context(context);

        const CustomData* p_type = static_cast<const CustomData*>(data);
        if (force_md5)
        {
            eprosima::fastdds::MD5 md5;
            md5.init();
            md5.update(&p_type->data, sizeof(p_type->data));
            md5.finalize();
            for (uint8_t i = 0; i < 16; ++i)
            {
                ihandle.value[i] = md5.digest[i];
            }
        }
        else
        {
            // If the key is smaller than 16 bytes, we can just copy it directly to the handle.
            std::memset(ihandle.value, 0, sizeof(ihandle.value));
            std::memcpy(ihandle.value, &p_type->data, sizeof(p_type->data));
        }
        return true;
    }

    bool is_bounded() const override
    {
        return is_bounded_ctx(nullptr);
    }

    bool is_bounded_ctx(
            const std::shared_ptr<Context>& context) const
    {
        CustomContext::check_context(context);

        return true;
    }

    bool is_plain(
            DataRepresentationId_t representation) const override
    {
        return is_plain_ctx(nullptr, representation);
    }

    bool is_plain_ctx(
            const std::shared_ptr<Context>& context,
            DataRepresentationId_t representation) const override
    {
        CustomContext::check_context(context);
        return eprosima::fastdds::dds::DataRepresentationId_t::XCDR_DATA_REPRESENTATION == representation;
    }

    bool construct_sample(
            void* memory) const override
    {
        return construct_sample_ctx(nullptr, memory);
    }

    bool construct_sample_ctx(
            const std::shared_ptr<Context>& context,
            void* memory) const override
    {
        CustomContext::check_context(context);
        CustomData* p_type = static_cast<CustomData*>(memory);
        new (p_type) CustomData();
        return true;
    }

    void register_type_object_representation() override
    {
        register_type_object_representation_ctx(nullptr);
    }

    void register_type_object_representation_ctx(
            const std::shared_ptr<Context>& context) override
    {
        CustomContext::check_context(context);
    }

    uint32_t get_max_serialized_size_ctx(
            const std::shared_ptr<Context>& context)
    {
        CustomContext::check_context(context);
        return max_serialized_type_size;
    }

};

}  // namespace custom_serialization_test
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
