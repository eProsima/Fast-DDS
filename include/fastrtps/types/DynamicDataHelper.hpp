// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTRTPS_TYPES_DYNAMICDATAHELPER_HPP_
#define _FASTRTPS_TYPES_DYNAMICDATAHELPER_HPP_

#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicTypeMember.h>

namespace eprosima {
namespace fastrtps {
namespace types {

class DynamicDataHelper
{
public:
    RTPS_DllAPI static void print(
            const DynamicData_ptr& data);

    RTPS_DllAPI static void print(
            const DynamicData* data);

protected:
    static void print_basic_element(
            DynamicData* data,
            MemberId id,
            TypeKind kind);

    static void print_collection(
            DynamicData* data,
            const std::string& tabs = "");

    static void fill_array_positions(
            const std::vector<uint32_t>& bounds,
            std::vector<std::vector<uint32_t>>& positions);

    static void get_index_position(
            uint32_t index,
            const std::vector<uint32_t>& bounds,
            std::vector<uint32_t>& position);

    static void aux_index_position(
            uint32_t index,
            uint32_t inner_index,
            const std::vector<uint32_t>& bounds,
            std::vector<uint32_t>& position);

    static void print_basic_collection(
            DynamicData* data);

    static void print_complex_collection(
            DynamicData* data,
            const std::string& tabs = "");

    static void print_complex_element(
            DynamicData* data,
            MemberId id,
            const std::string& tabs = "");

    static void print_member(
            DynamicData* data,
            const DynamicTypeMember* type,
            const std::string& tabs = "");
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTRTPS_TYPES_DYNAMICDATAHELPER_HPP_
