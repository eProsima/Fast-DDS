// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This program is commercial software licensed under the terms of the
// eProsima Software License Agreement Rev 03 (the "License")
//
// You may obtain a copy of the License at
// https://www.eprosima.com/licenses/LICENSE-REV03

// Tests for the eprosima::fastcdr::calculate_serialized_size/serialize/deserialize
// specializations for eprosima::fastdds::Buffer<uint8_t> declared in
// buffer_serialization.hpp.
//
// These specializations are never invoked directly: they are only reachable
// through fastcdr's generic entry points, so every test below drives them
// exclusively via CdrSizeCalculator::calculate_serialized_size() and the
// Cdr::operator<</operator>> stream operators, with three kinds of context:
//   - no context at all,
//   - a BufferAwareContext (with and without a backend registered), and
//   - a different (non-BufferAwareContext) Context type.

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>
#include <fastcdr/FastBuffer.h>

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/utils/buffer/buffer.hpp>
#include <fastdds/utils/buffer/BufferAwareContext.hpp>
#include <fastdds/utils/buffer/buffer_serialization.hpp>

#include "mock_buffer_backend.hpp"
#include "non_cpu_buffer_impl.hpp"

using eprosima::fastdds::Buffer;
using eprosima::fastdds::BufferAwareContext;

// ========== No context ==========

