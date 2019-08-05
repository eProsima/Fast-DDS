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
#ifndef OMG_DDS_TOPIC_DETAIL_ANY_TOPIC_HPP_
#define OMG_DDS_TOPIC_DETAIL_ANY_TOPIC_HPP_

/**
 * @cond
 * Ignore this file in the API
 */

// Implementation

#include <dds/topic/detail/TAnyTopicImpl.hpp>
#include <org/opensplice/topic/AnyTopicDelegate.hpp>

namespace dds { namespace topic { namespace detail {
  typedef dds::topic::TAnyTopic<org::opensplice::topic::AnyTopicDelegate> AnyTopic;
} } }

// End of implementation

/** @endcond */

#endif /* OMG_DDS_TOPIC_DETAIL_ANY_TOPIC_HPP_ */
