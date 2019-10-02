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
#ifndef EPROSIMA_DDS_CORE_XTYPES_DETAIL_DYNAMIC_TYPE_HPP_
#define EPROSIMA_DDS_CORE_XTYPES_DETAIL_DYNAMIC_TYPE_HPP_

#include <dds/core/xtypes/Annotation.hpp>

#include <dds/core/xtypes/TypeKind.hpp> // Allow this include from detail because is only an enum

#include <string>
#include <vector>
#include <functional>

namespace dds {
namespace core {
namespace xtypes {
namespace detail {

class DynamicType
{
public:
    DynamicType(
            const std::string& name,
            TypeKind kind)
        : name_(name)
        , kind_(kind)
    {}

    DynamicType(
            const std::string& name,
            TypeKind kind,
            const xtypes::Annotation& annotation)
        : name_(name)
        , kind_(kind)
    {
        annotations_.push_back((annotation));
    }

    DynamicType(
            const std::string& name,
            TypeKind kind,
            const std::vector<xtypes::Annotation>& annotations)
        : name_(name)
        , kind_(kind)
        , annotations_(annotations)
    {
    }

    template<typename AnnotationIter>
    DynamicType(
            const std::string& name,
            TypeKind kind,
            const AnnotationIter& begin,
            const AnnotationIter& end)
        : name_(name)
        , kind_(kind)
    {
        annotations_ = std::vector<xtypes::Annotation>(begin, end);
    }

    const std::string& name() const { return name_; }
    const TypeKind& kind() const { return kind_; }
    const std::vector<xtypes::Annotation>& annotations() const { return annotations_; }

    void name(const std::string& name) { name_ = name; }
    void kind(const TypeKind& kind) { kind_ = kind; }
    void annotations( const std::vector<xtypes::Annotation>& annotations) { annotations_ = annotations; }

    void add_annotation(
            const xtypes::Annotation& annotation)
    {
        annotations_.push_back(std::ref(annotation));
    }

    template<typename AnnotationIter>
    void annotations(
            AnnotationIter begin,
            AnnotationIter end)
    {
        annotations_ = std::vector<xtypes::Annotation>(begin, end);
    }

private:
    std::string name_;
    TypeKind kind_;
    std::vector<xtypes::Annotation> annotations_;
};

} //namespace detail
} //namespace xtypes
} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_XTYPES_DETAIL_DYNAMIC_TYPE_HPP_
