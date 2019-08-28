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
#include <dds/core/xtypes/Annotation.hpp>
#include <dds/core/Reference.hpp>

#include <vector>

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
        (void) name;
        (void) type;
    }

    TMemberType(const std::string& name,
                const dds::core::xtypes::DynamicType& type,
                const Annotation& annotation)
    {
        (void) name;
        (void) type;
        (void) annotation;
    }

    TMemberType(
            const std::string& name,
            const TDynamicType<OTHER_DELEGATE>& type,
            const std::vector<Annotation>& annotations)
    {
        (void) name;
        (void) type;
        (void) annotations;
    }

    template <typename AnnotationIter>
    TMemberType(
            const std::string& name,
            const TDynamicType<OTHER_DELEGATE>& type,
            const AnnotationIter& begin,
            const AnnotationIter& end)
    {
        (void) name;
        (void) type;
        (void) begin;
        (void) end;
    }

    const std::string& name() const
    {
    }

    const dds::core::xtypes::TDynamicType<OTHER_DELEGATE>& type() const
    {
    }

    TMemberType add_annotation(
            const Annotation& annotation)
    {
        (void) annotation;
    }

    TMemberType remove_annotation(
            const Annotation& annotation)
    {
        (void) annotation;
    }

    bool is_optional() const
    {
    }

    bool is_shared() const
    {
    }

    bool is_key() const
    {
    }

    bool is_must_understand() const
    {
    }

    bool is_bitset() const
    {
    }

    bool has_bitbound() const
    {
    }

    int32_t get_bitbound() const
    {
    }

    bool has_id() const
    {
    }

    int32_t get_id() const
    {
    }
};

typedef TMemberType<detail::MemberType, detail::DynamicType> MemberType;
typedef MemberType Member;

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_MEMBER_TYPE_HPP
