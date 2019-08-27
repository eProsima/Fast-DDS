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

#include <dds/core/xtypes/Annotation.hpp>

namespace dds {
namespace core {
namespace xtypes {

template<typename DELEGATE>
TypeKind TAnnotation<DELEGATE>::kind() const
{
}

template<typename DELEGATE>
TAnnotation<DELEGATE>::TAnnotation(
        const TypeKind& kind)
{
	(void) kind;
}

template<typename DELEGATE>
TIdAnnotation<DELEGATE>::TIdAnnotation(
        uint32_t id)
{
	(void) id;
}

template<typename DELEGATE>
uint32_t TIdAnnotation<DELEGATE>::id() const
{
}

template<typename DELEGATE>
TKeyAnnotation<DELEGATE>::TKeyAnnotation()
{
}

template<typename DELEGATE>
TSharedAnnotation<DELEGATE>::TSharedAnnotation()
{
}

template<typename DELEGATE>
TNestedAnnotation<DELEGATE>::TNestedAnnotation()
{
}

template<typename DELEGATE>
TExtensibilityAnnotation<DELEGATE>::TExtensibilityAnnotation(
        ExtensibilityKind xkind)
{
	(void) xkind;
}

template<typename DELEGATE>
ExtensibilityKind TExtensibilityAnnotation<DELEGATE>::extensibility_kind() const
{
}

template<typename DELEGATE>
TMustUnderstandAnnotation<DELEGATE>::TMustUnderstandAnnotation()
{
}

template<typename DELEGATE>
TVerbatimAnnotation<DELEGATE>::TVerbatimAnnotation(
        const std::string& text)
{
	(void) text;
}

template<typename DELEGATE>
const std::string& TVerbatimAnnotation<DELEGATE>::verbatim_text() const
{
}

template<typename DELEGATE>
TBitsetAnnotation<DELEGATE>::TBitsetAnnotation()
{
}

template<typename DELEGATE>
TBitBoundAnnotation<DELEGATE>::TBitBoundAnnotation(
        uint32_t bound)
{
	(void) bound;
}

namespace annotation {

// These functions can be used to get cached instances,
// to avoid the proliferation of small annotation objects.
IdAnnotation id(
        uint32_t);

KeyAnnotation key();

SharedAnnotation shared();

NestedAnnotation nested();

ExtensibilityAnnotation extensibility(
        ExtensibilityKind kind);

ExtensibilityAnnotation get_final();

ExtensibilityAnnotation extensible();

ExtensibilityAnnotation get_mutable();

MustUnderstandAnnotation must_understand();

VerbatimAnnotation verbatim(
        const std::string& text);

BitsetAnnotation bitset();

BitsetAnnotation bit_bound(
        uint32_t bound);

} //namespace annotation
} //namespace xtypes
} //namespace core
} //namespace dds
