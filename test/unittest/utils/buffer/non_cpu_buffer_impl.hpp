// Copyright 2026 Open Source Robotics Foundation, Inc.
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

#ifndef NON_CPU_BUFFER_IMPL_HPP_
#define NON_CPU_BUFFER_IMPL_HPP_

#include <cstddef>
#include <memory>
#include <string>

#include <fastdds/utils/buffer/buffer_impl_base.hpp>
#include <fastdds/utils/buffer/cpu_buffer_impl.hpp>

/// Minimal non-CPU implementation for testing that CPU-only operations throw.
template<typename T>
class NonCpuBufferImpl : public eprosima::fastdds::BufferImplBase<T>
{
public:

    explicit NonCpuBufferImpl(
            size_t count)
        : size_(count)
    {
    }

    std::string get_backend_type() const override
    {
        return "non_cpu_test";
    }

    size_t size() const override
    {
        return size_;
    }

    std::unique_ptr<eprosima::fastdds::BufferImplBase<T>> to_cpu() const override
    {
        auto cpu = std::make_unique<eprosima::fastdds::CpuBufferImpl<T>>();
        cpu->get_storage().resize(size_);
        return cpu;
    }

    std::unique_ptr<eprosima::fastdds::BufferImplBase<T>> clone() const override
    {
        return std::make_unique<NonCpuBufferImpl<T>>(size_);
    }

private:

    size_t size_;
};

#endif  // NON_CPU_BUFFER_IMPL_HPP_
