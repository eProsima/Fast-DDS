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

#ifndef OMG_DDS_CORE_XTYPES_DYNAMIC_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_DYNAMIC_TYPE_HPP_

#include <dds/core/xtypes/detail/DynamicType.hpp>

#include <dds/core/xtypes/TypeKind.hpp>
#include <dds/core/xtypes/Annotation.hpp>

#include <dds/core/Reference.hpp>

namespace dds {
namespace core {
namespace xtypes {

class DynamicType : public Reference<detail::DynamicType>
{
    OMG_DDS_REF_TYPE_PROTECTED_DC(
            DynamicType,
            Reference,
            detail::DynamicType)

public:
    const std::string& name() const
    {
        return impl()->name();
    }

    TypeKind kind() const
    {
        return impl()->kind();
    }

    const std::vector<Annotation>& annotations() const
    {
        return impl()->annotations();
    }

    DynamicType& add_annotation(const Annotation& annotation)
    {
        impl()->add_annotation(annotation);
        return *this;
    }

    bool is_primitive_type() const
    {
        return (impl()->kind().underlying() & TypeKind::PRIMITIVE_TYPE) != 0;
    }

    bool is_collection_type() const
    {
        return (impl()->kind().underlying() & TypeKind::COLLECTION_TYPE) != 0;
    }

    bool is_aggregation_type() const
    {
        return (impl()->kind().underlying() & TypeKind::AGGREGATION_TYPE) != 0;
    }

    bool is_constructed_type() const
    {
        return (impl()->kind().underlying() & TypeKind::CONSTRUCTED_TYPE) != 0;
    }

    /*
    bool operator ==(
            const DynamicType& that) const
    {
        return *(impl()) == *(that.impl());
    }

    bool operator !=(
            const DynamicType& that) const
    {
        return !(*this == that);
    }

    DynamicType(
            const DynamicType& other) = default; //CHECK: this
    */

protected:
    DynamicType(
            detail::DynamicType* detail)
        : Reference(detail)
    {}

    DynamicType(
            const std::string& name,
            TypeKind kind)
        : Reference(new detail::DynamicType(name, kind))
    {}

    DynamicType(
            const std::string& name,
            TypeKind kind,
            const Annotation& annotation)
        : Reference(new detail::DynamicType(name, kind, annotation))
    {}

    DynamicType(
            const std::string& name,
            TypeKind kind,
            const std::vector<Annotation>& annotations)
        : Reference(new detail::DynamicType(name, kind, annotations))
    {}

    template<typename AnnotationIter>
    DynamicType(
            const std::string& name,
            TypeKind kind,
            const AnnotationIter& begin,
            const AnnotationIter& end)
        : Reference(new detail::DynamicType(name, kind, begin, end))
    {}
};

template<typename T>
inline bool is_primitive_type(
        const DynamicType& t)
{
    return t.is_primitive_type();
}

template<typename T>
inline bool is_constructed_type(
        const DynamicType& t)
{
    return t.is_constructed_type();
}

template<typename T>
inline bool is_collection_type(
        const DynamicType& t)
{
    return t.is_collection_type();
}

template<typename T>
inline bool is_aggregation_type(
        const DynamicType& t)
{
    return t.is_aggregation_type();
}

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_TYPE_HPP_
