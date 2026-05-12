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

#ifndef ROSIDL_BUFFER__BUFFER_IMPL_BASE_HPP_
#define ROSIDL_BUFFER__BUFFER_IMPL_BASE_HPP_

#include <cstddef>
#include <memory>
#include <string>

namespace rosidl
{

/// Abstract base class for all buffer implementations (CPU, CUDA, ROCm, etc.).
///
/// This base keeps only what the Buffer<T> pimpl and the serialization layer
/// need.  Backend-specific APIs (element access, resize, iterators,
/// descriptor serialization, etc.) are the responsibility of each concrete
/// implementation or the BufferBackend plugin; the CPU path goes through
/// CpuBufferImpl directly.
template<typename T>
class BufferImplBase
{
public:
  virtual ~BufferImplBase() = default;

  /// Get the backend type identifier (e.g., "cpu", "cuda", "demo").
  /// Each concrete implementation returns its own fixed identifier.
  virtual std::string get_backend_type() const = 0;

  /// Get the number of elements in the buffer.
  /// Required by the serialization layer for all backends.
  virtual size_t size() const = 0;

  /// Create a CPU copy of this buffer.
  /// If already on CPU, may return a copy or the same instance.
  /// @return New BufferImplBase instance on CPU
  virtual std::unique_ptr<BufferImplBase<T>> to_cpu() const = 0;

  /// Create a deep copy of this buffer.
  /// @return New BufferImplBase instance with copied data
  virtual std::unique_ptr<BufferImplBase<T>> clone() const = 0;
};

}  // namespace rosidl

#endif  // ROSIDL_BUFFER__BUFFER_IMPL_BASE_HPP_
