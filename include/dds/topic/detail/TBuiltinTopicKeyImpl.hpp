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
#ifndef EPROSIMA_DDS_TOPIC_DETAIL_TBUILTINTOPICKEY_IMPL_HPP_
#define EPROSIMA_DDS_TOPIC_DETAIL_TBUILTINTOPICKEY_IMPL_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/topic/BuiltinTopicKey.hpp>

// Implementation

namespace dds {
namespace topic {

template<typename D>
const int32_t* TBuiltinTopicKey<D>::value() const
{
    return this->delegate().value();
}

template<typename D>
void TBuiltinTopicKey<D>::value(const int32_t v[])
{
    this->delegate().value(v);
}

}
}

// End of implementation

#endif /* EPROSIMA_DDS_TOPIC_DETAIL_TBUILTINTOPICKEY_IMPL_HPP_ */
