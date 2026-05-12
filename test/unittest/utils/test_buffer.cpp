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

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <fastdds/utils/buffer/buffer.hpp>
#include <fastdds/utils/buffer/buffer_impl_base.hpp>
#include <fastdds/utils/buffer/cpu_buffer_impl.hpp>

#include "non_cpu_buffer_impl.hpp"

using eprosima::fastdds::Buffer;
using eprosima::fastdds::BufferImplBase;
using eprosima::fastdds::CpuBufferImpl;

// Test default construction
TEST(TestBuffer, default_construction) {
  Buffer<uint8_t> buffer;
  EXPECT_EQ(0u, buffer.size());
  EXPECT_TRUE(buffer.empty());
  EXPECT_EQ("cpu", buffer.get_backend_type());
}

// Test construction with size
TEST(TestBuffer, sized_construction) {
  Buffer<uint8_t> buffer(10);
  EXPECT_EQ(10u, buffer.size());
  EXPECT_FALSE(buffer.empty());
  EXPECT_EQ("cpu", buffer.get_backend_type());
}

// Test construction with size and value
TEST(TestBuffer, sized_value_construction) {
  Buffer<uint8_t> buffer(5, 42);
  EXPECT_EQ(5u, buffer.size());
  for (size_t i = 0; i < buffer.size(); ++i) {
    EXPECT_EQ(42, buffer[i]);
  }
}

// Test copy construction
TEST(TestBuffer, copy_construction) {
  Buffer<uint8_t> buffer1(3, 100);
  Buffer<uint8_t> buffer2(buffer1);

  EXPECT_EQ(buffer1.size(), buffer2.size());
  EXPECT_EQ(buffer1.get_backend_type(), buffer2.get_backend_type());
  EXPECT_NE(buffer1.data(), buffer2.data());
  for (size_t i = 0; i < buffer1.size(); ++i) {
    EXPECT_EQ(buffer1[i], buffer2[i]);
    EXPECT_NE(&buffer1[i], &buffer2[i]);
  }
}

// Test move construction — moved-from buffer must be valid and empty
TEST(TestBuffer, move_construction) {
  Buffer<uint8_t> buffer1(3, 100);
  Buffer<uint8_t> buffer2(std::move(buffer1));

  EXPECT_EQ(3u, buffer2.size());
  EXPECT_EQ("cpu", buffer2.get_backend_type());

  // Moved-from buffer is in a valid, empty state (not null)
  EXPECT_EQ(0u, buffer1.size());
  EXPECT_TRUE(buffer1.empty());
  EXPECT_EQ("cpu", buffer1.get_backend_type());
  // Must not crash — moved-from buffer is usable
  buffer1.push_back(42);
  EXPECT_EQ(1u, buffer1.size());
  EXPECT_EQ(42, buffer1[0]);
}

// Test copy assignment
TEST(TestBuffer, copy_assignment) {
  Buffer<uint8_t> buffer1(3, 100);
  Buffer<uint8_t> buffer2;
  buffer2 = buffer1;

  EXPECT_EQ(buffer1.size(), buffer2.size());
  EXPECT_NE(buffer1.data(), buffer2.data());
  for (size_t i = 0; i < buffer1.size(); ++i) {
    EXPECT_EQ(buffer1[i], buffer2[i]);
    EXPECT_NE(&buffer1[i], &buffer2[i]);
  }
}

// Test move assignment — moved-from buffer must be valid and empty
TEST(TestBuffer, move_assignment) {
  Buffer<uint8_t> buffer1(3, 100);
  Buffer<uint8_t> buffer2;
  buffer2 = std::move(buffer1);

  EXPECT_EQ(3u, buffer2.size());

  // Moved-from buffer is in a valid, empty state (not null)
  EXPECT_EQ(0u, buffer1.size());
  EXPECT_TRUE(buffer1.empty());
  EXPECT_EQ("cpu", buffer1.get_backend_type());
  buffer1.push_back(7);
  EXPECT_EQ(1u, buffer1.size());
  EXPECT_EQ(7, buffer1[0]);
}

// Test element access via operator[]
TEST(TestBuffer, element_access) {
  Buffer<uint8_t> buffer(5);
  for (size_t i = 0; i < buffer.size(); ++i) {
    buffer[i] = static_cast<uint8_t>(i * 10);
  }

  for (size_t i = 0; i < buffer.size(); ++i) {
    EXPECT_EQ(static_cast<uint8_t>(i * 10), buffer[i]);
  }
}

// Test const element access
TEST(TestBuffer, const_element_access) {
  Buffer<uint8_t> buffer(3, 42);
  const Buffer<uint8_t> & const_buffer = buffer;

  EXPECT_EQ(42, const_buffer[0]);
  EXPECT_EQ(42, const_buffer[1]);
  EXPECT_EQ(42, const_buffer[2]);
}

