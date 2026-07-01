// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This program is commercial software licensed under the terms of the
// eProsima Software License Agreement Rev 03 (the "License")
//
// You may obtain a copy of the License at
// https://www.eprosima.com/licenses/LICENSE-REV03

#ifndef MOCK_BUFFER_BACKEND_HPP_
#define MOCK_BUFFER_BACKEND_HPP_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/utils/buffer/BufferBackend.hpp>
#include <fastdds/utils/buffer/buffer_impl_base.hpp>
#include <fastdds/utils/buffer/cpu_buffer_impl.hpp>

/// Minimal non-CPU buffer implementation that actually holds data, so a
/// registered backend can serialize/deserialize something meaningful.
class MockBufferImpl : public eprosima::fastdds::BufferImplBase<uint8_t>
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

    std::unique_ptr<eprosima::fastdds::BufferImplBase<uint8_t>> to_cpu() const override
    {
        auto cpu = new eprosima::fastdds::CpuBufferImpl<uint8_t>();
        cpu->get_storage() = data_;
        return std::unique_ptr<eprosima::fastdds::BufferImplBase<uint8_t>>(cpu);
    }

    std::unique_ptr<eprosima::fastdds::BufferImplBase<uint8_t>> clone() const override
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
/// original data, and dispatch to it can be counted.
class MockBufferBackend : public eprosima::fastdds::BufferBackend
{
public:

    size_t calculate_serialized_size(
            eprosima::fastcdr::CdrSizeCalculator& calculator,
            const eprosima::fastdds::BufferImplBase<uint8_t>& data,
            size_t& current_alignment) const override
    {
        const auto& mock = static_cast<const MockBufferImpl&>(data);
        ++times_size_calculated;
        return calculator.calculate_serialized_size(mock.data(), current_alignment);
    }

    void serialize(
            eprosima::fastcdr::Cdr& cdr,
            const eprosima::fastdds::BufferImplBase<uint8_t>& data) const override
    {
        const auto& mock = static_cast<const MockBufferImpl&>(data);
        cdr << mock.data();
        ++times_serialized;
    }

    std::unique_ptr<eprosima::fastdds::BufferImplBase<uint8_t>> deserialize(
            eprosima::fastcdr::Cdr& cdr) const override
    {
        std::vector<uint8_t> vec;
        cdr >> vec;
        ++times_deserialized;
        return std::unique_ptr<MockBufferImpl>(new MockBufferImpl(std::move(vec)));
    }

    mutable size_t times_size_calculated = 0;
    mutable size_t times_serialized = 0;
    mutable size_t times_deserialized = 0;
};

#endif  // MOCK_BUFFER_BACKEND_HPP_
