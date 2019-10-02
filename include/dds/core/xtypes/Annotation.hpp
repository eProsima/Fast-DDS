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

#ifndef OMG_DDS_CORE_XTYPES_ANNOTATION_HPP_
#define OMG_DDS_CORE_XTYPES_ANNOTATION_HPP_

#include <dds/core/xtypes/detail/Annotation.hpp>

#include <dds/core/xtypes/TypeKind.hpp>

#include <dds/core/SafeEnumeration.hpp>
#include <dds/core/Reference.hpp>

namespace dds {
namespace core {
namespace xtypes {

class Annotation : public Reference<detail::Annotation>
{
    OMG_DDS_REF_TYPE_BASE(
            Annotation,
            dds::core::Reference,
            detail::Annotation)

public:
    Annotation(detail::Annotation* annotation)
        : Reference<detail::Annotation>(annotation)
    {}

    virtual ~Annotation() = default;

    TypeKind kind() const { return TypeKind::ANNOTATION_TYPE; }

    const AnnotationKind& akind() const
    {
        return impl()->kind();
    }
};


class IdAnnotation : public Annotation
{
public:
    IdAnnotation(
            uint32_t id)
        : Annotation(std::make_shared<detail::IdAnnotation>(id))
    {}

    uint32_t id() const
    {
        return std::static_pointer_cast<detail::IdAnnotation>(impl())->id();
    }
};

/*
template<typename DELEGATE>
class TKeyAnnotation : public TAnnotation<DELEGATE>
{
using TAnnotation<DELEGATE>::impl;
public:
    TKeyAnnotation()
    {
    }
};

template<typename DELEGATE>
class TSharedAnnotation : public TAnnotation<DELEGATE>
{
public:
    TSharedAnnotation()
    {
    }
};

template<typename DELEGATE>
class TNestedAnnotation : public TAnnotation<DELEGATE>
{
public:
    TNestedAnnotation()
    {
    }
};

template<typename DELEGATE>
class TExtensibilityAnnotation : public TAnnotation<DELEGATE>
{
using TAnnotation<DELEGATE>::impl;
public:
    TExtensibilityAnnotation(
            ExtensibilityKind xkind)
    {
        impl()->xKind();
    }

    ExtensibilityKind extensibility_kind() const
    {
        return impl()->xKind();
    }
};

template<typename DELEGATE>
class TMustUnderstandAnnotation : public TAnnotation<DELEGATE>
{
public:
    TMustUnderstandAnnotation()
    {
    }
};

template<typename DELEGATE>
class TVerbatimAnnotation : public TAnnotation<DELEGATE>
{
using TAnnotation<DELEGATE>::impl;
public:
    TVerbatimAnnotation(
            const std::string& text)
    {
        impl()->vbt(text);
    }

    const std::string& verbatim_text() const
    {
        return impl()->vbt();
    }
};

template<typename DELEGATE>
class TBitsetAnnotation : public TAnnotation<DELEGATE>
{
public:
    TBitsetAnnotation()
    {
    }

};

template<typename DELEGATE>
class  TBitBoundAnnotation : public TAnnotation<DELEGATE>
{
using TAnnotation<DELEGATE>::impl;
public:
    TBitBoundAnnotation(
            uint32_t bound)
    {
        impl()->bsb(bound);
    }

    virtual uint32_t bound()const
    {
        return impl()->bsb();
    }

    void bound(
            uint32_t bound)
    {
        impl()->bsb(bound);
    }
};

*/

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_TANNOTATION_HPP_
