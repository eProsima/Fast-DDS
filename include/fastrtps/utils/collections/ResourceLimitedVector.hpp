// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ResourceLimitedVector.hpp
 *
 */

#ifndef FASTRTPS_UTILS_COLLECTIONS_RESOURCELIMITEDVECTOR_HPP_
#define FASTRTPS_UTILS_COLLECTIONS_RESOURCELIMITEDVECTOR_HPP_

#include "ResourceLimitedContainerConfig.hpp"

#include <assert.h>
#include <algorithm>
#include <type_traits>
#include <vector>

namespace eprosima {
namespace fastrtps {

/**
 * Resource limited wrapper of std::vector.
 *
 * This template class holds an unordered collection of elements using a std::vector or a replacement.
 * It makes use of a \ref ResourceLimitsConfig to setup the allocation behaviour regarding the number of
 * elements in the collection.
 *
 * It features linear increment of the capacity, initial preallocation, and maximum number of elements control.
 *
 * @tparam _Ty                 Element type.
 * @tparam _KeepOrderEnabler   Indicates if element order should be kept when removing items,
 *                             defaults to std::false_type.
 * @tparam _Alloc              Allocator to use on the underlying collection type, defaults to std::allocator<_Ty>.
 * @tparam _Collection         Type used to store the collection of items, defaults to std::vector<_Ty, _Alloc>.
 *
 * @ingroup UTILITIES_MODULE
 */
template <
    typename _Ty, 
    typename _KeepOrderEnabler = std::false_type,
    typename _Alloc = std::allocator<_Ty>, 
    typename _Collection = std::vector<_Ty, _Alloc> >
class ResourceLimitedVector
{
public:

    using collection_type = _Collection;
    using value_type = _Ty;
    using allocator_type = _Alloc;
    using pointer = typename collection_type::pointer;
    using const_pointer = typename collection_type::const_pointer;
    using reference = typename collection_type::reference;
    using const_reference = typename collection_type::const_reference;
    using size_type = typename collection_type::size_type;
    using difference_type = typename collection_type::difference_type;
    using iterator = typename collection_type::iterator;
    using const_iterator = typename collection_type::const_iterator;
    using reverse_iterator = typename collection_type::reverse_iterator;
    using const_reverse_iterator = typename collection_type::reverse_iterator;
    using size_type = typename collection_type::size_type;

    /**
     * Construct a ResourceLimitedVector.
     *
     * This constructor receives a \ref ResourceLimitsConfig to setup the allocation behaviour regarding the 
     * number of elements in the collection.
     *
     * The cfg parameter indicates the initial number to be reserved, the maximum number of items allowed,
     * and the capacity increment value.
     *
     * @param cfg     Resource limits configuration to use.
     * @param alloc   Allocator object. Forwarded to collection constructor.
     */
    ResourceLimitedVector(
            ResourceLimitedContainerConfig cfg = {}, 
            const allocator_type& alloc = allocator_type())
        : configuration_(cfg)
        , collection_(alloc)
    {
        collection_.reserve(cfg.initial);
    }

    /**
     * Add element at the end.
     *
     * Adds a new element at the end of the vector, after its current last element. 
     * The content of val is copied to the new element.
     *
     * @param val   Value to be copied to the new element.
     *
     * @return pointer to the new element, nullptr if resource limit is reached.
     */
    pointer push_back(const value_type& val)
    {
        return emplace_back(val);
    }

    /**
     * Add element at the end.
     *
     * Adds a new element at the end of the vector, after its current last element. 
     * The content of val is moved to the new element.
     *
     * @param val   Value to be moved to the new element.
     *
     * @return pointer to the new element, nullptr if resource limit is reached.
     */
    pointer push_back(value_type&& val)
    {
        return emplace_back(std::move(val));
    }

    /**
     * Construct and insert element at the end.
     *
     * Inserts a new element at the end of the vector, right after its current last element. 
     * This new element is constructed in place using args as the arguments for its constructor.
     *
     * @param args   Arguments forwarded to construct the new element.
     *
     * @return pointer to the new element, nullptr if resource limit is reached.
     */
    template<typename ... Args>
    pointer emplace_back(Args&& ... args)
    {
        if (!ensure_capacity())
        {
            // Indicate error by returning null pointer
            return nullptr;
        }

        // Keep record of insertion position
        size_type previous_size = collection_.size();

        // Construct new element at the end of the collection
        collection_.emplace_back(args...);

        // Return pointer to newly created element
        return &collection_[previous_size];
    }

    /**
     * Remove element.
     * 
     * Removes the first element in the vector that compares equal to val.
     * All iterators may become invalidated if this method returns true.
     *
     * @param val   Value to be compared.
     *
     * @return true if an element was removed, false otherwise.
     */
    bool remove(const value_type& val)
    {
        iterator it = std::find(collection_.begin(), collection_.end(), val);
        return remove(it);
    }

