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

#ifndef EPROSIMA_DDS_SUB_DETAIL_SAMPLES_HOLDER_HPP_
#define EPROSIMA_DDS_SUB_DETAIL_SAMPLES_HOLDER_HPP_

/**
 * @file
 */

#include <dds/sub/LoanedSamples.hpp>
#include <dds/sub/detail/SampleInfo.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace sub {
namespace detail {

//class SampleInfo;

template<typename T>
class LoanedSamplesHolder /*: public SamplesHolder*/
{
public:

    LoanedSamplesHolder(
            ::dds::sub::LoanedSamples<T>& samples)
        : samples_(samples)
        , index(0)
    {
    }

    void set_length(
            uint32_t len)
    {
        this->samples_.delegate()->resize(len);
    }

    uint32_t get_length() const
    {
        return this->index;
    }

    //    SamplesHolder& operator ++(
    //            int)
    //    {
    //        //To implement
    //        this->index++;
    //        return *this;
    //    }

    void* data()
    {
        return (*this->samples_.delegate())[this->index].delegate().data_ptr();
    }

    detail::SampleInfo* info()
    {
        return (*this->samples_.delegate())[this->index].delegate().info_ptr();
    }

private:

    ::dds::sub::LoanedSamples<T>&samples_;
    uint32_t index;
};



template<
    typename T,
    typename SamplesFWIterator>
class SamplesFWInteratorHolder /*: public SamplesHolder*/
{
public:

    SamplesFWInteratorHolder(
            SamplesFWIterator& it)
        : iterator(it)
        , length(0)
    {
    }

    void set_length(
            uint32_t len)
    {
        this->length = len;

    }

    uint32_t get_length() const
    {
        return this->length;
    }

    //    SamplesHolder& operator ++(
    //            int)
    //    {
    //        //To implement
    //        ++this->iterator;
    //        return *this;
    //    }

    void* data()
    {
        return (*iterator).delegate().data_ptr();
    }

    detail::SampleInfo* info()
    {
        return (*iterator).delegate().info_ptr();
    }

private:

    SamplesFWIterator& iterator;
    uint32_t length;

};

template<
    typename T,
    typename SamplesBIIterator>
class SamplesBIIteratorHolder /*: public SamplesHolder*/
{
public:

    SamplesBIIteratorHolder(
            SamplesBIIterator& it)
        : iterator(it)
        , length(0)
    {
    }

    void set_length(
            uint32_t len)
    {
        this->length = len;
    }

    uint32_t get_length() const
    {
        return this->length;
    }

    //    SamplesHolder& operator ++(
    //            int)
    //    {
    //        //To implement
    //        *this->iterator = this->sample;
    //        ++this->iterator;
    //        return *this;
    //    }

    void* data()
    {
        return this->sample.delegate().data_ptr();
    }

    detail::SampleInfo* info()
    {
        return this->sample.delegate().info_ptr();
    }

private:

    SamplesBIIterator& iterator;
    ::dds::sub::Sample<T, Sample> sample;
    uint32_t length;

};

} //namespace detail
} //namespace sub
} //namespace dds


/** @endcond */

#endif //EPROSIMA_DDS_SUB_DETAIL_SAMPLES_HOLDER_HPP_
