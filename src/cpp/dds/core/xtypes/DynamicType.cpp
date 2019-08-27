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

#include <dds/core/xtypes/DynamicType.hpp>

namespace dds {
namespace core {
namespace xtypes {

template<typename DELEGATE>
const std::string& TDynamicType<DELEGATE>::name() const
{
}

template<typename DELEGATE>
TypeKind TDynamicType<DELEGATE>::kind() const
{
}

template<typename DELEGATE>
const std::vector<Annotation>& TDynamicType<DELEGATE>::annotations() const
{
}

template<typename DELEGATE>
bool TDynamicType<DELEGATE>::operator ==(
        const TDynamicType& that) const
{
    (void) that;
}

template<typename DELEGATE>
TDynamicType<DELEGATE>::TDynamicType(
            const std::string& name,
            TypeKind kind)
{
    (void) name;
    (void) kind;
}

template<typename DELEGATE>
TDynamicType<DELEGATE>::TDynamicType(
            const std::string& name,
            TypeKind kind,
            const Annotation& annotation)
{
    (void) name;
    (void) kind;
    (void) annotation;
}

template<typename DELEGATE>
TDynamicType<DELEGATE>::TDynamicType(
            const std::string& name,
            TypeKind kind,
            const std::vector<Annotation>& annotations)
{
    (void) name;
    (void) kind;
    (void) annotations;
}

template<typename DELEGATE>
template<typename AnnotationIter>
TDynamicType<DELEGATE>::TDynamicType(
        const std::string& name,
        TypeKind kind,
        const AnnotationIter& begin,
        const AnnotationIter& end)
{
    (void) name;
    (void) kind;
    (void) begin;
    (void) end;
}


template<typename T>
bool is_primitive_type(
        const TDynamicType<T>& t)
{
    (void) t;
    return false;
}

template<typename T>
bool is_constructed_type(
        const TDynamicType<T>& t)
{
    (void) t;
    return false;
}

template<typename T>
bool is_collection_type(
        const TDynamicType<T>& t)
{
    (void) t;
    return false;
}

template<typename T>
bool is_aggregation_type(
        const TDynamicType<T>& t)
{
    (void) t;
    return false;
}


} //namespace xtypes
} //namespace core
} //namespace dds
