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

#include <dds/core/xtypes/UnionType.hpp>

namespace dds{
namespace core{
namespace xtypes{

template<typename DELEGATE>
TUnionForwardDeclaration<DELEGATE>::TUnionForwardDeclaration(
            const std::string& name)
{
    (void) name;
}

template<
        typename T,
        template<typename> class DELEGATE>
TUnionType<T, DELEGATE>::TUnionType(
    const std::string& name,
    const TPrimitiveType<T, DELEGATE<T>>& discriminator_type,
    const std::vector<UnionCase<T>>& cases)
{
    (void) name;
    (void) discriminator_type;
    (void) cases;
}

template<
        typename T,
        template<typename> class DELEGATE>
TUnionType<T, DELEGATE>::TUnionType(
    const std::string& name,
    const TPrimitiveType<T, DELEGATE<T> >& discriminator_type,
    const std::vector<UnionCase<T>>& cases,
    const Annotation& annotation)
{
    (void) name;
    (void) discriminator_type;
    (void) cases;
    (void) annotation;
}

template<
        typename T,
        template<typename> class DELEGATE>
TUnionType<T, DELEGATE>::TUnionType(
    const std::string& name,
    const TPrimitiveType<T, DELEGATE<T>>& discriminator_type,
    const std::vector<UnionCase<T>>& cases,
    const std::vector<Annotation>& annotations)
{
    (void) name;
    (void) discriminator_type;
    (void) cases;
    (void) annotations;
}

template<
        typename T,
        template<typename> class DELEGATE>
const std::vector<UnionCase<T>>& TUnionType<T, DELEGATE>::members() const
{
}

template<
        typename T,
        template<typename> class DELEGATE>
const MemberType& TUnionType<T, DELEGATE>::member(
        uint32_t id) const
{
    (void) id;
}

template<
        typename T,
        template<typename> class DELEGATE>
const MemberType& TUnionType<T, DELEGATE>::member(
        const std::string& name) const
{
    (void) name;
}

template<
        typename T,
        template<typename> class DELEGATE>
const std::vector<Annotation>& TUnionType<T, DELEGATE>::annotations() const
{
}

template<
        typename T,
        template<typename> class DELEGATE>
TUnionType<T, DELEGATE> TUnionType<T, DELEGATE>::add_member(
        const UnionCase<T>& member) const
{
    (void) member;
}

template<
        typename T,
        template<typename> class DELEGATE>
TUnionType<T, DELEGATE> TUnionType<T, DELEGATE>::remove_member(
        const UnionCase<T>& member) const
{
    (void) member;
}

template<
        typename T,
        template<typename> class DELEGATE>
TUnionType<T, DELEGATE> TUnionType<T, DELEGATE>::add_annotation(
        const Annotation& annotation) const
{
    (void) annotation;
}

template<
        typename T,
        template<typename> class DELEGATE>
TUnionType<T, DELEGATE> TUnionType<T, DELEGATE>::remove_annotation(
        const Annotation& annotation) const
{
    (void) annotation;
}

} //namespace xtypes
} //namespace core
} //namespace dds