// Test at() with bounds checking
TEST(TestBuffer, at_access) {
  Buffer<uint8_t> buffer(3, 99);
  EXPECT_EQ(99, buffer.at(0));
  EXPECT_EQ(99, buffer.at(2));
  EXPECT_THROW(buffer.at(3), std::out_of_range);
}

// Test front() and back()
TEST(TestBuffer, front_back_access) {
  Buffer<uint8_t> buffer;
  buffer.push_back(10);
  buffer.push_back(20);
  buffer.push_back(30);

  EXPECT_EQ(10, buffer.front());
  EXPECT_EQ(30, buffer.back());
}

// Test data() pointer access
TEST(TestBuffer, data_access) {
  Buffer<uint8_t> buffer(5, 7);
  uint8_t * ptr = buffer.data();
  ASSERT_NE(nullptr, ptr);
  EXPECT_EQ(7, ptr[0]);
  EXPECT_EQ(7, ptr[4]);

  ptr[2] = 100;
  EXPECT_EQ(100, buffer[2]);
}

// Test iterators
TEST(TestBuffer, iterators) {
  Buffer<uint8_t> buffer;
  buffer.push_back(1);
  buffer.push_back(2);
  buffer.push_back(3);

  int sum = 0;
  for (auto it = buffer.begin(); it != buffer.end(); ++it) {
    sum += *it;
  }
  EXPECT_EQ(6, sum);

  // Test const iterators
  const Buffer<uint8_t> & const_buffer = buffer;
  int const_sum = 0;
  for (auto it = const_buffer.begin(); it != const_buffer.end(); ++it) {
    const_sum += *it;
  }
  EXPECT_EQ(6, const_sum);
}

// Test reverse iterators
TEST(TestBuffer, reverse_iterators) {
  Buffer<uint8_t> buffer{1, 2, 3, 4, 5};

  std::vector<uint8_t> reversed;
  for (auto it = buffer.rbegin(); it != buffer.rend(); ++it) {
    reversed.push_back(*it);
  }
  ASSERT_EQ(5u, reversed.size());
  EXPECT_EQ(5, reversed[0]);
  EXPECT_EQ(4, reversed[1]);
  EXPECT_EQ(3, reversed[2]);
  EXPECT_EQ(2, reversed[3]);
  EXPECT_EQ(1, reversed[4]);

  // rbegin/rend are mutable
  *buffer.rbegin() = 99;
  EXPECT_EQ(99, buffer.back());
}

// Test const reverse iterators via crbegin/crend
TEST(TestBuffer, const_reverse_iterators) {
  Buffer<uint8_t> buffer{10, 20, 30};
  const Buffer<uint8_t> & cbuffer = buffer;

  std::vector<uint8_t> reversed;
  for (auto it = cbuffer.rbegin(); it != cbuffer.rend(); ++it) {
    reversed.push_back(*it);
  }
  EXPECT_EQ(30, reversed[0]);
  EXPECT_EQ(20, reversed[1]);
  EXPECT_EQ(10, reversed[2]);

  // crbegin/crend
  reversed.clear();
  for (auto it = buffer.crbegin(); it != buffer.crend(); ++it) {
    reversed.push_back(*it);
  }
  EXPECT_EQ(30, reversed[0]);
  EXPECT_EQ(20, reversed[1]);
  EXPECT_EQ(10, reversed[2]);

  // Type check: crbegin returns const_reverse_iterator
  static_assert(
    std::is_same_v<
      decltype(buffer.crbegin()),
      Buffer<uint8_t>::const_reverse_iterator>,
    "crbegin must return const_reverse_iterator");
  static_assert(
    std::is_same_v<
      decltype(buffer.crend()),
      Buffer<uint8_t>::const_reverse_iterator>,
    "crend must return const_reverse_iterator");
}

// Test reverse iterators work with std::reverse
TEST(TestBuffer, reverse_iterators_with_std_reverse) {
  Buffer<uint8_t> buffer{1, 2, 3, 4, 5};
  std::reverse(buffer.begin(), buffer.end());
  EXPECT_EQ(5, buffer[0]);
  EXPECT_EQ(4, buffer[1]);
  EXPECT_EQ(3, buffer[2]);
  EXPECT_EQ(2, buffer[3]);
  EXPECT_EQ(1, buffer[4]);
}

// Test resize
TEST(TestBuffer, resize) {
  Buffer<uint8_t> buffer(5, 10);
  EXPECT_EQ(5u, buffer.size());

  buffer.resize(10);
  EXPECT_EQ(10u, buffer.size());
  EXPECT_EQ(10, buffer[4]);

  buffer.resize(3);
  EXPECT_EQ(3u, buffer.size());
}

// Test resize with value
TEST(TestBuffer, resize_with_value) {
  Buffer<uint8_t> buffer(2, 5);
  buffer.resize(5, 99);

  EXPECT_EQ(5u, buffer.size());
  EXPECT_EQ(5, buffer[0]);
  EXPECT_EQ(5, buffer[1]);
  EXPECT_EQ(99, buffer[2]);
  EXPECT_EQ(99, buffer[4]);
}

