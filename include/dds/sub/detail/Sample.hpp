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

#ifndef EPROSIMA_DDS_PUB_DETAIL_SAMPLE_HPP_
#define EPROSIMA_DDS_PUB_DETAIL_SAMPLE_HPP_

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace sub {
namespace detail {

#include <dds/sub/SampleInfo.hpp>

template<typename T>
class Sample
{
public:
    Sample()
    {
    }

    Sample(
            const T& d,
            const dds::sub::SampleInfo& i)
    {
        //To implement
    }

    Sample(
            const Sample& other)
    {
        //To implement
    }

    Sample& operator=(
            const Sample& other)
    {
        //To implement
    }

    Sample& copy(
            const Sample& other)
    {
        //To implement
    }

    const T& data() const
    {
        //To implement
    }

    T& data()
    {
        //To implement
    }

    void data(
            const T& d)
    {
        //To implement
    }

    const dds::sub::SampleInfo& info() const
    {
        //To implement
    }

    dds::sub::SampleInfo& info()
    {
        //To implement
    }

    void info(
            const dds::sub::SampleInfo& i)
    {
        //To implement
    }

    bool operator ==(
            const Sample& other) const
    {
        //To implement
    }

    T* data_ptr()
    {
        //To implement
    }

    dds::sub::SampleInfo *info_ptr()
    {
        //To implement
    }


private:
    T data_;
    dds::sub::SampleInfo info_;
};

} //namespace detail
} //namespace sub
} //namespace dds

/** @endcond */

#endif //EPROSIMA_DDS_PUB_DETAIL_SAMPLE_HPP_
