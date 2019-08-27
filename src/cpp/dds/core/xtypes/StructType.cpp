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

#include <dds/core/xtypes/StructType.hpp>

namespace dds {
namespace core {
namespace xtypes {

template<typename DELEGATE>
TStructForwardDeclaration<DELEGATE>::TStructForwardDeclaration(
        const std::string& name)
{
    (void) name;
}

template<typename DELEGATE>
TStructType<DELEGATE>::TStructType(
        const std::string& name)
{
    (void) name;
}

template<typename DELEGATE>
TStructType<DELEGATE>::TStructType(
    const std::string& name,
    const TStructType& parent,
    const std::vector<MemberType>& members)
{
    (void) name;
    (void) parent;
    (void) members;
}

template<typename DELEGATE>
template<typename MemberIter>
TStructType<DELEGATE>::TStructType(
    const std::string& name,
    const TStructType& parent,
    const MemberIter& begin,
    const MemberIter& end)
{
    (void) name;
    (void) parent;
    (void) begin;
    (void) end;
}

template<typename DELEGATE>
TStructType<DELEGATE>::TStructType(
    const std::string& name,
    const TStructType& parent,
    const std::vector<MemberType>& members,
    const Annotation& annotation)
{
    (void) name;
    (void) parent;
    (void) members;
    (void) annotation;
}

template<typename DELEGATE>
TStructType<DELEGATE>::TStructType(
    const std::string& name,
    const TStructType& parent,
    const std::vector<MemberType>& members,
    const std::vector<Annotation>& annotations)
{
    (void) name;
    (void) parent;
    (void) members;
    (void) annotations;
}

template<typename DELEGATE>
template<
        typename AnnotationIter,
        typename MemberIter>
TStructType<DELEGATE>::TStructType(
    const std::string& name,
    const TStructType<DELEGATE>& parent,
    const MemberIter& member_begin,
    const MemberIter& member_end,
    const AnnotationIter& annotation_begin,
    const AnnotationIter& annotation_end)
{
    (void) name;
    (void) parent;
    (void) member_begin;
    (void) member_end;
    (void) annotation_begin;
    (void) annotation_end;
}


template<typename DELEGATE>
TStructType<DELEGATE> TStructType<DELEGATE>::parent() const
{
}

template<typename DELEGATE>
const std::vector<MemberType>& TStructType<DELEGATE>::members() const
{
}

template<typename DELEGATE>
const MemberType& TStructType<DELEGATE>::member(
        uint32_t id) const
{
    (void) id;
}

template<typename DELEGATE>
const MemberType& TStructType<DELEGATE>::member(
        const std::string& name) const
{
    (void) name;
}

template<typename DELEGATE>
const std::vector<Annotation>& TStructType<DELEGATE>::annotations() const
{
}

template<typename DELEGATE>
TStructType<DELEGATE> TStructType<DELEGATE>::add_member(
        const MemberType& member) const
{
    (void) member;
}

template<typename DELEGATE>
TStructType<DELEGATE> TStructType<DELEGATE>::remove_member(
        const MemberType& member) const
{
    (void) member;
}

template<typename DELEGATE>
TStructType<DELEGATE> TStructType<DELEGATE>::add_annotation(
        const Annotation& annotation) const
{
    (void) annotation;
}

template<typename DELEGATE>
TStructType<DELEGATE> TStructType<DELEGATE>::remove_annotation(
        const Annotation& annotation) const
{
    (void) annotation;
}


} //namespace xtypes
} //namespace core
} //namespace dds