// Test push_back
TEST(TestBuffer, push_back) {
  Buffer<uint8_t> buffer;
  EXPECT_TRUE(buffer.empty());

  buffer.push_back(10);
  EXPECT_EQ(1u, buffer.size());
  EXPECT_EQ(10, buffer[0]);

  buffer.push_back(20);
  EXPECT_EQ(2u, buffer.size());
  EXPECT_EQ(20, buffer[1]);
}

// Test push_back with rvalue
TEST(TestBuffer, push_back_rvalue) {
  Buffer<uint8_t> buffer;
  uint8_t val = 0xAB;
  buffer.push_back(std::move(val));

  EXPECT_EQ(1u, buffer.size());
  EXPECT_EQ(0xAB, buffer[0]);
}

// Test pop_back
TEST(TestBuffer, pop_back) {
  Buffer<uint8_t> buffer;
  buffer.push_back(1);
  buffer.push_back(2);
  buffer.push_back(3);

  EXPECT_EQ(3u, buffer.size());
  buffer.pop_back();
  EXPECT_EQ(2u, buffer.size());
  EXPECT_EQ(2, buffer.back());
}

// Test emplace_back
TEST(TestBuffer, emplace_back) {
  Buffer<uint8_t> buffer;
  buffer.emplace_back(0xFF);
  buffer.emplace_back(0x00);

  EXPECT_EQ(2u, buffer.size());
  EXPECT_EQ(0xFF, buffer[0]);
  EXPECT_EQ(0x00, buffer[1]);
}

#if __cplusplus >= 201703L
// Test emplace_back returns reference
TEST(TestBuffer, emplace_back_returns_reference) {
  Buffer<std::string> buffer;
  std::string & ref = buffer.emplace_back("hello");
  EXPECT_EQ("hello", ref);
  EXPECT_EQ(&ref, &buffer.back());

  ref = "modified";
  EXPECT_EQ("modified", buffer.back());

  static_assert(
    std::is_same_v<
      decltype(buffer.emplace_back("x")),
      std::string &>,
    "emplace_back must return reference");
}
#endif

// Test positional emplace at begin/middle/end
TEST(TestBuffer, emplace_positional) {
  Buffer<std::string> buffer;
  buffer.push_back("b");
  buffer.push_back("d");

  auto it_begin = buffer.emplace(buffer.begin(), "a");
  EXPECT_EQ("a", *it_begin);
  EXPECT_EQ(buffer.begin(), it_begin);

  auto it_mid = buffer.emplace(buffer.begin() + 2, "c");
  EXPECT_EQ("c", *it_mid);

  auto it_end = buffer.emplace(buffer.end(), "e");
  EXPECT_EQ("e", *it_end);

  ASSERT_EQ(5u, buffer.size());
  EXPECT_EQ("a", buffer[0]);
  EXPECT_EQ("b", buffer[1]);
  EXPECT_EQ("c", buffer[2]);
  EXPECT_EQ("d", buffer[3]);
  EXPECT_EQ("e", buffer[4]);
}

// Test erase single element — beginning, middle, end
TEST(TestBuffer, erase_single) {
  Buffer<uint8_t> buffer{1, 2, 3, 4, 5};

  // Erase middle: returns iterator to the element that followed it.
  auto it = buffer.erase(buffer.begin() + 2);
  ASSERT_EQ(4u, buffer.size());
  EXPECT_EQ(4, *it);
  EXPECT_EQ(1, buffer[0]);
  EXPECT_EQ(2, buffer[1]);
  EXPECT_EQ(4, buffer[2]);
  EXPECT_EQ(5, buffer[3]);

  // Erase beginning
  it = buffer.erase(buffer.begin());
  EXPECT_EQ(2, *it);
  EXPECT_EQ(3u, buffer.size());
  EXPECT_EQ(2, buffer[0]);

  // Erase last — returned iterator equals end()
  it = buffer.erase(buffer.end() - 1);
  EXPECT_EQ(buffer.end(), it);
  EXPECT_EQ(2u, buffer.size());
}

// Test erase range
TEST(TestBuffer, erase_range) {
  Buffer<uint8_t> buffer{1, 2, 3, 4, 5, 6};

  // Erase middle range [2, 4) -> removes 3, 4
  auto it = buffer.erase(buffer.begin() + 2, buffer.begin() + 4);
  ASSERT_EQ(4u, buffer.size());
  EXPECT_EQ(5, *it);
  EXPECT_EQ(1, buffer[0]);
  EXPECT_EQ(2, buffer[1]);
  EXPECT_EQ(5, buffer[2]);
  EXPECT_EQ(6, buffer[3]);

  // Empty range is a no-op
  auto it2 = buffer.erase(buffer.begin() + 1, buffer.begin() + 1);
  EXPECT_EQ(4u, buffer.size());
  EXPECT_EQ(2, *it2);

  // Erase all
  auto it3 = buffer.erase(buffer.begin(), buffer.end());
  EXPECT_EQ(0u, buffer.size());
  EXPECT_EQ(buffer.end(), it3);
}

