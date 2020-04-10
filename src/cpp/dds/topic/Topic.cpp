/*
 * Copyright 2020, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/topic/Topic.hpp>
#include <dds/topic/detail/Topic.hpp>
#include <dds/topic/TopicListener.hpp>

namespace dds {
namespace topic {

//Topic::Topic(
//        const dds::domain::DomainParticipant& dp)
//    : ::dds::core::Reference<detail::Topic>(
//        new detail::Topic(
//            dp.delegate().get(),
//            dp.default_Topic_qos(),
//            nullptr,
//            dds::core::status::StatusMask::all()))
//    , participant_(nullptr)
//{
//}

Topic::Topic(
        const dds::domain::DomainParticipant& dp,
        const std::string& topic_name,
        const std::string& type_name,
        const qos::TopicQos& qos,
        TopicListener* listener,
        const dds::core::status::StatusMask& mask)
    : ::dds::core::Reference<detail::Topic>(
        new detail::Topic(
            dp.delegate().get(), topic_name, type_name, qos, listener, mask))
    , participant_(nullptr)
{
}

Topic::~Topic()
{
}


} //namespace topic
} //namespace dds

