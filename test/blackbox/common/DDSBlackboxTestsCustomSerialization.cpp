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
#include <cstring>
#include <limits>
#include <memory>

#include <gtest/gtest.h>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/core/StackAllocatedSequence.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/utils/md5.hpp>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "UDPMessageSender.hpp"

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

bool operator ==(
        const CustomData& lhs,
        const CustomData& rhs)
{
    return lhs.data == rhs.data;
}

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
    // We directly encode the data inside the representation header, so we don't need to add any size for the data.
    static_cast<void>(calculator);
    static_cast<void>(data);
    return current_alignment;
}

template<>
void serialize(
        Cdr& scdr,
        const test::CustomData& data)
{
    // We directly encode the data inside the representation header, so we don't need to serialize any data.
    static_cast<void>(scdr);
    static_cast<void>(data);
}

template<>
void deserialize(
        Cdr& dcdr,
        test::CustomData& data)
{
    // We directly encode the data inside the representation header.
    std::array<uint8_t, 2> options = dcdr.get_dds_cdr_options();
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

    struct CustomTypeSupportCalls
    {
        bool serialize = false;
        bool deserialize = false;
        bool calculate_serialized_size = false;
        bool create_data = false;
        bool delete_data = false;
        bool compute_key_payload = false;
        bool compute_key_data = false;
        bool is_bounded = false;
        bool is_plain = false;
        bool construct_sample = false;
        bool register_type_object_representation = false;
        bool get_max_serialized_size = false;

        bool operator ==(
                const CustomTypeSupportCalls& other) const
        {
            return serialize == other.serialize &&
                   deserialize == other.deserialize &&
                   calculate_serialized_size == other.calculate_serialized_size &&
                   create_data == other.create_data &&
                   delete_data == other.delete_data &&
                   compute_key_payload == other.compute_key_payload &&
                   compute_key_data == other.compute_key_data &&
                   is_bounded == other.is_bounded &&
                   is_plain == other.is_plain &&
                   construct_sample == other.construct_sample &&
                   register_type_object_representation == other.register_type_object_representation &&
                   get_max_serialized_size == other.get_max_serialized_size;
        }

    };

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
        was_called(custom_calls_.serialize);
        CustomContext::check_context(context);

        eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.max_size);
        // Object that serializes the data.
        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
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
        was_called(custom_calls_.deserialize);
        CustomContext::check_context(context);

        try
        {
            // Convert DATA to pointer of your type
            CustomData* p_type = static_cast<CustomData*>(data);

            // Object that manages the raw buffer.
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.length);

            // Object that deserializes the data.
            eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN);

            // Deserialize encapsulation.
            deser.read_encapsulation();
            payload.encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

            // Deserialize the object.
            deser >> *p_type;

            // Check that the options are correct
            std::array<uint8_t, 2> options = deser.get_dds_cdr_options();
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
        was_called(custom_calls_.calculate_serialized_size);
        CustomContext::check_context(context);

        try
        {
            eprosima::fastcdr::CdrSizeCalculator calculator(
                data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ?
                eprosima::fastcdr::CdrVersion::XCDRv1 :eprosima::fastcdr::CdrVersion::XCDRv2);
            size_t current_alignment {0};
            const CustomData* p_type = static_cast<const CustomData*>(data);
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
            const std::shared_ptr<Context>& context) override
    {
        was_called(custom_calls_.create_data);
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
            void* data) override
    {
        was_called(custom_calls_.delete_data);
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
        was_called(custom_calls_.compute_key_payload);
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
        was_called(custom_calls_.compute_key_data);
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
            const std::shared_ptr<Context>& context) const override
    {
        was_called(custom_calls_.is_bounded);
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
        was_called(custom_calls_.is_plain);
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
        was_called(custom_calls_.construct_sample);
        CustomContext::check_context(context);
        CustomData* p_type = static_cast<CustomData*>(memory);
        new (p_type) CustomData();
        return true;
    }

    void register_type_object_representation() override
    {
        // Note this is called from the participant when the type is registered.
        // Adding the context, since we don't have access to the context here.
        register_type_object_representation_ctx(CustomContext::instance());
    }

    void register_type_object_representation_ctx(
            const std::shared_ptr<Context>& context) override
    {
        was_called(custom_calls_.register_type_object_representation);
        CustomContext::check_context(context);
    }

    uint32_t get_max_serialized_size_ctx(
            const std::shared_ptr<Context>& context) override
    {
        was_called(custom_calls_.get_max_serialized_size);
        CustomContext::check_context(context);
        return max_serialized_type_size;
    }

    CustomTypeSupportCalls get_calls() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return custom_calls_;
    }

