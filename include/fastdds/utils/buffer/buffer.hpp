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

#ifndef ROSIDL_BUFFER__BUFFER_HPP_
#define ROSIDL_BUFFER__BUFFER_HPP_

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "rosidl_buffer/buffer_impl_base.hpp"
#include "rosidl_buffer/cpu_buffer_impl.hpp"

namespace rosidl
{

/// Buffer<T> provides a drop-in replacement for std::vector<T> with support
/// for vendor-specific memory backends (CPU, GPU, etc.).
///
/// For CPU backends, it provides implicit conversion to std::vector<T>&
/// for seamless backward compatibility. For non-CPU backends, direct access
/// throws exceptions, requiring explicit conversion via to_vector().
template<typename T, typename Allocator = std::allocator<T>>
class Buffer
{
public:
  // Type aliases for std::vector compatibility
  using value_type = T;
  using allocator_type = Allocator;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using reference = T &;
  using const_reference = const T &;
  using pointer = typename std::vector<T, Allocator>::pointer;
  using const_pointer = typename std::vector<T, Allocator>::const_pointer;
  using iterator = typename std::vector<T, Allocator>::iterator;
  using const_iterator = typename std::vector<T, Allocator>::const_iterator;
  using reverse_iterator = typename std::vector<T, Allocator>::reverse_iterator;
  using const_reverse_iterator = typename std::vector<T, Allocator>::const_reverse_iterator;

  /// Default constructor creates CPU buffer
  Buffer()
  {
    set_impl(std::make_unique<CpuBufferImpl<T, Allocator>>());
  }

  /// Construct with a backend implementation (set once at construction).
  /// This is the only way to set a non-CPU backend; there is no post-
  /// construction setter, which avoids race conditions with concurrent reads.
  explicit Buffer(std::unique_ptr<BufferImplBase<T>> impl)
  {
    if (!impl) {
      throw std::invalid_argument("Buffer implementation must not be null");
    }
    set_impl(std::move(impl));
  }

  /// Construct with initial size (CPU backend)
  explicit Buffer(size_t count)
  {
    set_impl(std::make_unique<CpuBufferImpl<T, Allocator>>());
    cpu_impl_->get_storage().resize(count);
  }

  /// Construct with initial size and value (CPU backend)
  Buffer(size_t count, const T & value)
  {
    set_impl(std::make_unique<CpuBufferImpl<T, Allocator>>());
    cpu_impl_->get_storage().assign(count, value);
  }

  /// Construct from std::vector (copy) - for backward compatibility
  Buffer(const std::vector<T, Allocator> & vec)  // NOLINT(runtime/explicit)
  {
    set_impl(std::make_unique<CpuBufferImpl<T, Allocator>>());
    cpu_impl_->get_storage() = vec;
  }

  /// Construct from std::vector (move) - for backward compatibility
  Buffer(std::vector<T, Allocator> && vec)  // NOLINT(runtime/explicit) - intentionally implicit
  {
    set_impl(std::make_unique<CpuBufferImpl<T, Allocator>>());
    cpu_impl_->get_storage() = std::move(vec);
  }

  /// Construct from initializer list - for backward compatibility
  Buffer(std::initializer_list<T> init)
  {
    set_impl(std::make_unique<CpuBufferImpl<T, Allocator>>());
    cpu_impl_->get_storage() = init;
  }

  /// Copy constructor (deep copy via clone())
  Buffer(const Buffer & other)
  {
    set_impl(other.impl_->clone());
  }

  /// Move constructor — the moved-from buffer is left in a valid, empty state.
  Buffer(Buffer && other) noexcept
  {
    set_impl(std::move(other.impl_));
    other.set_impl(std::make_unique<CpuBufferImpl<T, Allocator>>());
  }

  /// Copy assignment (deep copy via clone())
  Buffer & operator=(const Buffer & other)
  {
    if (this != &other) {
      set_impl(other.impl_->clone());
    }
    return *this;
  }

  /// Move assignment — the moved-from buffer is left in a valid, empty state.
  Buffer & operator=(Buffer && other) noexcept
  {
    if (this != &other) {
      set_impl(std::move(other.impl_));
      other.set_impl(std::make_unique<CpuBufferImpl<T, Allocator>>());
    }
    return *this;
  }

  /// Assignment from initializer list - for backward compatibility
  /// This must come before vector assignment to resolve ambiguity with {{...}} syntax
  Buffer & operator=(std::initializer_list<T> init)
  {
    throw_if_not_cpu_backend();
    cpu_impl_->get_storage() = init;
    return *this;
  }

  /// Assignment from std::vector (copy) - for backward compatibility
  /// Uses SFINAE to avoid ambiguity with initializer lists
  template<typename U = std::vector<T, Allocator>,
    typename std::enable_if<std::is_same<U, std::vector<T, Allocator>>::value, int>::type = 0>
  Buffer & operator=(const U & vec)
  {
    throw_if_not_cpu_backend();
    cpu_impl_->get_storage() = vec;
    return *this;
  }

