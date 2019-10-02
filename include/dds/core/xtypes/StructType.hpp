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

template<typename DELEGATE>
class TStructForwardDeclaration : public TDynamicType<DELEGATE>
{
public:
    TStructForwardDeclaration(
            const std::string& name)
    {
        throw "Not implemented";
    }
};

/**
 * Create a dynamic structure type. If the members don't have Id associated
 * explicitly, then their ID will be the same as the ordinal position on the
 * members vector.
 */
template<typename DELEGATE>
class TStructType : public TDynamicType<DELEGATE>
{

    TStructType& parent_ts_;
public:
    explicit TStructType(
            const std::string& name)
    {
        throw "Not implemented";
    }

    TStructType(
        const std::string& name,
        const std::vector<MemberType>& members)
    {
        TDynamicType<DELEGATE>::impl()->name(name);
    }

    TStructType(
        const std::string& name,
        const TStructType& parent,
        const std::vector<MemberType>& members)
    {
        TDynamicType<DELEGATE>::impl()->name(name);
        parent_ts_ = parent;
        TDynamicType<DELEGATE>::impl()->members(members);
    }

    template<typename MemberIter>
    TStructType(
        const std::string& name,
        const TStructType& parent,
        const MemberIter& begin,
        const MemberIter& end)
    {
        TDynamicType<DELEGATE>::impl()->name(name);
        parent_ts_ = parent;
        TDynamicType<DELEGATE>::impl()->members(begin, end);
    }

    TStructType(
        const std::string& name,
        const TStructType& parent,
        const std::vector<MemberType>& members,
        const Annotation& annotation)
    {
        TDynamicType<DELEGATE>::impl()->name(name);
        parent_ts_ = parent;
        TDynamicType<DELEGATE>::impl()->members(members);
        TDynamicType<DELEGATE>::impl()->annotation(annotation);
    }

    TStructType(
        const std::string& name,
        const TStructType& parent,
        const std::vector<MemberType>& members,
        const std::vector<Annotation>& annotations)
    {
        TDynamicType<DELEGATE>::impl()->name(name);
        parent_ts_ = parent;
        TDynamicType<DELEGATE>::impl()->members(members);
        TDynamicType<DELEGATE>::impl()->annotations(annotations);
    }

    template<
            typename AnnotationIter,
            typename MemberIter>
    TStructType(
        const std::string& name,
        const TStructType& parent,
        const MemberIter& member_begin,
        const MemberIter& member_end,
        const AnnotationIter& annotation_begin,
        const AnnotationIter& annotation_end)
    {
        TDynamicType<DELEGATE>::impl()->name(name);
        parent_ts_ = parent;
        TDynamicType<DELEGATE>::impl()->members(member_begin, member_end);
        TDynamicType<DELEGATE>::impl()->annotations(annotation_begin, annotation_end);
    }

    template<typename BASE_DELEGATE>
    operator TDynamicType<BASE_DELEGATE>() const
    {
        throw "Not implemented";
    }

    TStructType parent() const
    {
        return parent_ts_;
    }

    const std::vector<MemberType>& members() const
    {
        return TDynamicType<DELEGATE>::impl()->members();
    }

    const MemberType& member(
            uint32_t id) const
    {
        return TDynamicType<DELEGATE>::impl()->member(id);
    }

    const MemberType& member(
            const std::string& name) const
    {
        return TDynamicType<DELEGATE>::impl()->member(name);
    }

    const std::vector<Annotation>& annotations() const
    {
        return TDynamicType<DELEGATE>::impl()->annotations();
    }

    TStructType add_member(
            const MemberType& member) const
    {
        TDynamicType<DELEGATE>::impl()->member(member);
        return *this;
    }

    TStructType remove_member(
            const MemberType& member) const
    {
        TDynamicType<DELEGATE>::impl()->remove_member();
        return *this;
    }

    TStructType add_annotation(
            const Annotation& annotation) const
    {
        TDynamicType<DELEGATE>::impl()->annotation(annotation);
        return *this;
    }

    TStructType remove_annotation(
            const Annotation& annotation) const
    {
        TDynamicType<DELEGATE>::impl()->remove_annotation(annotation);
        return *this;
    }
};

typedef TStructType<detail::StructType> StructType;

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_STRUCT_TYPE_HPP_
