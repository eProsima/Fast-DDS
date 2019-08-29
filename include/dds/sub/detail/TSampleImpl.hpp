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
#ifndef OSPL_DDS_SUB_TSAMPLE_HPP_
#define OSPL_DDS_SUB_TSAMPLE_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/sub/Sample.hpp>

// Implementation
namespace dds
{
namespace sub
{

template <typename T, template <typename Q> class DELEGATE>
Sample<T, DELEGATE>::Sample() : dds::core::Value< DELEGATE<T> >() {}

template <typename T, template <typename Q> class DELEGATE>
Sample<T, DELEGATE>::Sample(const T& data, const SampleInfo& info) : dds::core::Value< DELEGATE<T> >(data, info) { }

template <typename T, template <typename Q> class DELEGATE>
Sample<T, DELEGATE>::Sample(const Sample& other) : dds::core::Value< DELEGATE<T> >(other.delegate()) { }

template <typename T, template <typename Q> class DELEGATE>
const typename Sample<T, DELEGATE>::DataType& Sample<T, DELEGATE>::data() const
{
    return this->delegate().data();
}

template <typename T, template <typename Q> class DELEGATE>
void Sample<T, DELEGATE>::data(const DataType& d)
{
    this->delegate().data(d);
}

template <typename T, template <typename Q> class DELEGATE>
const SampleInfo& Sample<T, DELEGATE>::info() const
{
    return this->delegate().info();
}

template <typename T, template <typename Q> class DELEGATE>
void Sample<T, DELEGATE>::info(const SampleInfo& i)
{
    this->delegate().info(i);
}

}
}
// End of implementation
#endif /* OSPL_DDS_SUB_TSAMPLE_HPP_ */
