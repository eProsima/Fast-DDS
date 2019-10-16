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

#ifndef EPROSIMA_DDS_TOPIC_DETAIL_BUILTIN_TOPIC_HPP_
#define EPROSIMA_DDS_TOPIC_DETAIL_BUILTIN_TOPIC_HPP_

#include <dds/topic/Topic.hpp>
//#include <dds/topic/detail/TTopicImpl.hpp>
//#include <dds/topic/detail/TBuiltinTopicImpl.hpp>
#include <fastdds/dds/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/topic/TopicBuiltinTopicData.hpp>
#include <fastdds/dds/topic/PublicationBuiltinTopicData.hpp>
#include <fastdds/dds/topic/SubscriptionBuiltinTopicData.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace topic {
namespace detail {

using ParticipantBuiltinTopicData = eprosima::fastdds::dds::ParticipantBuiltinTopicData;

using TopicBuiltinTopicData = eprosima::fastdds::dds::TopicBuiltinTopicData;

using PublicationBuiltinTopicData = eprosima::fastdds::dds::PublicationBuiltinTopicData;

using SubscriptionBuiltinTopicData = eprosima::fastdds::dds::SubscriptionBuiltinTopicData;

} //namespace detail
} //namespace topic
} //namespace dds

/** @endcond */

#endif //EPROSIMA_DDS_TOPIC_DETAIL_BUILTIN_TOPIC_HPP_