  /// Assignment from std::vector (move) - for backward compatibility
  /// Uses SFINAE to avoid ambiguity with initializer lists
  template<typename U = std::vector<T, Allocator>,
    typename std::enable_if<std::is_same<U, std::vector<T, Allocator>>::value, int>::type = 0>
  Buffer & operator=(U && vec)
  {
    throw_if_not_cpu_backend();
    cpu_impl_->get_storage() = std::move(vec);
    return *this;
  }

  // ========== Element Access (CPU only) ==========

  /// Access element at position (CPU only)
  reference operator[](size_t pos)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage()[pos];
  }

  const_reference operator[](size_t pos) const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage()[pos];
  }

  /// Access element with bounds checking (CPU only)
  reference at(size_t pos)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().at(pos);
  }

  const_reference at(size_t pos) const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().at(pos);
  }

  /// Access first element (CPU only)
  reference front()
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().front();
  }

  const_reference front() const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().front();
  }

  /// Access last element (CPU only)
  reference back()
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().back();
  }

  const_reference back() const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().back();
  }

  /// Get pointer to data (CPU only)
  pointer data()
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().data();
  }

  const_pointer data() const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().data();
  }

  // ========== Iterators (CPU only) ==========

  iterator begin()
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().begin();
  }

  const_iterator begin() const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().begin();
  }

  const_iterator cbegin() const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().cbegin();
  }

  iterator end()
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().end();
  }

  const_iterator end() const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().end();
  }

  const_iterator cend() const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().cend();
  }

  reverse_iterator rbegin()
  {
    return reverse_iterator(end());
  }

  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator crbegin() const
  {
    return const_reverse_iterator(cend());
  }

  reverse_iterator rend()
  {
    return reverse_iterator(begin());
  }

  const_reverse_iterator rend() const
  {
    return const_reverse_iterator(begin());
  }

  const_reverse_iterator crend() const
  {
    return const_reverse_iterator(cbegin());
  }

  // ========== Capacity ==========

  /// Works for all backends (delegates to BufferImplBase::size()).
  bool empty() const {return impl_->size() == 0;}

  /// Works for all backends (delegates to BufferImplBase::size()).
  size_t size() const {return impl_->size();}

  size_t max_size() const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().max_size();
  }

  void reserve(size_t new_cap)
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().reserve(new_cap);
  }

  size_t capacity() const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().capacity();
  }

  void shrink_to_fit()
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().shrink_to_fit();
  }

  allocator_type get_allocator() const
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().get_allocator();
  }

  // ========== Modifiers (CPU only) ==========

  void assign(size_t count, const T & value)
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().assign(count, value);
  }

  template<typename InputIt>
  void assign(InputIt first, InputIt last)
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().assign(first, last);
  }

  void assign(std::initializer_list<T> ilist)
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().assign(ilist);
  }

  iterator insert(const_iterator pos, const T & value)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().insert(pos, value);
  }

  iterator insert(const_iterator pos, T && value)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().insert(pos, std::move(value));
  }

  iterator insert(const_iterator pos, size_t count, const T & value)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().insert(pos, count, value);
  }

  template<typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().insert(pos, first, last);
  }

  iterator insert(const_iterator pos, std::initializer_list<T> ilist)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().insert(pos, ilist);
  }

  void clear()
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().clear();
  }

  void resize(size_t n)
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().resize(n);
  }

  void resize(size_t n, const T & value)
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().resize(n, value);
  }

  void push_back(const T & value)
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().push_back(value);
  }

  void push_back(T && value)
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().push_back(std::move(value));
  }

  void pop_back()
  {
    throw_if_not_cpu_backend();
    get_cpu_impl()->get_storage().pop_back();
  }

  template<typename ... Args>
  iterator emplace(const_iterator pos, Args && ... args)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().emplace(pos, std::forward<Args>(args)...);
  }

  template<typename ... Args>
  reference emplace_back(Args && ... args)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().emplace_back(std::forward<Args>(args)...);
  }

  iterator erase(const_iterator pos)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().erase(pos);
  }

  iterator erase(const_iterator first, const_iterator last)
  {
    throw_if_not_cpu_backend();
    return get_cpu_impl()->get_storage().erase(first, last);
  }

  void swap(Buffer & other) noexcept
  {
    throw_if_not_cpu_backend();
    other.throw_if_not_cpu_backend();
    cpu_impl_->get_storage().swap(other.cpu_impl_->get_storage());
  }

  void swap(std::vector<T, Allocator> & vec) noexcept
  {
    throw_if_not_cpu_backend();
    cpu_impl_->get_storage().swap(vec);
  }

  // ========== Conversion Operators ==========

  /// Implicit conversion to std::vector<T, Allocator>& (CPU only).
  /// Provides backward compatibility with existing code.
  /// @throws std::runtime_error if backend is not CPU.
  operator std::vector<T, Allocator> &()
  {
    throw_if_not_cpu_backend();
    return cpu_impl_->get_storage();
  }

  operator const std::vector<T, Allocator> &() const
  {
    throw_if_not_cpu_backend();
    return cpu_impl_->get_storage();
  }

  /// Escape hatch: Explicit conversion to std::vector<T, Allocator> (all backends).
  /// For non-CPU backends, this triggers a copy to CPU memory.
  /// @return A std::vector containing a copy of the buffer data.
  std::vector<T, Allocator> to_vector() const
  {
    if (cpu_impl_) {
      return cpu_impl_->get_storage();
    }
    auto cpu_copy = impl_->to_cpu();
    auto * cpu = static_cast<CpuBufferImpl<T> *>(cpu_copy.get());
    if constexpr (std::is_same_v<Allocator, std::allocator<T>>) {
      return std::move(cpu->get_storage());
    } else {
      auto & src = cpu->get_storage();
      return std::vector<T, Allocator>(src.begin(), src.end());
    }
  }

  // ========== Comparison Operators ==========

  bool operator==(const Buffer & other) const
  {
    if (size() != other.size()) {
      return false;
    }
    if (cpu_impl_ && other.cpu_impl_) {
      return cpu_impl_->get_storage() == other.cpu_impl_->get_storage();
    }
    return to_vector() == other.to_vector();
  }

  bool operator!=(const Buffer & other) const
  {
    return !(*this == other);
  }

  // ========== Backend Management ==========

  /// Get the backend type identifier (e.g., "cpu", "cuda").
  /// Delegates to the underlying implementation — the impl is the single
  /// source of truth for its own backend type.
  std::string get_backend_type() const
  {
    return impl_->get_backend_type();
  }

  /// Get the implementation pointer (read-only).
  const BufferImplBase<T> * get_impl() const {return impl_.get();}

  /// Get the implementation pointer (mutable, e.g. for descriptor creation).
  BufferImplBase<T> * get_impl() {return impl_.get();}

  /// Throw exception if not CPU backend.
  /// @throws std::runtime_error if backend is not CPU.
  void throw_if_not_cpu_backend() const
  {
    if (!cpu_impl_) {
      throw std::runtime_error(
              "Operation requires CPU backend. Current backend: " +
              impl_->get_backend_type() +
              ". Use to_vector() for explicit conversion to CPU.");
    }
  }

