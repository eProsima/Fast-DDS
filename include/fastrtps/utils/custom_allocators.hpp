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

#include <atomic>
#include <cassert>
#include <memory>
#include <type_traits>

#if defined(__has_include) && __has_include(<version>)
#   include <version>
#endif // if defined(__has_include) && __has_include(<version>)

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

namespace eprosima {
// eprosima namespace
namespace detail {

/**
 * The purpose of this class is providing external reference counting to dynamic objects
 * @tparam T class to reference count
 */

template<class T>
class external_reference_counting
    : public std::enable_shared_from_this<T>
{
    //! Keeps the object alive is external references are held
    mutable std::shared_ptr<T> external_lock_;
    //! External references tracker
    mutable std::atomic_long counter_ = {0l};

public:

#ifndef __cpp_lib_enable_shared_from_this

    std::weak_ptr<T> weak_from_this() noexcept {
        return std::weak_ptr<T>{this->shared_from_this()};
    }

#endif

protected:

    constexpr external_reference_counting() noexcept = default;

    external_reference_counting(const external_reference_counting& e) noexcept
        : std::enable_shared_from_this<T>{e}
        , counter_{0}
    {}

    external_reference_counting& operator=(const external_reference_counting&) noexcept
    {
        return *this;
    }

    long use_count() const
    {
        return counter_;
    }

    long add_ref() const
    {
        long former = counter_;

        if (0l == former)
        {
            // no former external references
            while (!counter_.compare_exchange_strong(former, 1l))
            {
                if (former != 0l)
                {
                    // another thread was faster
                    return add_ref();
                }
            }

            // sync with other threads release() operations
            std::shared_ptr<T> empty;
            std::shared_ptr<T> inner = const_cast<external_reference_counting*>(this)->shared_from_this();

            while (!std::atomic_compare_exchange_strong(
                        &external_lock_,
                        &empty,
                        inner))
            {
                empty.reset();
            }

            return 1l;
        }
        else
        {
            // former external references
            long new_count;
            do
            {
                if (0l == former)
                {
                    // another thread was faster
                    return add_ref();
                }

                new_count = former + 1;
            }
            while (!counter_.compare_exchange_strong(former, new_count));

            return new_count;
        }
    }

    long release() const
    {
        long former = counter_;

        // add_ref() & release() calls must be matched
        assert(former != 0l);

        if (1l == former)
        {
            // try remove internal lock
            while (!counter_.compare_exchange_strong(former, 0l))
            {
                if (former != 1l)
                {
                    // another thread was faster
                    return release();
                }
            }

            // sync with other threads add_ref() operations
            std::shared_ptr<T> inner = const_cast<external_reference_counting*>(this)->shared_from_this(),
                    cmp = inner;

            while (!std::atomic_compare_exchange_strong(
                        &external_lock_,
                        &cmp,
                        std::shared_ptr<T>{}))
            {
                if (!cmp)
                {
                    cmp = inner;
                }
            }

            return 0l;
        }
        else
        {
            // former external references
            long new_count;
            do
            {
                if (1l == former)
                {
                    // another thread was faster
                    return release();
                }

                new_count = former - 1;
            }
            while (!counter_.compare_exchange_strong(former, new_count));

            return new_count;
        }
    }

};

} // detail namespace
} // eprosima namespace

#endif /* FASTRTPS_UTILS_CUSTOM_ALLOCATORS_HPP_ */