    /**
     * Remove element.
     * 
     * Removes the first element in the vector for which pred returns true.
     * All iterators may become invalidated if this method returns true.
     *
     * @param pred   Unary function that accepts an element in the range as argument and returns a value
     *               convertible to bool.
     *               The value returned indicates whether the element is considered a match in the context
     *               of this function.
     *               The function shall not modify its argument.
     *               This can either be a function pointer or a function object.
     *
     * @return true if an element was removed, false otherwise.
     */
    template<class UnaryPredicate>
    bool remove_if(UnaryPredicate pred)
    {
        iterator it = std::find_if(collection_.begin(), collection_.end(), pred);
        return remove(it);
    }

    /**
     * Assign vector content.
     * 
     * Assigns new contents to the vector, replacing its current contents, and modifying its size accordingly.
     *
     * @param first, last   Input iterators to the initial and final positions in a sequence.
     *                      The range used is [first,last), which includes all the elements between first and last,
     *                      including the element pointed by first but not the element pointed by last.
     *                      The function template argument InputIterator shall be an input iterator type that points
     *                      to elements of a type from which value_type objects can be constructed.
     *                      If the size of this range is greater than the maximum number of elements allowed on the
     *                      resource limits configuration, the elements exceeding that maximum will be silently
     *                      discarded.
     */
    template <class InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        size_type n = std::distance(first, last);
        n = std::min(n, configuration_.maximum);
        collection_.assign(first, first + n);
    }

    /**
     * Assign vector content.
     * 
     * Assigns new contents to the vector, replacing its current contents, and modifying its size accordingly.
     *
     * @param n     New size for the container.
     *              Will be truncated if greater than the maximum allowed on the resource limits configuration.
     * @param val   Value to fill the container with.
     *              Each of the n elements in the container will be initialized to a copy of this value.
     */
    void assign(size_type n, const value_type& val)
    {
        n = std::min(n, configuration_.maximum);
        collection_.assign(n, val);
    }

    /**
     * Assign vector content.
     * 
     * Assigns new contents to the vector, replacing its current contents, and modifying its size accordingly.
     *
     * @param il   An initializer_list object. 
     *             The compiler will automatically construct such objects from initializer list declarators.
     *             Member type value_type is the type of the elements in the container.
     *             If the size of this list is greater than the maximum number of elements allowed on the
     *             resource limits configuration, the elements exceeding that maximum will be silently discarded.
     */
    void assign(std::initializer_list<value_type> il)
    {
        size_type n = std::min(il.size(), configuration_.maximum);
        collection_.assign(il.begin(), il.begin() + n);
    }

    iterator begin() noexcept { return collection_.begin(); }
    const_iterator begin() const noexcept { return collection_.begin(); }
    iterator end() noexcept { return collection_.end(); }
    const_iterator end() const noexcept { return collection_.end(); }

    size_type capacity() const noexcept { return collection_.capacity(); }
    size_type size() const noexcept { return collection_.size(); }
    bool empty() const noexcept { return collection_.empty(); }
    void clear() { collection_.clear(); }

    /**
     * Const cast to underlying collection.
     *
     * Useful to easy integration on old APIs where a traditional container was used.
     *
     * @return const reference to the underlying collection.
     */
    operator const collection_type& () const noexcept { return collection_; }

private:
    ResourceLimitedContainerConfig configuration_;
    collection_type collection_;

    /**
     * Make room for one item.
     *
     * Tries to ensure that a new item can be added to the container.
     *
     * @return true if there is room for a new item, false if resource limit is reached.
     */
    bool ensure_capacity()
    {
        size_type size = collection_.size();
        size_type cap = collection_.capacity();
        if (size == cap)
        {
            // collection is full, check resource limit
            if (cap < configuration_.maximum)
            {
                // increase collection capacity
                assert(configuration_.increment > 0);
                cap += configuration_.increment;
                cap = std::min(cap, configuration_.maximum);
                collection_.reserve(cap);
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    bool remove(iterator it)
    {
        if (it != collection_.end())
        {
            do_remove(it);
            return true;
        }

        return false;
    }

    template <typename Enabler = _KeepOrderEnabler>
    typename std::enable_if<!Enabler::value, void>::type do_remove(iterator it)
    {
        // Copy last element into the element being removed
        if (collection_.size() > 1)
            *it = collection_.back();

        // Then drop last element
        collection_.pop_back();
    }

    template <typename Enabler = _KeepOrderEnabler>
    typename std::enable_if<Enabler::value, void>::type do_remove(iterator it)
    { 
        collection_.erase(it);
    }
};

}  // namespace fastrtps
}  // namespace eprosima

#endif /* FASTRTPS_UTILS_COLLECTIONS_RESOURCELIMITEDVECTOR_HPP_ */
