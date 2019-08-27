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

#include <dds/core/xtypes/MemberType.hpp>

namespace dds {
namespace core {
namespace xtypes {

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
TMemberType<DELEGATE, OTHER_DELEGATE>::TMemberType(
        const std::string& name,
        const TDynamicType<OTHER_DELEGATE>& type)
{
    (void) name;
    (void) type;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
TMemberType<DELEGATE, OTHER_DELEGATE>::TMemberType(
        const std::string& name,
        const dds::core::xtypes::DynamicType& type,
        const Annotation& annotation)
{
    (void) name;
    (void) type;
    (void) annotation;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
TMemberType<DELEGATE, OTHER_DELEGATE>::TMemberType(
        const std::string& name,
        const TDynamicType<OTHER_DELEGATE>& type,
        const std::vector<Annotation>& annotations)
{
    (void) name;
    (void) type;
    (void) annotations;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
template <typename AnnotationIter>
TMemberType<DELEGATE, OTHER_DELEGATE>::TMemberType(
        const std::string& name,
        const TDynamicType<OTHER_DELEGATE>& type,
        const AnnotationIter& begin,
        const AnnotationIter& end)
{
    (void) name;
    (void) type;
    (void) begin;
    (void) end;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
const std::string& TMemberType<DELEGATE, OTHER_DELEGATE>::name() const
{

}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
const dds::core::xtypes::TDynamicType<OTHER_DELEGATE>& TMemberType<DELEGATE, OTHER_DELEGATE>::type() const
{
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
TMemberType<DELEGATE, OTHER_DELEGATE> TMemberType<DELEGATE, OTHER_DELEGATE>::add_annotation(
        const Annotation& annotation)
{
    (void) annotation;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
TMemberType<DELEGATE, OTHER_DELEGATE> TMemberType<DELEGATE, OTHER_DELEGATE>::remove_annotation(
        const Annotation& annotation)
{
    (void) annotation;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
bool TMemberType<DELEGATE, OTHER_DELEGATE>::is_optional() const
{
    return false;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
bool TMemberType<DELEGATE, OTHER_DELEGATE>::is_shared() const
{
    return false;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
bool TMemberType<DELEGATE, OTHER_DELEGATE>::is_key() const
{
    return false;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
bool TMemberType<DELEGATE, OTHER_DELEGATE>::is_must_understand() const
{
    return false;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
bool TMemberType<DELEGATE, OTHER_DELEGATE>::is_bitset() const
{
    return false;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
bool TMemberType<DELEGATE, OTHER_DELEGATE>::has_bitbound() const
{
    return false;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
int32_t TMemberType<DELEGATE, OTHER_DELEGATE>::get_bitbound() const
{
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
bool TMemberType<DELEGATE, OTHER_DELEGATE>::has_id() const
{
    return false;
}

template<
        typename DELEGATE,
        typename OTHER_DELEGATE>
int32_t TMemberType<DELEGATE, OTHER_DELEGATE>::get_id() const
{

}


} //namespace xtypes
} //namespace core
} //namespace dds