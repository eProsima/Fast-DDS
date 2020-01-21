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

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/topic/Topic.hpp>

// Implementation

namespace dds {
namespace topic {


/***************************************************************************
*
* dds/topic/Topic<> WRAPPER implementation.
* Declaration can be found in dds/topic/TTopic.hpp
*
***************************************************************************/


template<typename T>
Topic<T>::Topic(
        const ::dds::domain::DomainParticipant& /*dp*/,
        const std::string& /*topic_name*/)
//    : ::dds::core::Reference<detail::Topic>(new detail::Topic(
//                dp.delegate().get(),
//                topic_name,
//                ""))
{
}

template<typename T>
Topic<T>::Topic(
        const ::dds::domain::DomainParticipant& /*dp*/,
        const std::string& /*topic_name*/,
        const std::string& /*type_name*/)
//    : ::dds::core::Reference<detail::Topic>(new detail::Topic(
//                dp.delegate().get(),
//                topic_name,
//                type_name))
//    , dds::topic::TAnyTopic<detail::Topic>(::dds::core::Reference<detail::Topic>::delegate())
{
}

template<typename T>
Topic<T>::Topic(
        const ::dds::domain::DomainParticipant& /*dp*/,
        const std::string& /*topic_name*/,
        const dds::topic::qos::TopicQos& /*qos*/,
        dds::topic::TopicListener<T>* /*listener*/,
        const ::dds::core::status::StatusMask& /*mask*/)
//    : ::dds::core::Reference<detail::Topic>(new detail::Topic(
//                dp.delegate().get(),
//                topic_name,
//                typeid(T).name(),
//                qos,
//                nullptr,//listener,
//                mask))
//    , dds::topic::TAnyTopic<detail::Topic>(::dds::core::Reference<detail::Topic>::delegate())
{
}

template<typename T>
Topic<T>::Topic(
        const ::dds::domain::DomainParticipant& dp,
        const std::string& topic_name,
        const std::string& type_name,
        const dds::topic::qos::TopicQos& qos,
        const eprosima::fastrtps::TopicAttributes& att,
        dds::topic::TopicListener<T>* listener,
        const ::dds::core::status::StatusMask& mask)
    : ::dds::core::Reference<detail::Topic>(
        new detail::Topic(
            dp.delegate().get(),
            topic_name,
            type_name,
            qos,
            att,
            listener,
            mask))
    , dds::topic::TAnyTopic<detail::Topic>(::dds::core::Reference<detail::Topic>::delegate())
{
}

template<typename T>
Topic<T>::~Topic()
{
}

template<typename T>
void Topic<T>::listener(
        Listener* listener,
        const ::dds::core::status::StatusMask& event_mask)
{
    delegate()->set_listener(listener, event_mask);
}

template<typename T>
typename Topic<T>::Listener* Topic<T>::listener() const
{
    return delegate()->get_listener();
}


} //namespace topic
} //namespace dds