protected:

    void was_called(
            bool& flag) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        flag = true;
    }

    mutable std::mutex mutex_;
    mutable CustomTypeSupportCalls custom_calls_;
};

class CustomTypeSupportUnbounded : public CustomTypeSupport
{
public:

    bool is_bounded_ctx(
            const std::shared_ptr<Context>& context) const override
    {
        was_called(custom_calls_.is_bounded);
        CustomContext::check_context(context);

        return false;
    }

};

TEST(CustomSerializationTests, XCDRv1_plain)
{
    auto context = CustomContext::instance();

    PubSubWriter<CustomTypeSupport> writer(TEST_TOPIC_NAME);
    PubSubReader<CustomTypeSupport> reader(TEST_TOPIC_NAME);
    CustomTypeSupport* writer_type = nullptr;
    CustomTypeSupport* reader_type = nullptr;
    CustomData sent_data;
    sent_data.data = 42;

    // Check reader initialization calls
    {
        CustomTypeSupport::CustomTypeSupportCalls expected_calls {};
        expected_calls.get_max_serialized_size = true;
        expected_calls.register_type_object_representation = true;
        expected_calls.is_bounded = true;
        expected_calls.is_plain = true;

        reader.reliability(eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS)
                .durability_kind(eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS)
                .data_representation({ eprosima::fastdds::dds::DataRepresentationId_t::XCDR_DATA_REPRESENTATION })
                .context(context)
                .init();
        ASSERT_TRUE(reader.isInitialized());
        reader_type = dynamic_cast<CustomTypeSupport*>(reader.get_type_support().get());
        ASSERT_NE(reader_type, nullptr);

        EXPECT_EQ(reader_type->get_calls(), expected_calls);
    }

    // Check writer initialization calls
    {
        CustomTypeSupport::CustomTypeSupportCalls expected_calls {};
        expected_calls.get_max_serialized_size = true;
        expected_calls.register_type_object_representation = true;
        expected_calls.is_bounded = true;
        expected_calls.is_plain = true;

        writer.reliability(eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS)
                .durability_kind(eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS)
                .data_representation({ eprosima::fastdds::dds::DataRepresentationId_t::XCDR_DATA_REPRESENTATION })
                .context(context)
                .init();
        ASSERT_TRUE(writer.isInitialized());
        writer_type = dynamic_cast<CustomTypeSupport*>(writer.get_type_support().get());
        ASSERT_NE(writer_type, nullptr);

        EXPECT_EQ(writer_type->get_calls(), expected_calls);
    }

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    DataReader& data_reader = reader.get_native_reader();
    DataWriter& data_writer = writer.get_native_writer();

    // Check calls after loaning and discarding a sample
    {
        auto expected_calls = writer_type->get_calls();
        expected_calls.construct_sample = true;

        void* loaned_sample = nullptr;
        ReturnCode_t loan_result = data_writer.loan_sample(
            loaned_sample, DataWriter::LoanInitializationKind::CONSTRUCTED_LOAN_INITIALIZATION);
        EXPECT_EQ(loan_result, RETCODE_OK);
        EXPECT_EQ(data_writer.discard_loan(loaned_sample), RETCODE_OK);

        EXPECT_EQ(writer_type->get_calls(), expected_calls);
    }

    // Check calls after sending a sample
    {
        auto expected_calls = writer_type->get_calls();
        expected_calls.compute_key_data = true;
        expected_calls.serialize = true;

        EXPECT_TRUE(writer.send_sample(sent_data));

        EXPECT_EQ(writer_type->get_calls(), expected_calls);
    }

    // Check calls after a message is received but before it is read or taken
    {
        auto expected_calls = reader_type->get_calls();
        // This is not called since the writer transmits the instance handle along with the data
        expected_calls.compute_key_payload = false;

        EXPECT_TRUE(data_reader.wait_for_unread_message({1, 0}));

        EXPECT_EQ(reader_type->get_calls(), expected_calls);
    }

    // Check calls after reading with loans
    {
        auto expected_calls = reader_type->get_calls();
        // No calls are added since the data is not deserialized

        FASTDDS_SEQUENCE(DataSeq, CustomData);
        DataSeq data_seq;
        SampleInfoSeq info_seq;
        EXPECT_EQ(data_reader.read(data_seq, info_seq), RETCODE_OK);
        EXPECT_EQ(data_seq.length(), 1u);
        EXPECT_EQ(info_seq.length(), 1u);
        EXPECT_TRUE(info_seq[0].valid_data);
        EXPECT_EQ(data_reader.return_loan(data_seq, info_seq), RETCODE_OK);

        EXPECT_EQ(reader_type->get_calls(), expected_calls);
    }

    // Check calls after taking without loans
    {
        auto expected_calls = reader_type->get_calls();
        expected_calls.deserialize = true;

        StackAllocatedSequence<CustomData, 1> data_seq_take;
        SampleInfoSeq info_seq_take(1);
        EXPECT_EQ(data_reader.take(data_seq_take, info_seq_take), RETCODE_OK);
        EXPECT_EQ(data_seq_take.length(), 1u);
        EXPECT_EQ(info_seq_take.length(), 1u);
        EXPECT_TRUE(info_seq_take[0].valid_data);
        EXPECT_EQ(sent_data, data_seq_take[0]);

        EXPECT_EQ(reader_type->get_calls(), expected_calls);
    }

    {
        auto expected_calls = reader_type->get_calls();
        expected_calls.compute_key_payload = true;
        expected_calls.compute_key_data = true;

        struct PacketNoInlineQos
        {
            std::array<char, 4> rtps_id{ {'R', 'T', 'P', 'S'} };
            std::array<uint8_t, 2> protocol_version{ {2, 3} };
            std::array<uint8_t, 2> vendor_id{ {0x01, 0x0F} };
            rtps::GuidPrefix_t sender_prefix{};

            struct DataSubMsg
            {
                uint8_t submessage_id = 0x15; // DATA
    #if FASTDDS_IS_BIG_ENDIAN_TARGET
                uint8_t flags = 0x04;         // D=1
    #else
                uint8_t flags = 0x05;         // E=1, D=1
    #endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
                uint16_t octets_to_next_header = 24;
                uint16_t extra_flags = 0;
                uint16_t octets_to_inline_qos = 16;
                rtps::EntityId_t reader_id{};
                rtps::EntityId_t writer_id{};
                rtps::SequenceNumber_t sn{ 0, 2u };
                // Minimal serialized payload: just the CDR_LE encapsulation header.
                std::array<uint8_t, 4> serialized_payload{ {0x00, 0x01, 24u, 0x00} };
            }
            data;
        };

        rtps::LocatorList locators;
        if ((RETCODE_OK == data_reader.get_listening_locators(locators)) && !locators.empty())
        {
            UDPMessageSender msg_sender;
            PacketNoInlineQos packet;
            packet.sender_prefix = data_writer.guid().guidPrefix;
            packet.data.writer_id = data_writer.guid().entityId;
            packet.data.reader_id = data_reader.guid().entityId;

            CDRMessage_t msg(0);
            uint32_t msg_len = static_cast<uint32_t>(sizeof(packet));
            msg.init(reinterpret_cast<octet*>(&packet), msg_len);
            msg.length = msg_len;
            msg.pos = msg_len;

            for (const auto& locator : locators)
            {
                msg_sender.send(msg, locator);
            }
        }

        EXPECT_TRUE(data_reader.wait_for_unread_message({ 1, 0 }));

        EXPECT_EQ(reader_type->get_calls(), expected_calls);
    }
}