// Test assign with count and value
TEST(TestBuffer, assign_count_value) {
  Buffer<uint8_t> buffer;
  buffer.assign(5, 42);
  EXPECT_EQ(5u, buffer.size());
  for (size_t i = 0; i < buffer.size(); ++i) {
    EXPECT_EQ(42, buffer[i]);
  }

  buffer.assign(3, 7);
  EXPECT_EQ(3u, buffer.size());
  for (size_t i = 0; i < buffer.size(); ++i) {
    EXPECT_EQ(7, buffer[i]);
  }
}

// Test assign with iterator range
TEST(TestBuffer, assign_iterator_range) {
  std::vector<uint8_t> source = {10, 20, 30, 40, 50};
  Buffer<uint8_t> buffer;
  buffer.assign(source.begin(), source.end());

  EXPECT_EQ(5u, buffer.size());
  for (size_t i = 0; i < source.size(); ++i) {
    EXPECT_EQ(source[i], buffer[i]);
  }

  uint8_t arr[] = {1, 2, 3};
  buffer.assign(std::begin(arr), std::end(arr));
  EXPECT_EQ(3u, buffer.size());
  EXPECT_EQ(1, buffer[0]);
  EXPECT_EQ(2, buffer[1]);
  EXPECT_EQ(3, buffer[2]);
}

// Test assign with initializer list
TEST(TestBuffer, assign_initializer_list) {
  Buffer<uint8_t> buffer(10, 0);
  buffer.assign({5, 10, 15});

  EXPECT_EQ(3u, buffer.size());
  EXPECT_EQ(5, buffer[0]);
  EXPECT_EQ(10, buffer[1]);
  EXPECT_EQ(15, buffer[2]);
}

// Test clear
TEST(TestBuffer, clear) {
  Buffer<uint8_t> buffer(10, 42);
  EXPECT_EQ(10u, buffer.size());
  EXPECT_FALSE(buffer.empty());

  buffer.clear();
  EXPECT_EQ(0u, buffer.size());
  EXPECT_TRUE(buffer.empty());
}

// Test reserve and capacity (CPU backend only)
TEST(TestBuffer, reserve_capacity) {
  Buffer<uint8_t> buffer;
  buffer.reserve(100);

  EXPECT_EQ(0u, buffer.size());
  EXPECT_GE(buffer.capacity(), 100u);
}

// Test shrink_to_fit
TEST(TestBuffer, shrink_to_fit) {
  Buffer<uint8_t> buffer;
  buffer.reserve(100);
  buffer.push_back(1);
  buffer.push_back(2);

  EXPECT_GE(buffer.capacity(), 100u);
  buffer.shrink_to_fit();
  EXPECT_EQ(2u, buffer.size());
  EXPECT_LE(buffer.capacity(), 100u);
}

// Test member swap between two Buffers (CPU)
TEST(TestBuffer, swap_member_buffer_buffer) {
  Buffer<uint8_t> a{1, 2, 3};
  Buffer<uint8_t> b{10, 20};

  const uint8_t * a_data_before = a.data();
  const uint8_t * b_data_before = b.data();

  a.swap(b);

  EXPECT_EQ(2u, a.size());
  EXPECT_EQ(10, a[0]);
  EXPECT_EQ(20, a[1]);

  EXPECT_EQ(3u, b.size());
  EXPECT_EQ(1, b[0]);
  EXPECT_EQ(2, b[1]);
  EXPECT_EQ(3, b[2]);

  // std::vector::swap semantics: underlying storage pointers are swapped,
  // not the elements copied — so data pointers should swap identities.
  EXPECT_EQ(b_data_before, a.data());
  EXPECT_EQ(a_data_before, b.data());
}

// Test self-swap is a no-op
TEST(TestBuffer, swap_member_self) {
  Buffer<uint8_t> a{1, 2, 3};
  a.swap(a);
  EXPECT_EQ(3u, a.size());
  EXPECT_EQ(1, a[0]);
  EXPECT_EQ(2, a[1]);
  EXPECT_EQ(3, a[2]);
}

// Test member swap: Buffer with std::vector
TEST(TestBuffer, swap_member_buffer_vector) {
  Buffer<uint8_t> buf{1, 2, 3};
  std::vector<uint8_t> vec{100, 101};

  buf.swap(vec);

  EXPECT_EQ(2u, buf.size());
  EXPECT_EQ(100, buf[0]);
  EXPECT_EQ(101, buf[1]);

  EXPECT_EQ(3u, vec.size());
  EXPECT_EQ(1, vec[0]);
  EXPECT_EQ(2, vec[1]);
  EXPECT_EQ(3, vec[2]);
}

