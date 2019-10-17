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

#ifndef EPROSIMA_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_
#define EPROSIMA_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_

#include <dds/core/detail/inttypes.hpp>

#include <vector>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace sub {
template<
        typename T,
        template<typename Q> class DELEGATE>
class Sample;

namespace detail {
template<typename T>
class Sample;

template<typename T>
class LoanedSamples
{
public:

    typedef std::vector< ::dds::sub::Sample<T, Sample>> LoanedSamplesContainer;
    typedef typename std::vector< ::dds::sub::Sample<T, Sample>>::iterator iterator;
    typedef typename std::vector< ::dds::sub::Sample<T, Sample>>::const_iterator const_iterator;

public:
    LoanedSamples() { }

    ~LoanedSamples()
    {

    }

public:

    iterator mbegin()
    {
        //To implement
    }

    const_iterator begin() const
    {
        //To implement
    }

    const_iterator end() const
    {
        //To implement
    }

    uint32_t length() const
    {
        //To implement
    }

    void reserve(
            uint32_t s)
    {
        //To implement
    }

    void resize(
            uint32_t s)
    {
         //To implement
    }

    ::dds::sub::Sample<T, Sample>& operator[] (
            uint32_t i)
    {
        //To implement
    }

    ::dds::sub::Sample<T, Sample> * get_buffer()
    {
        //To implement
    }


private:
    LoanedSamplesContainer samples_;
};

} //namespace detail
} //namespace sub
} //namespace dds

/** @endcond */

#endif //EPROSIMA_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_
