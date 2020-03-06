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

#ifndef EPROSIMA_DDS_SUB_DETAIL_SHARED_SAMPLES_HPP_
#define EPROSIMA_DDS_SUB_DETAIL_SHARED_SAMPLES_HPP_

/**
 * @cond
 * Ignore this file in the API
 */

#include <dds/sub/LoanedSamples.hpp>

namespace dds {
namespace sub {

template<
    typename T,
    template<typename Q> class DELEGATE>
class Sample;

namespace detail {

template<typename T>
class SharedSamples
{
public:

    typedef typename std::vector<::dds::sub::Sample<T, Sample> >::iterator iterator;
    typedef typename std::vector<::dds::sub::Sample<T, Sample> >::const_iterator const_iterator;

    SharedSamples()
    {
    }

    SharedSamples(
            ::dds::sub::LoanedSamples<T> ls)
        : samples_(ls)
    {
    }

    ~SharedSamples()
    {

    }

    iterator mbegin()
    {
        //To implement
        //        return samples_->begin();
    }

    const_iterator begin() const
    {
        //To implement
        //        return samples_.begin();
    }

    const_iterator end() const
    {
        //To implement
        //        return samples_.end();
    }

    uint32_t length() const
    {
        //To implement
        //        return static_cast<uint32_t>(samples_.length());
    }

    void resize(
            uint32_t s)
    {
        //To implement
        //        samples_.resize(s);
    }

private:

    ::dds::sub::LoanedSamples<T> samples_;
};

} //namespace detail
} //namespace sub
} //namespace dds

/** @endcond */

#endif //EPROSIMA_DDS_SUB_DETAIL_SHARED_SAMPLES_HPP_
