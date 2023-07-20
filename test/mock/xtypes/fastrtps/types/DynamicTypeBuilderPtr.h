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

#ifndef TYPES_DYNAMIC_TYPE_BUILDER_PTR_H
#define TYPES_DYNAMIC_TYPE_BUILDER_PTR_H

#include <memory>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypePtr.h>

namespace std {

template<>
class std::shared_ptr<const eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
{
};

template<>
class std::shared_ptr<eprosima::fastrtps::types::v1_3::DynamicTypeBuilder>
{
   using element_type = eprosima::fastrtps::types::v1_3::DynamicTypeBuilder;

   element_type * inner;

public:

    element_type* operator->() const
    {
        return inner;
    }

    element_type& operator*() const
    {
        return *inner;
    }
};

} // namespace std

namespace eprosima {
namespace fastrtps {
namespace types {

namespace v1_1 {

class TypeDescriptor;

class DynamicTypeBuilder_ptr
{
    DynamicTypeBuilder* inner;

public:

    DynamicTypeBuilder_ptr& operator=(DynamicTypeBuilder*)
    {
        return *this;
    }

    DynamicTypeBuilder* get() const
    {
        return inner;
    }

    DynamicTypeBuilder* operator->() const
    {
        return get();
    }

    operator bool() const
    {
        return true;
    }
};

} // v1_1

namespace v1_3 {

class TypeDescriptor;

using DynamicTypeBuilder_ptr = std::shared_ptr<DynamicTypeBuilder>;
using DynamicTypeBuilder_cptr = std::shared_ptr<const DynamicTypeBuilder>;

} // v1_3

} // eprosima
} // fastrtps
} // types

#endif // TYPES_DYNAMIC_TYPE_BUILDER_PTR_H
