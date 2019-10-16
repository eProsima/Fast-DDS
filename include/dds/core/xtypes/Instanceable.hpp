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
class StructMember;

class Instanceable
{
public:
    virtual ~Instanceable() = default;

    virtual size_t memory_size() const = 0;

    virtual void construct_instance(uint8_t* instance) const = 0;

    virtual void copy_instance(uint8_t* target, const uint8_t* source) const = 0;

    virtual void move_instance(uint8_t* target, uint8_t* source) const = 0;

    virtual void destroy_instance(uint8_t* instance) const = 0;

    virtual bool compare_instance(const uint8_t* instance, const uint8_t* other_instance) const = 0;

    struct InstanceNode
    {
        const InstanceNode* parent;
        const DynamicType& type;
        uint8_t* instance;
        size_t deep;
        union Access
        {
            size_t index;
            const StructMember* struct_member;
            Access(size_t index) : index(index) {}
            Access(const StructMember& member) : struct_member(&member) {}
        } access;

        InstanceNode(
                const DynamicType& type,
                uint8_t* instance)
            : parent(nullptr)
            , type(type)
            , instance(instance)
            , deep(0)
            , access(0)
        {}

        InstanceNode(
                const InstanceNode& parent,
                const DynamicType& type,
                uint8_t* instance,
                const Access& access)
            : parent(&parent)
            , type(type)
            , instance(instance)
            , deep(parent.deep + 1)
            , access(access)
        {}
    };

    using InstanceVisitor = std::function<void(const InstanceNode& node)>;
    virtual void for_each_instance(const InstanceNode& node, InstanceVisitor visitor) const = 0;

protected:
    Instanceable() = default;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_INSTANCIABLE_TYPE_HPP_
