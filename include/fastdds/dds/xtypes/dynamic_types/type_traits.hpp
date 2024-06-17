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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__TYPETRAITS_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__TYPETRAITS_HPP

namespace eprosima {
namespace fastdds {
namespace dds {

template<typename T>
struct traits;

template<typename T>
struct object_traits;

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#include "detail/type_traits.hpp"

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__TYPETRAITS_HPP