TEST(CustomSerializationTests, XCDRv2_not_plain)
{
    auto context = CustomContext::instance();

    PubSubWriter<CustomTypeSupportUnbounded> writer(TEST_TOPIC_NAME);
    PubSubReader<CustomTypeSupportUnbounded> reader(TEST_TOPIC_NAME);
    CustomTypeSupport* writer_type = nullptr;
    CustomTypeSupport* reader_type = nullptr;
    CustomData sent_data;
    sent_data.data = 42;

    // Check reader initialization calls
    {
        CustomTypeSupport::CustomTypeSupportCalls expected_calls{};
        expected_calls.get_max_serialized_size = true;
        expected_calls.register_type_object_representation = true;
        expected_calls.is_bounded = true;
        expected_calls.is_plain = true;

        reader.reliability(eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS)
                .durability_kind(eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS)
                .data_representation({ eprosima::fastdds::dds::DataRepresentationId_t::XCDR2_DATA_REPRESENTATION })
                .resource_limits_allocated_samples(0)
                .context(context)
                .init();
        ASSERT_TRUE(reader.isInitialized());
        reader_type = dynamic_cast<CustomTypeSupport*>(reader.get_type_support().get());
        ASSERT_NE(reader_type, nullptr);

        EXPECT_EQ(reader_type->get_calls(), expected_calls);
    }

    // Check writer initialization calls
    {
        CustomTypeSupport::CustomTypeSupportCalls expected_calls{};
        expected_calls.get_max_serialized_size = true;
        expected_calls.register_type_object_representation = true;
        expected_calls.is_bounded = true;
        expected_calls.is_plain = true;

        writer.reliability(eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS)
                .durability_kind(eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS)
                .data_representation({ eprosima::fastdds::dds::DataRepresentationId_t::XCDR2_DATA_REPRESENTATION })
                .context(context)
                .init();
        ASSERT_TRUE(writer.isInitialized());
        writer_type = dynamic_cast<CustomTypeSupport*>(writer.get_type_support().get());
        ASSERT_NE(writer_type, nullptr);

        EXPECT_EQ(writer_type->get_calls(), expected_calls);
    }

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    DataReader& data_reader = reader.get_native_reader();

    // Check calls after sending a sample
    {
        auto expected_calls = writer_type->get_calls();
        expected_calls.compute_key_data = true;
        expected_calls.serialize = true;
        expected_calls.calculate_serialized_size = true;

        EXPECT_TRUE(writer.send_sample(sent_data));

        EXPECT_EQ(writer_type->get_calls(), expected_calls);
    }

    // Check calls after a message is received but before it is read or taken
    {
        auto expected_calls = reader_type->get_calls();
        // This is not called since the writer transmits the instance handle along with the data
        expected_calls.compute_key_payload = false;

        EXPECT_TRUE(data_reader.wait_for_unread_message({ 1, 0 }));

        EXPECT_EQ(reader_type->get_calls(), expected_calls);
    }

    // Check calls after reading with loans
    {
        auto expected_calls = reader_type->get_calls();
        expected_calls.create_data = true;
        expected_calls.deserialize = true;
        expected_calls.delete_data = false; // Data is kept in the loan manager until the reader is destroyed.

        FASTDDS_SEQUENCE(DataSeq, CustomData);
        DataSeq data_seq;
        SampleInfoSeq info_seq;
        EXPECT_EQ(data_reader.read(data_seq, info_seq), RETCODE_OK);
        EXPECT_EQ(data_seq.length(), 1u);
        EXPECT_EQ(info_seq.length(), 1u);
        EXPECT_TRUE(info_seq[0].valid_data);
        EXPECT_EQ(data_reader.return_loan(data_seq, info_seq), RETCODE_OK);

        EXPECT_EQ(reader_type->get_calls(), expected_calls);
    }

    // Check calls after taking without loans
    {
        auto expected_calls = reader_type->get_calls();
        expected_calls.deserialize = true;

        StackAllocatedSequence<CustomData, 1> data_seq_take;
        SampleInfoSeq info_seq_take(1);
        EXPECT_EQ(data_reader.take(data_seq_take, info_seq_take), RETCODE_OK);
        EXPECT_EQ(data_seq_take.length(), 1u);
        EXPECT_EQ(info_seq_take.length(), 1u);
        EXPECT_TRUE(info_seq_take[0].valid_data);
        EXPECT_EQ(sent_data, data_seq_take[0]);

        EXPECT_EQ(reader_type->get_calls(), expected_calls);
    }

    // Check calls after destroying the reader
    {
        auto expected_calls = reader_type->get_calls();
        expected_calls.delete_data = true;

        reader.destroy();

        EXPECT_EQ(reader_type->get_calls(), expected_calls);
    }
}

}  // namespace custom_serialization_test
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima
