// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This program is commercial software licensed under the terms of the
// eProsima Software License Agreement Rev 03 (the "License")
//
// You may obtain a copy of the License at
// https://www.eprosima.com/licenses/LICENSE-REV03

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/exceptions/BadParamException.h>

#include <fastdds/utils/buffer/buffer.hpp>
#include <fastdds/utils/buffer/BufferAwareContext.hpp>
#include <fastdds/utils/buffer/BufferBackend.hpp>
#include <fastdds/utils/buffer/buffer_impl_base.hpp>
#include <fastdds/utils/buffer/buffer_serialization.hpp>
#include <fastdds/utils/buffer/cpu_buffer_impl.hpp>

#include "non_cpu_buffer_impl.hpp"

using eprosima::fastdds::Buffer;
using eprosima::fastdds::BufferAwareContext;
using eprosima::fastdds::BufferBackend;
using eprosima::fastdds::BufferImplBase;
using eprosima::fastdds::CpuBufferImpl;

namespace {

/// Minimal non-CPU buffer implementation that actually holds data, so a
/// registered backend can serialize/deserialize something meaningful.
class MockBufferImpl : public BufferImplBase<uint8_t>
{
public:

    explicit MockBufferImpl(
            std::vector<uint8_t> data)
        : data_(std::move(data))
    {
    }

    std::string get_backend_type() const override
    {
        return "mock";
    }

    size_t size() const override
    {
        return data_.size();
    }

    std::unique_ptr<BufferImplBase<uint8_t>> to_cpu() const override
    {
        std::unique_ptr<CpuBufferImpl<uint8_t>> cpu(new CpuBufferImpl<uint8_t>());
        cpu->get_storage() = data_;
        return std::move(cpu);
    }

    std::unique_ptr<BufferImplBase<uint8_t>> clone() const override
    {
        return std::unique_ptr<MockBufferImpl>(new MockBufferImpl(data_));
    }

    const std::vector<uint8_t>& data() const
    {
        return data_;
    }

private:

    std::vector<uint8_t> data_;
};

/// Backend that serializes/deserializes MockBufferImpl instances as a plain
/// CDR sequence of uint8_t, so round trips can be checked against the
/// original data.
class MockBufferBackend : public BufferBackend
{
public:

    size_t calculate_serialized_size(
            eprosima::fastcdr::CdrSizeCalculator& calculator,
            const BufferImplBase<uint8_t>& data,
            size_t& current_alignment) const override
    {
        const auto& mock = static_cast<const MockBufferImpl&>(data);
        return calculator.calculate_serialized_size(mock.data(), current_alignment);
    }

    void serialize(
            eprosima::fastcdr::Cdr& cdr,
            const BufferImplBase<uint8_t>& data) const override
    {
        const auto& mock = static_cast<const MockBufferImpl&>(data);
        cdr << mock.data();
        ++times_serialized;
    }

    std::unique_ptr<BufferImplBase<uint8_t>> deserialize(
            eprosima::fastcdr::Cdr& cdr) const override
    {
        std::vector<uint8_t> vec;
        cdr >> vec;
        ++times_deserialized;
        return std::unique_ptr<MockBufferImpl>(new MockBufferImpl(std::move(vec)));
    }

    mutable size_t times_serialized = 0;
    mutable size_t times_deserialized = 0;
};

}  // namespace

// ========== Backend registry ==========

TEST(TestBufferAwareContext, get_backend_not_registered_returns_null) {
    BufferAwareContext context;
    EXPECT_EQ(nullptr, context.get_backend("unknown"));
}

TEST(TestBufferAwareContext, register_and_get_backend) {
    BufferAwareContext context;
    auto backend = std::make_shared<MockBufferBackend>();

    context.register_backend("mock", backend);

    EXPECT_EQ(backend, context.get_backend("mock"));
    EXPECT_EQ(nullptr, context.get_backend("other"));
}

TEST(TestBufferAwareContext, register_backend_overwrites_previous) {
    BufferAwareContext context;
    auto backend_1 = std::make_shared<MockBufferBackend>();
    auto backend_2 = std::make_shared<MockBufferBackend>();

    context.register_backend("mock", backend_1);
    context.register_backend("mock", backend_2);

    EXPECT_EQ(backend_2, context.get_backend("mock"));
}

TEST(TestBufferAwareContext, for_each_backend_iterates_all_registered) {
    BufferAwareContext context;
    auto backend_1 = std::make_shared<MockBufferBackend>();
    auto backend_2 = std::make_shared<MockBufferBackend>();

    context.register_backend("mock1", backend_1);
    context.register_backend("mock2", backend_2);

    std::vector<std::string> visited_types;
    context.for_each_backend(
        [&](const std::string& backend_type, const std::shared_ptr<BufferBackend>& backend)
        {
            visited_types.push_back(backend_type);
            EXPECT_NE(nullptr, backend);
        });

    ASSERT_EQ(2u, visited_types.size());
    std::sort(visited_types.begin(), visited_types.end());
    EXPECT_EQ("mock1", visited_types[0]);
    EXPECT_EQ("mock2", visited_types[1]);
}

TEST(TestBufferAwareContext, register_same_backend_under_different_names) {
    BufferAwareContext context;
    auto backend = std::make_shared<MockBufferBackend>();

    context.register_backend("mock1", backend);
    context.register_backend("mock2", backend);

    EXPECT_EQ(backend, context.get_backend("mock1"));
    EXPECT_EQ(backend, context.get_backend("mock2"));
}

TEST(TestBufferAwareContext, for_each_backend_empty_registry) {
    BufferAwareContext context;
    size_t count = 0;
    context.for_each_backend(
        [&](const std::string&, const std::shared_ptr<BufferBackend>&)
        {
            ++count;
        });
    EXPECT_EQ(0u, count);
}

// ========== CPU buffer serialization ==========