// Test vector::swap(Buffer) works via implicit conversion operator
TEST(TestBuffer, swap_vector_member_with_buffer) {
  Buffer<uint8_t> buf{1, 2, 3};
  std::vector<uint8_t> vec{100, 101};

  // std::vector::swap takes vector&, Buffer's operator std::vector<T,A>&()
  // provides the needed implicit conversion.
  vec.swap(buf);

  EXPECT_EQ(2u, buf.size());
  EXPECT_EQ(100, buf[0]);
  EXPECT_EQ(101, buf[1]);

  EXPECT_EQ(3u, vec.size());
  EXPECT_EQ(1, vec[0]);
  EXPECT_EQ(2, vec[1]);
  EXPECT_EQ(3, vec[2]);
}

// Test non-member swap resolved via ADL (the "using std::swap" idiom)
TEST(TestBuffer, swap_non_member_adl_buffer_buffer) {
  Buffer<uint8_t> a{1, 2, 3};
  Buffer<uint8_t> b{10, 20};

  using std::swap;
  swap(a, b);   // should resolve to rosidl::swap via ADL

  EXPECT_EQ(2u, a.size());
  EXPECT_EQ(10, a[0]);
  EXPECT_EQ(3u, b.size());
  EXPECT_EQ(1, b[0]);
}

// Test non-member ADL swap: Buffer <-> std::vector (both directions)
TEST(TestBuffer, swap_non_member_adl_mixed) {
  Buffer<uint8_t> buf{1, 2, 3};
  std::vector<uint8_t> vec{100, 101};

  using std::swap;
  swap(buf, vec);   // rosidl::swap(Buffer&, std::vector&)

  EXPECT_EQ(2u, buf.size());
  EXPECT_EQ(100, buf[0]);
  EXPECT_EQ(3u, vec.size());
  EXPECT_EQ(1, vec[0]);

  swap(vec, buf);   // rosidl::swap(std::vector&, Buffer&)

  EXPECT_EQ(3u, buf.size());
  EXPECT_EQ(1, buf[0]);
  EXPECT_EQ(2u, vec.size());
  EXPECT_EQ(100, vec[0]);
}

// Test that std::swap (qualified) still works on Buffers via move fallback.
// This does not use our specialized swap, but should still be correct
// because Buffer has a noexcept move ctor and move assignment.
TEST(TestBuffer, swap_qualified_std_swap) {
  Buffer<uint8_t> a{1, 2, 3};
  Buffer<uint8_t> b{10, 20};

  std::swap(a, b);

  EXPECT_EQ(2u, a.size());
  EXPECT_EQ(10, a[0]);
  EXPECT_EQ(3u, b.size());
  EXPECT_EQ(1, b[0]);
}

// Test std::is_nothrow_swappable_v reports true, matching std::vector (only in C++17 and later).
TEST(TestBuffer, swap_is_noexcept) {
#if __cplusplus < 201703L
    GTEST_SKIP() << "std::is_nothrow_swappable_v requires C++17 or later";
#else
  static_assert(
    std::is_nothrow_swappable_v<Buffer<uint8_t>>,
    "rosidl::Buffer should be nothrow-swappable to match std::vector");
  SUCCEED();
#endif
}

// Test swap used by generic STL algorithm (via ADL internally).
TEST(TestBuffer, swap_used_by_std_algorithm) {
  std::vector<Buffer<uint8_t>> buffers;
  buffers.emplace_back(std::initializer_list<uint8_t>{3});
  buffers.emplace_back(std::initializer_list<uint8_t>{1});
  buffers.emplace_back(std::initializer_list<uint8_t>{2});

  std::sort(
    buffers.begin(), buffers.end(),
    [](const Buffer<uint8_t> & lhs, const Buffer<uint8_t> & rhs) {
      return lhs[0] < rhs[0];
    });

  EXPECT_EQ(1, buffers[0][0]);
  EXPECT_EQ(2, buffers[1][0]);
  EXPECT_EQ(3, buffers[2][0]);
}

// ========== Group 3: operator<=> and comparisons ==========

// Test equality between Buffers
TEST(TestBuffer, equality_buffer_buffer) {
  Buffer<uint8_t> a{1, 2, 3};
  Buffer<uint8_t> b{1, 2, 3};
  Buffer<uint8_t> c{1, 2, 4};
  Buffer<uint8_t> d{1, 2};

  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
  EXPECT_NE(a, d);
}

// Test equality between Buffer and std::vector (both directions)
TEST(TestBuffer, equality_buffer_vector) {
  Buffer<uint8_t> buf{1, 2, 3};
  std::vector<uint8_t> vec_eq{1, 2, 3};
  std::vector<uint8_t> vec_neq{1, 2, 4};

  EXPECT_EQ(buf, vec_eq);
  EXPECT_EQ(vec_eq, buf);

  EXPECT_NE(buf, vec_neq);
  EXPECT_NE(vec_neq, buf);
}

// ========== Group 4: max_size, get_allocator ==========

