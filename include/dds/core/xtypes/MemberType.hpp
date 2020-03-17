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
#include <dds/core/detail/inttypes.hpp>

#include <dds/core/xtypes/DynamicType.hpp>
#include <dds/core/Reference.hpp>



namespace dds {
namespace core {
namespace xtypes {

template<
    typename DELEGATE,
    typename OTHER_DELEGATE>
class TMemberType : public Reference<DELEGATE>
{
    OMG_DDS_REF_TYPE_PROTECTED_DC(
        TMemberType,
        Reference,
        DELEGATE)

public:

    TMemberType(
            const std::string& name,
            const TDynamicType<OTHER_DELEGATE>& type)
    {
        impl()->name(name);
        impl()->dt(type);
    }

    TMemberType(
            const std::string& name,
            const xtypes::DynamicType& type,
            const Annotation& annotation)
    {
        impl()->name(name);
        impl()->dt(type);
        impl()->annotation(annotation);
    }

    TMemberType(
            const std::string& name,
            const TDynamicType<OTHER_DELEGATE>& type,
            const std::vector<Annotation>& annotations)
    {
        impl()->name(name);
        impl()->dt(type);
        impl()->annotation(annotations);
    }

    template<typename AnnotationIter>
    TMemberType(
            const std::string& name,
            const TDynamicType<OTHER_DELEGATE>& type,
            const AnnotationIter& begin,
            const AnnotationIter& end)
    {
        impl()->name(name);
        impl()->dt(type);
        impl()->annotation(begin, end);
    }

    const std::string& name() const
    {
        return impl()->name();
    }

    const TDynamicType<OTHER_DELEGATE>& type() const
    {
        return impl()->dt();
    }

    TMemberType add_annotation(
            const Annotation& annotation)
    {
        impl()->annotation(annotation);
        return *this;
    }

    TMemberType remove_annotation(
            const Annotation& annotation)
    {
        impl().remove_annotation(annotation);
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

typedef TMemberType<detail::MemberType, detail::DynamicType> MemberType;
typedef MemberType Member;

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_MEMBER_TYPE_HPP