TEST(TestBufferSerializationNoContext, cpu_buffer_size_matches_serialized_length) {
    Buffer<uint8_t> original{1, 2, 3, 4, 5};

    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment = 0;
    size_t expected_size = calculator.calculate_serialized_size(original, current_alignment);

    std::vector<char> raw_buffer(1024, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    cdr << original;
    EXPECT_EQ(expected_size, cdr.get_serialized_data_length());
}

TEST(TestBufferSerializationNoContext, cpu_buffer_round_trip) {
    Buffer<uint8_t> original{5, 6, 7};

    std::vector<char> raw_buffer(64, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    cdr << original;

    cdr.reset();
    Buffer<uint8_t> deserialized;
    cdr >> deserialized;

    EXPECT_EQ("cpu", deserialized.get_backend_type());
    EXPECT_EQ(original, deserialized);
}

TEST(TestBufferSerializationNoContext, non_cpu_buffer_size_falls_back_to_vector) {
    Buffer<uint8_t> buffer(std::unique_ptr<NonCpuBufferImpl<uint8_t>>(new NonCpuBufferImpl<uint8_t>(6)));

    eprosima::fastcdr::CdrSizeCalculator calculator(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment_buffer = 0;
    size_t buffer_size = calculator.calculate_serialized_size(buffer, current_alignment_buffer);

    std::vector<uint8_t> equivalent_vector(6, 0);
    size_t current_alignment_vector = 0;
    size_t vector_size = calculator.calculate_serialized_size(equivalent_vector, current_alignment_vector);

    EXPECT_EQ(vector_size, buffer_size);
}

TEST(TestBufferSerializationNoContext, non_cpu_buffer_round_trip_falls_back_to_cpu) {
    Buffer<uint8_t> buffer(std::unique_ptr<NonCpuBufferImpl<uint8_t>>(new NonCpuBufferImpl<uint8_t>(5)));

    std::vector<char> raw_buffer(64, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    cdr << buffer;

    cdr.reset();
    Buffer<uint8_t> deserialized;
    cdr >> deserialized;

    EXPECT_EQ("cpu", deserialized.get_backend_type());
    EXPECT_EQ(5u, deserialized.size());
}

// ========== BufferAwareContext ==========

TEST(TestBufferSerializationBufferAwareContext, cpu_buffer_round_trip) {
    auto context = std::make_shared<BufferAwareContext>();
    Buffer<uint8_t> original{9, 8, 7, 6};

    eprosima::fastcdr::CdrSizeCalculator calculator(context, eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment = 0;
    size_t expected_size = calculator.calculate_serialized_size(original, current_alignment);

    std::vector<char> raw_buffer(1024, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, context, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    cdr << original;
    EXPECT_EQ(expected_size, cdr.get_serialized_data_length());

    cdr.reset();
    Buffer<uint8_t> deserialized;
    cdr >> deserialized;

    EXPECT_EQ("cpu", deserialized.get_backend_type());
    EXPECT_EQ(original, deserialized);
}

TEST(TestBufferSerializationBufferAwareContext, non_cpu_buffer_without_registered_backend_falls_back) {
    auto context = std::make_shared<BufferAwareContext>();
    // No backend registered for "non_cpu_test".
    Buffer<uint8_t> buffer(std::unique_ptr<NonCpuBufferImpl<uint8_t>>(new NonCpuBufferImpl<uint8_t>(4)));

    std::vector<char> raw_buffer(64, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, context, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    cdr << buffer;

    cdr.reset();
    std::vector<uint8_t> deserialized_vector;
    cdr >> deserialized_vector;
    EXPECT_EQ(4u, deserialized_vector.size());
}

TEST(TestBufferSerializationBufferAwareContext, size_uses_registered_backend) {
    auto context = std::make_shared<BufferAwareContext>();
    auto backend = std::make_shared<MockBufferBackend>();
    context->register_backend("mock", backend);

    Buffer<uint8_t> buffer(std::unique_ptr<MockBufferImpl>(new MockBufferImpl(std::vector<uint8_t>{1, 2, 3, 4, 5})));

    eprosima::fastcdr::CdrSizeCalculator calculator(context, eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment = 0;
    size_t size = calculator.calculate_serialized_size(buffer, current_alignment);

    EXPECT_EQ(1u, backend->times_size_calculated);
    // Markers (2 * uint32_t) + backend type string + backend-reported payload size.
    EXPECT_GT(size, 2 * sizeof(uint32_t));
}

TEST(TestBufferSerializationBufferAwareContext, round_trip_uses_registered_backend) {
    auto context = std::make_shared<BufferAwareContext>();
    auto backend = std::make_shared<MockBufferBackend>();
    context->register_backend("mock", backend);

    Buffer<uint8_t> original(std::unique_ptr<MockBufferImpl>(new MockBufferImpl(
                std::vector<uint8_t>{42, 43, 44})));

    eprosima::fastcdr::CdrSizeCalculator calculator(context, eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment = 0;
    size_t expected_size = calculator.calculate_serialized_size(original, current_alignment);

    std::vector<char> raw_buffer(1024, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, context, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    cdr << original;
    EXPECT_EQ(1u, backend->times_serialized);
    EXPECT_EQ(expected_size, cdr.get_serialized_data_length());

    cdr.reset();
    Buffer<uint8_t> deserialized;
    cdr >> deserialized;
    EXPECT_EQ(1u, backend->times_deserialized);

    EXPECT_EQ("mock", deserialized.get_backend_type());
    EXPECT_EQ(original.to_vector(), deserialized.to_vector());
}

TEST(TestBufferSerializationBufferAwareContext, cpu_buffer_does_not_use_registered_backend) {
    // A backend registered for "mock" must not be consulted when the buffer
    // being (de)serialized is a plain CPU buffer.
    auto context = std::make_shared<BufferAwareContext>();
    auto backend = std::make_shared<MockBufferBackend>();
    context->register_backend("mock", backend);

    Buffer<uint8_t> original{1, 2, 3};

    std::vector<char> raw_buffer(64, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, context, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    cdr << original;
    EXPECT_EQ(0u, backend->times_serialized);

    cdr.reset();
    Buffer<uint8_t> deserialized;
    cdr >> deserialized;
    EXPECT_EQ(0u, backend->times_deserialized);
    EXPECT_EQ("cpu", deserialized.get_backend_type());
}

// ========== A different (non-BufferAwareContext) context type ==========

TEST(TestBufferSerializationOtherContext, cpu_buffer_round_trip) {
    auto context = std::make_shared<eprosima::fastdds::dds::TopicDataType::Context>();
    Buffer<uint8_t> original{11, 22, 33};

    eprosima::fastcdr::CdrSizeCalculator calculator(context, eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment = 0;
    size_t expected_size = calculator.calculate_serialized_size(original, current_alignment);

    std::vector<char> raw_buffer(64, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, context, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    cdr << original;
    EXPECT_EQ(expected_size, cdr.get_serialized_data_length());

    cdr.reset();
    Buffer<uint8_t> deserialized;
    cdr >> deserialized;

    EXPECT_EQ("cpu", deserialized.get_backend_type());
    EXPECT_EQ(original, deserialized);
}

TEST(TestBufferSerializationOtherContext, non_cpu_buffer_falls_back_to_vector) {
    // A plain (non-BufferAwareContext) Context must not trigger custom
    // backend dispatch, even for non-cpu buffers: it is indistinguishable
    // from having no context at all.
    auto context = std::make_shared<eprosima::fastdds::dds::TopicDataType::Context>();
    Buffer<uint8_t> buffer(std::unique_ptr<NonCpuBufferImpl<uint8_t>>(new NonCpuBufferImpl<uint8_t>(4)));

    std::vector<char> raw_buffer(64, 0);
    eprosima::fastcdr::FastBuffer fastbuffer(raw_buffer.data(), raw_buffer.size());
    eprosima::fastcdr::Cdr cdr(fastbuffer, context, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv2);

    cdr << buffer;

    cdr.reset();
    std::vector<uint8_t> deserialized_vector;
    cdr >> deserialized_vector;
    EXPECT_EQ(4u, deserialized_vector.size());
}

TEST(TestBufferSerializationOtherContext, size_matches_no_context_fallback) {
    // The size calculation for a non-cpu buffer with an unrelated context
    // type must be identical to the no-context fallback: neither triggers
    // custom backend dispatch.
    auto context = std::make_shared<eprosima::fastdds::dds::TopicDataType::Context>();
    Buffer<uint8_t> buffer_with_context(std::unique_ptr<NonCpuBufferImpl<uint8_t>>(
                new NonCpuBufferImpl<uint8_t>(9)));
    Buffer<uint8_t> buffer_without_context(std::unique_ptr<NonCpuBufferImpl<uint8_t>>(
                new NonCpuBufferImpl<uint8_t>(9)));

    eprosima::fastcdr::CdrSizeCalculator calculator_with_context(context, eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment_with_context = 0;
    size_t size_with_context = calculator_with_context.calculate_serialized_size(
        buffer_with_context, current_alignment_with_context);

    eprosima::fastcdr::CdrSizeCalculator calculator_without_context(eprosima::fastcdr::CdrVersion::XCDRv2);
    size_t current_alignment_without_context = 0;
    size_t size_without_context = calculator_without_context.calculate_serialized_size(
        buffer_without_context, current_alignment_without_context);

    EXPECT_EQ(size_without_context, size_with_context);
}