TEST(TestBuffer, max_size) {
  Buffer<uint8_t> buffer;
  // max_size is implementation-defined but must be > 0 and >= size().
  EXPECT_GT(buffer.max_size(), 0u);
  buffer.resize(16);
  EXPECT_GE(buffer.max_size(), buffer.size());
}

TEST(TestBuffer, get_allocator) {
  Buffer<uint8_t> buffer;
  auto alloc = buffer.get_allocator();
  // std::allocator<uint8_t> instances always compare equal.
  EXPECT_EQ(alloc, std::allocator<uint8_t>{});

  static_assert(
    std::is_same_v<
      decltype(buffer.get_allocator()),
      Buffer<uint8_t>::allocator_type>,
    "get_allocator must return allocator_type");
}

// Test implicit conversion to std::vector&
TEST(TestBuffer, implicit_conversion_to_vector) {
  Buffer<uint8_t> buffer;
  buffer.push_back(1);
  buffer.push_back(2);
  buffer.push_back(3);

  std::vector<uint8_t> & vec_ref = buffer;
  EXPECT_EQ(3u, vec_ref.size());
  EXPECT_EQ(1, vec_ref[0]);
  EXPECT_EQ(2, vec_ref[1]);
  EXPECT_EQ(3, vec_ref[2]);

  vec_ref[1] = 99;
  EXPECT_EQ(99, buffer[1]);
}

// Test const implicit conversion
TEST(TestBuffer, const_implicit_conversion) {
  Buffer<uint8_t> buffer(3, 42);
  const Buffer<uint8_t> & const_buffer = buffer;

  const std::vector<uint8_t> & vec_ref = const_buffer;
  EXPECT_EQ(3u, vec_ref.size());
  EXPECT_EQ(42, vec_ref[0]);
}

// Test to_vector() escape hatch
TEST(TestBuffer, to_vector_escape_hatch) {
  Buffer<uint8_t> buffer;
  buffer.push_back(10);
  buffer.push_back(20);
  buffer.push_back(30);

  std::vector<uint8_t> copied = buffer.to_vector();
  EXPECT_EQ(buffer.size(), copied.size());
  EXPECT_EQ(10, copied[0]);
  EXPECT_EQ(20, copied[1]);
  EXPECT_EQ(30, copied[2]);

  copied[0] = 255;
  EXPECT_EQ(10, buffer[0]);
}

// Test backend type
TEST(TestBuffer, backend_type) {
  Buffer<uint8_t> buffer;
  EXPECT_EQ("cpu", buffer.get_backend_type());
}

// Test using Buffer with algorithms
TEST(TestBuffer, with_algorithms) {
  Buffer<uint8_t> buffer;
  for (uint8_t i = 0; i < 10; ++i) {
    buffer.push_back(i);
  }

  auto it = std::find(buffer.begin(), buffer.end(), 5);
  ASSERT_NE(buffer.end(), it);
  EXPECT_EQ(5, *it);

  buffer.push_back(5);
  auto count = std::count(buffer.begin(), buffer.end(), 5);
  EXPECT_EQ(2, count);

  Buffer<uint8_t> buffer2;
  buffer2.push_back(3);
  buffer2.push_back(1);
  buffer2.push_back(2);
  std::sort(buffer2.begin(), buffer2.end());
  EXPECT_EQ(1, buffer2[0]);
  EXPECT_EQ(2, buffer2[1]);
  EXPECT_EQ(3, buffer2[2]);
}

// Test CpuBufferImpl directly
TEST(TestCpuBufferImpl, basic_operations) {
  CpuBufferImpl<uint8_t> impl;
  EXPECT_EQ(0u, impl.get_storage().size());

  impl.get_storage().resize(5);
  EXPECT_EQ(5u, impl.get_storage().size());

  auto & storage = impl.get_storage();
  storage[0] = 100;
  EXPECT_EQ(100, storage[0]);

  impl.get_storage().clear();
  EXPECT_EQ(0u, impl.get_storage().size());
}

// Test CpuBufferImpl::to_cpu()
TEST(TestCpuBufferImpl, to_cpu) {
  CpuBufferImpl<uint8_t> impl;
  impl.get_storage().resize(3);
  auto & storage = impl.get_storage();
  storage[0] = 1;
  storage[1] = 2;
  storage[2] = 3;

  auto cpu_copy = impl.to_cpu();
  ASSERT_NE(nullptr, cpu_copy);

  auto * cpu_impl = static_cast<CpuBufferImpl<uint8_t> *>(cpu_copy.get());
  EXPECT_EQ(3u, cpu_impl->get_storage().size());
  EXPECT_EQ(1, cpu_impl->get_storage()[0]);
  EXPECT_EQ(2, cpu_impl->get_storage()[1]);
  EXPECT_EQ(3, cpu_impl->get_storage()[2]);
}

