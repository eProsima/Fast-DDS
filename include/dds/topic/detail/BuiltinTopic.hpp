#ifndef OMG_DDS_TOPIC_DETAIL_BUILTIN_TOPIC_HPP_
#define OMG_DDS_TOPIC_DETAIL_BUILTIN_TOPIC_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
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
 */

#include <dds/topic/Topic.hpp>
#include <dds/topic/detail/TTopicImpl.hpp>
#include <dds/topic/detail/TBuiltinTopicImpl.hpp>
#include <org/opensplice/topic/BuiltinTopicDelegate.hpp>
#include <org/opensplice/topic/BuiltinTopic.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds { namespace topic { namespace detail {

    typedef dds::topic::TParticipantBuiltinTopicData<org::opensplice::topic::ParticipantBuiltinTopicDataDelegate>
    ParticipantBuiltinTopicData;

    typedef dds::topic::TTopicBuiltinTopicData<org::opensplice::topic::TopicBuiltinTopicDataDelegate>
    TopicBuiltinTopicData;

    typedef dds::topic::TPublicationBuiltinTopicData<org::opensplice::topic::PublicationBuiltinTopicDataDelegate>
    PublicationBuiltinTopicData;

    typedef dds::topic::TSubscriptionBuiltinTopicData<org::opensplice::topic::SubscriptionBuiltinTopicDataDelegate>
    SubscriptionBuiltinTopicData;

} } }

/** @endcond */

#endif /* OMG_DDS_TOPIC_DETAIL_BUILTIN_TOPIC_HPP_ */
