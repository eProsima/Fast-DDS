// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file set_node_size_impl.hpp
 */

#ifndef SRC_CPP_UTILS_COLLECTIONS_IMPL_SET_NODE_SIZE_IMPL_HPP_
#define SRC_CPP_UTILS_COLLECTIONS_IMPL_SET_NODE_SIZE_IMPL_HPP_

#include "./config.hpp"

#if defined(USE_FOONATHAN_NODE_SIZES)
#include "foonathan/set_node_size_impl.hpp"
#elif defined(USE_STD_NODE_SIZES)
#include "std/set_node_size_impl.hpp"
#elif defined(USE_CUSTOM_NODE_SIZES)
#include "custom/set_node_size_impl.hpp"
#else
#error "Don't now which map_node_size implementation to use"
#endif // NODE SIZE CONFIG

#endif  /* SRC_CPP_UTILS_COLLECTIONS_IMPL_SET_NODE_SIZE_IMPL_HPP_ */