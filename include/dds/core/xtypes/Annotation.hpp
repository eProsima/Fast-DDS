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

#include <dds/core/Reference.hpp>
#include <dds/core/SafeEnumeration.hpp>
#include <dds/core/xtypes/TypeKind.hpp>

namespace dds {
namespace core {
namespace xtypes {

struct AnnotationKind_def
{
    enum Type
    {
        ID_ANNOTATION_TYPE,
        OPTIONAL_ANNOTATION_TYPE,
        KEY_ANNOTATION_TYPE,
        SHARED_ANNOTATION_TYPE,
        NESTED_ANNOTATION_TYPE,
        EXTENSIBILITY_ANNOTATION_TYPE,
        MUST_UNDERSTAND_ANNOTATION_TYPE,
        VERBATIM_ANNOTATION_TYPE,
        BITSET_ANNOTATION_TYPE
    };
};

typedef dds::core::SafeEnum<AnnotationKind_def> AnnotationKind;

struct ExtensibilityKind_def
{
    enum Type
    {
        FINAL,
        EXTENSIBLE,
        MUTABLE
    };
};
typedef dds::core::SafeEnum<ExtensibilityKind_def> ExtensibilityKind;

<<<<<<< HEAD:include/dds/core/xtypes/TAnnotation.hpp
template<typename DELEGATE>
class TAnnotation;

template<typename DELEGATE>
class TIdAnnotation;

template<typename DELEGATE>
class TKeyAnnotation;

template<typename DELEGATE>
class TSharedAnnotation;

template<typename DELEGATE>
class TNestedAnnotation;

template<typename DELEGATE>
class TExtensibilityAnnotation;

template<typename DELEGATE>
class TMustUnderstandAnnotation;

template<typename DELEGATE>
class TVerbatimAnnotation;

template<typename DELEGATE>
class TBitsetAnnotation;

template<typename DELEGATE>
class TBitBoundAnnotation;

=======
>>>>>>> Added annotations to MemberType and DynamicType. Style fixes.:include/dds/core/xtypes/Annotation.hpp

template <typename DELEGATE>
class TAnnotation : public Reference<DELEGATE>
{
public:

    OMG_DDS_REF_TYPE(
            TAnnotation,
            dds::core::Reference,
            DELEGATE)

    //TAnnotation();

    TypeKind kind() const;

protected:

    TAnnotation(const TypeKind& kind);
};

template<typename DELEGATE>
class TIdAnnotation : public TAnnotation<DELEGATE>
{
public:
    TIdAnnotation(
            uint32_t id);

    uint32_t id() const;
};

template<typename DELEGATE>
class TKeyAnnotation : public TAnnotation<DELEGATE>

{
public:
    TKeyAnnotation();
};

template<typename DELEGATE>
class TSharedAnnotation : public TAnnotation<DELEGATE>
{
public:
    TSharedAnnotation();
};

template<typename DELEGATE>
class TNestedAnnotation : public TAnnotation<DELEGATE>
{
public:
    TNestedAnnotation();
};

template<typename DELEGATE>
class TExtensibilityAnnotation : public TAnnotation<DELEGATE>
{
public:
    TExtensibilityAnnotation(
            ExtensibilityKind xkind);

    ExtensibilityKind extensibility_kind() const;
};

template<typename DELEGATE>
class TMustUnderstandAnnotation : public TAnnotation<DELEGATE>
{
public:
    TMustUnderstandAnnotation();
};

template<typename DELEGATE>
class TVerbatimAnnotation : public TAnnotation<DELEGATE>
{
public:
    TVerbatimAnnotation(
            const std::string& text);

    const std::string& verbatim_text() const;
};

template<typename DELEGATE>
class TBitsetAnnotation : public TAnnotation<DELEGATE>
{
public:
    TBitsetAnnotation();

};

template<typename DELEGATE>
class  TBitBoundAnnotation : public TAnnotation<DELEGATE>
{
public:
    TBitBoundAnnotation(
            uint32_t bound);
};

<<<<<<< HEAD:include/dds/core/xtypes/TAnnotation.hpp
=======
typedef TAnnotation<detail::Annotation> Annotation;
typedef TIdAnnotation<detail::IdAnnotation> IdAnnotation;
typedef TKeyAnnotation<detail::KeyAnnotation> KeyAnnotation;
typedef TSharedAnnotation<detail::SharedAnnotation> SharedAnnotation;
typedef TNestedAnnotation<detail::NestedAnnotation> NestedAnnotation;
typedef TExtensibilityAnnotation<detail::ExtensibilityAnnotation> ExtensibilityAnnotation;
typedef TMustUnderstandAnnotation<detail::MustUnderstandAnnotation> MustUnderstandAnnotation;
typedef TVerbatimAnnotation<detail::VerbatimAnnotation> VerbatimAnnotation;
typedef TBitsetAnnotation<detail::BitsetAnnotation> BitsetAnnotation;
typedef TBitBoundAnnotation<detail::BitBoundAnnotation> BitBoundAnnotation;

namespace annotation
{
    // These functions can be used to get cached instances,
    // to avoid the proliferation of small annotation objects.
    IdAnnotation id(uint32_t);
    KeyAnnotation key();
    SharedAnnotation shared();
    NestedAnnotation nested();
    ExtensibilityAnnotation extensibility(ExtensibilityKind kind);
    ExtensibilityAnnotation get_final();
    ExtensibilityAnnotation extensible();
    ExtensibilityAnnotation get_mutable();
    MustUnderstandAnnotation must_understand();
    VerbatimAnnotation verbatim(const std::string& text);
    BitsetAnnotation bitset();
    BitsetAnnotation bit_bound(uint32_t bound);

} //namespace annotation
>>>>>>> Added annotations to MemberType and DynamicType. Style fixes.:include/dds/core/xtypes/Annotation.hpp
} //namespace xtypes
} //namespace core
} //namespace dds

<<<<<<< HEAD:include/dds/core/xtypes/TAnnotation.hpp

#endif //OMG_DDS_CORE_XTYPES_TANNOTATION_HPP_

=======
#endif // OMG_DDS_CORE_XTYPES_ANNOTATION_HPP_
>>>>>>> Added annotations to MemberType and DynamicType. Style fixes.:include/dds/core/xtypes/Annotation.hpp
