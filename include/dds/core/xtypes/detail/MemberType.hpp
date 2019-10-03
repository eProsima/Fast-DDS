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

#ifndef EPROSIMA_DDS_CORE_XTYPES_DETAIL_MEMBER_TYPE_HPP_
#define EPROSIMA_DDS_CORE_XTYPES_DETAIL_MEMBER_TYPE_HPP_

#include <dds/core/xtypes/DynamicType.hpp>

#include <vector>
#include <functional>
#include <algorithm>

namespace dds {
namespace core {
namespace xtypes {
namespace detail {

class MemberType
{
public:
    MemberType(
            const std::string& name,
            const xtypes::DynamicType& dynamic_type)
        : name_(name)
        , dynamic_type_(dynamic_type)
    {}

    MemberType(
            const std::string& name,
            const xtypes::DynamicType& dynamic_type,
            const xtypes::Annotation& annotation)
        : name_(name)
        , dynamic_type_(dynamic_type)
    {
        annotations_.emplace_back(annotation);
    }

    MemberType(
            const std::string& name,
            const xtypes::DynamicType& dynamic_type,
            const std::vector<xtypes::Annotation>& annotations)
        : name_(name)
        , dynamic_type_(dynamic_type)
        , annotations_(annotations)
    {}


    template<typename AnnotationIter>
    MemberType(
            const std::string& name,
            const xtypes::DynamicType dynamic_type,
            const AnnotationIter& begin,
            const AnnotationIter& end)
        : name_(name)
        , dynamic_type_(dynamic_type)
        , annotations_(begin, end)
    {}

    const std::string& name() const { return name_; }
    const xtypes::DynamicType& dynamic_type() const { return dynamic_type_; }
    const std::vector<xtypes::Annotation>& annotations() const { return annotations_; }

    void name(const std::string& name) { name_ = name; }
    void dynamic_type(const xtypes::DynamicType& dynamic_type) { dynamic_type_ = dynamic_type; }
    void annotations(const std::vector<xtypes::Annotation>& annotations) { annotations_ = annotations; }

    void add_annotation(const xtypes::Annotation& annotation)
    {
        annotations_.emplace_back(annotation);
    }

    void remove_annotation(
            const xtypes::Annotation& annotation)
    {
        auto rem = std::find_if(
                annotations_.begin(),
                annotations_.end(),
                [&](xtypes::Annotation& a) { return a.akind() == annotation.akind(); } );

        if (rem != annotations_.end())
        {
            annotations_.erase(rem);
        }
    }

    bool is_optional()
    {
        AnnotationKind  kind = AnnotationKind_def::Type::OPTIONAL_ANNOTATION_TYPE;
        return has_annotation(kind);
    }

    bool is_shared()
    {
        AnnotationKind kind = AnnotationKind_def::Type::SHARED_ANNOTATION_TYPE;
        return has_annotation(kind);
    }

    bool is_key()
    {
        AnnotationKind kind = AnnotationKind_def::Type::KEY_ANNOTATION_TYPE;
        return has_annotation(kind);
    }

    bool is_must_understand()
    {
        AnnotationKind kind = AnnotationKind_def::Type::MUST_UNDERSTAND_ANNOTATION_TYPE;
        return has_annotation(kind);
    }

    bool is_bitset()
    {
        AnnotationKind kind = AnnotationKind_def::Type::BITSET_ANNOTATION_TYPE;
        return has_annotation(kind);
    }

    bool has_bitbound()
    {
        AnnotationKind kind = AnnotationKind_def::Type::BITSETBOUND_ANNOTATION_TYPE;
        return has_annotation(kind);
    }

    bool has_id()
    {
        AnnotationKind kind = AnnotationKind_def::Type::ID_ANNOTATION_TYPE;
        return has_annotation(kind);
    }

    uint32_t get_id()
    {
        AnnotationKind kind = AnnotationKind_def::Type::ID_ANNOTATION_TYPE;

        // creting a generic IdAnnotation that will be filled by annotation_iterator()
        xtypes::IdAnnotation ida(0);
        if (!annotation_iterator(kind, ida))
        {
            throw IllegalOperationError("No Id Annotation found");
        }
        return ida.id();
    }

    int32_t get_bitbound() const
    {
        return 0; //TODO //similar to get_id
    }

private:
    bool annotation_iterator(
            AnnotationKind kind,
            xtypes::Annotation& next)
    {
        auto retVal = std::find_if(
                annotations_.begin(),
                annotations_.end(),
                [&](xtypes::Annotation& a) { return (a.akind() == kind); } );

        if (retVal == annotations_.end())
        {
            return false;
        }

        next = *retVal;
        return true;
    }

    bool has_annotation(
            AnnotationKind kind)
    {
        return annotations_.end() !=  std::find_if(
                annotations_.begin(),
                annotations_.end(),
                [&](xtypes::Annotation& a) { return (a.akind() == kind);} );
    }

private:
    std::string name_;
    xtypes::DynamicType dynamic_type_;
    std::vector<xtypes::Annotation> annotations_;
};

typedef MemberType Member;

} //namespace detail
} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_MEMBER_TYPE_HPP_