TEST(TestBufferAwareContext, cpu_buffer_round_trip) {
    BufferAwareContext context;
    Buffer<uint8_t> original{1, 2, 3, 4, 5};

    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment = 0;
    size_t expected_size = context.calculate_serialized_size(calculator, original, current_alignment);
    EXPECT_GT(expected_size, 0u);

    std::vector<char> raw_buffer(1024, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    context.serialize(cdr, original);
    EXPECT_EQ(expected_size, cdr.get_serialized_data_length());

    cdr.reset();
    Buffer<uint8_t> deserialized;
    context.deserialize(cdr, deserialized);

    EXPECT_EQ("cpu", deserialized.get_backend_type());
    EXPECT_EQ(original, deserialized);
}

TEST(TestBufferAwareContext, cpu_buffer_empty_round_trip) {
    BufferAwareContext context;
    Buffer<uint8_t> original;

    std::vector<char> raw_buffer(64, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    context.serialize(cdr, original);

    cdr.reset();
    Buffer<uint8_t> deserialized{9, 9};
    context.deserialize(cdr, deserialized);

    EXPECT_TRUE(deserialized.empty());
}

// ========== Custom backend serialization ==========

TEST(TestBufferAwareContext, calculate_serialized_size_uses_registered_backend) {
    BufferAwareContext context;
    auto backend = std::make_shared<MockBufferBackend>();
    context.register_backend("mock", backend);

    Buffer<uint8_t> buffer(std::unique_ptr<MockBufferImpl>(new MockBufferImpl(std::vector<uint8_t>{1, 2, 3})));

    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment = 0;
    size_t size = context.calculate_serialized_size(calculator, buffer, current_alignment);

    // Markers (2 * uint32_t) + backend type string + backend-reported payload size.
    EXPECT_GT(size, 2 * sizeof(uint32_t));
}

TEST(TestBufferAwareContext, serialize_deserialize_round_trip_custom_backend) {
    BufferAwareContext context;
    auto backend = std::make_shared<MockBufferBackend>();
    context.register_backend("mock", backend);

    Buffer<uint8_t> original(std::unique_ptr<MockBufferImpl>(new MockBufferImpl(std::vector<uint8_t>{10, 20, 30, 40})));

    std::vector<char> raw_buffer(1024, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    context.serialize(cdr, original);
    EXPECT_EQ(1u, backend->times_serialized);

    cdr.reset();
    Buffer<uint8_t> deserialized;
    context.deserialize(cdr, deserialized);
    EXPECT_EQ(1u, backend->times_deserialized);

    EXPECT_EQ("mock", deserialized.get_backend_type());
    EXPECT_EQ(4u, deserialized.size());
    EXPECT_EQ(original.to_vector(), deserialized.to_vector());
}

TEST(TestBufferAwareContext, deserialize_unknown_backend_throws) {
    BufferAwareContext context;
    // No backend registered for "mock".

    std::vector<char> raw_buffer(64, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    // Manually write the custom-serialization marker sequence followed by an
    // unregistered backend type name, mimicking what serialize() would have
    // produced if a "mock" backend had been registered on the sender side.
    constexpr uint32_t marker1 = 0xFFFFFFFFu;
    constexpr uint32_t marker2 = 0x524F5332u;
    cdr << marker1;
    cdr << marker2;
    std::string backend_type = "mock";
    cdr << backend_type;

    cdr.reset();
    Buffer<uint8_t> deserialized;
    EXPECT_THROW(context.deserialize(cdr, deserialized), eprosima::fastcdr::exception::BadParamException);
}

// ========== Fallback behavior (no backend registered) ==========

TEST(TestBufferAwareContext, calculate_serialized_size_fallback_xcdrv1) {
    BufferAwareContext context;
    // No backend registered for the non-cpu impl's type.
    Buffer<uint8_t> buffer(std::unique_ptr<NonCpuBufferImpl<uint8_t>>(new NonCpuBufferImpl<uint8_t>(7)));

    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv1);
    size_t current_alignment = 0;
    size_t size = context.calculate_serialized_size(calculator, buffer, current_alignment);

    // Length field (aligned uint32_t) + raw data size, no backend involved.
    EXPECT_EQ(sizeof(uint32_t) + 7u, size);
}

TEST(TestBufferAwareContext, calculate_serialized_size_fallback_non_xcdrv1_matches_vector) {
    BufferAwareContext context;
    Buffer<uint8_t> buffer(std::unique_ptr<NonCpuBufferImpl<uint8_t>>(new NonCpuBufferImpl<uint8_t>(7)));

    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment_buffer = 0;
    size_t buffer_size = context.calculate_serialized_size(calculator, buffer, current_alignment_buffer);

    std::vector<uint8_t> equivalent_vector(7, 0);
    size_t current_alignment_vector = 0;
    size_t vector_size = calculator.calculate_serialized_size(equivalent_vector, current_alignment_vector);

    EXPECT_EQ(vector_size, buffer_size);
}

TEST(TestBufferAwareContext, serialize_fallback_no_backend_produces_plain_vector) {
    BufferAwareContext context;
    Buffer<uint8_t> buffer(std::unique_ptr<NonCpuBufferImpl<uint8_t>>(new NonCpuBufferImpl<uint8_t>(3)));

    std::vector<char> raw_buffer(64, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    context.serialize(cdr, buffer);

    // No backend was involved, so this is indistinguishable from a plain
    // std::vector<uint8_t> serialization: it can be read back without going
    // through the context at all.
    cdr.reset();
    std::vector<uint8_t> deserialized_vector;
    cdr >> deserialized_vector;
    EXPECT_EQ(3u, deserialized_vector.size());
}

TEST(TestBufferAwareContext, deserialize_plain_vector_without_markers) {
    BufferAwareContext context;

    std::vector<char> raw_buffer(64, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    std::vector<uint8_t> original_vector{1, 2, 3, 4};
    cdr << original_vector;

    cdr.reset();
    Buffer<uint8_t> deserialized;
    context.deserialize(cdr, deserialized);

    EXPECT_EQ("cpu", deserialized.get_backend_type());
    EXPECT_EQ(original_vector, deserialized.to_vector());
}
