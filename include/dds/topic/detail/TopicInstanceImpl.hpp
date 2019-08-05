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
#ifndef OSPL_DDS_TOPIC_TOPICINSTANCE_HPP_
#define OSPL_DDS_TOPIC_TOPICINSTANCE_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/topic/TopicInstance.hpp>

// Implementation

namespace dds
{
namespace topic
{

template <typename T>
TopicInstance<T>::TopicInstance() : h_(dds::core::null) {}

template <typename T>
TopicInstance<T>::TopicInstance(const ::dds::core::InstanceHandle& h)
    : h_(h), sample_() {}

template <typename T>
TopicInstance<T>::TopicInstance(const ::dds::core::InstanceHandle& h, const T& sample)
    : h_(h), sample_(sample) { }

template <typename T>
TopicInstance<T>::operator const ::dds::core::InstanceHandle() const
{
    return h_;
}

template <typename T>
const ::dds::core::InstanceHandle TopicInstance<T>::handle() const
{
    return h_;
}

template <typename T>
void TopicInstance<T>::handle(const ::dds::core::InstanceHandle& h)
{
    h_ = h;
}

template <typename T>
const T& TopicInstance<T>::sample() const
{
    return sample_;
}

template <typename T>
T& TopicInstance<T>::sample()
{
    return sample_;
}

template <typename T>
void TopicInstance<T>::sample(const T& sample)
{
    sample_ = sample;
}

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TOPICINSTANCE_HPP_ */
