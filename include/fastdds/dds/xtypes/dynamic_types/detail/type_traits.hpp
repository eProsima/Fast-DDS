// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_DETAIL__TYPE_TRAITS_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_DETAIL__TYPE_TRAITS_HPP

#include <memory>

#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

template<typename T>
struct traits
{
    using ref_type = typename ::std::shared_ptr<T>;
    using weak_ref_type = typename ::std::weak_ptr<T>;

    template<typename _Tp, typename = typename
            std::enable_if<std::is_base_of<T, _Tp>::value>::type>
    inline static std::shared_ptr<_Tp> narrow (
            ref_type obj)
    {
        return std::dynamic_pointer_cast<_Tp>(obj);
    }

    FASTDDS_EXPORTED_API static std::shared_ptr<T> make_shared();

};

template<typename T>
struct object_traits
{
    using ref_type = typename ::std::shared_ptr<T>;
    using weak_ref_type = typename ::std::weak_ptr<T>;

    template<typename _Tp, typename = typename
            std::enable_if<std::is_base_of<T, _Tp>::value>::type>
    inline static std::shared_ptr<_Tp> narrow (
            ref_type obj)
    {
        return std::dynamic_pointer_cast<_Tp>(obj);
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_DETAIL__TYPE_TRAITS_HPP

