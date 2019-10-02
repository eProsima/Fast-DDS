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

#ifndef OMG_DDS_CORE_XTYPES_MEMBER_TYPE_HPP
#define OMG_DDS_CORE_XTYPES_MEMBER_TYPE_HPP

#include <dds/core/xtypes/detail/MemberType.hpp>

//#include <dds/core/detail/inttypes.hpp>

#include <dds/core/xtypes/DynamicType.hpp>

#include <dds/core/Reference.hpp>

namespace dds {
namespace core {
namespace xtypes {

class MemberType : public Reference<detail::MemberType>
{
    OMG_DDS_REF_TYPE_PROTECTED_DC(
            MemberType,
            Reference,
            detail::MemberType)

public:
    MemberType(
            const std::string& name,
            const DynamicType& type)
        : Reference(new detail::MemberType(name, type))
    {}

    MemberType(const std::string& name,
            const DynamicType& type,
            const Annotation& annotation)
        : Reference(new detail::MemberType(name, type, annotation))
    {}

    MemberType(
            const std::string& name,
            const DynamicType& type,
            const std::vector<Annotation>& annotations)
        : Reference(new detail::MemberType(name, type, annotations))
    {}

    template<typename AnnotationIter>
    MemberType(
            const std::string& name,
            const DynamicType& type,
            const AnnotationIter& begin,
            const AnnotationIter& end)
        : Reference(new detail::MemberType(name, type, begin, end))
    {}

    const std::string& name() const
    {
        return impl()->name();
    }

    const DynamicType& type() const
    {
        return impl()->dynamic_type();
    }

    MemberType& add_annotation(
            const Annotation& annotation)
    {
        impl()->add_annotation(annotation);
        return *this;
    }

    MemberType& remove_annotation(
            const Annotation& annotation)
    {
        impl()->remove_annotation(annotation);
        return *this;
    }

    bool is_optional() const
    {
        return impl()->is_optional();
    }

    bool is_shared() const
    {
        return impl()->is_shared();
    }

    bool is_key() const
    {
        return impl()->is_key();
    }

    bool is_must_understand() const
    {
        return impl()->is_must_understand();
    }

    bool is_bitset() const
    {
        return impl()->is_bitset();
    }

    bool has_bitbound() const
    {
        return impl()->has_bitbound();
    }

    uint32_t get_bitbound() const
    {
        return impl()->get_bitbound();
    }

    bool has_id() const
    {
        return impl()->has_id();
    }

    uint32_t get_id() const
    {
        return impl()->get_bitbound();
    }
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_MEMBER_TYPE_HPP
