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

#include <dds/core/xtypes/Annotation.hpp>
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
            xtypes::DynamicType& dt)
        : name_(name)
#if (0)
        , dt_(dt.name(), dt.kind(), dt.annotations())
#else
        , dt_(dt)
#endif
        , ann_()
    {
    }

    MemberType(
            const std::string& name,
            xtypes::DynamicType& dt,
            xtypes::Annotation& a)
        : name_(name)
        , dt_(dt)
        , ann_()
    {
        ann_.push_back(a);
    }

    void name(
            const std::string& name)
    {
        name_ = name;
    }

    void dt(
            const xtypes::DynamicType& dt)
    {
        dt_ = dt;
    }

    void annotations(
            std::vector<xtypes::Annotation>& ann)
    {
        ann_.reserve(ann.size() + ann_.size());
        for (auto it = ann.begin(); it != ann.end(); ++it)
        {
            ann_.emplace_back(*it);
        }
    }

    template<typename AnnoIter>
    void annotations(
            AnnoIter begin,
            AnnoIter end)
    {
        ann_.reserve(ann_.size() + ( end - begin) );
        for (auto it = begin; it != end; ++it)
        {
            ann_.emplace_back(*it);
        }
    }

    void annotation(
            xtypes::Annotation& ann)
    {
        ann_.push_back(ann);
    }

    const std::string& name() const noexcept
    {
        return name_;
    }

    const xtypes::DynamicType& dynamic_type() const noexcept
    {
        return dt_;
    }

    const std::vector<xtypes::Annotation>& annotations()
    {
        return ann_;
    }

    void remove_annotation(
            const xtypes::Annotation& a)
    {
        auto rem = std::find_if(
            ann_.begin(),
            ann_.end(),
            [&]( xtypes::Annotation& b)
            {
                return b.akind() == a.akind();
            }
        );
        if ( rem != ann_.end() )
        {
            ann_.erase(rem);
        }
    }

    bool annotation_iterator(
            AnnotationKind& annotation_kind,
            xtypes::Annotation& retAnn)
    {
        auto retVal = std::find_if(
            ann_.begin(),
            ann_.end(),
            [&]( xtypes::Annotation& a)
            {
                return (a.akind() == annotation_kind);
            }
        );

        if (retVal == ann_.end())
        {
            return false;
        }
        retAnn = *retVal;
        return true;
    }

    bool find_annotation(
            AnnotationKind& annotation_kind)
    {
        return ann_.end() !=  std::find_if(
            ann_.begin(),
            ann_.end(),
            [&]( xtypes::Annotation& a)
            {
                return (a.akind() == annotation_kind);
            }
        );
    }

    bool is_optional()
    {
        AnnotationKind a = AnnotationKind_def::Type::OPTIONAL_ANNOTATION_TYPE;
        return find_annotation(a);
    }

    bool is_shared()
    {
        AnnotationKind a = AnnotationKind_def::Type::SHARED_ANNOTATION_TYPE;
        return find_annotation(a);
    }

    bool is_key()
    {
        AnnotationKind a = AnnotationKind_def::Type::KEY_ANNOTATION_TYPE;
        return find_annotation(a);
    }

    bool is_must_understand()
    {
        AnnotationKind a = AnnotationKind_def::Type::MUST_UNDERSTAND_ANNOTATION_TYPE;
        return find_annotation(a);
    }

    bool is_bitset()
    {
        AnnotationKind a = AnnotationKind_def::Type::BITSET_ANNOTATION_TYPE;
        return find_annotation(a);
    }

    bool has_bitbound()
    {
        AnnotationKind a = AnnotationKind_def::Type::BITSETBOUND_ANNOTATION_TYPE;
        return find_annotation(a);
    }

    /*
        uint32_t get_bitbound()
        {
            if(false == has_bitbound())
            {
                throw IllegalOperationError("No Bitsetbound Annotation found");
            }
            AnnotationKind a = AnnotationKind_def::Type::BITSETBOUND_ANNOTATION_TYPE;
            return annIt(a)->bound();
        }
     */
    bool has_id()
    {
        AnnotationKind a = AnnotationKind_def::Type::ID_ANNOTATION_TYPE;
        return find_annotation(a);
    }

    uint32_t get_id()
    {
        AnnotationKind a = AnnotationKind_def::Type::ID_ANNOTATION_TYPE;

        // creting a generic IdAnnotation that will be filled by annotation_iterator()
        xtypes::IdAnnotation ida(0);
        if (not annotation_iterator(a, ida))
        {
            throw IllegalOperationError("No Id Annotation found");
        }
        return ida->id();
    }

private:

    std::string name_;
    xtypes::DynamicType dt_;
    std::vector<xtypes::Annotation> ann_;
};

} //namespace detail
} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_MEMBER_TYPE_HPP_
