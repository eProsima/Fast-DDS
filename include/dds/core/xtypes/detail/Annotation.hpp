/*
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

#ifndef EPROSIMA_DDS_CORE_XTYPES_DETAIL_ANNOTATION_HPP_
#define EPROSIMA_DDS_CORE_XTYPES_DETAIL_ANNOTATION_HPP_
#include <stdint.h>
#include <string>
#include <dds/core/xtypes/AnnotationKind.hpp>

namespace dds {
namespace core {
namespace xtypes {
namespace detail {

class Annotation
{
public:

    Annotation(
            AnnotationKind ak)
        : ak_(ak)
    {
    }

    const AnnotationKind& akind()
    {
        return ak_;
    }

private:

    AnnotationKind ak_;
};

class IdAnnotation : public Annotation
{
public:

    IdAnnotation()
        : Annotation(AnnotationKind::OPTIONAL_ANNOTATION_TYPE)
        , id_()
    {
    }

    void id(
            uint32_t id )
    {
        id_ = id;
    }

    uint32_t id() const noexcept
    {
        return id_;
    }

    template<typename Q,
            template <typename> class K>
    operator  K<Q>&()
    {
        return reinterpret_cast<K<Q>&>(*this);
    }

    IdAnnotation(
            Annotation& a)
        : Annotation(AnnotationKind::ID_ANNOTATION_TYPE)
        , id_(static_cast<const detail::IdAnnotation&>(a).id())
    {
    }

private:

    uint32_t id_;
};

class KeyAnnotation : public Annotation
{
public:

    KeyAnnotation()
        : Annotation(AnnotationKind::KEY_ANNOTATION_TYPE)
    {
    }

};

class SharedAnnotation : public Annotation
{
public:

    SharedAnnotation()
        : Annotation(AnnotationKind::SHARED_ANNOTATION_TYPE)
    {
    }

};

class NestedAnnotation : public Annotation
{
public:

    NestedAnnotation()
        : Annotation(AnnotationKind::NESTED_ANNOTATION_TYPE)
    {
    }

};

class ExtensibilityAnnotation : public Annotation
{
public:

    ExtensibilityAnnotation(
            ExtensibilityKind xkind)
        : Annotation(AnnotationKind::EXTENSIBILITY_ANNOTATION_TYPE)
        , xk_(xkind)
    {
    }

    const ExtensibilityKind& xKind() const noexcept
    {
        return xk_;
    }

    void xKind(
            ExtensibilityKind xk)
    {
        xk_ = xk;
    }

private:

    ExtensibilityKind xk_;

};

class MustUnderstandAnnotation : public Annotation
{
    MustUnderstandAnnotation()
        : Annotation(AnnotationKind::MUST_UNDERSTAND_ANNOTATION_TYPE)
    {
    }

};

class VerbatimAnnotation : public Annotation
{
public:

    VerbatimAnnotation(
            const std::string& vbt)
        : Annotation(AnnotationKind::VERBATIM_ANNOTATION_TYPE)
        , vbt_(vbt)
    {
    }

    void vbt(
            const std::string& vbt)
    {
        vbt_ = vbt;
    }

    const std::string& vbt() const noexcept
    {
        return vbt_;
    }

private:

    std::string vbt_;
};


class BitsetAnnotation : public Annotation
{
    BitsetAnnotation()
        : Annotation(AnnotationKind::BITSET_ANNOTATION_TYPE)
    {
    }

};

class BitBoundAnnotation : public Annotation
{
    BitBoundAnnotation(
            uint32_t bsb )
        : Annotation(AnnotationKind::BITSETBOUND_ANNOTATION_TYPE)
        , bsb_(bsb)
    {
    }

    uint32_t bsb() const noexcept
    {
        return bsb_;
    }

    void bsb(
            uint32_t bsb)
    {
        bsb_ = bsb;
    }

private:

    uint32_t bsb_;
};

} //namespace detail
} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_ANNOTATION_HPP_