// Test CpuBufferImpl storage data pointer
TEST(TestCpuBufferImpl, storage_data_pointer) {
  CpuBufferImpl<uint8_t> impl;

  EXPECT_TRUE(impl.get_storage().empty());

  impl.get_storage().resize(5);
  const uint8_t * data_ptr = impl.get_storage().data();
  ASSERT_NE(nullptr, data_ptr);

  impl.get_storage()[0] = 42;
  EXPECT_EQ(42, data_ptr[0]);
}

// Test Buffer copy creates independent clone (deep copy via unique_ptr)
TEST(TestBuffer, deep_copy_semantics) {
  Buffer<uint8_t> buffer1;
  buffer1.push_back(10);
  buffer1.push_back(20);

  Buffer<uint8_t> buffer2 = buffer1;

  EXPECT_EQ(buffer1.size(), buffer2.size());
  EXPECT_EQ(buffer1[0], buffer2[0]);
  EXPECT_EQ(buffer1[1], buffer2[1]);

  buffer2[0] = 255;
  EXPECT_EQ(10, buffer1[0]);
  EXPECT_EQ(255, buffer2[0]);
}

// Test Buffer construction from a custom impl
TEST(TestBuffer, construct_from_impl) {
  auto custom_impl = std::make_unique<CpuBufferImpl<uint8_t>>();
  custom_impl->get_storage().resize(3);
  custom_impl->get_storage()[0] = 100;
  custom_impl->get_storage()[1] = 200;
  custom_impl->get_storage()[2] = 250;

  Buffer<uint8_t> buffer(std::move(custom_impl));

  EXPECT_EQ("cpu", buffer.get_backend_type());
  EXPECT_EQ(3u, buffer.size());
  EXPECT_EQ(100, buffer[0]);
  EXPECT_EQ(200, buffer[1]);
  EXPECT_EQ(250, buffer[2]);
}

// Test that constructing a Buffer with nullptr impl throws
TEST(TestBuffer, construct_from_null_impl_throws) {
  EXPECT_THROW(Buffer<uint8_t>(nullptr), std::invalid_argument);
}

// Test that Buffer works with non-trivial types (template generality)
TEST(TestBuffer, with_string) {
  Buffer<std::string> buffer;
  buffer.push_back("hello");
  buffer.push_back("world");

  EXPECT_EQ(2u, buffer.size());
  EXPECT_EQ("hello", buffer[0]);
  EXPECT_EQ("world", buffer[1]);

  std::vector<std::string> copied = buffer.to_vector();
  EXPECT_EQ("hello", copied[0]);
  EXPECT_EQ("world", copied[1]);
}

// Test Buffer with struct types
TEST(TestBuffer, with_struct)
{
  struct Point
  {
    double x;
    double y;
    double z;
  };

  Buffer<Point> buffer;
  buffer.push_back({1.0, 2.0, 3.0});
  buffer.push_back({4.0, 5.0, 6.0});

  EXPECT_EQ(2u, buffer.size());
  EXPECT_DOUBLE_EQ(1.0, buffer[0].x);
  EXPECT_DOUBLE_EQ(5.0, buffer[1].y);

  std::vector<Point> copied = buffer.to_vector();
  EXPECT_DOUBLE_EQ(3.0, copied[0].z);
}

TEST(TestBufferNonCpu, backend_type) {
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> buffer(std::move(impl));

  EXPECT_EQ("non_cpu_test", buffer.get_backend_type());
  EXPECT_EQ(4u, buffer.size());
  EXPECT_FALSE(buffer.empty());
}

TEST(TestBufferNonCpu, element_access_throws) {
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> buffer(std::move(impl));

  EXPECT_THROW(buffer[0], std::runtime_error);
  EXPECT_THROW(buffer.at(0), std::runtime_error);
  EXPECT_THROW(buffer.front(), std::runtime_error);
  EXPECT_THROW(buffer.back(), std::runtime_error);
  EXPECT_THROW(buffer.data(), std::runtime_error);
}

TEST(TestBufferNonCpu, const_element_access_throws) {
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  const Buffer<uint8_t> buffer(std::move(impl));

  EXPECT_THROW(buffer[0], std::runtime_error);
  EXPECT_THROW(buffer.at(0), std::runtime_error);
  EXPECT_THROW(buffer.front(), std::runtime_error);
  EXPECT_THROW(buffer.back(), std::runtime_error);
  EXPECT_THROW(buffer.data(), std::runtime_error);
}

TEST(TestBufferNonCpu, iterators_throw) {
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> buffer(std::move(impl));

  EXPECT_THROW(buffer.begin(), std::runtime_error);
  EXPECT_THROW(buffer.end(), std::runtime_error);
  EXPECT_THROW(buffer.cbegin(), std::runtime_error);
  EXPECT_THROW(buffer.cend(), std::runtime_error);
}

