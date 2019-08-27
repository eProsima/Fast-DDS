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

#ifndef EPROSIMA_DDS_CORE_DETAIL_REF_TRAITS_HPP_
#define EPROSIMA_DDS_CORE_DETAIL_REF_TRAITS_HPP_

#include <dds/core/types.hpp>     //null
#include <dds/core/Exception.hpp> //InvalidDowncastError

#include <memory>
#include <type_traits>

namespace dds {
namespace core {

template<
        typename T1,
        typename T2>
struct is_base_of : public ::std::is_base_of<T1, T2> { };

template<
        typename T1,
        typename T2>
struct is_same : public ::std::is_same<T1, T2> { };

template<typename T>
struct smart_ptr_traits {
    typedef ::std::shared_ptr<T>  ref_type;
    typedef ::std::weak_ptr<T>    weak_ref_type;
};

template<
        typename TO,
        typename FROM>
TO polymorphic_cast(
        FROM& from) {

    typename TO::DELEGATE_REF_T dr = ::std::dynamic_pointer_cast<typename TO::DELEGATE_T>(from.delegate());

    TO to(dr);
    if (to == null)
    {
        throw InvalidDowncastError("Attempted invalid downcast.");
    }
    return to;
}

} //namespace core
} //namespace dds

#endif //EPROSIMA_DDS_CORE_DETAIL_REF_TRAITS_HPP_
