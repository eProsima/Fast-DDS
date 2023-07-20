
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

#ifndef TYPES_DYNAMIC_BUILDER_H
#define TYPES_DYNAMIC_BUILDER_H

#include <fastrtps/types/DynamicTypeMember.h>

#include <cstdint>
#include <map>
#include <string>

namespace eprosima {
namespace fastrtps {
namespace types {

namespace v1_1 {

class TypeDescriptor;

using MemberId = uint32_t;

class DynamicType
{
    public:

    TypeDescriptor* get_descriptor() const
    {
        return nullptr;
    }

    std::string get_name() const
    {
        return {};
    }

    void get_all_members(std::map<MemberId, DynamicTypeMember*>& ) const
    {
    }
};

} // v1_1

namespace v1_3 {


class DynamicType
{
};

} // v1_3

} // eprosima
} // fastrtps
} // types

#endif // TYPES_DYNAMIC_BUILDER_H
