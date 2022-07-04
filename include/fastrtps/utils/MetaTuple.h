// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef DBQUEUE_H
#define DBQUEUE_H

#include <tuple>
#include <cstdint>

namespace eprosima {
namespace fastrtps {
namespace detail_cpp14 {

template <typename T>
using remove_ref_t = typename std::remove_reference<T>::type;

//////////////// TupleIndex auxiliar meta-function to extract the index of a type in a tuple, due to pre C++14 (std::get<T>(tuple))

// Auxiliar meta-function declaration
template <std::size_t Idx, typename ContainedT, typename ParametersTupleT>
struct TupleIndex;

//! Recursion end case (type not found)
template <std::size_t Idx, typename ContainedT>
struct TupleIndex< Idx, ContainedT, std::tuple<> >
{
    static constexpr std::size_t value = Idx;
};

//! Recursion general case
template <std::size_t Idx, typename ContainedT, typename TupleArgT, typename... TupleArgsTs>
struct TupleIndex< Idx, ContainedT, std::tuple<TupleArgT, TupleArgsTs...> >
{
    static constexpr std::size_t value =
        std::is_same<ContainedT, remove_ref_t<TupleArgT>>::value ? Idx : TupleIndex<Idx + 1, ContainedT, std::tuple<TupleArgsTs...> >::value;
};

//////////////// TupleContains auxiliar meta-function to check whether ContainedT is contained within ParametersTupleT tuple

//! Compile-time predicate to evaluate whether a type is contained in a std::tuple
template <typename ContainedT, typename ParametersTupleT>
struct TupleContains;

//! Recursion end-case
template <typename ContainedT>
struct TupleContains< ContainedT, std::tuple<> >
{
    static constexpr bool value = false;
};

//! Recursion general case
template <typename ContainedT, typename TupleArgT, typename... TupleArgsTs>
struct TupleContains< ContainedT, std::tuple<TupleArgT, TupleArgsTs...> >
{
    static constexpr bool value = std::is_same<ContainedT, remove_ref_t<TupleArgT>>::value || TupleContains<ContainedT, std::tuple<TupleArgsTs...>>::value;
};

//////////////// LengthCount metafunction to count the required serialized bytes given a set of parameter types

template <typename... TupleArgsTs>
struct LengthCount;

//! Recursion end-case
template <>
struct LengthCount<> {
    static constexpr uint32_t size = 0u;
};

//! Recursion general case
template <typename ParameterT, typename... ParametersTs>
struct LengthCount <ParameterT, ParametersTs...>
{
    // sizeof(ParameterId_t) + sizeof(parameter length) + ParameterLength + rest
    static constexpr uint32_t size = sizeof(uint16_t) + sizeof(uint16_t) + ParameterT::LENGTH + LengthCount<ParametersTs...>::size;
};

} // namespace detail_cpp14
} // namespace fastrtps
} // namespace eprosima

#endif
