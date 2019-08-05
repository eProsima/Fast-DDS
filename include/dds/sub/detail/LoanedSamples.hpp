/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#ifndef OMG_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_
#define OMG_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds
{
namespace sub
{
namespace detail
{

template <typename T>
class LoanedSamples
{
public:

    typedef std::vector< dds::sub::Sample<T, dds::sub::detail::Sample> > LoanedSamplesContainer;
    typedef typename std::vector< dds::sub::Sample<T, dds::sub::detail::Sample> >::iterator iterator;
    typedef typename std::vector< dds::sub::Sample<T, dds::sub::detail::Sample> >::const_iterator const_iterator;

public:
    LoanedSamples() { }

    ~LoanedSamples()
    {

    }

public:

    iterator mbegin()
    {
        return samples_.begin();
    }

    const_iterator begin() const
    {
        return samples_.begin();
    }

    const_iterator end() const
    {
        return samples_.end();
    }

    uint32_t length() const
    {
        return static_cast<uint32_t>(samples_.size());
    }

    void reserve(uint32_t s)
    {
        samples_.reserve(s);
    }

    void resize(uint32_t s)
    {
         samples_.resize(s);
    }

    dds::sub::Sample<T, dds::sub::detail::Sample>& operator[] (uint32_t i)
    {
        return this->samples_[i];
    }

    dds::sub::Sample<T, dds::sub::detail::Sample> * get_buffer() {
        return this->samples_.data();
    }


private:
    LoanedSamplesContainer samples_;
};

}
}
}

/** @endcond */

#endif /* OMG_SUB_DETAIL_LOANED_SAMPLES_IMPL_HPP_ */
