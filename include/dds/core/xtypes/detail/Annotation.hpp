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

#include <dds/core/xtypes/AnnotationKind.hpp>

#include <string>

namespace dds {
namespace core {
namespace xtypes {
namespace detail {

class Annotation
{
public:
    virtual ~Annotation() = default;

    const AnnotationKind& kind() { return kind_; }

protected:
    Annotation(
            AnnotationKind kind)
        : kind_(kind)
    {}

private:
    AnnotationKind kind_;
};

class IdAnnotation : public Annotation
{
public:
    IdAnnotation(uint32_t id)
        : Annotation(AnnotationKind::OPTIONAL_ANNOTATION_TYPE)
        , id_(id)
    {}

    IdAnnotation(const IdAnnotation& annotation)
        : Annotation(AnnotationKind::ID_ANNOTATION_TYPE)
        , id_(annotation.id())
    {}

    void id(uint32_t id) { id_ = id; }
    uint32_t id() const { return id_; }

    template<typename Q,
            template <typename> class K>
    operator  K<Q>&()
    {
        return reinterpret_cast<K<Q>&>(*this);
    }

private:
    uint32_t id_;
};

class KeyAnnotation : public  Annotation
{
public:
    KeyAnnotation()
        : Annotation(AnnotationKind::KEY_ANNOTATION_TYPE)
    {}
};

class SharedAnnotation : public  Annotation
{
public:
    SharedAnnotation()
        : Annotation(AnnotationKind::SHARED_ANNOTATION_TYPE)
    {}
};

class NestedAnnotation : public  Annotation
{
public:
    NestedAnnotation()
        : Annotation(AnnotationKind::NESTED_ANNOTATION_TYPE)
    {}
};

class ExtensibilityAnnotation : public  Annotation
{
public:
    ExtensibilityAnnotation(
            ExtensibilityKind xkind)
        : Annotation(AnnotationKind::EXTENSIBILITY_ANNOTATION_TYPE)
        , xkind_(xkind)
    {
    }

    const ExtensibilityKind& xKind() const { return xkind_; }

    void xKind( ExtensibilityKind xk) { xkind_ = xk; }

private:
    ExtensibilityKind xkind_;
};

class MustUnderstandAnnotation : public Annotation
{
    MustUnderstandAnnotation()
        : Annotation(AnnotationKind::MUST_UNDERSTAND_ANNOTATION_TYPE)
    {}
};

class VerbatimAnnotation : public Annotation
{
public:
    VerbatimAnnotation(
            const std::string& verbatim)
        : Annotation(AnnotationKind::VERBATIM_ANNOTATION_TYPE)
        , verbatim_(verbatim)
    {}

    const std::string& verbatim() const { return verbatim_; }

    void verbatim(const std::string& verbatim) { verbatim_ = verbatim; }

private:
    std::string verbatim_;
};


class BitsetAnnotation : public  Annotation
{
    BitsetAnnotation()
        : Annotation(AnnotationKind::BITSET_ANNOTATION_TYPE)
	{}
};

class BitBoundAnnotation : public  Annotation
{
    BitBoundAnnotation(
            uint32_t bitsetbound)
        : Annotation(AnnotationKind::BITSETBOUND_ANNOTATION_TYPE)
        , bitsetbound_(bitsetbound)
    {}

    uint32_t bitsetbound() const  { return bitsetbound_; }

    void bitsetbound(uint32_t bitsetbound) { bitsetbound_ = bitsetbound; }

private:
    uint32_t bitsetbound_;
};

} //namespace detail
} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_ANNOTATION_HPP_
