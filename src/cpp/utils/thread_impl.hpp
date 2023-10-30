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

#ifndef UTILS__THREAD_IMPL_HPP_
#define UTILS__THREAD_IMPL_HPP_

// threading.hpp implementations
#ifdef _WIN32
#include "thread_impl/thread_impl_win32.ipp"
#elif defined(__APPLE__)
#include "thread_impl/thread_impl_osx.ipp"
#elif defined(_POSIX_SOURCE) || defined(__QNXNTO__) || defined(__ANDROID__)
#include "thread_impl/thread_impl_pthread.ipp"
#else
#include "thread_impl/thread_impl_basic.ipp"
#endif // Platform selection

#endif  // UTILS__THREADING_IMPL_HPP_