private:
  /// Unique pointer for proper ownership and value semantics.
  /// The implementation is the sole source of truth for the backend type.
  std::unique_ptr<BufferImplBase<T>> impl_;

  /// Cached pointer for type-safe CPU backend detection.
  /// Set by set_impl() via dynamic_cast; null for non-CPU backends.
  CpuBufferImpl<T, Allocator> * cpu_impl_ = nullptr;

  /// Set the implementation and update the cached CPU pointer.
  void set_impl(std::unique_ptr<BufferImplBase<T>> impl)
  {
    impl_ = std::move(impl);
    cpu_impl_ = dynamic_cast<CpuBufferImpl<T, Allocator> *>(impl_.get());
  }

  /// Get CPU implementation (assumes throw_if_not_cpu_backend() was called)
  CpuBufferImpl<T, Allocator> * get_cpu_impl() const
  {
    return cpu_impl_;
  }
};

/// Free-function comparison: std::vector<T> == Buffer<T>
/// Needed because template argument deduction does not consider implicit conversions.
template<typename T, typename Allocator>
bool operator==(const std::vector<T, Allocator> & lhs, const Buffer<T, Allocator> & rhs)
{
  if (rhs.get_backend_type() == "cpu") {
    return lhs == static_cast<const std::vector<T, Allocator> &>(rhs);
  }
  return lhs == rhs.to_vector();
}

template<typename T, typename Allocator>
bool operator==(const Buffer<T, Allocator> & lhs, const std::vector<T, Allocator> & rhs)
{
  return rhs == lhs;
}

template<typename T, typename Allocator>
bool operator!=(const std::vector<T, Allocator> & lhs, const Buffer<T, Allocator> & rhs)
{
  return !(lhs == rhs);
}

template<typename T, typename Allocator>
bool operator!=(const Buffer<T, Allocator> & lhs, const std::vector<T, Allocator> & rhs)
{
  return !(lhs == rhs);
}

template<typename T, typename Allocator>
void swap(Buffer<T, Allocator> & lhs, Buffer<T, Allocator> & rhs) noexcept
{
  lhs.swap(rhs);
}

template<typename T, typename Allocator>
void swap(Buffer<T, Allocator> & lhs, std::vector<T, Allocator> & rhs) noexcept
{
  lhs.swap(rhs);
}

template<typename T, typename Allocator>
void swap(std::vector<T, Allocator> & lhs, Buffer<T, Allocator> & rhs) noexcept
{
  rhs.swap(lhs);
}

}  // namespace rosidl

#endif  // ROSIDL_BUFFER__BUFFER_HPP_
