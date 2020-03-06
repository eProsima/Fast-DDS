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

#ifndef EPROSIMA_DDS_SUB_DETAIL_MANIPULATOR_HPP_
#define EPROSIMA_DDS_SUB_DETAIL_MANIPULATOR_HPP_

/**
 * @file
 */

#include <dds/sub/Query.hpp>
#include <dds/sub/status/DataState.hpp>

namespace dds {
namespace sub {
namespace functors {
namespace detail {


/**
 * @brief
 * Support functor for dds::sub::max_samples read manipulator.
 */
class MaxSamplesManipulatorFunctor
{
public:

    /**
     * Create a manipulator object to only read a maximum of samples.
     *
     * @param n maximum number of samples to read
     */
    MaxSamplesManipulatorFunctor(
            uint32_t n)
        : n_(n)
    {
    }

    /** @cond */
    template<typename S>
    void operator ()(
            S& s)
    {
        //To implement
        //        s.max_samples(n_);
    }

    /** @endcond */

private:

    uint32_t n_;
};


/**
 * @brief
 * Support functor for dds::sub::content read manipulator.
 */
class ContentFilterManipulatorFunctor
{
public:

    /**
     * Create a manipulator object to filter read of samples according to their content.
     *
     * @param q sample content query, only samples that match are read
     */
    ContentFilterManipulatorFunctor(
            const dds::sub::Query& q)
        : query_(q)
    {
    }

    /** @cond */
    template<typename S>
    void operator ()(
            S& s)
    {
        //To implement
        //        s.content(query_);
    }

    /** @endcond */

private:

    const dds::sub::Query query_;
};


/**
 * @brief
 * Support functor for dds::sub::state read manipulator.
 */
class StateFilterManipulatorFunctor
{
public:

    /**
     * Create a manipulator object to filter read of samples according to their state.
     *
     * @param s sample state filter, only samples that match are read
     */
    StateFilterManipulatorFunctor(
            const dds::sub::status::DataState& s)
        : state_(s)
    {
    }

    /** @cond */
    template<typename S>
    void operator ()(
            S& s)
    {
        //To implement
        //        s.state(state_);
    }

    /** @endcond */

private:

    dds::sub::status::DataState state_;
};


/**
 * @brief
 * Support functor for dds::sub::instance read manipulator.
 */
class InstanceManipulatorFunctor
{
public:

    /**
     * Create a manipulator object to filter read of samples according to their instance.
     *
     * @param h sample instance filter, only samples of the given instance are read
     */
    InstanceManipulatorFunctor(
            const dds::core::InstanceHandle& h)
        : handle_(h)
    {
    }

    /** @cond */
    template<typename S>
    void operator ()(
            S& s)
    {
        //To implement
        //        s.instance(handle_);
    }

    /** @endcond */

private:

    dds::core::InstanceHandle handle_;
};


/**
 * @brief
 * Support functor for dds::sub::next_instance read manipulator.
 */
class NextInstanceManipulatorFunctor
{
public:

    /**
     * Create a manipulator object to filter read of samples according to their instance.
     *
     * @param h sample instance filter, samples of the 'next' instance will be read
     */
    NextInstanceManipulatorFunctor(
            const dds::core::InstanceHandle& h)
        : handle_(h)
    {
    }

    /** @cond */
    template<typename S>
    void operator ()(
            S& s)
    {
        //To implement
        //        s.next_instance(handle_);
    }

    /** @endcond */

private:

    dds::core::InstanceHandle handle_;
};


} //namespace detail
} //namespace functors
} //namespace sub
} //namespace dds

#endif //EPROSIMA_DDS_SUB_DETAIL_MANIPULATOR_HPP_
