// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file ObjectPool.hpp
 *
 */

#ifndef FASTDDS_UTILS_COLLECTIONS_OBJECTPOOL_HPP_
#define FASTDDS_UTILS_COLLECTIONS_OBJECTPOOL_HPP_

#include <memory>
#include <type_traits>
#include <vector>

namespace eprosima {
namespace fastdds {

/**
 * A generic pool of objects.
 *
 * This template class holds a circular queue of fixed size. Pushing a new element to a full queue
 * will result in an error.
 *
 * @tparam _Ty                 Element type.
 * @tparam _Alloc              Allocator to use on the underlying collection type, defaults to std::allocator<_Ty>.
 * @tparam _Collection         Underlying collection type, defaults to std::vector<_Ty, _Alloc>
 *
 * @ingroup UTILITIES_MODULE
 */
template <
    typename _Ty,
    typename _Alloc = std::allocator<_Ty>,
    typename _Collection = std::vector<_Ty, _Alloc>>
struct ObjectPool final
{
    using allocator_type = _Alloc;
    using value_type = _Ty;

    /**
     * Construct an ObjectPool.
     */
    ObjectPool(
            const allocator_type& alloc = allocator_type())
        : collection_(alloc)
    {
    }

    const _Collection& collection() const noexcept
    {
        return collection_;
    }

    _Collection& collection() noexcept
    {
        return collection_;
    }

    template<typename _DefaultGetter>
    value_type get(
            _DefaultGetter _Default)
    {
        if (collection_.empty())
        {
            return _Default();
        }

        value_type ret = collection_.back();
        collection_.pop_back();
        return ret;
    }

    template <class ... _Valty>
    void put(
            _Valty&&... _Val)
    {
        collection_.emplace_back(std::forward<_Valty>(_Val)...);
    }

private:

    _Collection collection_;

};

}  // namespace fastdds
}  // namespace eprosima

#endif /* FASTDDS_UTILS_COLLECTIONS_OBJECTPOOL_HPP_ */
