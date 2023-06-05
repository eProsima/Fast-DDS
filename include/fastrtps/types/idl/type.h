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

#ifndef TYPES_IDL_TYPE_H_
#define TYPES_IDL_TYPE_H_

#include <fastrtps/types/v1_3/DynamicType.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace idl {

class Module;

class Type
{
public:

    Type(
            Module& parent,
            const v1_3::DynamicType& type)
        : type_(std::make_shared<v1_3::DynamicType>(type))
        , parent_(parent)
    {
    }

    Type(
            Module& parent,
            v1_3::DynamicType&& type)
        : type_(std::make_shared<v1_3::DynamicType>(type))
        , parent_(parent)
    {
    }

    Type(
            Type&& type) = default;

    Module& parent() const
    {
        return parent_;
    }

    v1_3::DynamicType_ptr& get()
    {
        return type_;
    }

    const v1_3::DynamicType_ptr& get() const
    {
        return type_;
    }

    const v1_3::DynamicType& operator * () const
    {
        return *type_;
    }

    const v1_3::DynamicType* operator -> () const
    {
        return type_.get();
    }

private:

    Type(
            const Type& type) = delete;

    v1_3::DynamicType_ptr type_;

    Module& parent_;

};

} // namespace idl
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_IDL_TYPE_H_
