// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file sorted_vector_insert.hpp
 */

#ifndef SRC_CPP_UTILS_COLLECTIONS_SORTED_VECTOR_INSERT_HPP_
#define SRC_CPP_UTILS_COLLECTIONS_SORTED_VECTOR_INSERT_HPP_

#include <algorithm>
#include <functional>

namespace eprosima {
namespace utilities {
namespace collections {

/**
 * @brief Insert item into sorted vector-like collection
 *
 * @tparam CollectionType     Type of the collection to be modified.
 * @tparam ValueType          Type of the item to insert. The collection should support to insert a value of this type.
 * @tparam LessThanPredicate  Predicate that performs ValueType < CollectionType::value_type comparison.
 *
 * @param [in,out] collection The collection to be modified.
 * @param [in]     item       The item to be inserted.
 * @param [in]     pred       The predicate to use for comparisons.
 */
template<
    typename CollectionType,
    typename ValueType,
    typename LessThanPredicate = std::less<ValueType>>
void sorted_vector_insert(
        CollectionType& collection,
        const ValueType& item,
        const LessThanPredicate& pred = LessThanPredicate())
{
    // Insert at the end by default
    auto it = collection.end();

    // Find insertion position when item is less than last element in collection
    if (!collection.empty() && pred(item, *collection.rbegin()))
    {
        it = std::lower_bound(collection.begin(), collection.end(), item, pred);
    }
    collection.insert(it, item);
}

} // namespace collections
} // namespace utilities
} // namespace eprosima

#endif // SRC_CPP_UTILS_COLLECTIONS_SORTED_VECTOR_INSERT_HPP_