TEST(TestBufferNonCpu, modifiers_throw) {
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> buffer(std::move(impl));

  EXPECT_THROW(buffer.assign(3, 0), std::runtime_error);
  EXPECT_THROW(buffer.assign({1, 2}), std::runtime_error);
  std::vector<uint8_t> v = {1};
  EXPECT_THROW(buffer.assign(v.begin(), v.end()), std::runtime_error);
  EXPECT_THROW(buffer.resize(10), std::runtime_error);
  EXPECT_THROW(buffer.resize(10, 0), std::runtime_error);
  EXPECT_THROW(buffer.clear(), std::runtime_error);
  EXPECT_THROW(buffer.push_back(1), std::runtime_error);
  EXPECT_THROW(buffer.pop_back(), std::runtime_error);
  EXPECT_THROW(buffer.emplace_back(1), std::runtime_error);

  using ConstIt = Buffer<uint8_t>::const_iterator;
  EXPECT_THROW(buffer.erase(ConstIt{}), std::runtime_error);
  EXPECT_THROW(buffer.erase(ConstIt{}, ConstIt{}), std::runtime_error);
  EXPECT_THROW(buffer.emplace(ConstIt{}, 1), std::runtime_error);
}

TEST(TestBufferNonCpu, capacity_throws) {
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> buffer(std::move(impl));

  EXPECT_THROW(buffer.reserve(10), std::runtime_error);
  EXPECT_THROW(buffer.capacity(), std::runtime_error);
  EXPECT_THROW(buffer.shrink_to_fit(), std::runtime_error);
  EXPECT_THROW(buffer.max_size(), std::runtime_error);
  EXPECT_THROW(buffer.get_allocator(), std::runtime_error);
}

TEST(TestBufferNonCpu, reverse_iterators_throw) {
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> buffer(std::move(impl));

  EXPECT_THROW(buffer.rbegin(), std::runtime_error);
  EXPECT_THROW(buffer.rend(), std::runtime_error);
  EXPECT_THROW(buffer.crbegin(), std::runtime_error);
  EXPECT_THROW(buffer.crend(), std::runtime_error);

  auto impl2 = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  const Buffer<uint8_t> cbuffer(std::move(impl2));
  EXPECT_THROW(cbuffer.rbegin(), std::runtime_error);
  EXPECT_THROW(cbuffer.rend(), std::runtime_error);
}

TEST(TestBufferNonCpu, implicit_conversion_throws) {
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> buffer(std::move(impl));

  EXPECT_THROW(
    {std::vector<uint8_t> & ref = buffer; (void)ref;},
    std::runtime_error);

  auto impl2 = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  const Buffer<uint8_t> cbuffer(std::move(impl2));

  EXPECT_THROW(
    {const std::vector<uint8_t> & ref = cbuffer; (void)ref;},
    std::runtime_error);
}

TEST(TestBufferNonCpu, to_vector_works) {
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(3);
  Buffer<uint8_t> buffer(std::move(impl));

  std::vector<uint8_t> vec = buffer.to_vector();
  EXPECT_EQ(3u, vec.size());
}

// ========== Non-CPU swap death tests ==========
//
// Every swap entry point is declared noexcept to match std::vector::swap's
// noexcept contract and preserve std::is_nothrow_swappable_v<Buffer>.
// Calling swap on a non-CPU backend is therefore a precondition violation:
// throw_if_not_cpu_backend() throws a std::runtime_error which escapes the
// noexcept function and invokes std::terminate().

TEST(TestBufferNonCpuDeathTest, member_swap_buffer_terminates) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  auto impl_a = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> a(std::move(impl_a));
  Buffer<uint8_t> b{1, 2, 3};
  EXPECT_DEATH({a.swap(b);}, ".*");
}

TEST(TestBufferNonCpuDeathTest, member_swap_buffer_other_side_terminates) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  Buffer<uint8_t> a{1, 2, 3};
  auto impl_b = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> b(std::move(impl_b));
  EXPECT_DEATH({a.swap(b);}, ".*");
}

TEST(TestBufferNonCpuDeathTest, member_swap_vector_terminates) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> buf(std::move(impl));
  std::vector<uint8_t> vec{1, 2, 3};
  EXPECT_DEATH({buf.swap(vec);}, ".*");
}

TEST(TestBufferNonCpuDeathTest, free_swap_buffer_buffer_terminates) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  auto impl_a = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> a(std::move(impl_a));
  Buffer<uint8_t> b{1, 2, 3};
  EXPECT_DEATH({using std::swap; swap(a, b);}, ".*");
}

TEST(TestBufferNonCpuDeathTest, free_swap_buffer_vector_terminates) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> buf(std::move(impl));
  std::vector<uint8_t> vec{1, 2, 3};
  EXPECT_DEATH({using std::swap; swap(buf, vec);}, ".*");
}

TEST(TestBufferNonCpuDeathTest, free_swap_vector_buffer_terminates) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  auto impl = std::make_unique<NonCpuBufferImpl<uint8_t>>(4);
  Buffer<uint8_t> buf(std::move(impl));
  std::vector<uint8_t> vec{1, 2, 3};
  EXPECT_DEATH({using std::swap; swap(vec, buf);}, ".*");
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
