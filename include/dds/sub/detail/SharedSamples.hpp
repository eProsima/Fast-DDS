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
 *
 */
#ifndef OSPL_DDS_SUB_DETAIL_SHARED_SAMPLES_HPP_
#define OSPL_DDS_SUB_DETAIL_SHARED_SAMPLES_HPP_

/**
 * @cond
 * Ignore this file in the API
 */

#include <dds/sub/LoanedSamples.hpp>

// Implementation

namespace dds
{
namespace sub
{
namespace detail
{

template <typename T>
class SharedSamples
{
public:
    typedef typename std::vector< dds::sub::Sample<T, Sample> >::iterator iterator;
    typedef typename std::vector< dds::sub::Sample<T, Sample> >::const_iterator const_iterator;

public:
    SharedSamples() { }

    SharedSamples(dds::sub::LoanedSamples<T> ls) : samples_(ls) { }

    ~SharedSamples()
    {

    }

public:

    iterator mbegin()
    {
        return samples_->begin();
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
        /** @internal @todo Possible RTF size issue ? */
        return static_cast<uint32_t>(samples_.length());
    }

    void resize(uint32_t s)
    {
        samples_.resize(s);
    }

private:
    dds::sub::LoanedSamples<T> samples_;
};

}
}
}

/** @endcond */

// End of implementation

#endif /* OSPL_DDS_SUB_DETAIL_SHARED_SAMPLES_HPP_ */
