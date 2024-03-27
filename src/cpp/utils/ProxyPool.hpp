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

/*!
 * @file ProxyPool.hpp
 */

#ifndef _FASTDDS_UTILS_PROXY_POOL_HPP_
#define _FASTDDS_UTILS_PROXY_POOL_HPP_

#include <array>
#include <bitset>
#include <cassert>
#include <condition_variable>
#include <memory>
#include <mutex>

#if defined(__has_include) && __has_include(<version>)
#   include <version>
#endif // if defined(__has_include) && __has_include(<version>)

namespace eprosima {

// unnamed namespace for isolation
namespace {

// Detect if integer_sequence is availalbe
#if defined(__cpp_lib_integer_sequence) \
    && ((__cpp_lib_integer_sequence <= _MSVC_LANG) \
    || (__cpp_lib_integer_sequence <= __cplusplus))

// Array initialization usin C++14
template<class P, size_t... Ints>
std::array<P, sizeof...(Ints)> make_array(
        P&& i,
        std::index_sequence<Ints...> is)
{
    return { (Ints == is.size() - 1 ? std::move(i) : i)...};
}

template<size_t N, class P>
std::array<P, N> make_array(
        P&& i)
{
    return make_array<P>(std::move(i), std::make_index_sequence<N>{});
}

#else // C++11 fallback

template<size_t N, class P, class ... Ts>
std::array<P, N> make_array(
        P&& i,
        Ts&&... args);

template<bool, size_t N, class ... Ts>
struct make_array_choice
{
    template<class P>
    static std::array<P, N> res(
            P&& i,
            Ts&&... args)
    {
        P tmp(i);
        return make_array<N>(std::move(i), std::move(tmp), std::move(args)...);
    }

};

template<size_t N, class ... Ts>
struct make_array_choice<true, N, Ts...>
{
    template<class P>
    static std::array<P, N> res(
            P&& i,
            Ts&&... args)
    {
        return {std::move(i), std::move(args)...};
    }

};

template<size_t N, class P, class ... Ts>
std::array<P, N> make_array(
        P&& i,
        Ts&&... args)
{
    return make_array_choice < N == (sizeof...(Ts) + 1), N, Ts ... > ::res(std::move(i), std::move(args)...);
}

#endif // defined(__cpp_lib_integer_sequence)

} // namespace

template< class Proxy, std::size_t N = 4>
class ProxyPool
{
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::array<Proxy, N> heap_;
    std::bitset<N> mask_;

    // unique_ptr<Proxy> deleters
    class D
    {
        // Because ProxyPool will be destroy after all the proxies are returned
        // this reference is always valid
        ProxyPool& pool_;

        friend class ProxyPool;

        D(
                ProxyPool* pool)
            : pool_(*pool)
        {
        }

    public:

        void operator ()(
                Proxy* p) const
        {
            pool_.set_back(p);
        }

    }
    deleter_;

    friend class D;

    /*
     * Return an available proxy to the pool.
     * @param p pointer to the proxy.
     */
    void set_back(
            Proxy* p) noexcept
    {
        std::size_t idx = p - heap_.data();

        std::lock_guard<std::mutex> _(mtx_);

        // check is not there
        assert(!mask_.test(idx));

        // return the resource
        mask_.set(idx);

        // notify the resource is free
        cv_.notify_one();
    }

public:

    using smart_ptr = std::unique_ptr<Proxy, D&>;

    /*
     * Constructor of the pool object.
     * @param init Initialization value for all the proxies.
     */
    ProxyPool(
            Proxy&& init)
        : heap_(make_array<N>(std::move(init)))
        , deleter_(this)
    {
        // make all resources available
        mask_.set();
    }

    /*
     * Destructor for the pool object.
     * It waits till all the proxies are back in the pool to prevent data races.
     */
    ~ProxyPool()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [&]()
                {
                    return mask_.all();
                });
    }

    /*
     * Returns the number of proxies in the pool.
     * @return pool size
     */
    static constexpr std::size_t size()
    {
        return N;
    }

    /*
     * Returns the number of proxies available in the pool.
     * @return available proxies
     */
    std::size_t available() const noexcept
    {
        std::lock_guard<std::mutex> _(mtx_);
        return mask_.count();
    }

    /*
     * Retrieve an available proxy from the pool.
     * If not available a wait ensues.
     * Note deleter is referenced not copied to avoid heap allocations on smart pointer construction
     * @return unique_ptr referencing the proxy. On destruction the resource is returned.
     */
    std::unique_ptr<Proxy, D&> get()
    {
        std::unique_lock<std::mutex> lock(mtx_);

        // wait for available resources
        cv_.wait(lock, [&]()
                {
                    return mask_.any();
                });

        // find the first available
        std::size_t idx = 0;
        while (idx < mask_.size() && !mask_.test(idx))
        {
            ++idx;
        }

        // retrieve it
        mask_.reset(idx);
        return std::unique_ptr<Proxy, D&>(&heap_[idx], deleter_);
    }

};

} // eprosima namespace

#endif /* _FASTDDS_UTILS_PROXY_POOL_HPP_ */
