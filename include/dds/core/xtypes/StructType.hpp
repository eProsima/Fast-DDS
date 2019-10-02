/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
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
 */

#ifndef OMG_DDS_CORE_XTYPES_STRUCT_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_STRUCT_TYPE_HPP_

#include <dds/core/xtypes/detail/StructType.hpp>

#include <dds/core/xtypes/DynamicType.hpp>
#include <dds/core/xtypes/MemberType.hpp>

namespace dds {
namespace core {
namespace xtypes {

/**
 * Create a dynamic structure type. If the members don't have Id associated
 * explicitly, then their ID will be the same as the ordinal position on the
 * members vector.
 */ //TODO implementation regarding the commnet

class StructType : public DynamicType
{
public:
    StructType(
            const std::string& name)
        : DynamicType(new detail::StructType(name))
    {}

    StructType(
            const std::string& name,
            const StructType& parent)
        : DynamicType(new detail::StructType(name, parent))
    {}

    StructType(
            const std::string& name,
            const StructType& parent,
            const std::vector<MemberType>& members)
        : DynamicType(new detail::StructType(name, parent, members))
    {}

    template<typename MemberIter>
    StructType(
            const std::string& name,
            const StructType& parent,
            const MemberIter& begin,
            const MemberIter& end)
        : DynamicType(new detail::StructType(name, parent, begin, end))
    {}

    StructType(
            const std::string& name,
            const StructType& parent,
            const std::vector<MemberType>& members,
            const Annotation& annotation)
        : DynamicType(new detail::StructType(name, parent, members, annotation))
    {}

    StructType(
            const std::string& name,
            const StructType& parent,
            const std::vector<MemberType>& members,
            const std::vector<Annotation>& annotations)
        : DynamicType(new detail::StructType(name, parent, members, annotations))
    {}

    template<typename AnnotationIter, typename MemberIter>
    StructType(
            const std::string& name,
            const StructType& parent,
            const MemberIter& members_begin,
            const MemberIter& members_end,
            const AnnotationIter& annotations_begin,
            const AnnotationIter& annotations_end)
        : DynamicType(new detail::StructType(name, parent, members_begin, members_end, annotations_begin, annotations_end))
    {}

    const StructType& parent() const
    {
        return *std::static_pointer_cast<detail::StructType>(impl())->parent();
    }

    const std::vector<MemberType>& members() const
    {
        return std::static_pointer_cast<detail::StructType>(impl())->members();
    }

    const MemberType& member(
            uint32_t id) const
    {
        return std::static_pointer_cast<detail::StructType>(impl())->member(id);
    }

    const MemberType& member(
            const std::string& name) const
    {
        return std::static_pointer_cast<detail::StructType>(impl())->member(name);
    }

    StructType add_member(
            const MemberType& member) const
    {
        std::static_pointer_cast<detail::StructType>(impl())->add_member(member);
        return *this;
    }
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_STRUCT_TYPE_HPP_
