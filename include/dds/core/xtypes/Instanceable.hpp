/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#ifndef OMG_DDS_CORE_XTYPES_INSTANCEABLE_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_INSTANCEABLE_TYPE_HPP_

#include <cstdint>
#include <cstddef>
#include <functional>

namespace dds {
namespace core {
namespace xtypes {

class DynamicType;

class Instanceable
{
public:
    virtual ~Instanceable() = default;

    virtual size_t memory_size() const = 0;

    virtual void construct_instance(uint8_t* /*instance*/) const { } //Default does nothing

    virtual void copy_instance(uint8_t* target, const uint8_t* source) const = 0;

    virtual void destroy_instance(uint8_t* /*instance*/) const { } //Default does nothing

    virtual bool compare_instance(const uint8_t* instance, const uint8_t* other_instance) const = 0;

    using InstanceVisitor = std::function<void(const DynamicType& type, uint8_t* instance, size_t level)>;
    virtual void for_each_instance(uint8_t* instance, size_t level, InstanceVisitor visitor) const = 0;

protected:
    Instanceable() = default;

};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_INSTANCIABLE_TYPE_HPP_
