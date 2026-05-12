// Copyright 2026 Open Source Robotics Foundation, Inc.
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

#ifndef ROSIDL_BUFFER__CPU_BUFFER_IMPL_HPP_
#define ROSIDL_BUFFER__CPU_BUFFER_IMPL_HPP_

#include <memory>
#include <string>
#include <vector>

#include "rosidl_buffer/buffer_impl_base.hpp"

namespace rosidl
{

/// CPU buffer implementation wrapping std::vector.
/// Provides the reference implementation for CPU memory buffers.
template<typename T, typename Allocator = std::allocator<T>>
class CpuBufferImpl : public BufferImplBase<T>
{
public:
  CpuBufferImpl() = default;

  /// Get mutable reference to underlying std::vector.
  std::vector<T, Allocator> & get_storage() {return storage_;}

  /// Get const reference to underlying std::vector.
  const std::vector<T, Allocator> & get_storage() const {return storage_;}

  // ========== BufferImplBase overrides ==========

  std::string get_backend_type() const override {return "cpu";}

  size_t size() const override {return storage_.size();}

  std::unique_ptr<BufferImplBase<T>> to_cpu() const override
  {
    return clone();
  }

  std::unique_ptr<BufferImplBase<T>> clone() const override
  {
    auto copy = std::make_unique<CpuBufferImpl<T, Allocator>>();
    copy->storage_ = storage_;
    return copy;
  }

private:
  std::vector<T, Allocator> storage_;
};

}  // namespace rosidl

#endif  // ROSIDL_BUFFER__CPU_BUFFER_IMPL_HPP_
