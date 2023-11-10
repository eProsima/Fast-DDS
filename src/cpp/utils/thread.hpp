// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UTILS__THREAD_HPP_
#define UTILS__THREAD_HPP_

// thread.hpp declarations
#if defined(_WIN32) || defined(__APPLE__) || defined(_POSIX_SOURCE) || defined(__QNXNTO__) || defined(__ANDROID__)
#include "thread_impl/thread_impl_custom.hpp"
#else
#include "thread_impl/thread_impl_basic.hpp"
#endif // Platform selection

#endif  // UTILS__THREAD_HPP_
