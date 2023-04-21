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

/*!
 * @file custom_allocators.hpp
 */

#ifndef FASTRTPS_UTILS_CUSTOM_ALLOCATORS_HPP_
#define FASTRTPS_UTILS_CUSTOM_ALLOCATORS_HPP_

#include <memory>
#include <type_traits>

namespace eprosima {
namespace detail {

/**
 * The purpose of this class is providing an allocator suitable for factory class interaction.
 * @tparam T Instantiable class to allocate.
 * @tparam B Class factory which receives the callbacks.
 * @tparam stateful Decides if callbacks are instance methods or static ones.
 *
 * On object construction destruction callbacks would be triggered on the linked factory:
 * @code
 *  static? void B::after_construction(T* p)
 *
 *  static? void B::before_destruction(T* p)
 * @endcode @remarks
 *
 * The linking with the factory may be:
 *  + Stateful: The factory instance must outlive its children.
 *  + Stateless: The factory class must provide static callback methods.
 *
 * By using this allocator with `std::allocate_shared` is posible to:
 *  + get the object and its reference counting block int the same allocation block.
 *  + provide a custom deleter for the object.
 */
template<class T, class B, bool stateful>
class BuilderAllocator
    : public std::allocator<T>
{
    B& factory_; /*!< Reference to the factory class which receives the callbacks */

public:

    /**
     * Class to hint STL allocators.
     * @tparam Other class to be allocated
     */
    template <class Other>
    struct rebind
    {
        using other = typename std::conditional<
            std::is_same<Other, T>::value,
            BuilderAllocator,
            std::allocator<Other>>::type;
    };

    /**
     * Stateful constructor.
     * @param [in] factory The object that provides the callbacks
     */
    BuilderAllocator(
            B& factory)
        : factory_(factory)
    {
    }

    /**
     * Callback to be invoked after construction.
     * @param [in] p object just constructed
     */
    void after_construction(
            T* p)
    {
        // delegate into the reference
        factory_.after_construction(p);
    }

    /**
     * Callback to be invoked before destruction.
     * @param [in] p object to be destructed
     */
    void before_destruction(
            T* p)
    {
        // delegate into the reference
        factory_.before_destruction(p);
    }

};

/**
 * Specialization of @ref BuilderAllocator for stateless factories
 * @tparam T instantiable class to allocate.
 * @tparam B stateless class factory which receives the callbacks.
 */
template<class T, class B>
class BuilderAllocator<T, B, false>
    : public std::allocator<T>
{
public:

    /**
     * Class to hint STL allocators
     * @tparam Other class to be allocated
     */
    template <class Other>
    struct rebind
    {
        using other = typename std::conditional<
            std::is_same<Other, T>::value,
            BuilderAllocator,
            std::allocator<Other>>::type;
    };

    /**
     * static callback to be invoked after construction.
     * @param [in] p object just constructed
     */
    void after_construction(
            T* p)
    {
        // delegate into static method
        B::after_construction(p);
    }

    /**
     * Static callback to be invoked before destruction.
     * @param [in] p object to be destructed
     */
    void before_destruction(
            T* p)
    {
        // delegate into static method
        B::before_destruction(p);
    }

};

} // namespace detail
} // eprosima namespace

/**
 * Specialization of `std::allocator_traits` for `BuilderAllocator`.
 * @tparam T instantiable class to allocate.
 * @tparam B class factory which receives the callbacks.
 * @tparam stateful decides if callbacks are instance methods or static ones.
 *
 * The STL framework will use the static methods define here for T initialization and disposal.
 * @remarks The allocator_traits must be specialized in the outer namespace
 * [see N3730](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3730.html)
 */
template<class T, class B, bool state>
struct std::allocator_traits<eprosima::detail::BuilderAllocator<T, B, state>>
{
    using BA = eprosima::detail::BuilderAllocator<T, B, state>;

    template <class Other>
    using rebind_alloc = typename BA::template rebind<Other>::other;

    template <class ... Types>
    static void construct(
            BA& alloc,
            T* const p,
            Types&&... args)
    {
        std::allocator_traits<std::allocator<T>>::construct(
            alloc,
            p,
            std::forward<Types>(args)...);
        alloc.after_construction(p);
    }

    static void destroy(
            BA& alloc,
            T* p)
    {
        alloc.before_destruction(p);
        std::allocator_traits<std::allocator<T>>::destroy(alloc, p);
    }

};

#endif /* FASTRTPS_UTILS_CUSTOM_ALLOCATORS_HPP_ */
